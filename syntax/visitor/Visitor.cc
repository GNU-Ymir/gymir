#include <ymir/syntax/visitor/Visitor.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/lexing/Token.hh>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>
#include <ymir/syntax/declaration/_.hh>
#include <ymir/syntax/expression/_.hh>
#include <ymir/global/State.hh>
#include <ymir/utils/string.hh>

using namespace Ymir;

namespace syntax {

    Visitor::Visitor () {}

    Visitor Visitor::init (const std::string & path) {
	lexing::Lexer lexer (path.c_str (), lexing::RealFile::init (path.c_str ()), 
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
    
    Visitor Visitor::init (const lexing::Lexer & lexer, bool strRetIg) {
	Visitor visit {};
	visit._lex = lexer;
	visit._strRetIgnore = strRetIg;
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
	    Keys::STATIC, Keys::CATCH, Keys::COPY, Keys::DCOPY
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
	lexing::Word space = lexing::Word::eof ();
	lexing::Word token = lexing::Word::eof ();
	auto beginPos = this-> _lex.tell ();
	std::string comments;
	try {
	    auto next = this-> _lex.nextWithDocs (comments, {Keys::MOD});
	    space = visitNamespace ();
	    this-> _lex.consumeIf ({Token::SEMI_COLON});
	} catch (Error::ErrorList ATTRIBUTE_UNUSED list) {
	    space = lexing::Word::eof ();
	    this-> _lex.seek (beginPos);
	}
	do {
	    token = this-> _lex.consumeIf ({Keys::PUBLIC, Keys::PRIVATE, Keys::VERSION});
	    if (token == Keys::PUBLIC || token == Keys::PRIVATE) {
		decls.push_back (visitProtectionBlock (token == Keys::PRIVATE));
	    } else if (token == Keys::VERSION) {
		decls.push_back (visitVersionGlob (true));
	    } else {
		token = this-> _lex.consumeIf (this-> _declarations);		
		if (!token.isEof ()) {
		    this-> _lex.rewind ();
		    decls.push_back (visitDeclaration ());
		}
	    }
	} while (!token.isEof ());

	if (space.isEof ())
	    space = this-> _lex.fileLocus ();

	auto ret = Module::init (space, comments, decls, true);
	return ret;
    }
    
    Declaration Visitor::visitProtectionBlock (bool isPrivate) {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	std::vector <Declaration> decls;
	auto token = this-> _lex.consumeIf ({Token::LACC});
	bool end = (token != Token::LACC); //if the block is not surrounded with {}, we get only one declaration
		
	do {
	    token = this-> _lex.consumeIf ({Token::RACC});
	    if (token == Token::RACC && !end) {
		end = true;
	    } else decls.push_back (visitDeclaration ());	    
	} while (!end);
	
	return DeclBlock::init (location, comments, decls, isPrivate, false);	
    }

    Declaration Visitor::visitVersionGlob (bool global) {
	if (!global) return visitVersionGlobBlock (global);       
	return visitVersionGlobBlock (global);
    }

    Declaration Visitor::visitVersionGlobBlock (bool global) {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	auto ident = visitIdentifier ();
	std::vector <Declaration> decls;
	
	if (global::State::instance ().isVersionActive (ident.getStr ())) {
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
	
	return DeclBlock::init (location, comments, decls, true, false);
    }


    Declaration Visitor::visitExtern () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	auto token = this-> _lex.consumeIf ({Token::LPAR});

	lexing::Word from = lexing::Word::eof (), space = lexing::Word::eof ();
	if (token == Token::LPAR) {
	    from = this-> _lex.next ({Keys::CLANG, Keys::CPPLANG, Keys::DLANG, Keys::YLANG});
	    token = this-> _lex.next ({Token::COMA, Token::RPAR});
	    if (token == Token::COMA) {
		space = visitNamespace ();
		this-> _lex.next ({Token::RPAR});
	    }
	}

	return ExternBlock::init (location, comments, from, space, visitProtectionBlock (false));
    }

    Declaration Visitor::visitMacro () {
	std::string comments;
	auto loc = this-> _lex.rewind ().nextWithDocs (comments);
	auto name = this-> _lex.next ();

	auto content = visitMacroBlock ();
	
	return Macro::init (name, comments, content);
    }

    std::vector <Declaration> Visitor::visitMacroBlock () {
	std::vector <Declaration> decls;
	auto token = this-> _lex.next ({Token::LACC});
	do {
	    token = this-> _lex.consumeIf ({Keys::PUBLIC, Keys::VERSION, Token::RACC});
	    if (token == Keys::PUBLIC) {
		decls.push_back (visitPublicMacroBlock ());
	    } else if (token == Keys::VERSION) {
		decls.push_back (visitVersionMacro ());
	    } else if (token != Token::RACC) {
		decls.push_back (visitMacroContent ());
	    }
	} while (token != Token::RACC);
	return decls;
    }

    Declaration Visitor::visitPublicMacroBlock () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	std::vector <Declaration> decls;
	auto token = this-> _lex.consumeIf ({Token::LACC});
	bool end = (token != Token::LACC);

	do {
	    token = this-> _lex.consumeIf ({Token::RACC});
	    if (token == Token::RACC) end = true;
	    else {
		decls.push_back (visitMacroContent ());
	    }
	} while (!end);
	return DeclBlock::init (location, comments, decls, false, false);
    }

    Declaration Visitor::visitVersionMacro () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	auto ident = visitIdentifier ();
	std::vector <Declaration> decls;
	if (global::State::instance ().isVersionActive (ident.getStr ())) {
	    decls = visitMacroBlock ();
	    if (this-> _lex.consumeIf ({Keys::ELSE}) == Keys::ELSE) {
		ignoreBlock ();
	    }
	} else {
	    ignoreBlock ();
	    if (this-> _lex.consumeIf ({Keys::ELSE}) == Keys::ELSE) {
		decls = visitMacroBlock ();
	    }
	}
	return DeclBlock::init (location, comments, decls, false, false);
    }

