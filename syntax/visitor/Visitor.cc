#include <ymir/syntax/visitor/Visitor.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/lexing/Token.hh>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>
#include <ymir/syntax/declaration/_.hh>
#include <ymir/syntax/expression/_.hh>
#include <ymir/global/State.hh>

using namespace Ymir;

namespace syntax {

    Visitor::Visitor () {}

    Visitor Visitor::init (const std::string & path, FILE * file) {
	lexing::Lexer lexer (path.c_str (), file,
		     {Token::SPACE, Token::RETURN, Token::RRETURN, Token::TAB},
		     {
			 {Token::LCOMM1, {Token::RCOMM1, ""}},
			     {Token::LCOMM2, {Token::RETURN, ""}},
				 {Token::LCOMM3, {Token::RCOMM3, ""}},
				     {Token::LCOMM4, {Token::RCOMM3, Token::STAR}},
					 {Token::LCOMM5, {Token::RCOMM5, Token::PLUS}} 
		     }
	);
	
	return Visitor::init (lexer);
    }
    
    Visitor Visitor::init (const lexing::Lexer & lexer) {
	Visitor visit {};
	visit._lex = lexer;
	visit._forbiddenKeys = {
	    Keys::IMPORT, Keys::STRUCT, Keys::ASSERT, Keys::THROW_K, Keys::SCOPE,
	    Keys::DEF, Keys::IF, Keys::RETURN, Keys::PRAGMA, 
	    Keys::FOR,  Keys::WHILE, Keys::BREAK,
	    Keys::MATCH, Keys::IN, Keys::ELSE, Keys::DELEGATE, 
	    Keys::TRUE_, Keys::FALSE_, Keys::NULL_, Keys::CAST,
	    Keys::FUNCTION, Keys::DELEGATE, Keys::LET, Keys::IS, Keys::EXTERN,
	    Keys::PUBLIC, Keys::PRIVATE, Keys::TYPEOF, Keys::IMMUTABLE,
	    Keys::MACRO, Keys::TRAIT, Keys::REF, Keys::CONST,
	    Keys::MOD, Keys::USE, Keys::STRINGOF, Keys::CLASS, Keys::ALIAS,
	    Keys::STATIC
	};
	
	visit._operators = {
	    {
		Token::EQUAL, Token::DIV_AFF, Token::MINUS_AFF,
		Token::PLUS_AFF, Token::STAR_AFF,
		Token::PERCENT_AFF, Token::TILDE_AFF,
		Token::LEFTD_AFF, Token::RIGHTD_AFF
	    },
	    {Token::DPIPE},
	    {Token::DAND},
	    {Token::INF, Token::SUP, Token::INF_EQUAL,
	     Token::SUP_EQUAL, Token::NOT_EQUAL, Token::DEQUAL, Keys::OF, Keys::IS, Keys::IN},
	    {Token::TDOT, Token::DDOT},
	    {Token::LEFTD, Token::RIGHTD},
	    {Token::PIPE, Token::XOR, Token::AND},
	    {Token::PLUS, Token::TILDE, Token::MINUS},
	    {Token::STAR, Token::PERCENT, Token::DIV},
	    {Token::DXOR},
	};

	visit._specialOperators = {
	    {},
	    {}, 
	    {},
	    {Keys::OF, Keys::IS, Keys::IN},
	    {},
	    {},
	    {},
	    {},
	    {}, 
	    {},
	};

	visit._declarations = {
	    Keys::ALIAS, Keys::CLASS, Keys::ENUM,
	    Keys::DEF, Keys::STATIC, Keys::IMPORT,
	    Keys::MACRO, Keys::MOD, Keys::STRUCT,
	    Keys::TRAIT, Keys::USE, Keys::EXTERN
	};
	
	visit._declarationsBlock = {
	    Keys::CLASS, Keys::ENUM,  Keys::DEF,
	    Keys::IMPORT, Keys::STRUCT, Keys::TRAIT,
	    Keys::USE
	};

	visit._intrisics = {
	    Keys::COPY, Keys::EXPAND, Keys::TYPEOF, Keys::SIZEOF, Keys::ALIAS, Keys::MOVE, Keys::DCOPY
	};
	
	visit._operand_op = {
	    Token::MINUS, Token::AND, Token::STAR, Token::NOT
	};
	
	visit._fixedSuffixes = {
	    Keys::I8, Keys::U8, Keys::I16, Keys::U16, Keys::U32, Keys::I64, Keys::U64, Keys::USIZE, Keys::ISIZE
	};

	visit._floatSuffix = {
	    Keys::FLOAT_S
	};

	visit._charSuffix = {
	    Keys::C8, Keys::C32,
	    Keys::UNDER + Keys::C8,
	    Keys::UNDER + Keys::C32, 
	};

	visit._stringSuffix = {
	    Keys::S8, Keys::S32,
	    Keys::UNDER + Keys::S8,
	    Keys::UNDER + Keys::S32, 
	};
	
	return visit;
    }

    Declaration Visitor::visitModGlobal () {
	std::vector <Declaration> decls;
	lexing::Word space;
	lexing::Word token;
	auto beginPos = this-> _lex.tell ();
	TRY (
	    auto next = this-> _lex.next ({Keys::MOD});
	    space = visitNamespace ();
	    this-> _lex.consumeIf ({Token::SEMI_COLON});
	) CATCH (ErrorCode::EXTERNAL) {
	    CLEAR_ERRORS ();
	    space = lexing::Word::eof ();
	    this-> _lex.seek (beginPos);
	} FINALLY;
	
	do {	    
	    token = this-> _lex.consumeIf ({Keys::PUBLIC, Keys::PRIVATE, Keys::VERSION});
	    if (token == Keys::PUBLIC || token == Keys::PRIVATE) {
		decls.push_back (visitProtectionBlock (token == Keys::PRIVATE));
	    } else if (token == Keys::VERSION) {
		decls.push_back (visitVersionGlob (true));
	    } else if (!token.isEof ()) {
		decls.push_back (visitDeclaration ());
	    }
	} while (!token.isEof ());

	if (space.isEof ())
	    space.setLocus (this-> _lex.getFilename (), 0, 0);
	
	auto ret = Module::init (space, decls);
	ret.to<Module> ().isGlobal (true);
	return ret;
    }
    
    Declaration Visitor::visitProtectionBlock (bool isPrivate) {
	auto location = this-> _lex.rewind ().next ();
	std::vector <Declaration> decls;
	auto token = this-> _lex.consumeIf ({Token::LACC});
	bool end = (token != Token::LACC); //if the block is not surrounded with {}, we get only one declaration
		
	do {
	    token = this-> _lex.consumeIf ({Token::RACC});
	    if (token == Token::RACC && !end) {
		end = true;
	    } else decls.push_back (visitDeclaration ());	    
	} while (!end);

	return DeclBlock::init (location, decls, isPrivate, false);	
    }

    Declaration Visitor::visitVersionGlob (bool global) {
	if (!global) return visitVersionGlobBlock (global);
	
	auto location = this-> _lex.rewind ().next ();
	auto token = this-> _lex.consumeIf ({Token::EQUAL});
	if (token == Token::EQUAL) {
	    auto type = visitIdentifier ();
	    this-> _lex.consumeIf ({Token::SEMI_COLON});
	    global::State::instance ().activateVersion (type.str);
	    return Declaration::empty ();
	} else {
	    return visitVersionGlobBlock (global);
	}	
    }

    Declaration Visitor::visitVersionGlobBlock (bool global) {
	auto location = this-> _lex.rewind ().next ();
	auto ident = visitIdentifier ();
	std::vector <Declaration> decls;
	
	if (global::State::instance ().isVersionActive (ident.str)) {
	    auto token = this-> _lex.next ({Token::LACC});
	
	    do {
		token = this-> _lex.consumeIf ({Keys::PUBLIC, Keys::PRIVATE, Keys::VERSION, Token::RACC});
		if (token == Keys::PUBLIC || token == Keys::PRIVATE) {
		    decls.push_back (visitProtectionBlock (token == Keys::PRIVATE));
		} else if (token == Keys::VERSION) {
		    decls.push_back (visitVersionGlob (global));
		} else if (token != Token::RACC) {
		    decls.push_back (visitDeclaration ());
		}
	    } while (token != Token::RACC);
	    if (this-> _lex.consumeIf ({Keys::ELSE}) == Keys::ELSE) ignoreBlock ();
	} else {
	    ignoreBlock ();
	    if (this-> _lex.consumeIf ({Keys::ELSE}) == Keys::ELSE) {
		auto token = this-> _lex.next ({Token::LACC});	
		do {
		    token = this-> _lex.consumeIf ({Keys::PUBLIC, Keys::PRIVATE, Keys::VERSION, Token::RACC});
		    if (token == Keys::PUBLIC || token == Keys::PRIVATE) {
			decls.push_back (visitProtectionBlock (token == Keys::PRIVATE));
		    } else if (token == Keys::VERSION) {
			decls.push_back (visitVersionGlob (global));
		    } else if (token != Token::RACC) {
			decls.push_back (visitDeclaration ());
		    }
		} while (token != Token::RACC);
	    }
	}
	
	return DeclBlock::init (location, decls, true, false);
    }


