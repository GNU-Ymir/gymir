#include "syntax/_.hh"
#include "errors/_.hh"
#include "syntax/Keys.hh"

#include <map>
#include <vector>
#include <algorithm>

using namespace std;

namespace syntax {

    using namespace lexical;

    
    std::string join (std::vector <std::string> elems) {
	stringstream ss;
	int i = 0;
	for (auto it : elems) {
	    ss << "'" << Ymir::Error::BLUE << it << Ymir::Error::RESET << "'" << (i == (int) elems.size () - 1 ? "" : " ");
	    i++;
	}
	return ss.str ();
    }
    
    void syntaxError (Word token, std::vector <std::string> mandatories) {
	Ymir::Error::syntaxError (token,  join (mandatories).c_str ());
    }

    void unterminated (Word token) {
	Ymir::Error::unterminated (token);
    }
    
    void syntaxError (Word token) {
	Ymir::Error::syntaxError (token);
    }

    void syntaxErrorFor (Word token, Word tok2) {
	Ymir::Error::syntaxErrorFor (token, tok2);
    }

    void escapeError (Word token) {
	Ymir::Error::escapeError (token);
    }

    bool find (std::vector <std::string> vec, std::string elem) {
	return std::find (vec.begin (), vec.end (), elem) != vec.end ();
    }    

    bool find (std::vector <std::string> vec, Word elem) {
	return std::find (vec.begin (), vec.end (), elem.getStr ()) != vec.end ();
    }    

    Visitor::Visitor (Lexer & lex) : lex (lex) {
	this-> ultimeOp = {Token::DIV_AFF, Token::AND_AFF, Token::PIPE_EQUAL,
			   Token::MINUS_AFF, Token::PLUS_AFF, Token::LEFTD_AFF,
			   Token::RIGHTD_AFF, Token::EQUAL, Token::STAR_EQUAL,
			   Token::PERCENT_EQUAL, Token::XOR_EQUAL,
			   Token::DXOR_EQUAL, Token::TILDE_EQUAL};

	this-> expOp = {Token::DPIPE, Token::DAND};
		
	this-> ulowOp = {
	    Token::INF, Token::SUP, Token::INF_EQUAL,
	    Token::SUP_EQUAL, Token::NOT_EQUAL, Token::NOT_INF,
	    Token::NOT_INF_EQUAL, Token::NOT_SUP,
	    Token::NOT_SUP_EQUAL, Token::DEQUAL, Token::DDOT
	};

	this-> lowOp = {Token::PLUS, Token::PIPE, Token::LEFTD,
			Token::XOR, Token::TILDE, Token::MINUS,
			Token::RIGHTD};

	this-> highOp = {Token::DIV, Token::AND, Token::STAR, Token::PERCENT,
			 Token::DXOR};
	
	this-> suiteElem = {Token::LPAR, Token::LCRO, Token::DOT, Token::DCOLON};
	this-> afUnary = {Token::DPLUS, Token::DMINUS};	
	this-> befUnary = {Token::MINUS, Token::AND, Token::STAR, Token::NOT};
	this-> forbiddenIds = {Keys::IMPORT, Keys::STRUCT,
			       Keys::DEF, Keys::IF, Keys::RETURN,
			       Keys::FOR,  Keys::WHILE, Keys::BREAK,
			       Keys::MATCH, Keys::IN, Keys::ELSE,
			       Keys::TRUE_, Keys::FALSE_, Keys::NULL_, Keys::CAST,
			       Keys::FUNCTION, Keys::LET, Keys::IS, Keys::EXTERN,
			       Keys::PUBLIC, Keys::PRIVATE, Keys::TYPEOF, Keys::IMMUTABLE,
			       Keys::CONST, Keys::REF, Keys::TRAIT
	};

	this-> decoKeys = {Keys::IMMUTABLE, Keys::CONST, Keys::STATIC};
	this-> lambdaPossible = true;	
    }
    
    Program Visitor::visit () {
	return visitProgram ();
    }

    /**
       program := declaration | public | private
    */
    Program Visitor::visitProgram () {	
	auto token = this-> lex.next ();
	this-> lex.rewind ();
	std::vector<Declaration> decls;
	while (!token.isEof ()) {
	    auto decl = visitDeclaration (false);
	    if (decl != NULL) decls.push_back (decl);
	    else if (token == Keys::PUBLIC) {
		auto pub_decls = visitPublicBlock ();
		for (auto it : pub_decls) decls.push_back (it);
	    } else if (token == Keys::PRIVATE) {
		auto prv_decls = visitPrivateBlock ();
		for (auto it : prv_decls) decls.push_back (it);
	    } else {
		this-> lex.next ();
		syntaxError (token,
			     {Keys::DEF, Keys::IMPORT, Keys::EXTERN,
				     Keys::STRUCT, Keys::ENUM, Keys::STATIC,
				     Keys::SELF, Keys::TRAIT, Keys::IMPL,
				     Keys::PUBLIC, Keys::PRIVATE
				     }
		);
	    }
	    token = this-> lex.next ();
	    this-> lex.rewind ();
	}
	auto last = this-> lex.fileLocus ();
	return new (GC) IProgram (last, decls);
    }
    
    /**
       public := 'public' (declaration | ('{' declaration* '}'))
    */
    std::vector<Declaration> Visitor::visitPublicBlock () {
	auto begin = this-> lex.next ();
    	auto next = this-> lex.next ();
	std::vector <Declaration> decls;
    	if (next == Token::LACC) {
    	    while (true) {
    		auto decl = visitDeclaration (false);
    		if (decl) {
    		    decls.push_back (decl);
    		    decls.back ()-> is_public (true);
    		} else {
    		    auto tok = this-> lex.next ();
    		    if (tok != Token::RACC)
			syntaxError (tok,
				     {Keys::DEF, Keys::IMPORT,
					     Keys::EXTERN,
					     Keys::STRUCT, Keys::ENUM,
					     Keys::STATIC, Keys::SELF,
					     Keys::TRAIT, Keys::IMPL }
			);
    		    break;
    		}
    	    }
    	} else {
	    this-> lex.rewind ();
    	    decls.push_back (visitDeclaration (true));
    	    decls.back ()-> is_public (true);
    	}
    	return decls;
    }

    /**
       private := 'private' (declaration | ('{' declaration* '}'))
    */
    std::vector<Declaration> Visitor::visitPrivateBlock () {
	auto begin = this-> lex.next ();
    	auto next = this-> lex.next ();
	std::vector <Declaration> decls;
    	if (next == Token::LACC) {
    	    while (true) {
    		auto decl = visitDeclaration (false);
    		if (decl) {
    		    decls.push_back (decl);
    		    decls.back ()-> is_public (false);
    		} else {
    		    auto tok = this-> lex.next ();
    		    if (tok != Token::RACC)
			syntaxError (tok, {Keys::DEF, Keys::IMPORT,
				    Keys::EXTERN, Keys::STRUCT,
				    Keys::ENUM,   Keys::STATIC,
				    Keys::SELF, Keys::TRAIT, Keys::IMPL }
			);			
    		    break;
    		}
    	    }
    	} else {
	    this-> lex.rewind ();
    	    decls.push_back (visitDeclaration (true));
    	    decls.back ()-> is_public (false);
    	}
    	return decls;
    }