    Declaration Visitor::visitMacroContent () {
	std::string comments;
	auto token = this-> _lex.nextWithDocs (comments, {Keys::SELF, Keys::DEF, Keys::IMPORT});
	if (token == Keys::IMPORT) return visitImport ();
	
	auto name = token;
	if (token != Keys::SELF) name = visitIdentifier ();
	
	token = this-> _lex.next ({Token::LPAR}); 
	std::vector <Expression> inner;	
	lexing::Word end = token;
	bool or_ = false;
	do {
	    if (!or_) {
		if (can (&Visitor::visitMacroExpression))  {
		    inner.push_back (visitMacroExpression ());
		} else inner.push_back (visitExpression (10));
		
		if (inner.size () == 1) {
		    auto aux = this-> _lex.consumeIf ({Token::PIPE});
		    if (aux == Token::PIPE) {
			Expression next (Expression::empty ());
			if (can (&Visitor::visitMacroExpression)) 
			    next = visitMacroExpression ();
			else next = visitExpression (10);
			    
			inner.back () = MacroOr::init (aux, inner.back (), next);
			or_ = true;
			end = this-> _lex.next ({Token::RPAR, Token::PIPE});
		    } else end = this-> _lex.consumeIf ({Token::RPAR}); 
		} else end = this-> _lex.consumeIf ({Token::RPAR}); 
	    } else {
		Expression next (Expression::empty ());
		if (can (&Visitor::visitMacroExpression)) 
		    next = visitMacroExpression ();
		else next = visitExpression (10);
		inner.back () = MacroOr::init (end, inner.back (), next);
		end = this-> _lex.next ({Token::RPAR, Token::PIPE});
	    }		
	} while (end != Token::RPAR);

	Expression expr = MacroMult::init (token, end, inner, lexing::Word::eof ());

	std::vector <Expression> skips;
	if (this-> _lex.consumeIf ({Keys::SKIPS}) == Keys::SKIPS) {
	    this-> _lex.next ({Token::LPAR});
	    while (true) {
		auto str = visitString ();
		skips.push_back (MacroToken::init (str.getLocation (), str));
		if (this-> _lex.next ({Token::RPAR, Token::PIPE}) == Token::RPAR)
		    break;
	    }
	}
	
	if (name != Keys::SELF) {
	    auto nx = this-> _lex.consumeIf ({Token::SEMI_COLON});
	    if (nx == Token::SEMI_COLON) {
		return MacroRule::init (name, nx, comments, expr, "", skips);	
	    }
	}       	

	auto begin_content = this-> _lex.next ({Token::LACC});
	std::string open, close;
	open = Token::LACC;
	auto sec_open = Token::MACRO_ACC;
	close = Token::RACC;
	
	this-> _lex.skipEnable (Token::SPACE,   false);
	this-> _lex.skipEnable (Token::TAB,     false);
	this-> _lex.skipEnable (Token::RETURN,  false);
	this-> _lex.skipEnable (Token::RRETURN, false);
	this-> _lex.commentEnable (false);

	lexing::Word cursor = lexing::Word::eof ();
	Ymir::OutBuffer all;
	int nb = 1;
	do {
	    cursor = this-> _lex.next ();
	    if (cursor.isEof ()) {
		auto note = Ymir::Error::createNote (begin_content);
		Error::occurAndNote (cursor, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), cursor.getStr ());		
	    } else if (cursor == close) {
		nb -= 1;
		if (nb != 0)
		    all.write (cursor.getStr ());
	    } else if (cursor == open || cursor == sec_open) {
		nb += 1;
		all.write (cursor.getStr ());
	    } else {
		all.write (cursor.getStr ());
	    }
	} while (nb > 0);