    Declaration Visitor::visitExtern () {
	auto location = this-> _lex.rewind ().next ();
	auto token = this-> _lex.consumeIf ({Token::LPAR});

	lexing::Word from, space;
	if (token == Token::LPAR) {
	    from = this-> _lex.next ({Keys::CLANG, Keys::CPPLANG, Keys::DLANG, Keys::YLANG});
	    token = this-> _lex.next ({Token::COMA, Token::RPAR});
	    if (token == Token::COMA) {
		space = visitNamespace ();
		this-> _lex.next ({Token::RPAR});
	    }
	}

	return ExternBlock::init (location, from, space, visitProtectionBlock (false));
    }

    Declaration Visitor::visitDeclaration () {
	auto location = this-> _lex.next (this-> _declarations);	

	if (location == Keys::ALIAS) return visitAlias ();
	if (location == Keys::CLASS) return visitClass ();
	if (location == Keys::ENUM) return visitEnum ();
	if (location == Keys::DEF) return visitFunction ();
	if (location == Keys::STATIC) return visitGlobal ();
	if (location == Keys::IMPORT) return visitImport ();
	if (location == Keys::EXTERN) return visitExtern ();
	// if (location == Keys::MACRO) return visitMacro ();
	if (location == Keys::MOD) return visitLocalMod ();
	if (location == Keys::STRUCT) return visitStruct ();
	if (location == Keys::TRAIT) return visitTrait ();
	if (location == Keys::USE) return visitUse ();
	else {
	    Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Declaration::empty ();
	}
    }

    Declaration Visitor::visitAlias () {
	auto name = visitIdentifier ();
	std::vector <Expression> templates = visitTemplateParameters ();	
	this-> _lex.next ({Token::EQUAL});
	auto value = visitExpression ();
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	
	if (!templates.empty ()) {
	    return Template::init (name, templates, Alias::init (name, value));
	} else 
	    return Alias::init (name, value);
    }
    
    Declaration Visitor::visitClass () {
	auto location = this-> _lex.rewind ().next ();
	auto attribs = visitAttributes ();
	
	auto name = this-> _lex.next ();	
	auto templates = visitTemplateParameters ();
	Expression ancestor (Expression::empty ());
	
	auto token = this-> _lex.consumeIf ({Keys::OVER});	
	if (token == Keys::OVER) 
	    ancestor = visitExpression ();
	
	auto decls = visitClassBlock ();

	if (!templates.empty ()) {
	    return Template::init (name, templates, Class::init (name, ancestor, decls, attribs));
	} else
	    return Class::init (name, ancestor, decls, attribs);
    }

    std::vector <Declaration> Visitor::visitClassBlock (bool fromTrait) {
	std::vector <Declaration> decls;
	
	auto token = this-> _lex.next ({Token::LACC});
	do {
	    token = this-> _lex.consumeIf ({Keys::PRIVATE, Keys::PUBLIC, Keys::PROTECTED, Keys::VERSION, Token::RACC, Token::SEMI_COLON, Keys::IMMUTABLE});
	    if (token == Keys::PRIVATE || token == Keys::PUBLIC || token == Keys::PROTECTED) {
		decls.push_back (visitProtectionClassBlock (token == Keys::PRIVATE, token == Keys::PROTECTED));
	    } else if (token == Keys::VERSION) {
		decls.push_back (visitVersionClass (fromTrait));
	    } else if (token == Keys::IMMUTABLE) {
		decls.push_back (visitIfClass (fromTrait));
	    } else if (token != Token::RACC && token != Token::SEMI_COLON) {
		decls.push_back (visitClassContent (fromTrait));
	    } 
	} while (token != Token::RACC);
	return decls;
    }
    
    Declaration Visitor::visitProtectionClassBlock (bool isPrivate, bool isProtected, bool fromTrait) {
	auto location = this-> _lex.rewind ().next ();
	std::vector <Declaration> decls;
	auto token = this-> _lex.consumeIf ({Token::LACC});
	bool end = (token != Token::LACC);

	do {
	    token = this-> _lex.consumeIf ({Token::RACC, Token::SEMI_COLON});
	    if (token == Token::RACC && !end) end = true;
	    else if (token != Token::SEMI_COLON) {
		decls.push_back (visitClassContent (fromTrait));		
	    }
	} while (!end);
	return DeclBlock::init (location, decls, isPrivate, isProtected);
    }

    void Visitor::ignoreBlock () {
	auto token = this-> _lex.next ({Token::LACC});
	int close = 1;
	do {
	    token = this-> _lex.next ();
	    if (token == Token::LACC) close += 1;
	    if (token == Token::RACC) {
		close -= 1;
	    } else if (token.isEof ())
		Error::occur (token, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.str);
	} while (close != 0);
    }

    
    
    Declaration Visitor::visitVersionClass (bool fromTrait) {
	auto location = this-> _lex.rewind ().next ();
	auto ident = visitIdentifier ();
	std::vector <Declaration> decls;
	if (global::State::instance ().isVersionActive (ident.str)) {
	    decls = visitClassBlock (fromTrait);
	    if (this-> _lex.consumeIf ({Keys::ELSE}) == Keys::ELSE) {
		ignoreBlock ();
	    }	   
	} else {
	    ignoreBlock ();
	    if (this-> _lex.consumeIf ({Keys::ELSE}) == Keys::ELSE) {
		decls = visitClassBlock (fromTrait);
	    }
	}   
	return DeclBlock::init (location, decls, false, false);
    }

    Declaration Visitor::visitIfClass (bool fromTrait) {
	auto location = this-> _lex.next ({Keys::IF});
	auto test = this-> visitExpression ();
	auto decls = visitClassBlock (fromTrait);
	auto next = this-> _lex.consumeIf ({Keys::ELSE});
	if (next == Keys::ELSE) {
	    auto next2 = this-> _lex.consumeIf ({Keys::IF});
	    if (next2 == Keys::IF) {
		this-> _lex.rewind ();
		return CondBlock::init (location, test, decls, visitIfClass (fromTrait));
	    } else {
		auto elseDecls = visitClassBlock (fromTrait);
		return CondBlock::init (location, test, decls, DeclBlock::init (next, elseDecls, true, false));
	    }
	}
	return CondBlock::init (location, test, decls, DeclBlock::init (location, {}, true, false));
    }
    
    Declaration Visitor::visitClassContent (bool fromTrait) {
	lexing::Word token;
	if (fromTrait)
	    token = this-> _lex.next ({Keys::DEF, Keys::OVER}); // Trait can only have method definitions
	else
	    token = this-> _lex.next ({Keys::DEF, Keys::OVER, Keys::LET, Keys::SELF, Keys::IMPL});
	
	if (token == Keys::SELF) {
	    return visitClassConstructor ();
	} else if (token == Keys::DEF) {
	    return visitFunction (true);	    
	} else if (token == Keys::OVER) {
	    auto func = visitFunction (true);
	    func.to <Function> ().setOver ();
	    return func;
	} else if (token == Keys::LET) {
	    this-> _lex.rewind ();
	    return Expression::toDeclaration (visitVarDeclaration ());
	} else if (token == Keys::IMPL) {
	    return visitClassMixin ();
	} else {
	    Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Declaration::empty ();	
	}
    }
    
    Declaration Visitor::visitClassMixin () {
	auto location = this-> _lex.rewind ().next ({Keys::IMPL});
	auto content = visitExpression (10); // (priority of dot operator)
	if (this-> _lex.consumeIf ({Token::SEMI_COLON}) != Token::SEMI_COLON) {	    
	    std::vector <Declaration> decls = visitClassBlock (false);	    
	    return Mixin::init (location, content, decls);
	} else
	    return Mixin::init (location, content, {});
    }