    /**
       declaration :=   function 
       | import
       | extern
       | struct
       | enum
       | global 
       | self
       #| trait
       | impl
       
    */
    Declaration Visitor::visitDeclaration (bool fatal) {
    	auto token = this-> lex.next ();
    	if (token == Keys::DEF) return visitFunction ();
    	else if (token == Keys::IMPORT) return visitImport ();
    	else if (token == Keys::EXTERN) return visitExtern ();
    	else if (token == Keys::STRUCT) return visitStruct ();
    	else if (token == Keys::ENUM) return visitEnum ();
    	else if (token == Keys::STATIC) return visitGlobal ();
    	else if (token == Keys::SELF) return visitSelf ();
    	else if (token == Keys::TRAIT) return NULL;// visitTrait ();
    	else if (token == Keys::IMPL) return visitImpl ();	      	
    	else if (fatal) syntaxError (token,
				     {Keys::DEF, Keys::IMPORT, Keys::EXTERN,
					     Keys::STRUCT, Keys::ENUM, Keys::STATIC,
					     Keys::SELF, Keys::TRAIT, Keys::IMPL}
	);
    	else this-> lex.rewind ();
    	return NULL;
    }

    /**
       impl := 'impl' identifiant ('from identifiant)? '{' (functionImpl | constructor)* '}'       
    */
    Impl Visitor::visitImpl () {
	auto ident = visitIdentifiant ();
	auto what = Word::eof ();
	auto next = this-> lex.next ({Keys::FROM, Token::LACC});

	if (next == Keys::FROM) {
	    what = visitIdentifiant ();
	    this-> lex.next ({Token::LACC});
	}

	next = this-> lex.next ({Token::RACC, Keys::DEF, Keys::OVER});
	std::vector <Function> methods; std::vector <bool> herit;
	std::vector <Constructor> csts;
	bool isOver = (next == Keys::OVER);

	if (isOver && what.isEof ()) syntaxError (next, {Token::RACC, Keys::DEF});
	if (next != Token::RACC) {
	    while (true) {
		bool isCst = false;
		auto meth = visitFunctionImpl (isCst);
		if (!isCst) {
		    methods.push_back ((Function) meth);
		    herit.push_back (isOver);
		} else csts.push_back ((Constructor) (meth));
		next = this-> lex.next ({Token::RACC, Keys::DEF, Keys::OVER});
		if (next == Token::RACC) break;
		else if (next == Keys::OVER) {
		    if (what.isEof ()) syntaxError (next, {Token::RACC, Keys::DEF});
		    isOver = true;
		} else isOver = false;
	    }
	}
	
	if (what.isEof ())
	    return new (GC) IImpl (ident, methods, csts);
	else return new (GC) IImpl (ident, what, methods, herit, csts);	
    }

    /**
       constructor := 'def' '(' 'self' (',' varDeclaration)* ')' block
    */
    Constructor Visitor::visitConstructor () {
	auto begin = this-> lex.next ({Keys::SELF});
	std::vector <Var> params;

	while (true) {
	    auto next = this-> lex.next ({Token::COMA, Token::RPAR});
	    if (next == Token::RPAR) break;
	    else {
		params.push_back (visitVarDeclaration ());		
	    }
	}
	return new (GC) IConstructor (begin, params, visitBlock ());
    }

    /**
       functionImpl := ('over'|'def') identifiant '(' ('self')? (',' varDeclaration)* ')' block
    */    
    Declaration Visitor::visitFunctionImpl (bool &isCst) {
	auto begin = this-> lex.rewind ().next ();
	auto next = this-> lex.next ();
	if (next == Token::LPAR && begin == Keys::DEF) {
	    isCst = true;
	    return visitConstructor ();
	} else this-> lex.rewind ();
	
	isCst = false;
	auto ident = visitIdentifiant ();
	std::vector <Var> params;
	this-> lex.next ({Token::LPAR});
	next = this-> lex.next ();
	if (next != Token::RPAR) {
	    this-> lex.rewind ();
	    while (true) {
		if (params.size () == 0) {
		    next = this-> lex.next ();
		    if (next == Keys::SELF)
			params.push_back (new (GC) IVar (next));
		    else {
			this-> lex.rewind ();
			params.push_back (visitVarDeclaration ());
		    }
		} else
		    params.push_back (visitVarDeclaration ());
		next = this-> lex.next ({Token::RPAR, Token::COMA});
		if (next == Token::RPAR) break;		
	    } 
	}

	Var type;
	next = this-> lex.next ();
	if (next == Token::COLON) {
	    auto deco = this-> lex.next ();
	    if (deco != Keys::REF) {
		deco = Word::eof ();
		this-> lex.rewind ();
	    }
	    type = visitType ();
	    type-> deco = deco;
	} else this-> lex.rewind ();
	return new (GC) IFunction (ident, params, {}, NULL, visitBlock ());
    }


    /**
       self := 'self' '(' ')' block
    */
    Self Visitor::visitSelf () {
	auto begin = this->lex.rewind ().next ();
	this-> lex.next ({Token::LPAR});
	this-> lex.next ({Token::RPAR});
	return new (GC) ISelf (begin, visitBlock ());
    }

    /**
       global := 'static' identifiant (('=' expression) | (':' type)) ';'
    */
    Global Visitor::visitGlobal () {
	auto begin = this-> lex.rewind ().next ();
	auto ident = visitIdentifiant ();
	auto next = this-> lex.next ({Token::EQUAL, Token::COLON});
	if (next == Token::EQUAL) {
	    auto expr = visitExpression ();
	    this-> lex.next ({Token::SEMI_COLON});
	    return new (GC) IGlobal (ident, expr);
	} else {
	    Expression type;
	    next = this-> lex.next ();
	    if (next == Keys::FUNCTION) type = visitFuncPtrSimple ();
	    else {
		this-> lex.rewind ();
		type = visitType ();
	    }
	    this-> lex.next ({Token::SEMI_COLON});
	    return new (GC) IGlobal (ident, NULL, type);
	}
    }
    

    /**
       space := identifiant ('.' identifiant)*
    */
    std::string Visitor::visitSpace () {
	stringstream buf;
	while (true) {
	    auto next = visitIdentifiant ();
	    buf << next.getStr ();
	    auto nt = this-> lex.next ();
	    if (nt == Token::DOT) buf << ".";
	    else break;
	}
	this-> lex.rewind ();
	return buf.str ();
    }

    Import Visitor::visitImport () {
	auto begin = this-> lex.rewind ().next ();
	std::vector <Word> idents;
	bool end = true;
	while (true) {
	    auto name = this-> lex.next ();
	    if (!end && name == Keys::UNDER) {
		idents.back ().setStr (idents.back ().getStr () + name.getStr ());
		auto word = this-> lex.next ({Token::COMA, Token::SEMI_COLON});
		if (word == Token::SEMI_COLON) break;
		else end = true;
	    } else {
		this-> lex.rewind ();
		auto ident = visitIdentifiant ();
		if (end) {
		    end = false;
		    idents.push_back (ident);
		} else idents.back ().setStr (idents.back ().getStr () + ident.getStr ());
		auto word = this-> lex.next ({Token::COMA, Token::DOT, Token::SEMI_COLON});
		if (word == Token::COMA) end = true;
		else if (word == Token::DOT) idents.back ().setStr (idents.back ().getStr () + "/");
		else break;
	    }
	}
	return new (GC) IImport (begin, idents);
    }

    std::vector <Expression> Visitor::visitTemplateStruct () {
	std::vector<Expression> expr;
	auto next = this-> lex.next ();
	if (next != Token::RPAR) {
	    this-> lex.rewind ();
	    while (next != Token::RPAR) {
		expr.push_back (visitOf ());
		next = this-> lex.next ({Token::RPAR, Token::COMA});
	    }
	}
	return expr;
    }