	this-> _lex.skipEnable (Token::SPACE,   true);
	this-> _lex.skipEnable (Token::TAB,     true); 
	this-> _lex.skipEnable (Token::RETURN,  true);
	this-> _lex.skipEnable (Token::RRETURN, true);
	this-> _lex.commentEnable (true);
	if (name == Keys::SELF) {
	    return MacroConstructor::init (name, begin_content, comments, expr, Ymir::trim (all.str ()), skips);
	} else {
	    return MacroRule::init (name, begin_content, comments, expr, Ymir::trim (all.str ()), skips);
	}
    }

    Expression Visitor::visitMacroExpression () {
	auto tok = this-> _lex.consumeIf ({Token::LPAR});
	if (tok == Token::LPAR) {
	    std::vector <Expression> inner;
	    std::vector <std::string> multiplicators = {Token::STAR, Token::PLUS, Token::INTEG};
	    lexing::Word end = tok;
	    bool or_ = false;
	    do {
		if (!or_) {
		    if (can (&Visitor::visitMacroExpression)) 
			inner.push_back (visitMacroExpression ());
		    else inner.push_back (visitExpression (10));
		    
		    if (inner.size () == 1) {
			auto aux = this-> _lex.consumeIf ({Token::PIPE});
			if (aux == Token::PIPE) {
			    Expression next (Expression::empty ());
			    if (can (&Visitor::visitMacroExpression)) 
				next = visitMacroExpression ();
			    else next = visitExpression (10);
			    
			    inner.back () = MacroOr::init (aux, inner.back (), next);
			    or_ = true;
			    end = this-> _lex.next ({Token::RPAR, Token::PIPE});
			} else end = this-> _lex.consumeIf ({Token::RPAR}); 
		    } else end = this-> _lex.consumeIf ({Token::RPAR}); 
		} else {
		    Expression next (Expression::empty ());
		    if (can (&Visitor::visitMacroExpression)) 
			next = visitMacroExpression ();
		    else next = visitExpression (10);
		    inner.back () = MacroOr::init (end, inner.back (), next);
		    end = this-> _lex.next ({Token::RPAR, Token::PIPE});
		}		
	    } while (end != Token::RPAR);
	    auto mult = this-> _lex.consumeIf (multiplicators);
	    return MacroMult::init (tok, end, inner, mult);       				 
	} else if (canVisitIdentifier ()) {
	    auto ident = visitIdentifier ();

	    this-> _lex.next ({Token::EQUAL});
	    Expression expr (Expression::empty ());
	    if (can (&Visitor::visitMacroExpression)) {
		expr = visitMacroExpression ();	    
	    } else {
		expr = visitExpression (10);
	    }
	    return MacroVar::init (ident, expr);
	} else {
	    auto str = visitString ();
	    return MacroToken::init (str.getLocation (), str);
	}
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
	if (location == Keys::MACRO) return visitMacro ();
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
	std::string comments;
	this-> _lex.rewind ().nextWithDocs (comments);
	auto name = visitIdentifier ();
	std::vector <Expression> templates = visitTemplateParameters ();	
	this-> _lex.next ({Token::EQUAL});
	auto value = visitExpression ();
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	
	if (!templates.empty ()) {
	    return Template::init (name, comments, templates, Alias::init (name, comments, value), Expression::empty ());
	} else 
	    return Alias::init (name, comments, value);
    }
    
    Declaration Visitor::visitClass () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	auto attribs = visitAttributes ();
	
	auto name = this-> _lex.next ();	
	auto templates = visitTemplateParameters ();
	Expression ancestor (Expression::empty ());
	
	auto token = this-> _lex.consumeIf ({Keys::OVER});	
	if (token == Keys::OVER) 
	    ancestor = visitExpression ();
	
	auto decls = visitClassBlock ();

	if (!templates.empty ()) {
	    return Template::init (name, comments, templates, Class::init (name, comments, ancestor, decls, attribs), Expression::empty ());
	} else
	    return Class::init (name, comments, ancestor, decls, attribs);
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
	std::string comments;	
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
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
	
	if (decls.size () == 1 && decls [0].getComments () == "") // The protection can be on only one element
	    decls [0].setComments (comments);
	
	return DeclBlock::init (location, comments, decls, isPrivate, isProtected);
    }

    void Visitor::ignoreBlock () {
	auto token = this-> _lex.next ({Token::LACC});
	int close = 1;
	do {
	    token = this-> _lex.next ();
	    if (token == Token::LACC || token == Token::MACRO_ACC) close += 1;
	    if (token == Token::RACC) {
		close -= 1;
	    } else if (token.isEof ())
		Error::occur (token, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.getStr ());
	} while (close != 0);
    }

    
    
    Declaration Visitor::visitVersionClass (bool fromTrait) {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	auto ident = visitIdentifier ();
	std::vector <Declaration> decls;
	if (global::State::instance ().isVersionActive (ident.getStr ())) {
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
	return DeclBlock::init (location, comments, decls, false, false);
    }

    Declaration Visitor::visitIfClass (bool fromTrait) {
	std::string comments;
	auto location = this-> _lex.nextWithDocs (comments, {Keys::IF, Keys::ASSERT});
	if (location == Keys::IF) {
	    auto test = this-> visitExpression ();
	
	    auto decls = visitClassBlock (fromTrait);
	    auto next = this-> _lex.consumeIf ({Keys::ELSE});
	    if (next == Keys::ELSE) {
		auto next2 = this-> _lex.consumeIf ({Keys::IF});
		if (next2 == Keys::IF) {
		    this-> _lex.rewind ();
		    return CondBlock::init (location, comments, test, decls, visitIfClass (fromTrait));
		} else {
		    auto elseDecls = visitClassBlock (fromTrait);
		    return CondBlock::init (location, comments, test, decls, DeclBlock::init (next, comments, elseDecls, true, false));
		}
	    }
	    return CondBlock::init (location, comments, test, decls, DeclBlock::init (location, comments, {}, true, false));
	} else {
	    auto token = this-> _lex.consumeIf ({Token::LPAR});
	    auto test = visitExpression ();
	    if (token == Token::LPAR) {
		token = this-> _lex.next ({Token::RPAR, Token::COMA});
		if (token == Token::COMA) {
		    auto msg = visitExpression ();
		    this-> _lex.next ({Token::RPAR});
		    return Expression::toDeclaration (Assert::init (location, test, msg), comments);
		} 
	    }
	    return Expression::toDeclaration (Assert::init (location, test, Expression::empty ()), comments);
	}
    }
    
    Declaration Visitor::visitClassContent (bool fromTrait) {
	lexing::Word token = lexing::Word::eof ();
	std::string docs;
	if (fromTrait)
	    token = this-> _lex.nextWithDocs (docs, {Keys::DEF, Keys::OVER}); // Trait can only have method definitions
	else
	    token = this-> _lex.nextWithDocs (docs, {Keys::DEF, Keys::OVER, Keys::LET, Keys::SELF, Keys::IMPL, Keys::IMPORT});

	if (token == Keys::SELF) {
	    return visitClassConstructor ();
	} else if (token == Keys::DEF) {
	    return visitFunction (true);	    
	} else if (token == Keys::OVER) {
	    return visitFunction (true, true);
	} else if (token == Keys::LET) {
	    this-> _lex.rewind ();
	    return Expression::toDeclaration (visitVarDeclaration (), docs);
	} else if (token == Keys::IMPL) {
	    return visitClassMixin ();
	} else if (token == Keys::IMPORT) {
	    return visitImport ();
	} else {
	    Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Declaration::empty ();	
	}
    }
    
    Declaration Visitor::visitClassMixin () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments, {Keys::IMPL});
	auto content = visitExpression (10); // (priority of dot operator)
	if (this-> _lex.consumeIf ({Token::SEMI_COLON}) != Token::SEMI_COLON) {	    
	    std::vector <Declaration> decls = visitClassBlock (false);	    
	    return Mixin::init (location, comments, content, decls);
	} else
	    return Mixin::init (location, comments, content, {});
    }

    Declaration Visitor::visitClassConstructor () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	auto cas = visitAttributes ();
	
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
			Error::occur (ident, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), ident.getStr ());
		    this-> _lex.next ({Token::LPAR});
		    supers = visitParamList (true);
		    this-> _lex.next ({Token::RPAR});
		} else if (ident == Keys::SELF) {
		    getSelf = ident;
		    if (supers.size () != 0)
			Error::occur (ident, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), ident.getStr ());
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
	
	auto throws = this-> _lex.consumeIf ({Keys::THROWS});
	std::vector <syntax::Expression> throwers;
	if (throws == Keys::THROWS) {
	    throwers = visitThrowers ();
	}

	auto body = visitExpression ();
	
	if (templates.size () != 0) {
	    return Template::init (location, comments, templates, Constructor::init (location, comments, proto, supers, constructions, body, getSuper, getSelf, cas, throwers), Expression::empty ());
	} else 
	    return Constructor::init (location, comments, proto, supers, constructions, body, getSuper, getSelf, cas, throwers);
    }

    std::vector <syntax::Expression> Visitor::visitThrowers () {
	std::vector <syntax::Expression> throwers;
	do {
	    throwers.push_back (visitExpression ());
	    auto n =this-> _lex.consumeIf ({Token::SEMI_COLON, Token::COMA});
	    if (n != Token::COMA) {
		break;
	    }
	} while (true);
	return throwers;
    }
    
    Declaration Visitor::visitEnum () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments, {Keys::ENUM});
	Expression type (Expression::empty ());
	if (this-> _lex.consumeIf ({Token::COLON}) == Token::COLON)
	    type = visitExpression (10);
	
	lexing::Word end = lexing::Word::eof ();
	std::vector <Expression> values;
	std::vector <std::string> comms;
	bool fst = true;
	do {
	    std::string docs;
	    end = this-> _lex.nextWithDocs (docs, {Token::ARROW, Token::PIPE});
	    if (!fst) comms.push_back (docs);
	    fst = false;
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
	    return Template::init (name, comments, templates, Enum::init (name, comments, type, values, comms), Expression::empty ());
	} else return Enum::init (name, comments, type, values, comms);
    }    
    
    Declaration Visitor::visitFunction (bool isClass, bool isOver) {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	
	Expression test (Expression::empty ());
	
	auto token = this-> _lex.consumeIf ({Keys::IF});
	lexing::Word ifLoc = lexing::Word::eof ();
	if (token == Keys::IF) {
	    ifLoc = token;
	    test = visitExpression ();
	}

	auto attribs = visitAttributes ();
	auto name = visitIdentifier ();
	if (name == Keys::SELF)
	    Error::occur (name, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), name.getStr ());
	
	auto befTemplates = this-> _lex.tell ();
	auto templates = visitTemplateParameters ();
	
	token = this-> _lex.next ();
	if (token != Token::LPAR) {
	    this-> _lex.seek (befTemplates);
	    templates.clear ();
	}
	else this-> _lex.rewind ();

	auto proto = visitFunctionPrototype (false, isClass);

	auto throws = this-> _lex.consumeIf ({Keys::THROWS});
	std::vector <syntax::Expression> throwers;
	if (throws == Keys::THROWS) {
	    throwers = visitThrowers ();
	}

	auto body = visitFunctionBody ();
	
	
	auto function = Function::init (name, comments, proto, body, attribs, throwers, isOver);
	
	if (templates.size () != 0) {
	    return Template::init (name, comments, templates, function, test);
	} else {
	    if (!test.isEmpty ())
		Error::occur (ifLoc, ExternalError::get (SYNTAX_ERROR_IF_ON_NON_TEMPLATE)); 
	    return function;
	}
    }

    Function::Prototype Visitor::visitFunctionPrototype (bool isClosure, bool isClass) {
	std::vector <Expression> vars;
	lexing::Word token = lexing::Word::eof ();
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

    Expression Visitor::visitFunctionBody () {	
	auto token = this-> _lex.consumeIf ({Token::SEMI_COLON});
	if (token == Token::SEMI_COLON) return Expression::empty ();
	
	return visitExpression ();
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
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	auto decl = visitSingleVarDeclaration ();
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	return Global::init (location, comments, decl);
    }

    Declaration Visitor::visitImport () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments);
	lexing::Word token = lexing::Word::eof ();
	std::vector <Declaration> imports;
	do {
	    auto space = visitNamespace ();
	    lexing::Word as = lexing::Word::eof ();
	    
	    token = this-> _lex.consumeIf ({Token::COMA, Keys::AS});
	    if (token == Keys::AS) {
		as = visitIdentifier ();
		token = this-> _lex.consumeIf ({Token::COMA});
	    }
	    
	    imports.push_back (Import::init (location, comments, space));
	} while (token == Token::COMA);

	this-> _lex.consumeIf ({Token::SEMI_COLON});
	return DeclBlock::init (location, comments, imports, true, false);
    }

    Declaration Visitor::visitLocalMod () {
	std::string comments;
	this-> _lex.rewind ().nextWithDocs (comments, {Keys::MOD});
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
	    return Template::init (name, comments, templates, Module::init (name, comments, decls, false), Expression::empty ());
	} else return Module::init (name, comments, decls, false);
    }

    Declaration Visitor::visitStruct () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments, {Keys::STRUCT});
	lexing::Word end = lexing::Word::eof ();
	std::vector <Expression> vars;
	std::vector <std::string> comms;
	std::vector <lexing::Word> attrs = visitAttributes ();
	bool fst = true;
	do {
	    std::string docs;
	    end = this-> _lex.nextWithDocs (docs, {Token::ARROW, Token::PIPE});
	    if (!fst) comms.push_back (docs); // The comment is after the var decl
	    fst = false;
	    
	    if (end != Token::ARROW) {
		vars.push_back (visitSingleVarDeclaration (true, false));
		if (this-> _lex.consumeIf ({Token::EQUAL}) == Token::EQUAL) {
		    auto bVdecl = vars.back ().to <VarDecl> ();
		    auto vdecl = VarDecl::init (bVdecl.getName (), bVdecl.getDecorators (), bVdecl.getType (), visitExpression (10));
		    vars.back () = vdecl;
		}
	    }
	} while (end != Token::ARROW);
	auto name = visitIdentifier ();
	auto templates = visitTemplateParameters ();
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	if (templates.size () != 0) {
	    return Template::init (name, comments, templates, Struct::init (name, comments, attrs, vars, comms), Expression::empty ());
	} else return Struct::init (name, comments, attrs, vars, comms);
    }

    Declaration Visitor::visitTrait () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments, {Keys::TRAIT});
	auto name = visitIdentifier ();
	auto templates = visitTemplateParameters ();

	std::vector <Declaration> decls = visitClassBlock (false);
	
	if (!templates.empty ()) {
	    return Template::init (name, comments, templates, Trait::init (name, comments, decls), Expression::empty ());
	} else return Trait::init (name, comments, decls);	
    }

    Declaration Visitor::visitUse () {
	std::string comments;
	auto location = this-> _lex.rewind ().nextWithDocs (comments, {Keys::USE});
	auto content = visitExpression (10);
	this-> _lex.consumeIf ({Token::SEMI_COLON});
	
	return Use::init (location, comments, content);
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
	    try {
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
	    } catch (Error::ErrorList ATTRIBUTE_UNUSED list) {
		this-> _lex.seek (begin);
		return {};	
	    }
	    
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
		return visitExpression (Binary::init (lexing::Word::init (token, token.getStr () + next.getStr ()), left, right, ctype), priority);
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
	if (next == Keys::LET)      return visitVarDeclaration ();
	if (next == Keys::RETURN)   return visitReturn ();
	if (next == Keys::FUNCTION) return visitFunctionType ();
	if (next == Keys::DELEGATE) return visitFunctionType ();
	if (next == Keys::LOOP)     return visitWhile ();
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
	    lexing::Word end = lexing::Word::eof ();
	    if (location == Token::LPAR) end = this-> _lex.next ({Token::RPAR});
	    else end = this-> _lex.next ({Token::RCRO});
	    return visitOperand1 (MultOperator::init (location, end, value, params));
	} else if (location == Token::MACRO_ACC || location == Token::MACRO_CRO || location == Token::MACRO_PAR) {
	    return visitMacroCall (value);
	} else if (location == Token::DOT || location == Token::DOT_AND) {
	    auto right = visitOperand3 (false);
	    return visitOperand1 (visitTemplateCall (Binary::init (location, value, right, Expression::empty ())));
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
	
	if (can (&Visitor::visitLiteral))  return visitLiteral ();
	return visitVar (canBeTemplateCall);
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

    Expression Visitor::visitBlock (bool canCatcher) {	
	std::vector <Declaration> decls;	
	std::vector <Expression> content;
	auto begin = this-> _lex.next ({Token::LACC});

	lexing::Word end = lexing::Word::eof ();
	bool last = false;
	do {
	    end = this-> _lex.consumeIf ({Token::RACC, Token::SEMI_COLON});
	    if (end != Token::RACC && end != Token::SEMI_COLON) {
		last = false;
		if (this-> _lex.consumeIf (this-> _declarationsBlock).getStr () != "") {
		    this-> _lex.rewind ();
		    decls.push_back (visitDeclaration ());
		} else 
		    content.push_back (visitExpression ());
	    } else if (end == Token::SEMI_COLON)
		last = true;
	} while (end != Token::RACC);

	Expression catcher (Expression::empty ());

	std::vector <Expression> scopes;
	if (canCatcher) {
	    do {
		
		lexing::Word next = lexing::Word::eof ();
		if (catcher.isEmpty ())
		    next = this-> _lex.consumeIf ({Keys::EXIT, Keys::SUCCESS, Keys::FAILURE, Keys::CATCH});
		else next = this-> _lex.consumeIf ({Keys::EXIT, Keys::SUCCESS, Keys::FAILURE});
		if (next == Keys::CATCH) {
		    catcher = visitCatch ();
		} else if (next != "") {
		    this-> _lex.consumeIf ({Token::DARROW});
		    scopes.push_back (Scope::init (next, visitBlock (false)));
		} else break;
	    } while (true);
	}
	
	if (last) content.push_back (Unit::init (end));
	if (decls.size () != 0) {
	    return Block::init (begin, end, Module::init (lexing::Word::init (begin, "_"), "", decls, false), content, catcher, scopes);
	} else {
	    return Block::init (begin, end, Declaration::empty (), content, catcher, scopes);
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
	lexing::Word token = lexing::Word::eof ();
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
	auto begin = this-> _lex.rewind ().next ();
	auto next = this-> _lex.consumeIf ({Token::LACC});
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
	
	return Catch::init (begin, matchs, actions);
    }
    
    Expression Visitor::visitVersion () {
	auto location = this-> _lex.next ({Keys::VERSION});
	auto ident = visitIdentifier ();
	if (global::State::instance ().isVersionActive (ident.getStr ())) {
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
	
	return Unit::init (location);
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
		    Error::occurAndNote (token, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.getStr ());
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
	}
	
	return Var::init (name);	
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

    Expression Visitor::visitMacroCall (const Expression & left) {
	auto tok = this-> _lex.rewind ().next ();
	std::string open, close;
	if (tok == Token::MACRO_ACC) {
	    open = Token::LACC; close = Token::RACC;
	} else if (tok == Token::MACRO_CRO) {
	    open = Token::LCRO; close = Token::RCRO;
	} else {
	    open = Token::LPAR; close = Token::RPAR;
	}

	this-> _lex.skipEnable (Token::SPACE,   false);
	this-> _lex.skipEnable (Token::TAB,     false);
	this-> _lex.skipEnable (Token::RETURN,  false);
	this-> _lex.skipEnable (Token::RRETURN, false);
	this-> _lex.commentEnable (false);

	lexing::Word cursor = lexing::Word::eof ();
	Ymir::OutBuffer all;
	int nb = 1;
	do {
	    cursor = this-> _lex.next ();
	    if (cursor.isEof ()) {
		auto note = Ymir::Error::createNote (tok);
		Error::occurAndNote (cursor, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), cursor.getStr ());		
	    } else if (cursor == close) {
		nb -= 1;
		if (nb != 0)
		    all.write (cursor.getStr ());
	    } else if (cursor == open) {
		nb += 1;
		all.write (cursor.getStr ());
	    } else {
		all.write (cursor.getStr ());
	    }
	} while (nb > 0);

	this-> _lex.skipEnable (Token::SPACE,   true);
	this-> _lex.skipEnable (Token::TAB,     true); 
	this-> _lex.skipEnable (Token::RETURN,  true);
	this-> _lex.skipEnable (Token::RRETURN, true);
	this-> _lex.commentEnable (true);
	
	return MacroCall::init (tok, cursor, left, all.str ());	
    }
        
    std::vector <Expression> Visitor::visitParamList (bool withNamed) {
	std::vector <Expression> params;
	auto begin = this-> _lex.tell ();
	try {
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
	} catch (Error::ErrorList ATTRIBUTE_UNUSED list) {
	    this-> _lex.seek (begin);
	    return {};
	} 

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

    Expression Visitor::visitMacroEval () {
	auto tok = this-> _lex.next ({Token::MACRO_ACC, Token::MACRO_PAR, Token::MACRO_CRO});

	auto expr = visitExpression ();

	lexing::Word end = lexing::Word::eof ();
	
	if (tok == Token::MACRO_ACC) {
	    end = this-> _lex.next ({Token::RACC});
	} else if (tok == Token::MACRO_CRO) {
	    end = this-> _lex.next ({Token::RCRO});
	} else {
	    end = this-> _lex.next ({Token::RPAR});
	}
	
	return MacroEval::init (tok, end, expr);
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
	lexing::Word token = lexing::Word::eof ();
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
	if ((tok.getStr () [0] >= '0' && tok.getStr () [0] <= '9') || (tok.getStr () [0] == '_' && tok.getStr ().length () != 1))
	                                               return visitNumeric ();
	if (tok == Token::DOT)                         return visitFloat (lexing::Word::eof ());
	if (tok == Token::APOS)                        return visitChar ();
	if (tok == Token::GUILL)                       return visitString ();
	if (tok == Keys::TRUE_ || tok == Keys::FALSE_) return Bool::init (this-> _lex.next ());
	if (tok == Keys::NULL_)                        return Null::init (this-> _lex.next ());
	if (tok == Token::DOLLAR)                      return Dollar::init (this-> _lex.next ());
	
	Error::occur (tok, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), tok.getStr ());
	return Expression::empty ();
    }

    Expression Visitor::visitNumeric () {
	auto begin = this-> _lex.next ();	
	if (begin.getStr ().length () >= 4) {
	    auto suffix = lexing::Word::init (begin.getStr ().substr (begin.getStr ().length () - 3),
					      begin.getFile (),
					      begin.getFilename (),
					      begin.getLine (),
					      begin.getColumn () + begin.getStr ().length () - 3,
					      begin.getSeek () + begin.getStr ().length () - 3
		);
	    
	    if (suffix.is (this-> _fixedSuffixes)) {
		auto value = begin.getStr ().substr (0, begin.getStr ().length () - 3);
		verifNumeric (begin, value);
		return Fixed::init (lexing::Word::init (begin, value), suffix);
	    }
	}
	
	if (begin.getStr ().length () >= 3) {
	    auto suffix = lexing::Word::init (begin.getStr ().substr (begin.getStr ().length () - 2),
					      begin.getFile (),
					      begin.getFilename (),
					      begin.getLine (),
					      begin.getColumn () + begin.getStr ().length () - 2,
					      begin.getSeek () + begin.getStr ().length () - 2
		);
	    
	    if (suffix.is (this-> _fixedSuffixes)) {
		auto value = begin.getStr ().substr (0, begin.getStr ().length () - 2);
		verifNumeric (begin, value);
		return Fixed::init (lexing::Word::init (begin, value), suffix);
	    }
	}
	
	auto value = begin.getStr ();
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
		    Error::occur (loc, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), loc.getStr ());
		}
	    }
	    return true;
	} else if (value.length () > 2 && value [0] == '0' && value [1] == 'o') {
	    for (uint i = 2 ; i < value.length (); i++) {
		if ((value [i] < '0' || value [i] > '7') && value [i] != Keys::UNDER [0]) {
		    Error::occur (loc, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), loc.getStr ());
		}
	    }
	    return true;
	} else {
	    for (uint i = 0 ; i < value.length (); i++) {
		if ((value [i] < '0' || value [i] > '9') && value [i] != Keys::UNDER [0]) {
		    Error::occur (loc, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), loc.getStr ());
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
	
	if (after.getStr () [0] >= '0' && after.getStr () [0] <= '9') {
	    if (after.getStr ().length () >= 2) {		
		auto suffix = lexing::Word::init (after.getStr ().substr (after.getStr ().length () - 1),
						  after.getFile (),
						  after.getFilename (),
						  after.getLine (),
						  after.getColumn () + after.getStr ().length () - 1,
						  after.getSeek () + after.getStr ().length () - 1
		    );
	    
		if (suffix.is (this-> _floatSuffix)) {
		    auto value = after.getStr ().substr (0, after.getStr ().length () - 1);
		    if (!verifNumeric (after, value))
			return Float::init (dot, begin, lexing::Word::init (after, value), suffix);
		    else
			Error::occur (after, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), after.getStr ());
		}
	    }
	
	    auto value = after.getStr ();
	    for (uint i = 0 ; i < value.length (); i++) {
		if ((value [i] < '0' || value [i] > '9') && value [i] != Keys::UNDER [0]) {
		    if (begin.isEof ())
			Error::occur (after, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), after.getStr ());
		    else {
			this-> _lex.rewind ();
			after = lexing::Word::eof ();
			break;
		    }
		} 
	    }
	    return Float::init (dot, begin, after, lexing::Word::eof ());
	} else if (begin.isEof ()) {
	    Error::occur (after, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), after.getStr ());
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
	Ymir::OutBuffer all;
	do {
	    cursor = this-> _lex.next ();
	    if (cursor == Token::TAB || cursor == Token::RETURN || cursor == Token::RRETURN) {
		std::list <Ymir::Error::ErrorMsg> notes;
		notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (MUST_ESCAPE_CHAR)));
		notes.push_back (Ymir::Error::createNote (begin));
		Error::occurAndNote (cursor, notes, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), cursor.getStr ());		
	    } else if (cursor.isEof ()) {
		auto note = Ymir::Error::createNote (begin);
		Error::occurAndNote (cursor, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), cursor.getStr ());		
	    } else if (cursor != Token::APOS) {
		all.write (cursor.getStr ());
	    }
	} while (cursor != Token::APOS);

	auto format = this-> _lex.consumeIf (this-> _charSuffix);
	if (!format.isEof () && format.getStr () [0] == '_') {
	    format = lexing::Word::init (format, format.getStr ().substr (1, format.getStr ().length  ()- 1));
	}
	// We restore the skip and comments
	this-> _lex.skipEnable (Token::SPACE, true);
	this-> _lex.skipEnable (Token::TAB, true);
	this-> _lex.skipEnable (Token::RETURN, true);
	this-> _lex.skipEnable (Token::RRETURN, true);
	this-> _lex.commentEnable (true);
	
	return Char::init (begin, cursor, lexing::Word::init (begin, all.str ()), format);
    }

    Expression Visitor::visitString () {
	auto begin = this-> _lex.next ({Token::GUILL});
	this-> _lex.skipEnable (Token::SPACE,   false);
	if (!this-> _strRetIgnore) {
	    this-> _lex.skipEnable (Token::TAB,     false);
	    this-> _lex.skipEnable (Token::RETURN,  false);
	    this-> _lex.skipEnable (Token::RRETURN, false);
	}
	
	this-> _lex.commentEnable (false);

	lexing::Word cursor = lexing::Word::eof ();
	Ymir::OutBuffer all;
	do {
	    cursor = this-> _lex.next ();
	    if (cursor.isEof ()) {
		auto note = Ymir::Error::createNote (begin);
		Error::occurAndNote (cursor, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), cursor.getStr ());		
	    } else if (cursor != begin) {
		all.write (cursor.getStr ());
	    }
	} while (cursor != begin);

	auto format = this-> _lex.consumeIf (this-> _stringSuffix);
	if (!format.isEof () && format.getStr () [0] == '_') {
	    format = lexing::Word::init (format, format.getStr ().substr (1, format.getStr ().length  ()- 1));
	}
	
	this-> _lex.skipEnable (Token::SPACE,   true);
	if (!this-> _strRetIgnore) {
	    this-> _lex.skipEnable (Token::TAB,     true); // Need to add the escape char to get a tab or a line break
	    this-> _lex.skipEnable (Token::RETURN,  true);
	    this-> _lex.skipEnable (Token::RRETURN, true);
	}
	this-> _lex.commentEnable (true);

	return String::init (begin, cursor, lexing::Word::init (begin, all.str ()), format);
    }
    
    Expression Visitor::visitVarDeclaration () {
	auto location = this-> _lex.next ({Keys::LET});
	lexing::Word token = lexing::Word::eof ();
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
		    Error::occurAndNote (token, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.getStr ());
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
		Error::occur (name, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), name.getStr ());
	    
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
		    Error::occurAndNote (token, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.getStr ());
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
	    Error::occur (name, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), name.getStr ());
	
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
	lexing::Word next = lexing::Word::eof ();
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
			Error::occurAndNote (token, note, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.getStr ());
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
	try {
	    visitSingleVarDeclaration (mandType, withValue);
	} catch (Error::ErrorList ATTRIBUTE_UNUSED list) {
	    this-> _lex.seek (begin);
	    return false;
	}
	this-> _lex.seek (begin);	
	return true;
    }

    
    lexing::Word Visitor::visitNamespace () {
	lexing::Word begin = this-> _lex.next ();
	Ymir::OutBuffer ident;
	this-> _lex.rewind ();
	do {
	    auto token = this-> _lex.next ();
	    if (token == Keys::UNDER) {
		ident.write (token.getStr ());
		break;
	    }
	    
	    this-> _lex.rewind ();
	    ident.write (visitIdentifier ().getStr ());

	    token = this-> _lex.next ();
	    if (token != Token::DCOLON) {
		this-> _lex.rewind ();
		break;
	    } else ident.write (token.getStr ());
	    
	} while (true);
	
	return lexing::Word::init (begin, ident.str ());	
    }

    lexing::Word Visitor::visitIdentifier () {
	if (!canVisitIdentifier ()) {
	    auto token = this-> _lex.next ();
	    Error::occur (token, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), token.getStr ());		
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
	for (auto it : token.getStr ()) {
	    if ((it >= 'a' && it <= 'z') || (it >= 'A' && it <= 'Z')) {
		found = true;
		break;
	    } else if (it != '_')
		return false;
	    i ++;
	}
	
	i ++;
	if (i < (int) token.getStr ().length ()) {
	    for (auto it : token.getStr ().substr (i)) {
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
	try {
	    (*this.*func) ();
	} catch (Error::ErrorList ATTRIBUTE_UNUSED list) {
	    this-> _lex.seek (begin);
	    return false;
	}
	this-> _lex.seek (begin);
	return true;
    }

    bool Visitor::isEof () {
	auto current = this-> _lex.tell ();
	auto next =  this-> _lex.next ();
	this-> _lex.seek (current);
	return next.isEof ();
    }

    lexing::Lexer & Visitor::getLexer () {
	return this-> _lex;
    }

    Visitor::~Visitor () {
	this-> _lex.getFile ().close ();
    }
    
}
