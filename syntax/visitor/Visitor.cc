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
	    Keys::FUNCTION, Keys::LET, Keys::IS, Keys::EXTERN,
	    Keys::PUBLIC, Keys::PRIVATE, Keys::TYPEOF, Keys::IMMUTABLE,
	    Keys::MACRO, Keys::TRAIT, Keys::REF, Keys::CONST,
	    Keys::MOD, Keys::USE, Keys::STRINGOF, Keys::TYPE, Keys::ALIAS,
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
	    Keys::ALIAS, Keys::TYPE, Keys::ENUM,
	    Keys::DEF, Keys::STATIC, Keys::IMPORT,
	    Keys::MACRO, Keys::MOD, Keys::STRUCT,
	    Keys::TRAIT, Keys::USE, Keys::MIXIN, Keys::EXTERN
	};
	
	visit._declarationsBlock = {
	    Keys::TYPE, Keys::ENUM,  Keys::DEF,
	    Keys::IMPORT, Keys::STRUCT, Keys::TRAIT,
	    Keys::USE, Keys::MIXIN
	};

	visit._intrisics = {
	    Keys::COPY, Keys::EXPAND, Keys::TYPEOF, Keys::SIZEOF, Keys::ALIAS
	};
	
	visit._operand_op = {
	    Token::MINUS, Token::AND, Token::STAR, Token::NOT
	};
	
	visit._fixedSuffixes = {
	    Keys::I8, Keys::U8, Keys::I16, Keys::U16, Keys::U32, Keys::I64, Keys::U64
	};

	visit._floatSuffix = {
	    Keys::FLOAT_S
	};

	visit._charSuffix = {
	    Keys::C8, Keys::C16, Keys::C32
	};

	visit._stringSuffix = {
	    Keys::S8, Keys::S16, Keys::S32
	};
	
	return visit;
    }

    Declaration Visitor::visitModGlobal () {
	std::vector <Declaration> decls;
	lexing::Word space;
	lexing::Word token;
	auto beginPos = this-> _lex.tell ();
	TRY (
	    auto next = this-> _lex.consumeIf ({Keys::MOD});
	    if (next == Keys::MOD) {
		space = visitNamespace ();
		this-> _lex.consumeIf ({Token::SEMI_COLON});
	    } 
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

	return DeclBlock::init (location, decls, isPrivate);	
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
	
	return DeclBlock::init (location, decls, false);
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
	if (location == Keys::TYPE) return visitClass ();
	if (location == Keys::ENUM) return visitEnum ();
	if (location == Keys::DEF) return visitFunction ();
	if (location == Keys::STATIC) return visitGlobal ();
	if (location == Keys::IMPORT) return visitImport ();
	if (location == Keys::EXTERN) return visitExtern ();
	// if (location == Keys::MACRO) return visitMacro ();
	if (location == Keys::MOD) return visitLocalMod ();
	if (location == Keys::STRUCT) return visitStruct ();
	if (location == Keys::TRAIT) return visitTrait ();
	if (location == Keys::MIXIN) return visitTrait ();
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
	auto name = this-> _lex.next ();	
	auto templates = visitTemplateParameters ();
	Expression ancestor (Expression::empty ());
	
	auto token = this-> _lex.consumeIf ({Keys::OVER});	
	if (token == Keys::OVER) 
	    ancestor = visitExpression ();
	
	auto decls = visitClassBlock ();

	if (!templates.empty ()) {
	    return Template::init (name, templates, Class::init (name, ancestor, decls));
	} else
	    return Class::init (name, ancestor, decls);
    }

    std::vector <Declaration> Visitor::visitClassBlock () {
	std::vector <Declaration> decls;
	
	auto token = this-> _lex.next ({Token::LACC});
	do {
	    token = this-> _lex.consumeIf ({Keys::PRIVATE, Keys::PROTECTED, Keys::VERSION, Token::RACC, Token::SEMI_COLON});
	    if (token == Keys::PRIVATE || token == Keys::PROTECTED) {
		decls.push_back (visitProtectionClassBlock (token == Keys::PRIVATE));
	    } else if (token == Keys::VERSION) {
		decls.push_back (visitVersionClass ());
	    } else if (token != Token::RACC && token != Token::SEMI_COLON) {
		decls.push_back (visitClassContent ());
	    } 
	} while (token != Token::RACC);
	return decls;
    }
    
    Declaration Visitor::visitProtectionClassBlock (bool isPrivate) {
	auto location = this-> _lex.rewind ().next ();
	std::vector <Declaration> decls;
	auto token = this-> _lex.consumeIf ({Token::LACC});
	bool end = (token != Token::LACC);

	do {
	    token = this-> _lex.consumeIf ({Token::RACC});
	    if (token == Token::RACC && !end) end = true;
	    else {
		decls.push_back (visitDeclaration ());
	    }
	} while (!end);
	
	return DeclBlock::init (location, decls, isPrivate);
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

    
    
    Declaration Visitor::visitVersionClass () {
	auto location = this-> _lex.rewind ().next ();
	auto ident = visitIdentifier ();
	std::vector <Declaration> decls;
	if (global::State::instance ().isVersionActive (ident.str)) {
	    decls = visitClassBlock ();
	    if (this-> _lex.consumeIf ({Keys::ELSE}) == Keys::ELSE) {
		ignoreBlock ();
	    }	   
	} else {
	    ignoreBlock ();
	    if (this-> _lex.consumeIf ({Keys::ELSE}) == Keys::ELSE) {
		decls = visitClassBlock ();
	    }
	}   
	return DeclBlock::init (location, decls, false);
    }
    
    Declaration Visitor::visitClassContent () {
	auto token = this-> _lex.next ({Keys::DEF, Keys::LET, Keys::SELF, Keys::MIXIN});
	if (token == Keys::SELF) {
	    if (this-> _lex.consumeIf ({Token::TILDE}) == Token::TILDE)
		return visitClassDestructor ();
	    else return visitClassConstructor ();
	} else if (token == Keys::DEF) {
	    return visitFunction ();	    
	} else if (token == Keys::LET) {
	    this-> _lex.rewind ();
	    return Expression::toDeclaration (visitVarDeclaration ());
	} else if (token == Keys::MIXIN) {
	    return visitClassMixin ();
	} else {
	    Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Declaration::empty ();	
	}
    }

    Declaration Visitor::visitClassMixin () {
	auto location = this-> _lex.rewind ().next ({Keys::MIXIN});
	auto content = visitExpression (10); // (priority of dot operator)
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	return Mixin::init (location, content);
    }

    Declaration Visitor::visitClassConstructor () {
	auto location = this-> _lex.rewind ().next ();
	auto before_template = this-> _lex.tell ();
	auto templates = visitTemplateParameters ();
	auto token = this-> _lex.next ();
	if (token != Token::LPAR) { // If there is no remaining parameters, it means that the read templates may be the runtime parameters
	    templates.clear (); // So, we remove them
	    this-> _lex.seek (before_template); // And we seek to this location to get them as runtime parameters
	} else this-> _lex.rewind ();

	auto proto = visitFunctionPrototype ();
	auto body = visitFunctionBody ();
	if (templates.size () != 0) {
	    return Template::init (location, templates, Function::init (location, proto, body));
	} else 
	    return Function::init (location, proto, body);
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
    
    Declaration Visitor::visitFunction () {       
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
	    templates.clear ();
	    this-> _lex.seek (befTemplates);
	}
	else this-> _lex.rewind ();

	auto proto = visitFunctionPrototype ();
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

    Function::Prototype Visitor::visitFunctionPrototype () {
	std::vector <Expression> vars;
	auto token = this-> _lex.next ({Token::LPAR});
	bool isVariadic = false;
	do {
	    token = this-> _lex.consumeIf ({Token::RPAR, Token::TDOT});
	    if (token == Token::TDOT) {
		isVariadic = true;
		token = this-> _lex.next ({Token::RPAR});
	    } else if (token != Token::RPAR) {
		vars.push_back (visitSingleVarDeclaration (false, true));
		token = this-> _lex.next ({Token::RPAR, Token::COMA, Token::TDOT});
	    } 	    	     
	} while (token != Token::RPAR);
	
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
	return DeclBlock::init (location, imports, false);
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
	auto location = this-> _lex.rewind ().next ();
	auto name = visitIdentifier ();
	auto templates = visitTemplateParameters ();

	auto token = this-> _lex.next ({Token::LACC});
	std::vector <Declaration> decls = visitClassBlock ();
	
	if (!templates.empty ()) {
	    return Template::init (name, templates, Trait::init (name, decls, location == Keys::MIXIN));
	} else return Trait::init (name, decls, location == Keys::MIXIN);	
    }

    Declaration Visitor::visitUse () {
	auto location = this-> _lex.rewind ().next ({Keys::USE});
	auto content = visitExpression (10);
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	
	return Use::init (location, content);
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
				list.push_back (VarDecl::init (name, {}, visitExpression (), Expression::empty ()));
			    }
			} else if (token == Keys::OF)
			    list.push_back (OfVar::init (name, visitExpression ()));
			else if (token == Token::TDOT) {
			    list.push_back (VariadicVar::init (name, false));
			} else {
			    this-> _lex.rewind (2);
			    list.push_back (visitExpression (10));
			}
		    } else list.push_back (visitExpression (10));
		    
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
	} else if (next == Token::DOT) {
	    auto right = visitOperand3 (false);
	    return visitOperand2 (Binary::init (next, value, right, Expression::empty ()));
	} this-> _lex.rewind ();
	return value;
    }
    
    Expression Visitor::visitOperand3 (bool canBeTemplateCall) {
	auto begin = this-> _lex.next ();
	this-> _lex.rewind ();
	if (begin == Token::LPAR) {
	    auto loc = this-> _lex.tell ();
	    Expression lambda (Expression::empty ());
	    TRY (
	    	// Could not return directly because of scope guard
		// The problem has been solved since, but no need to change that
	    	lambda = visitLambda ();		
	    ) CATCH (ErrorCode::EXTERNAL) {
	    	CLEAR_ERRORS ();
		
	    	this-> _lex.seek (loc);
		return visitTuple ();
	    } FINALLY;
	    
	    return lambda;
	}

	if (begin == Token::LCRO)   return visitArray ();
	if (begin == Token::LACC)   return visitBlock ();
	if (begin == Keys::IF)      return visitIf ();
	if (begin == Keys::DO)      return visitDoWhile ();
	if (begin == Keys::WHILE)   return visitWhile ();
	if (begin == Keys::LOOP)    return visitWhile ();
	if (begin == Keys::FOR)     return visitFor ();
	if (begin == Keys::MATCH)   return visitMatch ();
	if (begin == Keys::LET)     return visitVarDeclaration ();
	if (begin == Keys::BREAK)   return visitBreak ();
	if (begin == Keys::ASSERT)  return visitAssert ();
	if (begin == Keys::THROW_K) return visitThrow ();
	if (begin == Keys::PRAGMA)  return visitPragma ();
	if (begin == Keys::SCOPE)   return visitScope ();
	if (begin == Keys::VERSION) return visitVersion ();
	if (begin == Keys::RETURN)  return visitReturn ();
	if (begin.is (this-> _intrisics)) {
	    auto loc = this-> _lex.next ();
	    return Intrinsics::init (loc, visitExpression (10));
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
	    else return If::init (location, test, content, visitExpression ());
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
	Error::halt ("%(r) - need match", "TODO");
	return Expression::empty ();
    }    

    Expression Visitor::visitBreak () {
	auto location = this-> _lex.next ({Keys::BREAK});
	return Break::init (location, visitExpression());
    }

    Expression Visitor::visitAssert () {
	auto location = this-> _lex.next ();
	auto test = visitExpression ();
	return Assert::init (location, test, visitExpression());
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
	this-> _lex.next ({Token::DARROW});
	if (name == Keys::FAILURE) {
	    auto next = this-> _lex.next ();
	    if (next != Token::LACC || !canVisitSingleVarDeclaration (true, false)) {
		this-> _lex.rewind ();
		return Scope::init (name, visitExpression ());
	    } else {
		std::vector<Expression> types, values;
		do {
		    types.push_back (visitSingleVarDeclaration (false));
		    this-> _lex.next ({Token::DARROW});
		    values.push_back (visitExpression ());
		    next = this-> _lex.next ();
		    if (next != Token::RACC) this-> _lex.rewind ();
		} while (next != Token::RACC);
		return ScopeFailure::init (location, types, values);
	    }
	} else return Scope::init (name, visitExpression ());
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
		auto lex = this-> _lex.consumeIf ({Token::INTEG});
		if (lex == Token::INTEG) {
		    done = true;
		    auto name = visitIdentifier ();
		    this-> _lex.next ({Token::EQUAL});
		    auto inner = visitExpression ();
		    params.push_back (NamedExpression::init (name, inner));
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
		auto lex = this-> _lex.consumeIf ({Token::INTEG});
		if (lex == Token::INTEG) {
		    done = true;
		    auto name = visitIdentifier ();
		    this-> _lex.next ({Token::EQUAL});
		    auto inner = visitExpression ();
		    params.push_back (NamedExpression::init (name, inner));
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
	auto begin = this-> _lex.consumeIf ({Keys::REF});

	auto proto = visitFunctionPrototype ();
	this-> _lex.next ({Token::DARROW});
	return Lambda::init (begin, proto, visitExpression ());
    }       
    
    Expression Visitor::visitLiteral () {
	auto tok = this-> _lex.next ();
	this-> _lex.rewind ();
	if ((tok.str [0] >= '0' && tok.str [0] <= '9') || (tok.str [0] == '_' && tok.str.length () != 1))
	                                               return visitNumeric ();
	if (tok == Token::DOT)                         return visitFloat (lexing::Word::eof ());
	//if (tok == Token::GUILL)                       return visitString ();
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
	auto dot = this-> _lex.next ();
	auto after = this-> _lex.next ();
	if (after.str [0] >= '0' && after.str [0] <= '9') {
	    if (after.str.length () >= 2) {
		auto suffix = lexing::Word {after, after.str.substr (after.str.length () - 1)};
		suffix.column += after.str.length () - 1;
	    
		if (suffix.is (this-> _floatSuffix)) {
		    auto value = after.str.substr (0, after.str.length () - 1);
		    if (!verifNumeric (after, value))
			return Float::init (begin, {after, value}, suffix);
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
	
	    return Float::init (begin, after, lexing::Word::eof ());
	} else if (begin.isEof ()) {
	    Error::occur (after, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), after.str);
	} else this-> _lex.rewind ();
	return Float::init (begin, lexing::Word::eof (), lexing::Word::eof ());
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
	this-> _lex.skipEnable (Token::TAB,     false);
	this-> _lex.skipEnable (Token::RETURN,  false);
	this-> _lex.skipEnable (Token::RRETURN, false);
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
	
	this-> _lex.skipEnable (Token::SPACE,   true);
	this-> _lex.skipEnable (Token::TAB,     true);
	this-> _lex.skipEnable (Token::RETURN,  true);
	this-> _lex.skipEnable (Token::RRETURN, true);
	this-> _lex.commentEnable (true);

	return String::init (begin, cursor, all, format);
    }
    
    Expression Visitor::visitVarDeclaration () {
	auto location = this-> _lex.next ({Keys::LET});
	lexing::Word token;
	std::vector <Expression> decls;
	do {
	    decls.push_back (visitSingleVarDeclaration ());
	    token = this-> _lex.next ();
	} while (token == Token::COMA);

	this-> _lex.rewind (); // The last token read wasn't a coma
	if (decls.size () == 1) return decls [0];
	else return Set::init (location, decls);
    }    

    Expression Visitor::visitSingleVarDeclaration (bool mandType, bool withValue) {
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
	
	if (mandType) token = this-> _lex.next ({Token::COLON});
	else token = this-> _lex.next ();
	
	if (token == Token::COLON) {
	    type = visitExpression (10);
	    token = this-> _lex.next ();
	} 

	if (token == Token::EQUAL && withValue)
	    value = visitExpression ();
	else this-> _lex.rewind ();
	
	return VarDecl::init (name, decos, type, value);
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