    Struct Visitor::visitStruct () {
	Word word = this-> lex.next (), ident;
	std::vector <Var> exps;
	std::vector <Expression> temps;

	if (word == Token::LPAR) {
	    temps = visitTemplateStruct ();
	    word = this-> lex.next ({Token::PIPE});
	}
	
	if (word == Token::PIPE) {
	    while (true) {
		exps.push_back (visitStructVarDeclaration ());
		word = this-> lex.next ({Token::ARROW, Token::PIPE});
		if (word == Token::ARROW) break;
	    }
	    ident = visitIdentifiant ();
	    word = this-> lex.next ({Token::SEMI_COLON});
 	} else if (word != Token::ARROW) {
	    this-> lex.rewind ();
	    ident = visitIdentifiant ();
	    auto next = this-> lex.next ({Token::LPAR, Token::LACC});
	    if (next == Token::LPAR) {
		temps = visitTemplateStruct ();
		this-> lex.next ({Token::LACC});
	    }
	    next = this-> lex.next ();
	    if (next != Token::RACC) {
		this-> lex.rewind ();
		while (true) {
		    exps.push_back (visitStructVarDeclaration ());
		    word = this-> lex.next ({Token::COMA, Token::RACC});
		    if (word == Token::RACC) break;
		}
	    }
	} else {
	    visitIdentifiant ();
	    this-> lex.next ({Token::COMA});    
	}
	return new (GC) IStruct (ident, temps, exps);
    }

    Enum Visitor::visitEnum () {
	std::vector<Word> names;
	std::vector<Expression> values;
	auto word = this-> lex.next ();
	Word ident;
	Var type = NULL;
	if (word == Token::COLON) type = visitType ();
	else this-> lex.rewind ();
	word = this-> lex.next ();
	if (word != Token::PIPE)  {
	    this-> lex.rewind ();
	    ident = visitIdentifiant ();
	    auto next = this-> lex.next ({Token::COLON, Token::LACC});
	    if (next == Token::COLON) type = visitType ();
	    while (true) {
		names.push_back (visitIdentifiant ());
		next = this-> lex.next ({Token::COLON});
		values.push_back (visitPth ());
		next = this-> lex.next ({Token::RACC, Token::COMA});
		if (next == Token::RACC || next.isEof ()) break;
	    }
	} else {
	    while (true) {
		names.push_back (visitIdentifiant ());
		auto next = this-> lex.next ({Token::COLON});
		values.push_back (visitPth ());
		next = this-> lex.next ({Token::ARROW, Token::PIPE});
		if (next == Token::ARROW || next.isEof ()) break;
	    }
	
	    ident = visitIdentifiant ();       	    
	    word = this-> lex.next ({Token::SEMI_COLON});
	}
	return new (GC) IEnum (ident, type, names, values);
    }

    
    Expression Visitor::visitIfFunction () {
	auto next = this-> lex.next ();
	bool lpar = false;
	if (next == Token::LPAR) lpar = true;
	else this-> lex.rewind ();
	auto expr = visitExpression ();
	if (lpar) {
	    next = this-> lex.next( );
	    if (next != Token::RPAR) syntaxError (next, {Token::RPAR});
	}
	return expr;
    }

    /**
       function := 'def' Identifiant ('(' var (',') var)* ')' )? '(' (var (',' var)*)? ')' (':' type)? '{' block '}'
    */
    Function Visitor::visitFunction () {
	auto ident = visitIdentifiant ();
	bool templates = false;
	std::vector<Var> exps;
	std::vector <Expression> temps;
	
	auto word = this-> lex.next (), _ifToken = word;
	Expression test = NULL;
	if (word == Keys::IF) { test = visitIfFunction (); word = this-> lex.next (); }
	if (word != Token::LPAR) syntaxError (word, {Token::LPAR});
	this-> lex.next (word);
	if (word != Token::RPAR) {
	    this-> lex.rewind ();
	    while (true) {
		auto constante = visitConstante ();
		if (constante == NULL) 
		    temps.push_back (visitOf ());
		else {
		    templates = true;
		    temps.push_back (constante);
		}
		this-> lex.next (word);
		if (word == Token::RPAR) break;
		else if (word != Token::COMA) {
		    syntaxError (word, {Token::RPAR, Token::COMA});
		    break;
		}
	    }
	}

	this-> lex.next (word);
	if (word == Token::LPAR) {
	    this-> lex.next (word);
	    if (word != Token::RPAR) {
		this-> lex.rewind ();
		while (true) {
		    exps.push_back (visitVarDeclaration ());
		    this-> lex.next (word);
		    if (word == Token::RPAR) break;
		    else if (word != Token::COMA) {
			syntaxError (word, {Token::RPAR, Token::COMA});
			break;
		    }
		}
	    }
	    this-> lex.next (word);
	} else if (!templates) {
	    if (test) syntaxError (_ifToken, {Token::RPAR});
	    for (auto it : temps) exps.push_back ((Var) it);
	    temps.clear ();
	} else syntaxError (word, {Token::RPAR});

	if (word == Token::COLON) {
	    auto deco = this-> lex.next ();
	    if (deco != Keys::REF) {
		deco = Word::eof ();
		this-> lex.rewind ();
	    }
	    auto type = visitType ();
	    type-> deco = deco;
	    return new (GC) IFunction (ident, type, exps, temps, test, visitBlock ());
	} else this-> lex.rewind ();	
	return new (GC) IFunction (ident, exps, temps, test, visitBlock ());
    }

    Proto Visitor::visitExtern () {
	auto word = this-> lex.next ();
	bool isVariadic = false;
	Word from = Word::eof ();
	std::string space = "";
	if (word == Token::LPAR) {
	    from = visitIdentifiant ();
	    word = this-> lex.next ({Token::COMA, Token::RPAR});
	    if (word == Token::COMA) {
		if (from != Keys::DLANG) syntaxError (word, {Token::RPAR});
		space = visitSpace ();
		this-> lex.next ({Token::RPAR});
	    }
	} else this-> lex.rewind ();
	auto ident = visitIdentifiant ();
	std::vector <Var> exps;

	word = this-> lex.next ();
	if (word != Token::LPAR) syntaxError (word, {Token::LPAR});
	this-> lex.next (word);
	if (word != Token::RPAR) {
	    this-> lex.rewind ();
	    while (true) {
		word = this-> lex.next ();
		if (word == Token::TDOT) {
		    isVariadic = true;
		    word = this-> lex.next ();
		    if (word != Token::RPAR) syntaxError (word, {Token::RPAR});
		    break;
		} else this-> lex.rewind ();
		exps.push_back (visitVarDeclaration ());
		this-> lex.next (word);
		if (word == Token::RPAR) break;
		else if (word != Token::COMA)
		    syntaxError (word, {Token::RPAR, Token::COMA});
	    }
	}
	word = this-> lex.next ({Token::COLON, Token::SEMI_COLON});
	Var type = NULL;
	if (word == Token::COLON) {
	    type = visitType ();
	    word = this-> lex.next ({Token::SEMI_COLON});
	}
	
	auto ret = new (GC) IProto (ident, type, exps, space, isVariadic);
	ret-> from = from.getStr ();
	return ret;
    }

    /**
       var := type; 
    */
    Var Visitor::visitVar () {
	return visitType ();
    }

    Expression Visitor::visitOf () {
	auto deco = this-> lex.next ();
	if (deco != Keys::CONST && deco != Keys::REF) {
	    this-> lex.rewind ();
	    deco = Word::eof ();
	}
	
	auto ident = visitIdentifiant ();
	Word next = this-> lex.next ();
	if (next == Token::COLON) {
	    next = this-> lex.next ();
	    if (next == Keys::FUNCTION) {
		auto type = visitFuncPtrSimple ();
		return new (GC) ITypedVar (ident, type, deco);
	    } else {
		this-> lex.rewind ();
		auto type = visitType ();
		return new (GC) ITypedVar (ident, type, deco);
	    }
	} else if (next == Keys::OF && deco.isEof ()) {
	    auto type = visitType ();
	    return new (GC) IOfVar (ident, type);	    
	} else if (next == Token::TDOT) {
	    return new IVariadicVar (ident);
	} else this-> lex.rewind ();
	return new (GC) IVar (ident, deco);
    }
        