    Declaration Visitor::visitClassConstructor () {
	auto location = this-> _lex.rewind ().next ();
	auto before_template = this-> _lex.tell ();
	auto templates = visitTemplateParameters ();
	auto token = this-> _lex.next ();
	if (token != Token::LPAR) { // If there is no remaining parameters, it means that the read templates may be the runtime parameters
	    if (canBeParameters (templates))
		this-> _lex.seek (before_template); // And we seek to this location to get them as runtime parameters
	    templates.clear (); // So, we remove them	    
	} else this-> _lex.rewind ();

	auto proto = visitFunctionPrototype ();
	token = this-> _lex.consumeIf ({Keys::WITH});
	std::vector <std::pair <lexing::Word, Expression> > constructions;
	std::vector <Expression> supers;
	lexing::Word getSuper = lexing::Word::eof ();
	lexing::Word getSelf = lexing::Word::eof ();
	if (token == Keys::WITH) {
	    while (token != Token::LACC) {
		auto ident = this-> visitIdentifier ();
		if (ident == Keys::SUPER) {
		    getSuper = ident;
		    if (supers.size () != 0)
			Error::occur (ident, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), ident.str);
		    this-> _lex.next ({Token::LPAR});
		    supers = visitParamList (true);
		    this-> _lex.next ({Token::RPAR});
		} else if (ident == Keys::SELF) {
		    getSelf = ident;
		    if (supers.size () != 0)
			Error::occur (ident, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), ident.str);
		    this-> _lex.next ({Token::LPAR});
		    supers = visitParamList (true);
		    this-> _lex.next ({Token::RPAR});
		} else {
		    this-> _lex.next ({Token::EQUAL});
		    auto expr = this-> visitExpression ();
		    constructions.push_back ({ident, expr});
		}

		token = this-> _lex.next ({Token::LACC, Token::COMA});
	    }
	    this-> _lex.rewind (); // We rewind the LACC, since it is part of the following expression
	}
	
	auto body = visitExpression ();
	if (templates.size () != 0) {
	    return Template::init (location, templates, Constructor::init (location, proto, supers, constructions, body, getSuper, getSelf));
	} else 
	    return Constructor::init (location, proto, supers, constructions, body, getSuper, getSelf);
    }
    
    Declaration Visitor::visitClassDestructor () {
	auto location = this-> _lex.rewind (2).next ();
	this-> _lex.next ();
	location.str = Keys::SELF_TILDE;
	
	this-> _lex.next ({Token::LPAR});
	this-> _lex.next ({Token::RPAR});
	
	auto body = visitFunctionBody ();
	return Function::init (location, Function::Prototype::init ({}, Expression::empty (), false), body);	
    }

    Declaration Visitor::visitEnum () {
	auto location = this-> _lex.rewind ().next ({Keys::ENUM});
	Expression type (Expression::empty ());
	if (this-> _lex.consumeIf ({Token::COLON}) == Token::COLON)
	    type = visitExpression (10);
	
	lexing::Word end;
	std::vector <Expression> values;
	do {
	    end = this-> _lex.next ({Token::ARROW, Token::PIPE});
	    if (end != Token::ARROW) {
		auto name = visitIdentifier ();
		if (this-> _lex.consumeIf ({Token::EQUAL}) == Token::EQUAL)
		    values.push_back (VarDecl::init (name, {}, Expression::empty (), visitExpression (10)));
		else values.push_back (VarDecl::init (name, {}, Expression::empty (), Expression::empty ()));
	    }
	} while (end != Token::ARROW);

	auto name = visitIdentifier ();
	auto templates = visitTemplateParameters ();
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	if (templates.size () != 0) {
	    return Template::init (name, templates, Enum::init (name, type, values));
	} else return Enum::init (name, type, values);
    }    
    
    Declaration Visitor::visitFunction (bool isClass) {       
	auto location = this-> _lex.rewind ().next ();
	
	Expression test (Expression::empty ());
	
	auto token = this-> _lex.consumeIf ({Keys::IF});
	lexing::Word ifLoc;
	if (token == Keys::IF) {
	    ifLoc = token;
	    test = visitExpression ();
	}

	auto attribs = visitAttributes ();
	auto name = visitIdentifier ();
	if (name == Keys::SELF)
	    Error::occur (name, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), name.str);
	
	auto befTemplates = this-> _lex.tell ();
	auto templates = visitTemplateParameters ();
	
	token = this-> _lex.next ();
	if (token != Token::LPAR) {
	    this-> _lex.seek (befTemplates);
	    templates.clear ();
	}
	else this-> _lex.rewind ();

	auto proto = visitFunctionPrototype (false, isClass);
	auto body = visitFunctionBody ();
	auto function = Function::init (name, proto, body);
	function.to <Function> ().setCustomAttributes (attribs);

	if (templates.size () != 0) {
	    return Template::init (name, templates, function, test);
	} else {
	    if (!test.isEmpty ())
		Error::occur (ifLoc, ExternalError::get (SYNTAX_ERROR_IF_ON_NON_TEMPLATE)); 
	    return function;
	}
    }

    Function::Prototype Visitor::visitFunctionPrototype (bool isClosure, bool isClass) {
	std::vector <Expression> vars;
	lexing::Word token;
	if (isClosure)
	    token = this-> _lex.next ({Token::PIPE, Token::DPIPE});
	else token = this-> _lex.next ({Token::LPAR});
	
	bool isVariadic = false;
	if (token != Token::DPIPE) { // || begin of a closure with not params
	    do {
		if (isClosure) {
		    token = this-> _lex.consumeIf ({Token::PIPE});
		    if (token != Token::PIPE) {
			vars.push_back (visitSingleVarDeclaration (false, false));
			token = this-> _lex.next ({Token::PIPE, Token::COMA});
		    }
		} else {
		    if (!isClass || vars.size () > 0) // If we get a method prototype, we need at least one param
			token = this-> _lex.consumeIf ({Token::RPAR, Token::TDOT});
		    if (token == Token::TDOT) {
			isVariadic = true;
			token = this-> _lex.next ({Token::RPAR});
		    } else if (token != Token::RPAR) {
			vars.push_back (visitSingleVarDeclaration (true, true, isClass && vars.size () == 0));
			token = this-> _lex.next ({Token::RPAR, Token::COMA, Token::TDOT});
		    }
		}
	    } while ((isClosure && token != Token::PIPE) || (!isClosure && token != Token::RPAR));
	}
	
	token = this-> _lex.consumeIf ({Token::ARROW});
	if (token == Token::ARROW) 
	    return Function::Prototype::init (vars, visitExpression (), isVariadic);
	else 
	    return Function::Prototype::init (vars, Expression::empty (), isVariadic);
    }

    Function::Body Visitor::visitFunctionBody () {
	Expression in (Expression::empty ()), body (Expression::empty ()), out (Expression::empty ());
	lexing::Word name;
	
	auto token = this-> _lex.next ();	
	if (token == Keys::PRE) {
	    in = visitExpression ();
	    token = this-> _lex.next ();
	}
	
	if (token == Keys::POST) {
	    name = visitIdentifier ();
	    out = visitExpression ();
	} else this-> _lex.rewind ();

	if (in.isEmpty () && out.isEmpty ()) {
	    token = this-> _lex.consumeIf ({Token::SEMI_COLON});
	    if (token == Token::SEMI_COLON)
		return Function::Body::init (in, Expression::empty (), out, name);	
	}
	return Function::Body::init (in, visitExpression (), out, name);	
    }

    std::vector <lexing::Word> Visitor::visitAttributes () {
	auto token = this-> _lex.next ();
	if (token != Token::AT) {
	    this-> _lex.rewind ();
	    return {};
	}
	
	token = this-> _lex.next ();
	if (token == Token::LACC) {
	    std::vector <lexing::Word> vars;
	    do {
		vars.push_back (visitIdentifier ());
		token = this-> _lex.next ({Token::COMA, Token::RACC});
	    } while (token != Token::RACC);
	    return vars;
	} else {
	    this-> _lex.rewind ();
	    return {visitIdentifier ()};
	}	
    }

    Declaration Visitor::visitGlobal () {
	auto location = this-> _lex.rewind ().next ();
	auto decl = visitSingleVarDeclaration ();
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	return Global::init (location, decl);
    }

    Declaration Visitor::visitImport () {
	auto location = this-> _lex.rewind ().next ();
	lexing::Word token;
	std::vector <Declaration> imports;
	do {
	    auto space = visitNamespace ();
	    lexing::Word as;
	    
	    token = this-> _lex.consumeIf ({Token::COMA, Keys::AS});
	    if (token == Keys::AS) {
		as = visitIdentifier ();
		token = this-> _lex.consumeIf ({Token::COMA});
	    }
	    
	    imports.push_back (Import::init (space, as));
	} while (token == Token::COMA);

	this-> _lex.consumeIf ({Token::SEMI_COLON});
	return DeclBlock::init (location, imports, true, false);
    }

    Declaration Visitor::visitLocalMod () {
	auto name = visitIdentifier ();
	auto templates = visitTemplateParameters ();
	auto token = this-> _lex.next ({Token::LACC});
	std::vector <Declaration> decls;
	
	do {
	    token = this-> _lex.consumeIf ({Keys::PUBLIC, Keys::PRIVATE, Keys::VERSION, Token::RACC});
	    if (token == Keys::PUBLIC || token == Keys::PRIVATE) {
		decls.push_back (visitProtectionBlock (token == Keys::PRIVATE));
	    } else if (token == Keys::VERSION) {
		decls.push_back (visitVersionGlob (false));
	    } else if (token != Token::RACC) {
		decls.push_back (visitDeclaration ());
	    }
	} while (token != Token::RACC);

	if (templates.size () != 0) {
	    return Template::init (name, templates, Module::init (name, decls));
	} else return Module::init (name, decls);
    }

    Declaration Visitor::visitStruct () {
	auto location = this-> _lex.rewind ().next ({Keys::STRUCT});
	lexing::Word end;
	std::vector <Expression> vars;
	std::vector <lexing::Word> attrs = visitAttributes ();
	do {
	    end = this-> _lex.next ({Token::ARROW, Token::PIPE});
	    if (end != Token::ARROW) {
		vars.push_back (visitSingleVarDeclaration (true, false));
		if (this-> _lex.consumeIf ({Token::EQUAL}) == Token::EQUAL)
		    vars.back ().to <VarDecl> ().setValue (visitExpression (10));		
	    }
	} while (end != Token::ARROW);
	auto name = visitIdentifier ();
	auto templates = visitTemplateParameters ();
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	if (templates.size () != 0) {
	    return Template::init (name, templates, Struct::init (name, attrs, vars));
	} else return Struct::init (name, attrs, vars);
    }

    Declaration Visitor::visitTrait () {
	auto location = this-> _lex.rewind ().next ({Keys::TRAIT});
	auto name = visitIdentifier ();
	auto templates = visitTemplateParameters ();

	std::vector <Declaration> decls = visitClassBlock (false);
	
	if (!templates.empty ()) {
	    return Template::init (name, templates, Trait::init (name, decls));
	} else return Trait::init (name, decls);	
    }

    Declaration Visitor::visitUse () {
	auto location = this-> _lex.rewind ().next ({Keys::USE});
	auto content = visitExpression (10);
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	
	return Use::init (location, content);
    }

    bool Visitor::canBeParameters (const std::vector <Expression> & params) {
	for (auto & it : params) {
	    // It can be a var in constructor of method (for self)
	    if (!it.is <VarDecl> () && !it.is <Var> ()) return false;
	}
	return true;
    }
    
    std::vector <Expression> Visitor::visitTemplateParameters () {
	std::vector <Expression> list;
	auto begin = this-> _lex.tell ();
	auto token = this-> _lex.next ();
	if (token == Token::LPAR) {
	    TRY (
		do {
		    if (canVisitIdentifier ()) {
			auto name = visitIdentifier ();
			token = this-> _lex.next ();
			if (token == Token::COLON) {
			    token = this-> _lex.next ();
			    if (token == Token::TDOT) list.push_back (VariadicVar::init (name, true));
			    else {
				this-> _lex.rewind ();
				auto type = visitExpression (10);
				token = this-> _lex.consumeIf ({Token::EQUAL});
				if (token == Token::EQUAL) {
				    list.push_back (VarDecl::init (name, {}, type, visitExpression ()));
				} else
				    list.push_back (VarDecl::init (name, {}, type, Expression::empty ()));
			    }
			} else if (token == Token::EQUAL) {
			    list.push_back (VarDecl::init (name, {}, Expression::empty (), visitExpression ()));
			} else if (token == Keys::OF)
			    list.push_back (OfVar::init (name, visitExpression ()));
			else if (token == Keys::IMPL)
			    list.push_back (ImplVar::init (name, visitExpression ()));
			else if (token == Token::TDOT) {
			    list.push_back (VariadicVar::init (name, false));
			} else {
			    this-> _lex.rewind (2);
			    list.push_back (visitExpression (10));
			}
		    } else {
			auto x = this-> _lex.next ();
			if (x == Keys::STRUCT) {
			    auto name = visitIdentifier ();
			    list.push_back (StructVar::init (name));
			} else if (x == Keys::CLASS) {
			    auto name = visitIdentifier ();
			    list.push_back (ClassVar::init (name));
			} else if (x == Keys::ALIAS) {
			    auto name = visitIdentifier ();
			    list.push_back (AliasVar::init (name));
			} else {
			    this-> _lex.rewind ();
			    list.push_back (visitExpression (10));
			}
		    }
		    
		    token = this-> _lex.next ({Token::RPAR, Token::COMA});
		} while (token != Token::RPAR);
	    ) CATCH (ErrorCode::EXTERNAL) {
		CLEAR_ERRORS ();
		this-> _lex.seek (begin);
		return {};	
	    } FINALLY;
	    
	    return list;
	} else this-> _lex.rewind ();
	return {};
    }

    Expression Visitor::visitExpression (uint priority) {
	if (priority == this-> _operators.size ()) return visitOperand0 ();
	else {
	    auto left = visitExpression (priority + 1);
	    return visitExpression (left, priority);
	}
    }
    
    Expression Visitor::visitExpression (const Expression & left, uint priority) {
	auto token = this-> _lex.next ();
	if (token.is (this-> _operators [priority])) {
	    Expression ctype (Expression::empty ());
	    {
		auto next = this-> _lex.next ();
		if (next == Token::COLON) ctype = visitExpression (0);
		else this-> _lex.rewind ();
	    }
	    
	    auto right = visitExpression (priority + 1);
	    return visitExpression (Binary::init (token, left, right, ctype), priority);	    
	} else if (token == Token::NOT) {
	    auto next = this-> _lex.next ();
	    if (next.is (this-> _specialOperators [priority])) {
		Expression ctype (Expression::empty ());
		{
		    auto next = this-> _lex.next ();
		    if (next == Token::COLON) ctype = visitExpression (0);
		    else this-> _lex.rewind ();
		}

		auto right = visitExpression (priority + 1);
		return visitExpression (Binary::init (token + next, left, right, ctype), priority);
	    } else this-> _lex.rewind ();
	} this-> _lex.rewind ();
	return left;
    }    

    Expression Visitor::visitOperand0 () {
	auto location = this-> _lex.consumeIf (this-> _operand_op);
	if (location.is (this-> _operand_op)) {
	    return Unary::init (location, visitOperand1 ());
	}
	
	return visitOperand1 ();    
    }

    Expression Visitor::visitOperand1 () {
	auto next = this-> _lex.next ();
	this-> _lex.rewind ();
	if (next == Token::LACC)  return visitBlock ();
	if (next == Keys::IF)       return visitIf ();
	if (next == Keys::WHILE)    return visitWhile ();
	if (next == Keys::ASSERT)   return visitAssert ();
	if (next == Keys::BREAK)    return visitBreak ();
	if (next == Keys::DO)       return visitDoWhile ();
	if (next == Keys::FOR)      return visitFor ();
	if (next == Keys::MATCH)    return visitMatch ();
	if (next == Keys::SCOPE)    return visitScope ();
	if (next == Keys::LET)      return visitVarDeclaration ();
	if (next == Keys::RETURN)   return visitReturn ();
	if (next == Keys::FUNCTION) return visitFunctionType ();
	if (next == Keys::DELEGATE) return visitFunctionType ();
	if (next == Keys::LOOP)     return visitWhile ();
	if (next == Keys::CATCH)    return visitCatch ();
	if (next == Keys::THROW_K)  return visitThrow ();
	if (next == Keys::VERSION)  return visitVersion ();
	if (next == Keys::PRAGMA)   return visitPragma ();
	
	auto value = visitOperand2 ();
	return visitOperand1 (value);
    }

    Expression Visitor::visitOperand1 (const Expression & value) {
	auto location = this-> _lex.next ();
	if (location == Token::LPAR || location == Token::LCRO) {
	    auto params = visitParamList (location == Token::LPAR);
	    lexing::Word end;
	    if (location == Token::LPAR) end = this-> _lex.next ({Token::RPAR});
	    else end = this-> _lex.next ({Token::RCRO});
	    return visitOperand1 (MultOperator::init (location, end, value, params));
	} else if (location == Token::DOT) {
	    auto right = visitOperand3 (false);
	    return visitOperand1 (Binary::init (location, value, visitTemplateCall (right), Expression::empty ()));
	} this-> _lex.rewind ();
	return value;	
    }
    
    Expression Visitor::visitOperand2 () {
	auto value = visitOperand3 ();
	return visitOperand2 (value);
    }

    Expression Visitor::visitOperand2 (const Expression & value) {
	auto next = this-> _lex.next ();
	if (next == Token::DCOLON) {
	    auto right =  visitOperand3 (false);	    
	    return visitOperand2 (visitTemplateCall (Binary::init (next, value, right, Expression::empty ())));
	} this-> _lex.rewind ();
	return value;
    }
    
    Expression Visitor::visitOperand3 (bool canBeTemplateCall) {
	auto begin = this-> _lex.next ();
	this-> _lex.rewind ();

	if (begin == Keys::CAST)     return visitCast ();
	if (begin == Keys::TEMPLATE) return visitTemplateChecker ();
	if (begin == Token::LCRO)    return visitArray ();
	if (begin == Token::LPAR)    return visitTuple ();
	if (begin == Token::PIPE || begin == Token::DPIPE)
	    return visitLambda ();

	if (begin.is (this-> _intrisics)) {
	    auto loc = this-> _lex.next ();
	    auto tok = this-> _lex.consumeIf ({Token::LPAR});
	    auto inner = tok == Token::LPAR ? visitExpression () : visitExpression (10);	    
	    if (inner.is<Lambda> () && begin == Keys::MOVE) {
		if (tok == Token::LPAR) this-> _lex.next ({Token::RPAR});
		return Lambda::moveClosure (inner);
	    }
	    
	    if (tok == Token::LPAR) this-> _lex.next ({Token::RPAR});
	    return Intrinsics::init (loc, inner);
	}
	
	if (begin.is (DecoratorWord::members ())) {
	    return visitDecoratedExpression ();
	}
	
	if (can (&Visitor::visitVar))  return visitVar (canBeTemplateCall);       
	return visitLiteral ();
    }    

    Expression Visitor::visitArray () {
	auto begin = this-> _lex.next ({Token::LCRO});
	auto end = this-> _lex.consumeIf ({Token::RCRO});
	if (end == Token::RCRO) return List::init (begin, end, {});
	else {
	    std::vector <Expression> params;
	    params.push_back (visitExpression ());
	    end = this-> _lex.next ({Token::SEMI_COLON, Token::RCRO, Token::COMA});
	    if (end == Token::SEMI_COLON) {
		bool isDynamic = false;
		{ // Check if 'new' 
		    end = this-> _lex.next ();
		    if (end == Keys::NEW) isDynamic = true;
		    else this-> _lex.rewind ();
		}
		
		auto size = visitExpression ();		
		end = this-> _lex.next ({Token::RCRO});
		return ArrayAlloc::init (begin, params [0], size, isDynamic);
	    } else {
		while (end != Token::RCRO) {
		    params.push_back (visitExpression ());
		    end = this-> _lex.next ({Token::COMA, Token::RCRO});		   
		}

		return List::init (begin, end, params);
	    }
	}
    }

    Expression Visitor::visitBlock () {	
	std::vector <Declaration> decls;	
	std::vector <Expression> content;
	auto begin = this-> _lex.next ({Token::LACC});

	lexing::Word end;
	bool last = false;
	do {
	    end = this-> _lex.consumeIf ({Token::RACC, Token::SEMI_COLON});
	    if (end != Token::RACC && end != Token::SEMI_COLON) {
		last = false;
		if (this-> _lex.consumeIf (this-> _declarationsBlock).str != "") {
		    this-> _lex.rewind ();
		    decls.push_back (visitDeclaration ());
		} else 
		    content.push_back (visitExpression ());
	    } else if (end == Token::SEMI_COLON)
		last = true;
	} while (end != Token::RACC);
	
	if (last) content.push_back (Unit::init (end));
	if (decls.size () != 0) {
	    return Block::init (begin, end, Module::init ({begin, "_"}, decls), content);
	} else {
	    return Block::init (begin, end, Declaration::empty (), content);
	}
    }    

    Expression Visitor::visitIf () {
	auto location = this-> _lex.next ({Keys::IF});
	auto test = visitExpression ();
	auto content = visitExpression ();	
	auto next = this-> _lex.consumeIf ({Keys::ELSE});
	if (next == Keys::ELSE) {
	    next = this-> _lex.next ();
	    this-> _lex.rewind ();
	    if (next == Keys::IF) return If::init (location, test, content, visitIf ());
	    else {
		auto el_exp = visitExpression ();		
		return If::init (location, test, content, el_exp);
	    }
	}
	
	return If::init (location, test, content, Expression::empty ());
    }

    Expression Visitor::visitWhile () {
	auto location = this-> _lex.next ({Keys::WHILE, Keys::LOOP});
	Expression test (Expression::empty ());
	if (location != Keys::LOOP) test = visitExpression ();
	auto content = visitExpression ();
	return While::init (location, test, content);
    }

    Expression Visitor::visitDoWhile () {
	auto location = this-> _lex.next ({Keys::DO});
	auto content = visitExpression ();
	this-> _lex.next ({Keys::WHILE});
	auto test = visitExpression ();
	return While::init (location, test, content, true);
    }
    
    Expression Visitor::visitFor () {
	std::vector <Expression> decls;
	auto location = this-> _lex.next ({Keys::FOR});
	lexing::Word token;
	do {
	    decls.push_back (visitSingleVarDeclaration (false, false));	    
	    token = this-> _lex.next ({Token::COMA, Keys::IN});
	} while (token == Token::COMA);

	Expression iter = visitExpression ();
	return For::init (location, decls, iter, visitExpression ());
    }

    Expression Visitor::visitMatch () {
	auto begin = this-> _lex.next ();
	auto content = this-> visitExpression ();
	auto next = this-> _lex.next ({Token::LACC});
	std::vector <Expression> matchs;
	std::vector <Expression> actions;
	do {
	    auto expr = visitMatchExpression ();
	    this-> _lex.next ({Token::DARROW});
	    auto action = visitExpression (10);
	    this-> _lex.consumeIf ({Token::SEMI_COLON});
	    matchs.push_back (expr);
	    actions.push_back (action);
	    next = this-> _lex.consumeIf ({Token::RACC});	    
	} while (next != Token::RACC);
       
	return Match::init (begin, content, matchs, actions);
    }    

    Expression Visitor::visitMatchExpression () {
	auto next = this-> _lex.next ();
	this-> _lex.rewind ();
	if (can(&Visitor::visitSingleVarDeclarationForMatch)) {
	    return visitSingleVarDeclarationForMatch ();
	} else if (canVisitIdentifier () || next == Keys::UNDER) {
	    auto name = this-> _lex.next ();
	    next = this-> _lex.next ();	 
	    Expression var (Expression::empty ());
	    if (name != Keys::UNDER) {
		var = Var::init (name);
		while (true) {
		    if (next == Token::NOT) {
			this-> _lex.rewind ();
			var = visitTemplateCall (var);
			next = this-> _lex.next ();
			break;
		    } else if (next == Token::DCOLON) {
			var = Binary::init (next, var, Var::init (visitIdentifier ()), Expression::empty ());
			next = this-> _lex.next ();
		    } else break;
		}
	    }
	    
	    if (next == Token::LPAR) {
		std::vector <Expression> params;
		auto end = this-> _lex.consumeIf ({Token::RPAR, Token::COMA});
		while (end != Token::RPAR) {
		    params.push_back (visitMatchExpression ());
		    end = this-> _lex.next ({Token::COMA, Token::RPAR});
		}
		if (var.isEmpty ())
		    return MultOperator::init (name, end, Var::init (name), params);
		else
		    return MultOperator::init (name, end, var, params);
	    } else if (next == Token::ARROW && var.isEmpty ()) {
		return NamedExpression::init (name, visitMatchExpression ());
	    } else if (next == Token::ARROW && var.is <Var> ()) {
		return NamedExpression::init (var.getLocation (), visitMatchExpression ());
	    }
	    
	    this-> _lex.rewind ();
	    if (var.isEmpty ())		
		return Var::init (name);
	    return var;
	} else if (next == Token::LPAR) {
	    this-> _lex.next (); // Consume LPAR
	    std::vector <Expression> params;
	    auto end = this-> _lex.consumeIf ({Token::RPAR, Token::COMA});
	    while (end != Token::RPAR) {
		params.push_back (visitMatchExpression ());
		end = this-> _lex.next ({Token::COMA, Token::RPAR});
	    }
	    return List::init (next, end, params);
	} else if (next == Token::LCRO) {
	    this-> _lex.next ();
	    std::vector <Expression> params;
	    auto end = this-> _lex.consumeIf ({Token::RCRO});
	    while (end != Token::RCRO) {
		params.push_back (visitMatchExpression ());
		end = this-> _lex.next ({Token::COMA, Token::RCRO});
	    }
	    return List::init (next, end, params);	    
	} else {
	    return visitExpression ();
	}
    }
    
    Expression Visitor::visitBreak () {
	auto location = this-> _lex.next ({Keys::BREAK});
	return Break::init (location, visitExpression());
    }

    Expression Visitor::visitAssert () {	
	auto location = this-> _lex.next ();
	auto token = this-> _lex.consumeIf ({Token::LPAR});	
	auto test = visitExpression ();
	if (token == Token::LPAR) {
	    token = this-> _lex.next ({Token::RPAR, Token::COMA});
	    Expression msg (Expression::empty ());
	    if (token == Token::COMA) {
		msg = visitExpression();
		this-> _lex.next ({Token::RPAR});
	    }
	    return Assert::init (location, test, msg);
	}
	
	return Assert::init (location, test, Expression::empty ());
    }

    Expression Visitor::visitThrow () {
	auto location = this-> _lex.next ();
	return Throw::init (location, visitExpression ());
    }

    Expression Visitor::visitPragma () {
	auto location = this-> _lex.next ();
	this-> _lex.next ({Token::NOT});
	auto name = visitIdentifier ();

	this-> _lex.next ({Token::LPAR});
	auto params = visitParamList ();
	this-> _lex.next ({Token::RPAR});
	return Pragma::init (name, params);	
    }

    Expression Visitor::visitScope () {
	auto location = this-> _lex.next ();
	auto name = visitIdentifier ();
	this-> _lex.consumeIf ({Token::DARROW}); // Arrow is not mandatory
	return Scope::init (name, visitExpression ());
    }

    Expression Visitor::visitCatch () {
	auto begin = this-> _lex.next ();
	auto next = this-> _lex.consumeIf ({Token::LACC});
	std::vector <Expression> vars;
	std::vector <Expression> actions;
	if (next == Token::LACC) {
	    do {
		lexing::Word name = this-> _lex.consumeIf ({Keys::UNDER});
		if (name != Keys::UNDER) {
		    name = visitIdentifier ();
		}
		Expression type (Expression::empty ());
		if (this-> _lex.consumeIf ({Token::COLON}) == Token::COLON)
		    type = visitExpression (10);
		vars.push_back (VarDecl::init (name, {}, type, Expression::empty ()));				    
		this-> _lex.next ({Token::DARROW});
		actions.push_back (visitExpression ());
		next = this->_lex.consumeIf ({Token::RACC});
	    } while (next != Token::RACC);	    
	} else {
	    lexing::Word name = this-> _lex.consumeIf ({Keys::UNDER});
	    if (name != Keys::UNDER) {
		name = visitIdentifier ();
	    }
	    Expression type (Expression::empty ());
	    if (this-> _lex.consumeIf ({Token::COLON}) == Token::COLON)
		type = visitExpression (10);
	    vars.push_back (VarDecl::init (name, {}, type, Expression::empty ()));				    
	    this-> _lex.next ({Token::DARROW});
	    actions.push_back (visitExpression ());
	}
	return Catch::init (begin, vars, actions);
    }
    
    Expression Visitor::visitVersion () {
	auto location = this-> _lex.next ({Keys::VERSION});
	auto ident = visitIdentifier ();
	if (global::State::instance ().isVersionActive (ident.str)) {
	    auto value = visitBlock ();
	    if (this-> _lex.consumeIf ({Keys::ELSE}) == Keys::ELSE) {
		ignoreBlock ();
	    }
	    return value;
	} else {
	    ignoreBlock ();
	    if (this-> _lex.consumeIf ({Keys::ELSE}) == Keys::ELSE) {
		return visitBlock ();
	    }
	}
	
	return Expression::empty ();
    }

    Expression Visitor::visitReturn () {
	auto location = this-> _lex.next ();
	return Return::init (location, visitExpression ());
    }

    Expression Visitor::visitCast () {
	auto location = this-> _lex.next ();
	this-> _lex.next ({Token::NOT});
	auto type = visitOperand3 ();
	this-> _lex.next ({Token::LPAR});
	auto inner = visitExpression ();
	this-> _lex.next ({Token::RPAR});
	return Cast::init (location, type, inner);
    }

    Expression Visitor::visitTemplateChecker () {
	auto name = this-> _lex.next ();

	this-> _lex.next ({Token::NOT});
	this-> _lex.rewind ();
	auto call = this-> visitTemplateCall (Var::init (name));
	
	this-> _lex.next ({Token::LPAR});
	this-> _lex.rewind ();	
	auto params = visitTemplateParameters ();
	
	return TemplateChecker::init (name, call.to <TemplateCall> ().getParameters (), params);
    }
    
    Expression Visitor::visitDecoratedExpression () {
	std::vector<DecoratorWord> decos;

	lexing::Word token = this-> _lex.next (DecoratorWord::members ());
	while (token.is (DecoratorWord::members ())) {
	    auto deco = DecoratorWord::init (token);
	    for (auto d : decos) {
		if (d.getValue () == deco.getValue ()) {
		    auto note = Ymir::Error::createNote (d.getLocation ());
		    Error::occurAndNote (token, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.str);
		}
	    }
	    
	    decos.push_back (deco);
	    token = this-> _lex.consumeIf (DecoratorWord::members ());	    
	}
	
	auto content = visitExpression (10);
	// Lambda if deco is REF
	if (content.is<Lambda> () && decos.size () == 1 && decos [0].getValue () == Decorator::REF)
	    return Lambda::refClosure (content);
	return DecoratedExpression::init (content.getLocation (), decos, content);	
    }

    Expression Visitor::visitVar () {
	return visitVar (true);
    }
	    
    Expression Visitor::visitVar (bool canBeTemplateCall) {	
	auto name = visitIdentifier ();
	if (canBeTemplateCall) {
	    return visitTemplateCall (Var::init (name));
	} else return Var::init (name);	
    }

    Expression Visitor::visitTemplateCall (const Expression & left) {
	static bool inTmpCall = false;
	auto next = this-> _lex.next ();
	auto begin = next;
	if (!inTmpCall && next == Token::NOT) {
	    next = this-> _lex.next ();
	    if (next.is ({Keys::OF, Keys::IS, Keys::IN})) { // !is, !of and !in
		this-> _lex.rewind ().rewind ();
		return left;
	    }
	    
	    if (next == Token::LPAR) {
		inTmpCall = false;
		auto list = visitParamList ();
		this-> _lex.next ({Token::RPAR});
		return TemplateCall::init (begin, list, left);
	    } else {
		inTmpCall = true;
		this-> _lex.rewind ();
		auto ret = TemplateCall::init (begin, {visitOperand3 ()}, left);
		inTmpCall = false;
		return ret;
	    }
	} else if (next == Token::NOT) {
	    Error::occur (next, ExternalError::get (SYNTAX_ERROR_MISSING_TEMPL_PAR));
	}
	
	this-> _lex.rewind ();
	return left;
    }

    std::vector <Expression> Visitor::visitParamList (bool withNamed) {
	std::vector <Expression> params;
	auto begin = this-> _lex.tell ();
	TRY (
	    bool done = false;
	    if (withNamed) {
		if (canVisitIdentifier ()) {
		    auto begin = this-> _lex.tell ();
		    auto name = visitIdentifier ();
		    auto next = this-> _lex.consumeIf ({Token::ARROW});
		    if (next == Token::ARROW) {
			params.push_back (NamedExpression::init (name, visitExpression ()));
			done = true;
		    } else this-> _lex.seek (begin);
		}
	    }
	    if (!done)
		params.push_back (visitExpression ());
	) CATCH (ErrorCode::EXTERNAL) {
	    CLEAR_ERRORS ();
	    this-> _lex.seek (begin);
	    return {};
	} FINALLY;

	auto token = this-> _lex.next ();
	while (token == Token::COMA) {
	    bool done = false;
	    if (withNamed) {
		if (canVisitIdentifier ()) {
		    auto begin = this-> _lex.tell ();
		    auto name = visitIdentifier ();
		    auto next = this-> _lex.consumeIf ({Token::ARROW});
		    if (next == Token::ARROW) {
			params.push_back (NamedExpression::init (name, visitExpression ()));
			done = true;
		    } else this-> _lex.seek (begin);
		}
	    }
	    if (!done)
		params.push_back (visitExpression ());

	    token = this-> _lex.next ();
	}
	
	this-> _lex.rewind ();
	return params;
    }

    Expression Visitor::visitTuple () {
	auto begin = this-> _lex.next ({Token::LPAR});	
	auto end = this-> _lex.consumeIf ({Token::RPAR});
	if (end == Token::RPAR) return List::init (begin, end, {});
	
	std::vector <Expression> params;
	params.push_back (visitExpression ());
	
	auto token = this-> _lex.next ({Token::COMA, Token::RPAR});	
	if (token == Token::COMA) {
	    token = this-> _lex.next ();
	    if (token != Token::RPAR) {
		this-> _lex.rewind ();
		do {
		    params.push_back (visitExpression ());
		    token = this-> _lex.next ();
		} while (token == Token::COMA);
	    }
	    
	    return List::init (begin, token, params);	    
	}

	return params [0];
    }

    Expression Visitor::visitLambda () {
	auto begin = this-> _lex.next ();
	this-> _lex.rewind ();

	auto proto = visitFunctionPrototype (true);	
	this-> _lex.consumeIf ({Token::DARROW});
	return Lambda::init (begin, proto, visitExpression ());
    }       

    Expression Visitor::visitFunctionType () {
	auto begin = this-> _lex.next ();
	auto open = this-> _lex.next ({Token::LPAR, Token::PIPE});
	lexing::Word token;
	std::vector <Expression> vars;
	do {
	    if (open == Token::PIPE) {
		token = this-> _lex.consumeIf ({Token::PIPE});
		if (token != Token::PIPE) {
		    vars.push_back (visitExpression (10));
		    token = this-> _lex.next ({Token::PIPE, Token::COMA});
		}
	    } else {
		token = this-> _lex.consumeIf ({Token::RPAR});
		if (token != Token::RPAR) {
		    vars.push_back (visitExpression (10));
		    token = this-> _lex.next ({Token::RPAR, Token::COMA});
		}
	    }
	} while ((token != Token::PIPE && open == Token::PIPE) || (token != Token::RPAR && open == Token::LPAR));
	token = this-> _lex.next ({Token::ARROW});
	return FuncPtr::init (begin, visitExpression (), vars);
    }
    
    Expression Visitor::visitLiteral () {
	auto tok = this-> _lex.next ();
	this-> _lex.rewind ();
	if ((tok.str [0] >= '0' && tok.str [0] <= '9') || (tok.str [0] == '_' && tok.str.length () != 1))
	                                               return visitNumeric ();
	if (tok == Token::DOT)                         return visitFloat (lexing::Word::eof ());
	if (tok == Token::APOS)                        return visitChar ();
	if (tok == Token::GUILL)                       return visitString ();
	if (tok == Keys::TRUE_ || tok == Keys::FALSE_) return Bool::init (this-> _lex.next ());
	if (tok == Keys::NULL_)                        return Null::init (this-> _lex.next ());
	if (tok == Token::DOLLAR)                      return Dollar::init (this-> _lex.next ());
	
	Error::occur (tok, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), tok.str);
	return Expression::empty ();
    }

    Expression Visitor::visitNumeric () {
	auto begin = this-> _lex.next ();	
	if (begin.str.length () >= 4) {
	    auto suffix = lexing::Word {begin, begin.str.substr (begin.str.length () - 3)};
	    suffix.column += begin.str.length () - 3;
	    
	    if (suffix.is (this-> _fixedSuffixes)) {
		auto value = begin.str.substr (0, begin.str.length () - 3);
		verifNumeric (begin, value);
		return Fixed::init ({begin, value}, suffix);
	    }
	}
	
	if (begin.str.length () >= 3) {
	    auto suffix = lexing::Word {begin, begin.str.substr (begin.str.length () - 2)};
	    suffix.column += begin.str.length () - 2;
	    
	    if (suffix.is (this-> _fixedSuffixes)) {
		auto value = begin.str.substr (0, begin.str.length () - 2);
		verifNumeric (begin, value);
		return Fixed::init ({begin, value}, suffix);
	    }
	}
	
	auto value = begin.str;
	if (!verifNumeric (begin, value)) {
	    auto next = this-> _lex.consumeIf ({Token::DOT});
	    if (next == Token::DOT) {
		this-> _lex.rewind ();
		return visitFloat (begin);
	    } else {
		return Fixed::init (begin, lexing::Word::eof ());
	    }
	} return Fixed::init (begin, lexing::Word::eof ());
    }

    bool Visitor::verifNumeric (const lexing::Word & loc, const std::string & value) {
	if (value.length () > 2 && value [0] == '0' && value [1] == Keys::LX [0]) {
	    for (uint i = 2 ; i < value.length (); i++) {
		if ((value [i] < '0' || value [i] > '9') && (value [i] < 'A' || value [i] > 'F') && (value [i] < 'a' || value [i] > 'f') && value [i] != Keys::UNDER [0]) {
		    Error::occur (loc, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), loc.str);
		}
	    }
	    return true;
	} else if (value.length () > 2 && value [0] == '0' && value [1] == 'o') {
	    for (uint i = 2 ; i < value.length (); i++) {
		if ((value [i] < '0' || value [i] > '7') && value [i] != Keys::UNDER [0]) {
		    Error::occur (loc, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), loc.str);
		}
	    }
	    return true;
	} else {
	    for (uint i = 0 ; i < value.length (); i++) {
		if ((value [i] < '0' || value [i] > '9') && value [i] != Keys::UNDER [0]) {
		    Error::occur (loc, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), loc.str);
		}
	    }
	    return false;
	}	    
    }    

    Expression Visitor::visitFloat (const lexing::Word & begin) {
	// No space inside a floating point value
	this-> _lex.skipEnable (Token::SPACE, false);
	this-> _lex.skipEnable (Token::TAB, false);		
	this-> _lex.skipEnable (Token::RETURN, false);
	this-> _lex.skipEnable (Token::RRETURN, false);
	auto dot = this-> _lex.next ({Token::DOT});
	auto after = this-> _lex.next ();	
	this-> _lex.skipEnable (Token::SPACE, true);
	this-> _lex.skipEnable (Token::TAB, true);		
	this-> _lex.skipEnable (Token::RETURN, true);
	this-> _lex.skipEnable (Token::RRETURN, true);
	
	if (after.str [0] >= '0' && after.str [0] <= '9') {
	    if (after.str.length () >= 2) {
		auto suffix = lexing::Word {after, after.str.substr (after.str.length () - 1)};
		suffix.column += after.str.length () - 1;
	    
		if (suffix.is (this-> _floatSuffix)) {
		    auto value = after.str.substr (0, after.str.length () - 1);
		    if (!verifNumeric (after, value))
			return Float::init (dot, begin, {after, value}, suffix);
		    else
			Error::occur (after, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), after.str);
		}
	    }
	
	    auto value = after.str;
	    for (uint i = 0 ; i < value.length (); i++) {
		if ((value [i] < '0' || value [i] > '9') && value [i] != Keys::UNDER [0]) {
		    if (begin.isEof ())
			Error::occur (after, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), after.str);
		    else {
			this-> _lex.rewind ();
			after = lexing::Word::eof ();
			break;
		    }
		} 
	    }
	    return Float::init (dot, begin, after, lexing::Word::eof ());
	} else if (begin.isEof ()) {
	    Error::occur (after, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), after.str);
	} else if (after.is (this-> _floatSuffix)) {
	    return Float::init (dot, begin, lexing::Word::eof (), after);	    
	} else this-> _lex.rewind ();
	
	return Float::init (dot, begin, lexing::Word::eof (), lexing::Word::eof ());
    }

    Expression Visitor::visitChar () {
	auto begin = this-> _lex.next ();
	// We disable the skip of space and tabulation and comments
	this-> _lex.skipEnable (Token::SPACE, false);
	this-> _lex.skipEnable (Token::TAB, false);		
	this-> _lex.skipEnable (Token::RETURN, false);
	this-> _lex.skipEnable (Token::RRETURN, false);
	
	this-> _lex.commentEnable (false);
	lexing::Word cursor = lexing::Word::eof ();
	lexing::Word all = lexing::Word::eof ();
	do {
	    cursor = this-> _lex.next ();
	    if (cursor == Token::RETURN || cursor == Token::RRETURN || cursor.isEof ()) {
		Error::occur (cursor, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), cursor.str);		
	    } else if (cursor != Token::APOS) {
		all += cursor;
	    }
	} while (cursor != Token::APOS);

	auto format = this-> _lex.consumeIf (this-> _charSuffix);
	if (!format.isEof () && format.str [0] == '_') {
	    format = lexing::Word {format, format.str.substr (1, format.str.length  ()- 1)};
	}
	// We restore the skip and comments
	this-> _lex.skipEnable (Token::SPACE, true);
	this-> _lex.skipEnable (Token::TAB, true);
	this-> _lex.skipEnable (Token::RETURN, true);
	this-> _lex.skipEnable (Token::RRETURN, true);
	this-> _lex.commentEnable (true);
	
	return Char::init (begin, cursor, all, format);
    }

    Expression Visitor::visitString () {
	auto begin = this-> _lex.next ();

	this-> _lex.skipEnable (Token::SPACE,   false);
	// this-> _lex.skipEnable (Token::TAB,     false);
	// this-> _lex.skipEnable (Token::RETURN,  false);
	// this-> _lex.skipEnable (Token::RRETURN, false);
	this-> _lex.commentEnable (false);

	lexing::Word cursor = lexing::Word::eof ();
	lexing::Word all    = lexing::Word::eof ();
	do {
	    cursor = this-> _lex.next ();
	    if (cursor.isEof ()) {
		Error::occur (cursor, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), cursor.str);		
	    } else if (cursor != begin) {
		all += cursor;
	    } 
	} while (cursor != begin);

	auto format = this-> _lex.consumeIf (this-> _stringSuffix);
	if (!format.isEof () && format.str [0] == '_') {
	    format = lexing::Word {format, format.str.substr (1, format.str.length  ()- 1)};
	}
	
	this-> _lex.skipEnable (Token::SPACE,   true);
	// this-> _lex.skipEnable (Token::TAB,     true); // Need to add the escape char to get a tab or a line break
	// this-> _lex.skipEnable (Token::RETURN,  true);
	// this-> _lex.skipEnable (Token::RRETURN, true);
	this-> _lex.commentEnable (true);

	return String::init (begin, cursor, all, format);
    }
    
    Expression Visitor::visitVarDeclaration () {
	auto location = this-> _lex.next ({Keys::LET});
	lexing::Word token;
	std::vector <Expression> decls;
	do {
	    auto tok = this-> _lex.consumeIf ({Token::LPAR});
	    if (tok == Token::LPAR) {
		decls.push_back (visitDestructVarDeclaration ());
		token = this-> _lex.next ();
		break;
	    } else {
		decls.push_back (visitSingleVarDeclaration ());
		token = this-> _lex.next ();
	    }
	} while (token == Token::COMA);

	this-> _lex.rewind (); // The last token read wasn't a coma
	if (decls.size () == 1) return decls [0];
	else return Set::init (location, decls);
    }    

    Expression Visitor::visitSingleVarDeclaration (bool mandType, bool withValue, bool isClass) {
	std::vector<DecoratorWord> decos;
	Expression type (Expression::empty ()), value (Expression::empty ());
	
	lexing::Word token = this-> _lex.consumeIf (DecoratorWord::members ());
	while (token.is (DecoratorWord::members ())) {
	    auto deco = DecoratorWord::init (token);
	    for (auto d : decos) {
		if (d.getValue () == deco.getValue ()) {
		    auto note = Ymir::Error::createNote (d.getLocation ());
		    Error::occurAndNote (token, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.str);
		}
	    }
	    
	    decos.push_back (deco);
	    token = this-> _lex.consumeIf (DecoratorWord::members ());	    
	}

	lexing::Word name = this-> _lex.consumeIf ({Keys::UNDER});
	if (name != Keys::UNDER) {
	    name = visitIdentifier ();
	}

	if (name != Keys::SELF || !isClass) {
	    if (name == Keys::SELF || isClass)
		Error::occur (name, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), name.str);
	    
	    if (mandType) token = this-> _lex.next ({Token::COLON});
	    else token = this-> _lex.next ();
	
	    if (token == Token::COLON) {
		type = visitExpression (10);
		token = this-> _lex.next ();
	    } 

	    if (token == Token::EQUAL && withValue)
		value = visitExpression ();
	    else this-> _lex.rewind ();
	}
	
	return VarDecl::init (name, decos, type, value);
    }

    Expression Visitor::visitSingleVarDeclarationForMatch () {
	std::vector<DecoratorWord> decos;
	Expression type (Expression::empty ()), value (Expression::empty ());
	
	lexing::Word token = this-> _lex.consumeIf (DecoratorWord::members ());
	while (token.is (DecoratorWord::members ())) {
	    auto deco = DecoratorWord::init (token);
	    for (auto d : decos) {
		if (d.getValue () == deco.getValue ()) {
		    auto note = Ymir::Error::createNote (d.getLocation ());
		    Error::occurAndNote (token, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.str);
		}
	    }
	    
	    decos.push_back (deco);
	    token = this-> _lex.consumeIf (DecoratorWord::members ());	    
	}

	lexing::Word name = this-> _lex.consumeIf ({Keys::UNDER});
	if (name != Keys::UNDER) {
	    name = visitIdentifier ();
	}
	
	if (name == Keys::SELF)
	    Error::occur (name, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), name.str);
	
	token = this-> _lex.next ({Token::COLON});	
	token = this-> _lex.consumeIf ({Keys::UNDER});
	if (token != Keys::UNDER)
	    type = visitExpression (10);
	
	token = this-> _lex.next ();	 
	if (token == Token::EQUAL)
	    value = visitMatchExpression ();
	else this-> _lex.rewind ();

	return VarDecl::init (name, decos, type, value);
    }
    
    Expression Visitor::visitDestructVarDeclaration () {
	auto begin = this-> _lex.rewind ().next ();
	lexing::Word next;
	std::vector <Expression> params;
	bool isVariadic = false;
	do {
	    std::vector<DecoratorWord> decos;
	    Expression type (Expression::empty ());

	    lexing::Word token = this-> _lex.consumeIf (DecoratorWord::members ());
	    while (token.is (DecoratorWord::members ())) {
		auto deco = DecoratorWord::init (token);
		for (auto d : decos) {
		    if (d.getValue () == deco.getValue ()) {
			auto note = Ymir::Error::createNote (d.getLocation ());
			Error::occurAndNote (token, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.str);
		    }
		}
	    
		decos.push_back (deco);
		token = this-> _lex.consumeIf (DecoratorWord::members ());
	    }
	    
	    lexing::Word name = this-> _lex.consumeIf ({Keys::UNDER});
	    if (name != Keys::UNDER) {
		name = visitIdentifier ();
	    }
		
		
	    token = this-> _lex.next ();
	    if (token == Token::COLON) {
		type = visitExpression (10);
	    } else this-> _lex.rewind ();
	    
	    params.push_back (VarDecl::init (name, decos, type, Expression::empty ()));

	    if (params.size () == 1) 
		next = this-> _lex.next ({Token::COMA, Token::RPAR});
	    else {
		next = this-> _lex.next ({Token::TDOT, Token::COMA, Token::RPAR});
		if (next == Token::TDOT) {
		    isVariadic = true;
		    this-> _lex.next ({Token::RPAR});
		}
	    }
	} while (next == Token::COMA);

	this-> _lex.next ({Token::EQUAL});
	
	return DestructDecl::init (begin, params, visitExpression (), isVariadic);
    }

    bool Visitor::canVisitSingleVarDeclaration (bool mandType, bool withValue) {
	auto begin = this-> _lex.tell ();
	TRY (
	    visitSingleVarDeclaration (mandType, withValue);
	) CATCH (ErrorCode::EXTERNAL) {
	    CLEAR_ERRORS ();
	    this-> _lex.seek (begin);
	    return false;
	} FINALLY;
	this-> _lex.seek (begin);	
	return true;
    }

    
    lexing::Word Visitor::visitNamespace () {
	lexing::Word ident;
	do {
	    auto token = this-> _lex.next ();
	    if (token == Keys::UNDER) return ident + token;
	    
	    this-> _lex.rewind ();
	    ident += visitIdentifier ();

	    token = this-> _lex.next ();
	    if (token != Token::DCOLON) {
		this-> _lex.rewind ();
		break;
	    } else ident += token;
	    
	} while (true);
	
	return ident;	
    }

    lexing::Word Visitor::visitIdentifier () {
	if (!canVisitIdentifier ()) {
	    auto token = this-> _lex.next ();
	    Error::occur (token, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.str);		
	}
	return this-> _lex.next ();
    }

    bool Visitor::canVisitIdentifier () {
	auto token = this-> _lex.next ();
	this-> _lex.rewind ();

	if (token.isToken ()) return false;

	if (token.is (this-> _forbiddenKeys))
	    return false;
	
	int i = 0;
	bool found = false;
	for (auto it : token.str) {
	    if ((it >= 'a' && it <= 'z') || (it >= 'A' && it <= 'Z')) {
		found = true;
		break;
	    } else if (it != '_')
		return false;
	    i ++;
	}
	
	i ++;
	if (i < (int) token.str.length ()) {
	    for (auto it : token.str.substr (i)) {
		if ((it < 'a' || it > 'z')
		    && (it < 'A' || it > 'Z')
		    && (it != '_')
		    && (it < '0' || it > '9')) {
		    return false;
		}
	    }
	} else return found;
	
	return true;
    }

    bool Visitor::can (Expression (Visitor::*func)()) {
	auto begin = this-> _lex.tell ();
	TRY (
	    (*this.*func) ();
	) CATCH (ErrorCode::EXTERNAL) {
	    CLEAR_ERRORS ();
	    this-> _lex.seek (begin);
	    return false;
	} FINALLY;
	this-> _lex.seek (begin);
	return true;
    }
        
    
}
