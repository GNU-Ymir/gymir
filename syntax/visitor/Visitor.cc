#include <ymir/syntax/visitor/Visitor.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/lexing/Token.hh>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>
#include <ymir/syntax/declaration/_.hh>
#include <ymir/syntax/expression/_.hh>

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
	    {Token::DPIPE},
	    {Token::DAND},
	    {Token::INF, Token::SUP, Token::INF_EQUAL,
	     Token::SUP_EQUAL, Token::NOT_EQUAL,
	     Token::NOT_INF, Token::NOT_INF_EQUAL, Token::NOT_SUP,
	     Token::NOT_SUP_EQUAL, Token::DEQUAL, Keys::OF, Keys::IS, Keys::IN},
	    {Token::TDOT, Token::DDOT},
	    {Token::LEFTD, Token::RIGHTD},
	    {Token::PIPE, Token::XOR, Token::AND},
	    {Token::PLUS, Token::TILDE, Token::MINUS},
	    {Token::STAR, Token::PERCENT, Token::DIV},
	    {Token::DXOR},
	    {Token::DOT},
	    {Token::DCOLON}
	};

	visit._specialOperators = {
	    {}, 
	    {},
	    {Keys::OF, Keys::IS, Keys::IN},
	    {},
	    {},
	    {},
	    {},
	    {}, 
	    {},
	    {},
	    {}
	};
	
	visit._operand_op = {
	    Token::MINUS, Token::AND, Token::STAR, Token::NOT
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
	else this-> _lex.rewind ();

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

	auto proto = visitFunctionPrototype ();
	auto function = Function::init (name, proto, visitFunctionBody ());
	function.to <Function> ().setCustomAttributes (attribs);

	if (templates.size () != 0) return Template::init (templates, function);
	else return function;
    }

    Function::Prototype Visitor::visitFunctionPrototype () {
	std::vector <Expression> vars;
	auto token = this-> _lex.next ();
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
	    } CATCH (ErrorCode::EXTERNAL) {
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
	auto location = this-> _lex.next ();
	if (location.is (this-> _operand_op)) {
	    return Unary::init (location, visitOperand1 ());
	}
	
	this-> _lex.rewind ();
	return visitOperand1 ();    
    }

    Expression Visitor::visitOperand1 () {
	auto value = visitOperand2 ();
	auto location = this-> _lex.next ();
	if (location == Token::LPAR || location == Token::LCRO) {
	    auto params = visitParamList ();
	    lexing::Word end;
	    if (location == Token::LPAR) end = this-> _lex.next ({Token::RPAR});
	    else end = this-> _lex.next ({Token::RCRO});
	    return MultOperator::init (location, end, value, params);
	} else this-> _lex.rewind ();
	return value;
    }

    Expression Visitor::visitOperand2 () {
	auto begin = this-> _lex.next ();
	this-> _lex.rewind ();
	if (begin == Token::LPAR) {
	    auto loc = this-> _lex.tell ();
	    Expression lambda (Expression::empty ());
	    TRY {
		// Could not return directly because of scope guard
		lambda = visitLambda ();
	    } CATCH (ErrorCode::EXTERNAL) {
		CLEAR_ERRORS ();
		this-> _lex.seek (loc);
		return visitTuple ();
	    } FINALLY;
	    
	    return lambda;
	}
	if (begin == Token::LCRO)   return visitArray ();
	if (begin == Token::LACC)   return visitBlock ();
	if (begin == Keys::IF)      return visitIf ();
	if (begin == Keys::WHILE)   return visitWhile ();
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
	if (canVisitIdentifier ())  return visitVar ();       
	return visitLiteral ();
    }    

    Expression Visitor::visitArray () {
	auto begin = this-> _lex.next ({Token::LCRO});
	auto end = this-> _lex.next ();
	if (end == Token::RCRO) return List::init (begin, end, {});
	else {
	    this-> _lex.rewind ();
	    std::vector <Expression> params;
	    params.push_back (visitExpression ());
	    end = this-> _lex.next ({Token::SEMI_COLON, Token::RCRO, Token::COMA});
	    if (end == Token::DCOLON) {
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
		    end = this-> _lex.next ({Token::SEMI_COLON, Token::RCRO});		   
		}

		return List::init (begin, end, params);
	    }
	}
    }

    Expression Visitor::visitBlock () {
	std::vector <Expression> content;
	auto begin = this-> _lex.next ({Token::LACC});
	lexing::Word end;
	do {
	    end = this-> _lex.next ();
	    if (end != Token::RACC && end != Token::SEMI_COLON) {
		this-> _lex.rewind ();
		content.push_back (visitExpression ());
	    }
	} while (end != Token::RACC);
	return Block::init (begin, end, content);
    }    

    Expression Visitor::visitIf () {
	auto location = this-> _lex.next ({Keys::IF});
	auto test = visitExpression ();
	auto content = visitExpression ();
	auto next = this-> _lex.next ();
	if (next == Keys::ELSE) {
	    next = this-> _lex.next ();
	    this-> _lex.rewind ();
	    if (next == Keys::IF) return If::init (location, test, content, visitIf ());
	    else return If::init (location, test, content, visitExpression ());
	}
	return If::init (location, test, content, Expression::empty ());
    }

    Expression Visitor::visitWhile () {
	auto location = this-> _lex.next ({Keys::WHILE});
	return While::init (location, visitExpression (), visitExpression ());
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
	Error::halt ("%(r) - need version", "TODO");
	return Expression::empty ();
    }

    Expression Visitor::visitReturn () {
	auto location = this-> _lex.next ();
	return Return::init (location, visitExpression ());
    }

    Expression Visitor::visitVar () {
	static bool inVar = false;
	auto name = visitIdentifier ();
	auto next = this-> _lex.next ();
	if (!inVar && next == Token::NOT) {
	    next = this-> _lex.next ();
	    if (next == Token::LPAR) {
		inVar = false;
		auto list = visitParamList ();
		this-> _lex.next ({Token::RPAR});
		return TemplateCall::init (list, Var::init (name));
	    } else {
		inVar = true; // Cannot have template parameters for inner type : A!(A!B) is Ok, not $A!A!B
		this-> _lex.rewind ();
		auto ret = TemplateCall::init ({visitExpression ()}, Var::init (name));
		inVar = false;
		return ret;
	    }
	} else if (next == Token::NOT) {
	    Error::occur (next, ExternalError::get (SYNTAX_ERROR_MISSING_TEMPL_PAR));
	}
	
	this-> _lex.rewind ();
	return Var::init (name);
    }       

    std::vector <Expression> Visitor::visitParamList () {
	std::vector <Expression> params;
	auto begin = this-> _lex.tell ();
	TRY {
	    params.push_back (visitExpression ());
	} CATCH (ErrorCode::EXTERNAL) {
	    CLEAR_ERRORS ();
	    this-> _lex.seek (begin);
	    return {};
	} FINALLY;

	auto token = this-> _lex.next ();
	while (token == Token::COMA) {
	    params.push_back (visitExpression ());
	    token = this-> _lex.next ();
	}
	
	this-> _lex.rewind ();
	return params;
    }

    Expression Visitor::visitTuple () {
	auto begin = this-> _lex.next ({Token::LPAR});
	
	std::vector <Expression> params;
	auto beg_loc = this-> _lex.tell ();
	TRY {
	    params.push_back (visitExpression ());
	} CATCH (ErrorCode::EXTERNAL) {
	    CLEAR_ERRORS ();
	    this-> _lex.seek (beg_loc);
	    auto end = this-> _lex.next ({Token::RPAR});
	    return List::init (begin,  end, {});
	} FINALLY;
	
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
	if (begin != Keys::REF) {
	    this-> _lex.rewind ();
	    begin = lexing::Word::eof ();
	}

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
	//if (tok == Token::APOS)                        return visitChar ();
	if (tok == Keys::TRUE_ || tok == Keys::FALSE_) return Bool::init (tok);
	if (tok == Keys::NULL_)                         return Null::init (tok);
	
	Error::occur (tok, ExternalError::get (SYNTAX_ERROR_AT_SIMPLE), tok.str);
	return Expression::empty ();
    }

    Expression Visitor::visitNumeric () {
	auto begin = this-> _lex.next ();	
	if (begin.str.length () >= 3) {
	    auto suffix = lexing::Word {begin, begin.str.substr (begin.str.length () - 2)};
	    suffix.column += begin.str.length () - 2;
	    
	    if (suffix.is (this-> _fixedSuffixes)) {
		auto value = begin.str.substr (0, begin.str.length () - 2);
		verifNumeric (begin, value);
		return Fixed::init ({begin, value}, suffix);
	    }
	}
	
	if (begin.str.length () >= 2) {
	    auto suffix = lexing::Word {begin, begin.str.substr (begin.str.length () - 1)};
	    suffix.column += begin.str.length () - 1;
	    
	    if (suffix.is (this-> _fixedSuffixes)) {
		auto value = begin.str.substr (0, begin.str.length () - 1);
		verifNumeric (begin, value);
		return Fixed::init ({begin, value}, suffix);
	    }
	}
	
	auto value = begin.str;
	if (!verifNumeric (begin, value)) {
	    auto next = this-> _lex.next ();
	    this-> _lex.rewind ();
	    if (next == Token::DOT) return visitFloat (begin);
	    else {
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

    Expression Visitor::visitSingleVarDeclaration (bool mandType, bool withValue) {
	std::vector<Decorator> decos;
	Expression type (Expression::empty ()), value (Expression::empty ());
	
	lexing::Word token = this-> _lex.next ();
	while (token.is (Decorators::members ())) {
	    decos.push_back (Decorators::init (token));
	    token = this-> _lex.next ();	    
	}

	auto name = visitIdentifier ();
	if (mandType) token = this-> _lex.next ({Token::COLON});
	else token = this-> _lex.next ();
	
	if (token == Token::COLON) {
	    type = visitExpression ();
	    this-> _lex.next ();
	} 

	if (token == Token::EQUAL && withValue)
	    value = visitExpression ();
	else this-> _lex.rewind ();
	
	return VarDecl::init (name, decos, type, value);
    }    

    bool Visitor::canVisitSingleVarDeclaration (bool mandType, bool withValue) {
	auto begin = this-> _lex.tell ();
	TRY {
	    visitSingleVarDeclaration (mandType, withValue);
	} CATCH (ErrorCode::EXTERNAL) {
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

    
    
}