    /**
       vardecl := var (':' type)?
    */
    Var Visitor::visitVarDeclaration () {
	auto deco = this->lex.next ();
	if (deco != Keys::CONST && deco != Keys::REF) {
	    this-> lex.rewind ();
	    deco = Word::eof ();
	}
	auto ident = visitIdentifiant ();
	Word next = this-> lex.next ();
	if (next == Token::COLON) {
	    next = this-> lex.next ();
	    if (next == Keys::FUNCTION) {
		auto type = visitFuncPtrSimple ();
		return new (GC) ITypedVar (ident, type, deco);
	    } else {
		this-> lex.rewind ();
		auto type = visitType ();
		return new (GC) ITypedVar (ident, type, deco);
	    }
	} else this-> lex.rewind ();
	return new (GC) IVar (ident, deco);
    }
    
    TypedVar Visitor::visitStructVarDeclaration () {
	auto ident = visitIdentifiant ();
	Word next  = this-> lex.next ({Token::COLON});
	next = this-> lex.next ();
	if (next == Keys::FUNCTION) {
	    auto type = visitFuncPtrSimple ();
	    return new (GC) ITypedVar (ident, type, Word::eof ());
	} else if (next == Token::LCRO) {
	    auto begin = next;
	    next = this-> lex.next ();
	    Expression type;
	    if (next == Keys::FUNCTION) type = visitFuncPtrSimple ();
	    else {
		this-> lex.rewind ();
		type = visitType ();
	    }
	    next = this-> lex.next ({Token::RCRO, Token::SEMI_COLON});
	    if (next == Token::SEMI_COLON) {
		auto len = visitNumeric (this-> lex.next ());
		this-> lex.next ({Token::RCRO});
		return new (GC) ITypedVar (ident, new (GC) IArrayAlloc (begin, type, len), Word::eof ());
	    } else return new (GC) ITypedVar (ident, new (GC) IArrayVar (begin, type), Word::eof ());
	} else {
	    this-> lex.rewind ();
	    auto type = visitType ();
	    return new (GC) ITypedVar (ident, type, Word::eof ());
	}
    }

    
    TypedVar Visitor::visitTypedVarDeclaration () {
	auto deco = this-> lex.next ();
	if (deco != Keys::CONST && deco != Keys::REF) {
	    this-> lex.rewind ();
	    deco = Word::eof ();
	}

	auto ident = visitIdentifiant ();
	Word next = this-> lex.next ();
	if (next == Token::COLON) {
	    next = this-> lex.next ();
	    if (next == Keys::FUNCTION) {
		auto type = visitFuncPtrSimple ();
		return new (GC) ITypedVar (ident, type, deco);
	    } else {
		this-> lex.rewind ();
		auto type = visitType ();
		return new (GC) ITypedVar (ident, type, deco);
	    }
	}
	syntaxError (next, {Token::COLON});
	return NULL;
    }


    /**
       vardecl := var (':' type)?
    */
    bool Visitor::canVisitVarDeclaration () {
	auto nb = this-> lex.tell ();
	auto ret = false;
	auto deco = this-> lex.next ();
	if (deco != Keys::CONST && deco != Keys::REF) {
	    this-> lex.rewind ();
	    deco = Word::eof ();
	}
	
	if (canVisitIdentifiant ()) {
	    auto ident = visitIdentifiant ();
	    Word next = this-> lex.next ();
	    if (next == Token::COLON) {
		ret = true;
	    }
	}
	this-> lex.seek (nb);
	return ret;
    }

    /**
       type := Identifiant ('!' (('(' expression (',' expression)* ')') | expression ) 
    */
    Var Visitor::visitType () {
	auto begin = this-> lex.next ();
	if (begin == Token::LCRO) {
	    auto next = this-> lex.next ();
	    Expression type;
	    if (next == Keys::FUNCTION) 
		type = visitFuncPtrSimple ();
	    else { 
		this-> lex.rewind ();
		type = visitType ();
	    }
	    
	    auto end = this-> lex.next ();
	    if (end != Token::RCRO) syntaxError (end, {Token::RCRO});
	    return new (GC) IArrayVar (begin, type);
	} else this-> lex.rewind ();
	auto ident = visitIdentifiant ();
	auto next = this-> lex.next ();
	if (next == Token::NOT) {
	    std::vector <Expression> params;
	    next = this-> lex.next ();
	    if (next == Token::LPAR) {
		while (true) {
		    params.push_back (visitExpression ());
		    next = this-> lex.next ();
		    if (next == Token::RPAR) break;
		    else if (next != Token::COMA) {
			syntaxError (next, {Token::RPAR, Token::COMA});
			break;
		    }
		}
	    } else if (next != Keys::IS) {
		this-> lex.rewind ();
		auto constante = visitConstante ();
		if (constante != NULL) 
		    params.push_back (constante);
		else {
		    auto ident_ = visitIdentifiant ();
		    params.push_back (new (GC) IVar (ident_));
		}
	    } else this-> lex.rewind (2);
	    return new (GC) IVar (ident, params);
	} else this-> lex.rewind ();
	return new (GC) IVar (ident);
    }

        
    /**
       Identifiant := ('_')* ([a-z]|[A-Z]) ([a-z]|[A-Z]|'_')|[0-9])*
    */
    Word Visitor::visitIdentifiant () {
	auto ident = this-> lex.next ();
	/*if (ident.isEof () && this-> lex.isMixinContext ())
	  return Word.eof ();*/
	
	if (ident.isToken ())
	    syntaxError (ident, {"'Identifiant'"});
	
	if (std::find (this-> forbiddenIds.begin (), this-> forbiddenIds.end (), ident.getStr ()) != this-> forbiddenIds.end ())
	    syntaxError (ident, {"'Identifiant'"});
	
	if (ident.getStr ().length () == 0) syntaxError (ident, {"'Identifiant'"});
	
	auto i = 0;
	for (auto it : ident.getStr ()) {
	    if ((it >= 'a' && it <= 'z') || (it >= 'A' && it <= 'Z')) break;
	    else if (it != '_')  {
		syntaxError (ident, {"'identifiant'"});
		break;
	    }
	    i++;
	}
	i++;
	if (((int) ident.getStr ().length ()) < i)
	    syntaxError (ident, {"'Identifiant'"});
	
	for (auto it : ident.getStr ().substr (i, ident.getStr ().length ())) {
	    if ((it < 'a' || it > 'z')
		&& (it < 'A' || it > 'Z')
		&& (it != '_')
		&& (it < '0' || it > '9')) {
		syntaxError (ident, {"'Identifiant'"});
		break;
	    }
	}
	
	return ident;
    }
   
    bool Visitor::canVisitIdentifiant () {	
	auto ident = this-> lex.next ();
	this-> lex.rewind ();
	if (ident.isToken ())
	    return false;

	if (std::find (this-> forbiddenIds.begin (), this-> forbiddenIds.end (), ident.getStr ()) != this-> forbiddenIds.end ())
	    return false;
	
	if (ident.getStr ().length () == 0) return false;
	
	auto i = 0;
	for (auto it : ident.getStr ()) {
	    if ((it >= 'a' && it <= 'z') || (it >= 'A' && it <= 'Z')) break;
	    else if (it != '_')  {
		return false;
	    }
	    i++;
	}
	i++;
	if (((int) ident.getStr ().length ()) < i) return false;
	
	for (auto it : ident.getStr ().substr (i, ident.getStr ().length ())) {
	    if ((it < 'a' || it > 'z')
		&& (it < 'A' || it > 'Z')
		&& (it != '_')
		&& (it < '0' || it > '9')) {
		return false;
	    }
	}

	return true;
    }
    
    Block Visitor::visitBlockOutSide () {
	return visitBlock ();
    }

