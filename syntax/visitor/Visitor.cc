#include <ymir/syntax/visitor/Visitor.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/lexing/Token.hh>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>
#include <ymir/syntax/declaration/_.hh>
#include <ymir/syntax/expression/_.hh>
#include <algorithm>

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
	
	return visit;
    }

    Declaration Visitor::visitModGlobal () {
	std::vector <Declaration> decls;
	lexing::Word space;
	lexing::Word token;
	auto beginPos = this-> _lex.tell ();
	TRY {
	    auto next = this-> _lex.next ();
	    if (next == Keys::MOD) {
		space = visitNamespace ();
		this-> _lex.next ({Token::SEMI_COLON});
	    } else this-> _lex.rewind ();
	} CATCH (ErrorCode::EXTERNAL) {
	    CLEAR_ERRORS ();
	    space = lexing::Word::eof ();
	    this-> _lex.seek (beginPos);
	} FINALLY;
	
	do {	    
	    token = this-> _lex.next ();
	    if (token == Keys::PUBLIC || token == Keys::PRIVATE) {
		decls.push_back (visitProtectionBlock (token == Keys::PRIVATE));
	    } else if (token == Keys::VERSION) {
		decls.push_back (visitVersionGlob (false));
	    } else if (!token.isEof ()) {
		this-> _lex.rewind ();
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
	auto token = this-> _lex.next ();
	bool end = false; //if the block is not surrounded with {}, we get only one declaration
	
	if (token != Token::LACC) {
	    end = true;
	    this-> _lex.rewind ();
	}
	
	do {
	    token = this-> _lex.next ();
	    if (token == Token::RACC && !end) {
		end = true;
	    } else {
		this-> _lex.rewind ();
		decls.push_back (visitDeclaration ());
	    }
	} while (!end);

	return DeclBlock::init (location, decls, isPrivate);	
    }

    Declaration Visitor::visitVersionGlob (bool global) {
	if (!global) return visitVersionGlobBlock ();
	
	auto location = this-> _lex.rewind ().next ();
	auto token = this-> _lex.next ();
	if (token == Token::EQUAL) {
	    auto type = visitIdentifier ();
	    this-> _lex.next ({Token::SEMI_COLON});
	    Error::halt ("%(r) - need version", "TODO");
	    return Declaration::empty ();
	} else {
	    this-> _lex.rewind ();
	    return visitVersionGlobBlock ();
	}	
    }

    Declaration Visitor::visitVersionGlobBlock () {
	auto location = this-> _lex.rewind ().next ();
	auto ident = visitIdentifier ();

	Error::halt ("%(r) - need version", "TODO");
	return Declaration::empty ();
    }


    Declaration Visitor::visitExtern () {
	auto location = this-> _lex.rewind ().next ();
	auto token = this-> _lex.next ();

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
	auto declarations = {Keys::ALIAS, Keys::TYPE, Keys::ENUM, Keys::DEF, Keys::STATIC, Keys::IMPORT, Keys::MACRO, Keys::MOD, Keys::STRUCT, Keys::TRAIT, Keys::USE};
	auto location = this-> _lex.next (declarations);	

	if (location == Keys::ALIAS) return visitAlias ();
	if (location == Keys::TYPE) return visitClass ();
	// if (location == Keys::ENUM) return visitEnum ();
	if (location == Keys::DEF) return visitFunction ();
	// if (location == Keys::STATIC) return visitGlobal ();
	// if (location == Keys::IMPORT) return visitImport ();
	// if (location == Keys::MACRO) return visitMacro ();
	// if (location == Keys::MOD) return visitLocalMod ();
	// if (location == Keys::STRUCT) return visitStruct ();
	// if (location == Keys::TRAIT) return visitTrait ();
	// if (location == Keys::USE) return visitUse ();
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
	if (!templates.empty ()) {
	    return Template::init (templates, Alias::init (name, value));
	} else 
	    return Alias::init (name, value);
    }
    
    Declaration Visitor::visitClass () {
	auto location = this-> _lex.rewind ().next ();
	auto name = this-> _lex.next ();
	auto templates = visitTemplateParameters ();
	auto token = this-> _lex.next ();
	Expression ancestor (Expression::empty ());
	if (token == Keys::OVER) 
	    ancestor = visitExpression ();

	this-> _lex.next ({Token::LACC});
	std::vector <Declaration> decls;
	do {
	    token = this-> _lex.next ();
	    if (token == Keys::PRIVATE || token == Keys::PROTECTED) {
		decls.push_back (visitProtectionClassBlock (token == Keys::PRIVATE));
	    } else if (token == Keys::VERSION) {
		decls.push_back (visitVersionClass ());
	    } else if (token != Token::RACC) {
		this-> _lex.rewind ();
		decls.push_back (visitClassContent ());
	    }
	} while (token != Token::RACC);

	if (!templates.empty ()) {
	    return Template::init (templates, Class::init (name, ancestor, decls));
	} else
	    return Class::init (name, ancestor, decls);
    }

    Declaration Visitor::visitProtectionClassBlock (bool isPrivate) {
	auto location = this-> _lex.rewind ().next ();
	std::vector <Declaration> decls;
	auto token = this-> _lex.next ();
	bool end = false;
	if (token != Token::LACC) {
	    this-> _lex.rewind ();
	    end = true;
	}

	do {
	    token = this-> _lex.next ();
	    if (token == Token::RACC && !end) end = true;
	    else {
		this-> _lex.rewind ();
		decls.push_back (visitDeclaration ());
	    }
	} while (!end);
	
	return DeclBlock::init (location, decls, isPrivate);
    }

    Declaration Visitor::visitVersionClass () {
	auto location = this-> _lex.rewind ().next ();
	auto ident = visitIdentifier ();

	Error::halt ("%(r) - need version", "TODO");
	return Declaration::empty ();
    }
    
    Declaration Visitor::visitClassContent () {
	auto token = this-> _lex.next ({Keys::DEF, Token::TILDE, Keys::LET, Keys::SELF});
	if (token == Keys::SELF) {
	    return visitClassConstructor ();
	} else if (token == Keys::DEF) {
	    return visitFunction ();
	} else if (token == Token::TILDE) {
	    this-> _lex.next ({Keys::SELF});
	    return visitClassDestructor ();
	} else if (token == Keys::LET) {
	    return Expression::toDeclaration (visitVarDeclaration ());
	} else {
	    Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Declaration::empty ();	
	}
    }

    Declaration Visitor::visitClassConstructor () {
	auto location = this-> _lex.rewind ().next ();
	auto before_template = this-> _lex.tell ();
	auto templates = visitTemplateParameters ();
	auto token = this-> _lex.next ();
	if (token != Token::LPAR) {
	    templates.clear ();
	    this-> _lex.seek (before_template);
	} else this-> _lex.rewind ();

	if (templates.size () != 0) {
	    return Template::init (templates, Function::init (location, visitFunctionPrototype (), visitFunctionBody ()));
	} else 
	    return Function::init (location, visitFunctionPrototype (), visitFunctionBody ());
    }
    
    Declaration Visitor::visitClassDestructor () {
	auto location = this-> _lex.rewind (2).next () + this-> _lex.next (); // Get ~self
	this-> _lex.next ({Token::LPAR});
	this-> _lex.next ({Token::RPAR});
	
	return Function::init (location, Function::Prototype::init ({}, Expression::empty ()), visitFunctionBody ());	
    }

    Declaration Visitor::visitFunction () {       
	auto location = this-> _lex.rewind ().next ();

	Expression test (Expression::empty ());
	
	auto token = this-> _lex.next ();
	if (token == Keys::IF) test = visitExpression ();
	else this-> _lex.rewind ();

	auto attribs = visitAttributes ();
	auto name = visitIdentifier ();

	auto befTemplates = this-> _lex.tell ();
	auto templates = visitTemplateParameters ();

	token = this-> _lex.next ();
	if (token != Token::LPAR) {
	    templates.clear ();
	    this-> _lex.seek (befTemplates);
	}
	else this-> _lex.rewind ();

	
	auto function = Function::init (name, visitFunctionPrototype (), visitFunctionBody ());
	function.to <Function> ().setCustomAttributes (attribs);

	if (templates.size () != 0) return Template::init (templates, function);
	else return function;
    }

    Function::Prototype Visitor::visitFunctionPrototype () {
	std::vector <Expression> vars;
	auto token = this-> _lex.next ({Token::LPAR});
	do {
	    token = this-> _lex.next ();
	    this-> _lex.rewind ();
	    if (token != Token::RPAR) {
		vars.push_back (visitSingleVarDeclaration ());
	    }
	    
	    this-> _lex.next ({Token::RPAR, Token::COMA});	    
	} while (token != Token::RPAR);

	token = this-> _lex.next ();
	if (token == Token::ARROW) {
	    return Function::Prototype::init (vars, visitExpression ());
	} else this-> _lex.rewind ();
	return Function::Prototype::init (vars, Expression::empty ());
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
    
    std::vector <Expression> Visitor::visitTemplateParameters () {
	std::vector <Expression> list;
	auto begin = this-> _lex.tell ();
	auto token = this-> _lex.next ();
	if (token == Token::LPAR) {
	    TRY {
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
			    list.push_back (visitExpression ());
			}
		    } else list.push_back (visitExpression ());
		    
		    token = this-> _lex.next ({Token::RPAR, Token::COMA});
		} while (token != Token::RPAR);
		return list;
	    } CATCH (ErrorCode::EXTERNAL) {		
		this-> _lex.seek (begin);
		return {};	
	    } FINALLY;
	} else this-> _lex.rewind ();
	return {};
    }
       
    
    Expression Visitor::visitExpression () {
	Error::halt ("%(r) - reaching unhandled point", "Critical");
	return Expression::empty ();
    }    

    Expression Visitor::visitVarDeclaration () {
	auto location = this-> _lex.rewind ().next ();
	lexing::Word token;
	std::vector <Expression> decls;
	do {
	    decls.push_back (visitSingleVarDeclaration ());
	    token = this-> _lex.next ();
	} while (token == Token::COMA);

	this-> _lex.rewind (); // The last token read wasn't a coma
	if (decls.size () == 1) return decls [0];
	else return Set::init (decls);
    }    

    Expression Visitor::visitSingleVarDeclaration () {
	std::vector<Decorator> decos;
	Expression type (Expression::empty ()), value (Expression::empty ());
	
	lexing::Word token = this-> _lex.next ();
	while (token.is (Decorators::members ())) {
	    decos.push_back (Decorators::init (token));
	    token = this-> _lex.next ();	    
	}

	auto name = visitIdentifier ();
	token = this-> _lex.next ();
	if (token == Token::COLON) {
	    type = visitExpression ();
	    this-> _lex.next ();
	}

	if (token == Token::EQUAL)
	    value = visitExpression ();
	else this-> _lex.rewind ();
	
	return VarDecl::init (name, decos, type, value);
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

	if (std::find (this-> _forbiddenKeys.begin (), this-> _forbiddenKeys.end (), token.str) !=
	    this-> _forbiddenKeys.end ())
	    return false;
	
	int i = 0;
	for (auto it : token.str) {
	    if ((it >= 'a' && it <= 'z') || (it >= 'A' && it <= 'Z')) break;
	    else if (it != '_')
		return false;
	    i ++;
	}
	
	i ++;
	for (auto it : token.str.substr (i)) {
	    if ((it < 'a' || it > 'z')
		&& (it < 'A' || it > 'Z')
		&& (it != '_')
		&& (it < '0' || it > '9')) {
		return false;
	    }
	}
	
	return true;
    }

    
    
}