    Block Visitor::visitBlock () {
	auto begin = this-> lex.next ();
	if (begin == Token::LACC) {
	    std::vector <Declaration> decls;
	    std::vector <Instruction> insts;
	    while (true) {
		auto next = this-> lex.next ();
		if (next == Keys::DEF) decls.push_back (visitFunction ());
		else if (next == Keys::IMPORT) decls.push_back (visitImport ());
		else if (next == Keys::EXTERN) decls.push_back (visitExtern ());
		else if (next == Keys::STRUCT) decls.push_back (visitStruct ());
		else if (next == Token::LACC) {
		    this-> lex.rewind ();
		    insts.push_back (visitBlock ());		
		} else if (next == Token::SEMI_COLON) {}
		else if (next == Token::RACC) break;
		else {
		    this-> lex.rewind ();
		    insts.push_back (visitInstruction ());
		}
	    }
	    return new (GC) IBlock (begin, decls, insts);
	} else this-> lex.rewind ();
	return new (GC) IBlock (begin, {}, {visitInstruction ()});
    }

    Instruction Visitor::visitInstruction () {
	auto tok = this-> lex.next ();
	if (tok == Keys::IF) return visitIf ();
	else if (tok == Keys::RETURN) return visitReturn ();
	else if (tok == Keys::FOR) return visitFor ();
	else if (tok == Keys::WHILE) return visitWhile ();
	else if (tok == Keys::LET) return visitLet ();
	else if (tok == Keys::BREAK) return visitBreak ();
	else if (tok == Keys::ASSERT) return visitAssert ();
	else if (tok == Keys::STATIC) {
	    tok = this-> lex.next ();
	    Instruction inst;
	    if (tok == Keys::IF) inst = visitIf ();
	    else if (tok == Keys::ASSERT) inst = visitAssert ();
	    else {
		syntaxError (tok, {Keys::IF, Keys::ASSERT});
		return NULL;
	    }
	    inst-> setStatic (true);
	    return inst;
	}
	else if (tok == Token::SEMI_COLON) {
	    // TODO warn
	    return new (GC) INone (tok);
	} else {
	    this-> lex.rewind ();
	    auto retour = (Instruction) visitExpressionUlt ();
	    if (!retour-> is<IMatch> ()) {
		this-> lex.next ({Token::SEMI_COLON});	    
	    }
	    return retour;
	}	
    }

    /**
     let := 'let' (var ('=' right)? ',')* (var ('=' right)? ';')
     */
    Instruction Visitor::visitLet () {
	auto tok = this-> lex.rewind ().next ();
	Word token;
	std::vector <Var> decls;
	std::vector <Word> decos;
	std::vector <Expression> insts;
	auto nextW = this-> lex.next ();
	if (nextW == Token::LPAR) return visitLetDestruct (tok);
	else this-> lex.rewind ();
	while (true) {
	    auto deco = this-> lex.next ();
	    if (find (this-> decoKeys, deco))
		decos.push_back (deco);
	    else {
		decos.push_back (Word::eof ()); this-> lex.rewind ();
	    }
	    
	    auto var = visitVar ();
	    decls.push_back (var);
	    this-> lex.next (token).rewind ();
	    if (token == Token::EQUAL) {
		auto next = visitExpressionUlt (var);		
		if (next != var) {
		    insts.push_back (next);
		} else insts.push_back (NULL);
	    } else {
		insts.push_back (NULL);
	    }
	    token = this-> lex.next ();
	    if (token == Token::SEMI_COLON) break;
	    else if (token != Token::COMA) {
		syntaxError (token, {Token::SEMI_COLON, Token::COMA});
		break;
	    }
	    
	}
	return new (GC) IVarDecl (tok, decos, decls, insts);
    }
   
    Instruction Visitor::visitLetDestruct (Word begin) {
	bool isVariadic;
	std::vector <Var> decls;
	while (true) {
	    decls.push_back (visitVar ());
	    auto next = this-> lex.next ({Token::COMA, Token::RPAR, Token::TDOT});
	    if (next == Token::RPAR) break;
	    else if (next == Token::TDOT) {
		isVariadic = true;
		this-> lex.next ({Token::RPAR});
		break;
	    }
	}

	auto next = this-> lex.next ({Token::EQUAL});
	auto right = visitExpressionUlt ();
	this-> lex.next ({Token::SEMI_COLON});
	return new (GC) ITupleDest (begin, isVariadic, decls, right);
    }    
    
    Expression Visitor::visitExpressionOutSide () {
	return visitExpressionUlt ();
    }

    Expression Visitor::visitExpressionUlt () {
	auto left = visitExpression ();
	auto tok = this-> lex.next ();
	if (find (ultimeOp, tok)) {
	    auto right = visitExpressionUlt ();
	    return visitExpressionUlt (new (GC) IBinary (tok, left, right));
	} else this-> lex.rewind ();
	return left;
    }    

    Expression Visitor::visitExpressionUlt (Expression left) {
	auto tok = this-> lex.next ();
	if (find (ultimeOp, tok)) {
	    auto right = visitExpressionUlt ();
	    return visitExpressionUlt (new (GC) IBinary (tok, left, right));
	} else this-> lex.rewind ();
	return left;
    }
    
    Expression Visitor::visitExpression () {
	auto left = visitUlow ();
	auto tok = this-> lex.next ();
	if (find (expOp, tok)) {
	    auto right = visitUlow ();
	    return visitExpression (new (GC) IBinary (tok, left, right));
	} else this-> lex.rewind ();
	return left;
    }

    Expression Visitor::visitExpression (Expression left) {
	auto tok = this-> lex.next ();
	if (find (expOp, tok)) {
	    auto right = visitUlow ();
	    return visitExpression (new (GC) IBinary (tok, left, right));
	} else this-> lex.rewind ();
	return left;
    }
    
    Expression Visitor::visitUlow () {
	auto left = visitLow ();
	auto tok = this-> lex.next ();
	if (find (ulowOp, tok) || tok == Keys::IS) {
	    auto right = visitLow ();
	    return visitUlow (new (GC) IBinary (tok, left, right));
	} else {
	    if (tok == Token::NOT) {
		auto suite = this-> lex.next ();
		if (suite == Keys::IS) {
		    auto right = visitLow ();
		    tok.setStr (Keys::NOT_IS);
		    return visitUlow (new (GC) IBinary (tok, left, right));
		} else this-> lex.rewind ();
	    }
	    this-> lex.rewind ();
	}
	return left;
    }

    Expression Visitor::visitUlow (Expression left) {
	auto tok = this-> lex.next ();
	if (find (ulowOp, tok) || tok == Keys::IS) {
	    auto right = visitLow ();
	    return visitUlow (new (GC) IBinary (tok, left, right));
	} else {
	    if (tok == Token::NOT) {
		auto suite = this-> lex.next ();
		if (suite == Keys::IS) {
		    auto right = visitLow ();
		    tok.setStr (Keys::NOT_IS);
		    return visitUlow (new (GC) IBinary (tok, left, right));
		} else this-> lex.rewind ();
	    } else if (tok == Token::DDOT) {
		auto right = visitLow ();
		return visitHigh (new (GC) IConstRange (tok, left, right));
	    } 
	    this-> lex.rewind ();
	}
	return left;
    }

    Expression Visitor::visitLow () {
	auto left = visitHigh ();
	auto tok = this-> lex.next ();
	if (find (lowOp, tok)) {
	    auto right = visitHigh ();
	    return visitLow (new (GC) IBinary (tok, left, right));
	} else this-> lex.rewind ();
	return left;
    }

    Expression Visitor::visitLow (Expression left) {
	auto tok = this-> lex.next ();
	if (find (lowOp, tok)) {
	    auto right = visitHigh ();
	    return visitLow (new (GC) IBinary (tok, left, right));
	} else this-> lex.rewind ();
	return left;
    }

    Expression Visitor::visitHigh () {
    	auto left = visitPth ();
    	auto tok = this-> lex.next ();
    	if (find (highOp, tok)) {
    	    auto right = visitPth ();
    	    return visitHigh (new (GC) IBinary (tok, left, right));
    	} else if (tok == Keys::IN) {
	    auto right = visitPth ();
	    return visitHigh (new (GC) IBinary (tok, left, right));
	} else this-> lex.rewind ();
    	return left;
    }

    Expression Visitor::visitHigh (Expression left) {
	auto tok = this-> lex.next ();
	if (find (highOp, tok)) {
	    auto right = visitPth ();
	    return visitHigh (new (GC) IBinary (tok, left, right));
	} else if (tok == Keys::IN) {
	    auto right = visitPth ();
	    return visitHigh (new (GC) IBinary (tok, left, right));
	} else this-> lex.rewind ();
	return left;
    }
    
    Expression Visitor::visitPth () {
	auto tok = this-> lex.next ();
	if (find (befUnary, tok)) {
	    return visitBeforePth (tok);
	} else {
	    if (tok == Token::LPAR)
		return visitPthPar (tok);
	    else return visitPthWPar (tok);
	}
    }

    Expression Visitor::visitPthPar (Word token) {
	std::vector <Expression> params;
	Expression exp;
	Word tok, next;
	bool isTuple = false, isLambda = false;
	if (this-> lambdaPossible && canVisitVarDeclaration ()) return visitLambda ();
	tok = this-> lex.next ();	
	if (tok == Token::RPAR) {
	    this-> lex.next (next).rewind ();	
	    if (next == Token::LACC || next == Token::DARROW) {
		return visitLambdaEmpty ();
	    }
	    isTuple = true;
	} else {
	    this-> lex.rewind ();
	    while (true) {
		if (isLambda || canVisitVarDeclaration ()) {
		    isLambda = true;
		    params.push_back (visitVarDeclaration ());
		} else {
		    params.push_back (visitExpressionUlt ());
		}
		tok = this-> lex.next ({Token::RPAR, Token::COMA});
		if (tok == Token::RPAR) break;
		else {
		    isTuple = true;
		    next = this-> lex.next ();
		    if (next == Token::RPAR) break;
		    else this-> lex.rewind ();
		}	    
	    }
	    
	    next = this-> lex.next ();
	    if (next == Token::DARROW || next == Token::LACC) {
		isLambda = true;
		std::vector <Var> realParams;
		for (auto it : params) {
		    if (!it-> is<IVar> ())
			if (next == Token::DARROW) syntaxErrorFor (it-> token, next);
			else {
			    isLambda = false;
			    break;
			}
		    else realParams.push_back ((Var) it);
		}
		if (isLambda) {
		    if (next == Token::DARROW) {
			return new (GC) ILambdaFunc (tok, realParams, visitExpressionUlt ());
		    } else {
			this-> lex.rewind ();
			return new (GC) ILambdaFunc (tok, realParams, visitBlock ());
		    }
		} else this-> lex.rewind ();
	    } else if (isLambda) {
		syntaxError (next, {Token::LACC, Token::DARROW});
	    } else this-> lex.rewind ();
	}
		
	if (params.size () != 1 || isTuple) exp = new (GC) IConstTuple (token, tok, params);
	else exp = params [0];
	
	tok = this-> lex.next ();
	if (find (suiteElem, tok)) {
	    return visitSuite (tok, exp);
	} else if (find (afUnary, tok)) {
	    return visitAfter (tok, exp);
	} else this-> lex.rewind ();
	return exp;
    }

    Expression Visitor::visitConstante () {       
	auto tok = this-> lex.next ();
	if (tok.isEof ()) return NULL;
	if (tok.getStr () [0] >= '0'&& tok.getStr () [0] <= '9')
	    return visitNumeric (tok);
	else if (tok == Token::DOT)
	    return visitFloat (tok);
	else if (tok == Token::APOS || tok == Token::GUILL || tok == Token::BSTRING)
	    return visitString (tok);
	else if (tok == Keys::TRUE_ || tok == Keys::FALSE_)
	    return new (GC) IBool (tok);
	else if (tok == Keys::NULL_)
	    return new (GC) INull (tok);
	else if (tok == Keys::EXPAND)
	    return visitExpand ();
	else if (tok == Keys::IS) 
	    return visitIs ();
	else if (tok == Keys::TYPEOF)
	    return visitTypeOf ();
	else this-> lex.rewind ();
	return NULL;
    }
        
    Expression Visitor::visitExpand () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ({Token::LPAR});
	auto expr = visitExpression ();
	next = this-> lex.next ({Token::RPAR});
	return new (GC) IExpand (begin, expr);
    }

    Expression Visitor::visitTypeOf () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ({Token::LPAR});
	auto expr = visitExpression ();
	next = this-> lex.next ({Token::RPAR});
	return new (GC) ITypeOf (begin, expr);
    }
    
    Expression Visitor::visitIs () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ({Token::LPAR});
	this-> lambdaPossible = false;
	auto expr = visitExpression ();
	this-> lambdaPossible = true;
	next = this-> lex.next ({Token::COLON});
	next = this-> lex.next ();
	if (next == Keys::FUNCTION || next == Keys::STRUCT || next == Keys::TUPLE) {
	    auto expType = next;
	    next = this-> lex.next ({Token::RPAR});
	    return new (GC) IIs (begin, expr, expType);
	} else {
	    this-> lex.rewind ();
	    auto type = visitType ();
	    next = this-> lex.next ({Token::RPAR});
	    return new (GC) IIs (begin, expr, type);
	}
    }

    Expression Visitor::visitNumeric (Word begin) {
	for (int it = 0 ; it < (int) begin.getStr ().length (); it++) {
	    if (begin.getStr () [it] < '0' || begin.getStr() [it] > '9') {		
		if (begin.getStr () .substr (it, begin.getStr ().length ()) == "ub" || begin.getStr () .substr (it, begin.getStr ().length ()) == "UB")
		    return new (GC) IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::UBYTE);
		else if (begin.getStr () .substr (it, begin.getStr ().length ()) == "b" || begin.getStr () .substr (it, begin.getStr ().length ()) == "B")
		    return new (GC) IFixed ({begin.getLocus (), begin.getStr ().substr (0, it)}, FixedConst::BYTE);
		else if (begin.getStr () .substr (it, begin.getStr ().length ()) == "s" || begin.getStr () .substr (it, begin.getStr ().length ()) == "S")
		    return new (GC) IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::SHORT);
		else if (begin.getStr () .substr (it, begin.getStr ().length ()) == "us" || begin.getStr () .substr (it, begin.getStr ().length ()) == "US")
		    return new (GC) IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::USHORT);
		else if (begin.getStr () .substr (it, begin.getStr ().length ()) == "u" || begin.getStr () .substr (it, begin.getStr ().length ()) == "U")
		    return new (GC) IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::UINT);
		else if (begin.getStr () .substr (it, begin.getStr ().length ()) == "ul" || begin.getStr () .substr (it, begin.getStr ().length ()) == "UL")
		    return new (GC) IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::ULONG);
		else if (begin.getStr () .substr (it, begin.getStr ().length ()) == "l" || begin.getStr () .substr (it, begin.getStr ().length ()) == "L")
		    return new (GC) IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::LONG);
		else {
		    syntaxError (begin);
		    return NULL;
		}
	    }
	}
	
	auto next = this->lex.next ();
	if (next == Token::DOT) {
	    next = this-> lex.next ();
	    auto suite = next.getStr ();
	    for (auto it : next.getStr ()) {		
		if (it < '0' || it > '9') {
		    suite = "0";
		    this-> lex.rewind ();
		    break;
		}		    
	    }
	    return new (GC) IFloat (begin, suite);
	} else this-> lex.rewind ();
	return new (GC) IFixed (begin, FixedConst::INT);
    }    
    
    Expression Visitor::visitFloat (Word) {
	auto next = this-> lex.next ();
	for (auto it : next.getStr ()) {
	    if (it < '0' || it > '9') 
		syntaxError (next);	    
	}
	return new (GC) IFloat (next);
    }

    Expression Visitor::visitString (Word word) {
	this-> lex.skipEnable (Token::SPACE, false);       
	this-> lex.commentEnable (false);
	if (word == Token::BSTRING) {
	    word.setStr (Token::ESTRING);
	    this-> lex.skipEnable (Token::RETURN, false);
	    this-> lex.skipEnable (Token::RRETURN, false);
	    this-> lex.skipEnable (Token::TAB, false);
	}
	
	Word next, beg;
	string val = ""; bool anti = false;
	auto loc = this-> lex.tell ();
	std::vector <int> positions;
	while (1) {
	    next = this-> lex.next ();
	    if (next.isEof ()) unterminated (word);	    
	    else if (next.getStr () == word.getStr () && !anti) break;
	    else {
		positions.push_back (val.length ());
		val += next.getStr ();
	    }
	    if (next == Keys::ANTI) anti = !anti;
	    else anti = false;
	}
	
	this-> lex.skipEnable (Token::SPACE);
	this-> lex.skipEnable (Token::RETURN);
	this-> lex.skipEnable (Token::RRETURN);	
	this-> lex.skipEnable (Token::TAB);
	this-> lex.commentEnable ();

	std::stringstream ss;
	for (ulong i = 0 ; i < val.length () ;) {
	    auto c = IString::isChar (val, i);	    
	    if (c != -1) ss << (char) c;
	    else {
		auto nb = 0;
		for (uint j = 0 ; j < positions.size (); j++) {
		    if (i > (ulong) positions [j]) nb ++;
		}
		this-> lex.seek (loc + nb - 2);
		auto get = this-> lex.next ();
		escapeError (get);
	    }
	}
	
	auto res = ss.str ();	
	if (word == Token::APOS) {
	    if (res.length () == 1) 
		return new (GC) IChar (word, (ubyte) res [0]);
	}
	
	return new (GC) IString (word, res);
    }

    Expression Visitor::visitPthWPar (Word tok) {
	this-> lex.rewind ();
	auto constante = visitConstante ();
	if (constante != NULL) {
	    tok = this-> lex.next ();
	    if (find (suiteElem, tok)) {
		return visitSuite (tok, constante);
	    } else this-> lex.rewind ();
	    return constante;
	}
	auto left = visitLeftOp ();
	tok = this-> lex.next ();
	if (find (afUnary, tok)) {
	    return visitAfter (tok, left);
	} else this-> lex.rewind ();
	return left;
    }

    Expression Visitor::visitLeftOp () {
	auto word = this-> lex.next ();
	if (word == Keys::CAST) {
	    return visitCast ();
	} else if (word == Token::LCRO) {
	    return visitConstArray ();
	} else if (word == Keys::FUNCTION) {
	    return visitFuncPtr ();
	} else if (word == Keys::MIXIN) {
	    return visitMixin ();
	} else  if (word == Keys::MATCH) {
	    return visitMatch ();
	} else this-> lex.rewind ();
	auto var = visitVar ();
	auto next = this-> lex.next ();
	if (find (suiteElem, next)) 
	    return visitSuite (next, var);
	else this-> lex.rewind ();
	return var;
    }
    
    Expression Visitor::visitConstArray () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto word = this-> lex.next ();
	std::vector <Expression> params;
	if (word != Token::RCRO) {
	    this-> lex.rewind ();
	    auto fst = visitExpression ();
	    auto next = this-> lex.next ();
	    if (next == Token::SEMI_COLON) {
		auto size = visitExpression ();
		next = this-> lex.next ({Token::RCRO});
		return new (GC) IArrayAlloc (begin, fst, size);
	    } else {
		params.push_back (fst);
		this-> lex.rewind ();
		while (true) {
		    word = this-> lex.next ({Token::RCRO, Token::COMA});
		    if (word == Token::RCRO) break; 
		    params.push_back (visitExpression ());
		}
	    }
	}
	return new (GC) IConstArray (begin, params);
    }

        
    /**
     cast := 'cast' ':' type '(' expression ')'
     */
    Expression Visitor::visitCast () {
	this-> lex.rewind ();
	Expression type;
	auto begin = this-> lex.next ();
	auto word = this-> lex.next ({Token::COLON});
	auto next = this-> lex.next ();
	if (next == Keys::FUNCTION) {
	    type = visitFuncPtrSimple ();	    
	} else {
	    this-> lex.rewind ();
	    type = visitType ();
	}
	word = this-> lex.next ({Token::LPAR});
	auto expr = visitExpression ();
	word = this-> lex.next ({Token::RPAR});
	return new (GC) ICast (begin, type, expr);	
    }
    

    Expression Visitor::visitFuncPtrSimple () {
	std::vector <Var> params;
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto word = this-> lex.next ({Token::LPAR});
	word = this-> lex.next ();
	if (word != Token::RPAR) {
	    this-> lex.rewind ();
	    while (true) {
		params.push_back (visitType ());
		word = this-> lex.next ({Token::RPAR, Token::COMA});
		if (word == Token::RPAR) break;
	    }	    
	}
	word = this-> lex.next ({Token::ARROW});
	auto ret = visitType ();
	return new (GC) IFuncPtr (begin, params, ret);
    }

    Expression Visitor::visitFuncPtr () {
	std::vector <Var> params;
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto word = this-> lex.next ({Token::LPAR});
	word = this-> lex.next ();
	if (word != Token::RPAR) {
	    this-> lex.rewind ();
	    while (true) {
		params.push_back (visitType ());
		word = this-> lex.next ({Token::RPAR, Token::COMA});
		if (word == Token::RPAR) break;
	    }	    
	}
	
	word = this-> lex.next ({Token::ARROW});
	auto ret = visitType ();
	word = this-> lex.next ();
	if (word == Token::LPAR) {
	    auto expr = visitExpression ();
	    word = this-> lex.next ({Token::RPAR});
	    return new (GC) IFuncPtr (begin, params, ret, expr);
	} else this-> lex.rewind ();
	return new (GC) IFuncPtr (begin, params, ret);
    }

    Expression Visitor::visitLambdaEmpty () {
	auto next = this-> lex.next ({Token::DARROW, Token::LACC});
	if (next == Token::DARROW) {
	    auto expr = visitExpressionUlt ();
	    return new (GC) ILambdaFunc (next, {}, expr);
	} else if (next == Token::LACC) {
	    this-> lex.rewind ();
	    return new (GC) ILambdaFunc (next, {}, visitBlock ());
	} else return NULL;
    }
    
    Expression Visitor::visitLambda () {
	std::vector <Var> params;
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	while (true) {
	    params.push_back (visitVarDeclaration ());
	    auto next = this-> lex.next ({Token::RPAR, Token::COMA});
	    if (next == Token::RPAR) break;
	}
	
	auto next = this-> lex.next ({Token::DARROW, Token::LACC});
	if (next == Token::DARROW) {
	    auto expr = visitExpressionUlt ();
	    return new (GC) ILambdaFunc (begin, params, expr);
	} else if (next == Token::LACC) {
	    this-> lex.rewind ();
	    auto block = visitBlock ();
	    return new (GC) ILambdaFunc (begin, params, block);
	} else return NULL;
    }    
    
    
    Expression Visitor::visitSuite (Word token, Expression left) {
	if (token == Token::LPAR) return visitPar (left);
	else if (token == Token::LCRO) return visitAccess (left);
	else if (token == Token::DOT) return visitDot (left);
	else if (token == Token::DCOLON) return visitDColon (left);
	else {
	    syntaxError (token, {Token::LPAR, Token::LCRO, Token::DOT, Token::DCOLON});
	    return NULL;
	} 
    }

    Expression Visitor::visitPar (Expression left) {
	this-> lex.rewind ();
	auto beg = this-> lex.next (), next = this-> lex.next ();
	auto suite = next;
	std::vector <Expression> params;
	if (next != Token::RPAR) {
	    this-> lex.rewind ();
	    while (true) {
		params.push_back (visitExpression ());
		next = this-> lex.next ({Token::RPAR, Token::COMA});
		if (next == Token::RPAR) break;
	    }
	}
	auto retour = new (GC) IPar (beg, next, left, new (GC) IParamList (suite, params));
	next = this-> lex.next ();
	if (find (suiteElem, next))
	    return visitSuite (next, retour);
	else if (find (afUnary, next))
	    return visitAfter (next, retour);
	this-> lex.rewind ();
	return retour;
    }

    /**
     access := '[' (expression (',' expression)*)? ']'
     */
    Expression Visitor::visitAccess (Expression left) {
	this-> lex.rewind ();
	auto beg = this-> lex.next (), next = this-> lex.next ();
	auto suite = next;
	std::vector <Expression> params;
	if (next != Token::RCRO) {
	    this-> lex.rewind ();
	    while (true) {
		params.push_back (visitExpression ());
		next = this-> lex.next ({Token::RCRO, Token::COMA});
		if (next == Token::RCRO) break;
	    }
	}
	
	auto retour = new (GC) IAccess (beg, next, left, new (GC) IParamList (suite, params));
	next = this-> lex.next ();
	if (find (suiteElem, next))
	    return visitSuite (next, retour);
	else if (find (afUnary, next))
	    return visitAfter (next, retour);
	this-> lex.rewind ();
	return retour;
    }
    

    Expression Visitor::visitDot (Expression left) {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ();
	Expression retour;
	if (next == Keys::EXPAND) {
	    retour = new (GC) IExpand (next, left);
	} else if (next == Keys::TYPEOF) {
	    retour = new (GC) ITypeOf (next, left);
	} else {
	    this-> lex.rewind ();
	    Expression right = visitConstante ();
	    if (right == NULL) right = visitVar ();
	    retour = new (GC) IDot (begin, left, right);
	}
	
	next = this-> lex.next ();
	if (find (suiteElem, next))
	    return visitSuite (next, retour);
	else if (find (afUnary, next))
	    return visitAfter (next, retour);
	this-> lex.rewind ();
	return retour;
    }

    Expression Visitor::visitDColon (Expression left) {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto right = visitVar ();
	auto retour = new (GC) IDColon (begin, left, right);
	auto next = this-> lex.next ();
	if (find (suiteElem, next))
	    return visitSuite (next, retour);
	else if (find (afUnary, next))
	    return visitAfter (next, retour);
	this-> lex.rewind ();
	return retour;
    }
    
    Expression Visitor::visitMixin () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto expr = visitPth ();
	return new (GC) IMixin (begin, expr);
    }

    Expression Visitor::visitMatch () {
	std::vector <Expression> values;
	std::vector <Block> insts;
	Block defaultInsts = NULL;
	
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto expr = visitExpression ();
	
	auto next = this-> lex.next ({Token::LACC});
	while (true) {
	    next = this-> lex.next ();
	    if (next != Keys::UNDER) {
		this-> lex.rewind ();
		values.push_back (visitExpression ());
		next = this-> lex.next ({Token::DARROW, Token::TDOT});
		if (next == Token::TDOT) {
		    values.back () = new (GC) IMatchPair (next, values.back (), visitExpression ());
		    next = this-> lex.next ({Token::DARROW});
		}
		
		insts.push_back (visitBlock ());
		next = this-> lex.next ();
		if (next == Token::RACC) break;
		else this-> lex.rewind ();
	    } else {
		this-> lex.next ({Token::DARROW});
		defaultInsts = visitBlock ();
		this-> lex.next ({Token::RACC});
		break;
	    }
	}
	
	return new (GC) IMatch (begin, expr, values, insts, defaultInsts);
    }
    
    Expression Visitor::visitAfter (Word word, Expression left) {
	return new (GC) IUnary (word, left);
    }
    
    Expression Visitor::visitBeforePth (Word word) {
	auto elem = visitPth ();
	return new (GC) IUnary (word, elem);
    }
        
    Instruction Visitor::visitIf () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto test = visitExpression ();
	auto block = visitBlock ();
	auto next = this-> lex.next ();
	if (next == Keys::ELSE) {
	    return new (GC) IIf (begin, test, block, visitElse ()); 
	} else this-> lex.rewind ();
	return new (GC) IIf (begin, test, block);
    }
        
    If Visitor::visitElse () {
	this-> lex.rewind ();
	auto begin = this-> lex.next (), next = this-> lex.next ();
	if (next == Keys::IF) {
	    auto test = visitExpression ();
	    auto block = visitBlock ();
	    next = this-> lex.next ();
	    if (next == Keys::ELSE) {
		return new (GC) IIf (begin, test, block, visitElse ());
	    } else this-> lex.rewind ();
	    return new (GC) IIf (begin, test, block);
	} else this-> lex.rewind ();
	return new (GC) IIf (begin, NULL, visitBlock ());
    }
    
    Assert Visitor::visitAssert () {
	this-> lex.rewind ();
	auto begin = this-> lex.next (), next = this-> lex.next ({Token::LPAR});
	auto expr = visitExpression ();
	Expression msg = NULL;
	next = this-> lex.next ({Token::COMA, Token::RPAR});
	if (next == Token::COMA) {
	    msg = visitExpression ();
	    next = this-> lex.next ({Token::RPAR});
	}
	
	next = this-> lex.next ({Token::SEMI_COLON});
	return new (GC) IAssert (begin, expr, msg);
    }
        
    Instruction Visitor::visitReturn () {
	this-> lex.rewind ();
	auto begin = this-> lex.next (), next = this-> lex.next ();
	if (next == Token::SEMI_COLON) 
	    return new (GC) IReturn (begin);
	else this-> lex.rewind ();
	auto exp = visitExpression ();
	next = this-> lex.next ({Token::SEMI_COLON});
	return new (GC) IReturn (begin, exp);	
    }
    
    
    Instruction Visitor::visitBreak () {
	this-> lex.rewind ();
	auto begin = this-> lex.next (), next = this-> lex.next ();
	if (next == Token::SEMI_COLON) {
	    return new (GC) IBreak (begin);
	} else this-> lex.rewind ();
	auto id = visitIdentifiant ();
	next = this-> lex.next ({Token::SEMI_COLON});
	return new (GC) IBreak (begin, id);
    }

    Instruction Visitor::visitWhile () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ();
	if (next == Token::COLON) {
	    auto id = visitIdentifiant ();
	    next = this-> lex.next ({Token::LPAR});
	    auto test = visitExpression ();
	    next = this-> lex.next ({Token::RPAR});
	    return new (GC) IWhile (begin, id, test, visitBlock ());
	} else {
	    this-> lex.rewind ();
	    auto test = visitExpression ();
	    return new (GC) IWhile (begin, test, visitBlock ());
	}
    }

    Instruction Visitor::visitFor () {	
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ();
	Word id = Word::eof ();
	bool need = false;
	if (next == Token::COLON) {
	    id = visitIdentifiant ();
	    next = this-> lex.next ({Token::LPAR});
	    need = true;
	} else if (next == Token::LPAR) {
	    need = true;
	} else this-> lex.rewind ();
	std::vector <Var> vars;
	while (true) {	    
	    vars.push_back (visitVar ());
	    next = this-> lex.next ({Keys::IN, Token::COMA});
	    if (next == Keys::IN) break;
	}

	auto iter = visitExpression ();
	if (need) {
	    next = this-> lex.next ({Token::RPAR});
	}
	return new (GC) IFor (begin, id, vars, iter, visitBlock ());
    }       
    
};
