#include "syntax/_.hh"
#include "errors/_.hh"
#include "syntax/Keys.hh"
#include <ymir/utils/Version.hh>

#include <map>
#include <vector>
#include <algorithm>

using namespace std;

namespace syntax {

    bool Visitor::failure = false;
    
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

    void unterminated (const Word& token) {
	Ymir::Error::unterminated (token);
    }
    
    void syntaxError (const Word& token) {
	Ymir::Error::syntaxError (token);
    }

    void syntaxErrorFor (const Word& token, Word tok2) {
	Ymir::Error::syntaxErrorFor (token, tok2);
    }
    
    void escapeError (const Word& token) {
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
	    Token::NOT_SUP_EQUAL, Token::DEQUAL, Token::DDOT, Token::TDOT
	};

	this-> lowOp = {Token::PLUS, Token::PIPE, Token::LEFTD,
			Token::XOR, Token::TILDE, Token::MINUS,
			Token::RIGHTD};

	this-> highOp = {Token::DIV, Token::AND, Token::STAR, Token::PERCENT,
			 Token::DXOR};
	
	this-> suiteElem = {Token::LPAR, Token::LCRO, Token::DOT, Token::DCOLON, Token::LACC, Token::COLON};
	this-> afUnary = {Token::DPLUS, Token::DMINUS};	
	this-> befUnary = {Token::MINUS, Token::AND, Token::STAR, Token::NOT};
	this-> forbiddenIds = {Keys::IMPORT, Keys::STRUCT, Keys::ASSERT, Keys::SCOPE,
			       Keys::DEF, Keys::IF, Keys::RETURN, Keys::PRAGMA, 
			       Keys::FOR,  Keys::WHILE, Keys::BREAK,
			       Keys::MATCH, Keys::IN, Keys::ELSE, Keys::DELEGATE, 
			       Keys::TRUE_, Keys::FALSE_, Keys::NULL_, Keys::CAST,
			       Keys::FUNCTION, Keys::LET, Keys::IS, Keys::EXTERN,
			       Keys::PUBLIC, Keys::PRIVATE, Keys::TYPEOF, Keys::IMMUTABLE,
			       Keys::MACRO, Keys::TRAIT, Keys::REF, Keys::CONST,
			       Keys::MOD, Keys::USE, Keys::STRINGOF, Keys::TYPE, Keys::ALIAS
	};
	
	this-> decoKeys = {Keys::IMMUTABLE, Keys::CONST, Keys::STATIC};
	this-> lambdaPossible = true;
	this-> isInMatch = false;
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
	    } else if (token == Keys::VERSION) {
		auto ver_decls = visitVersionGlob ();
		for (auto it : ver_decls) decls.push_back (it);
	    } else if (token == Keys::EXTERN) {
		auto ext_decls = visitExtern ();
		for (auto it : ext_decls) decls.push_back (it);
	    } else {
		this-> lex.next ();
		syntaxError (token,
			     {Keys::DEF, Keys::MACRO, Keys::USE, Keys::MOD, Keys::IMPORT,
				     Keys::EXTERN, Keys::STRUCT, Keys::UNION, Keys::ENUM,
				     Keys::STATIC, Keys::IMMUTABLE, Keys::SELF, (Token::TILDE + Keys::SELF), Keys::VERSION, Keys::TYPE
				     }
		);
	    }
	    token = this-> lex.next ();
	    this-> lex.rewind ();
	}
	auto last = this-> lex.fileLocus ();
	return new (Z0)  IProgram (last, decls);
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
		    if (tok == Keys::VERSION) {
			auto ver_decls = visitVersionGlob ();
			for (auto it : ver_decls) {
			    it-> is_public (true);
			    decls.push_back (it);
			}
		    } else if (tok == Keys::EXTERN)  {
			this-> lex.rewind (); 
			auto ext_decls = visitExtern ();
			for (auto it : ext_decls) {
			    it-> is_public (true);
			    decls.push_back (it);
			}
		    } else if (tok != Token::RACC) {
			syntaxError (tok,
				     {Keys::DEF, Keys::MACRO, Keys::USE, Keys::MOD, Keys::IMPORT,
					     Keys::EXTERN, Keys::STRUCT, Keys::UNION, Keys::ENUM,
					     Keys::STATIC, Keys::IMMUTABLE, Keys::SELF, (Token::TILDE + Keys::SELF), Keys::TYPE, Keys::ALIAS
					     }
			);
		    } else 
			break;
    		}
    	    }
    	} else {
	    this-> lex.rewind ();
	    if (next == Keys::EXTERN) {
		decls = visitExtern ();
		for (auto it : decls) it-> is_public (true);
	    } else {
		decls.push_back (visitDeclaration (true));
		decls.back ()-> is_public (true);
	    }
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
		    if (tok == Keys::VERSION) {
			auto ver_decls = visitVersionGlob ();
			for (auto it : ver_decls) {
			    it-> is_public (false);
			    decls.push_back (it);
			}
		    } else if (tok == Keys::EXTERN)  {
			this-> lex.rewind ();
			auto ext_decls = visitExtern ();
			for (auto it : ext_decls) {
			    it-> is_public (false);
			    decls.push_back (it);
			}
		    } else if (tok != Token::RACC)
			syntaxError (tok,
				     {Keys::DEF, Keys::MACRO, Keys::USE, Keys::MOD, Keys::IMPORT,
					     Keys::EXTERN, Keys::STRUCT, Keys::UNION, Keys::ENUM,
					     Keys::STATIC, Keys::IMMUTABLE, Keys::SELF, (Token::TILDE + Keys::SELF), Keys::TYPE
					     }
			);			
    		    break;
    		}
    	    }
    	} else {	    
	    this-> lex.rewind ();
	    if (next == Keys::EXTERN) {
		decls = visitExtern ();
		for (auto it : decls) it-> is_public (false);
	    } else {
		decls.push_back (visitDeclaration (true));
		decls.back ()-> is_public (false);
	    }
    	}
    	return decls;
    }


    std::vector <Declaration> Visitor::visitDeclBlock () {
	std::vector <Declaration> decls;
	auto token = this-> lex.next ({Token::LACC});
	while (true) {
	    auto decl = visitDeclaration (false);
	    if (decl != NULL) decls.push_back (decl);
	    else if (token == Keys::PUBLIC) {
		auto pub_decls = visitPublicBlock ();
		for (auto it : pub_decls) decls.push_back (it);
	    } else if (token == Keys::PRIVATE) {
		auto prv_decls = visitPrivateBlock ();
		for (auto it : prv_decls) decls.push_back (it);
	    } else if (token == Keys::VERSION) {
		auto ver_decls = visitVersionGlob ();
		for (auto it : ver_decls) decls.push_back (it);
	    } else if (token == Keys::EXTERN)  {
		auto ext_decls = visitExtern ();
		for (auto it : ext_decls) decls.push_back (it);
	    } else if (token != Token::RACC) {
		this-> lex.next ();
		syntaxError (token,
			     {Keys::DEF, Keys::MACRO, Keys::USE, Keys::MOD, Keys::IMPORT,
				     Keys::EXTERN, Keys::STRUCT, Keys::UNION, Keys::ENUM,
				     Keys::STATIC, Keys::IMMUTABLE, Keys::SELF, (Token::TILDE + Keys::SELF), Keys::VERSION, Keys::TYPE, Keys::ALIAS
				     }
		);
	    } else {
		this-> lex.next ();
		break;
	    }
	    
	    token = this-> lex.next ();
	    this-> lex.rewind ();
	}
	return decls;
    }
        
    std::vector <Declaration> Visitor::visitVersionGlob () {
	auto begin = this-> lex.next ();
	auto type = this-> visitIdentifiant ();
	
	if (Version::isOn (type.getStr ())) {
	    auto bl = visitDeclBlock ();
	    auto next = this-> lex.next ();
	    if (next == Keys::ELSE) {
		this-> lex.next ({Token::LACC});
		auto nb = 1;
		while (nb != 0) {
		    auto next = this-> lex.next ();
		    if (next == Token::RACC) nb --;
		    else if (next == Token::LACC) nb ++;		
		    else if (next.isEof ())
			syntaxError (next);		    
		}
	    } else this-> lex.rewind ();
	    return bl;
	} else {
	    this-> lex.next ({Token::LACC});
	    auto nb = 1;
	    while (nb != 0) {
		auto next = this-> lex.next ();
		if (next == Token::RACC) nb --;
		else if (next == Token::LACC) nb ++;		
		else if (next.isEof ()) syntaxError (next);
	    }

	    auto next = this-> lex.next ();
	    if (next == Keys::ELSE) {
		return visitDeclBlock ();
	    } else {
		this-> lex.rewind ();
		return {};
	    }
	}
    }
    
    /**
       declaration :=   function 
       | import
       | extern
       | struct
       | enum
       | global 
       | self
       | trait
       | impl
       
    */
    Declaration Visitor::visitDeclaration (bool fatal) {
    	auto token = this-> lex.next ();
    	if (token == Keys::DEF) return visitFunction ();
	else if (token == Keys::MACRO) return visitMacro ();
	else if (token == Keys::USE) return visitUse ();
	else if (token == Keys::MOD) return visitModule ();
    	else if (token == Keys::IMPORT) return visitImport ();
    	else if (token == Keys::STRUCT) return visitStruct ();
	else if (token == Keys::UNION) return visitStruct (true);
    	else if (token == Keys::ENUM) return visitEnum ();
    	else if (token == Keys::STATIC) return visitGlobal ();
	else if (token == Keys::IMMUTABLE) return visitGlobalImut ();
    	else if (token == Keys::SELF) return visitSelf ();
	else if (token == Token::TILDE) return visitDestSelf ();
	else if (token == Keys::TYPE) return visitTypeCreator ();
	else if (token == Keys::ALIAS) return visitAlias ();
	else if (token == Keys::TRAIT) return visitTrait ();
    	else if (fatal) syntaxError (token,
				     {Keys::DEF, Keys::MACRO, Keys::USE, Keys::MOD, Keys::IMPORT,
					     Keys::EXTERN, Keys::STRUCT, Keys::UNION, Keys::ENUM,
					     Keys::STATIC, Keys::IMMUTABLE, Keys::SELF, (Token::TILDE + Keys::SELF), Keys::TYPE, Keys::ALIAS
					     }
	);
    	else this-> lex.rewind ();
    	return NULL;
    }

    Macro Visitor::visitMacro () {
	auto ident = visitIdentifiant ();
	auto word = this-> lex.next ({Token::LACC});
	std::vector <MacroExpr> exprs;
	std::vector <Block> blocks;
	while (true) {	    
	    exprs.push_back (visitMacroExpression ());
	    blocks.push_back (visitBlock ());	    
	    auto next = this-> lex.next ();
	    if (next == Token::RACC)
		break;
	    else this-> lex.rewind ();
	}
	return new (Z0) IMacro (ident, exprs, blocks);
    }

    MacroExpr Visitor::visitMacroExpression (bool in_repeat) {
	Word begin;
	if (!in_repeat)
	    begin = this-> lex.next ({Token::LPAR});
	
	Word end;
	auto endTok = Token::RPAR;
	if (in_repeat) endTok = Token::COMA;
	
	std::vector <MacroElement> elements;
	while (true) {
	    auto next = this-> lex.next ({Token::STAR, Token::DOLLAR, Token::PLUS, Token::GUILL, endTok, Token::RPAR});
	    if (next == Token::STAR || next == Token::PLUS) elements.push_back (visitMacroRepeat (next == Token::PLUS));
	    else if (next == Token::DOLLAR) elements.push_back (visitMacroVar ());
	    else if (next == Token::GUILL) elements.push_back (visitMacroToken ());
	    else { end = next; break; } 
	}
	return new (Z0) IMacroExpr (begin, end, elements);
    }

    MacroRepeat Visitor::visitMacroRepeat (bool atLeastOneTime) {
	auto ident = visitIdentifiant ();
	this-> lex.next ({Token::COLON});
	this-> lex.next ({Token::LPAR});
	auto expr = visitMacroExpression (true);
	auto end = this-> lex.rewind ().next ();
	MacroToken tok = NULL;
	if (end == Token::COMA) {
	    this-> lex.next ({Token::GUILL});
	    tok = visitMacroToken ();
	    this-> lex.next ({Token::RPAR});
	}
	return new (Z0) IMacroRepeat (ident, expr, tok, atLeastOneTime);
    }
        
    MacroVar Visitor::visitMacroVar () {
	auto ident = this-> visitIdentifiant ();
	this-> lex.next ({Token::COLON});
	auto val = this-> lex.next ({Keys::MACRO_EXPR, Keys::MACRO_IDENT, Keys::MACRO_BLOCK, Keys::MACRO_TOKEN});
	if (val == Keys::MACRO_IDENT)
	    return new (Z0) IMacroVar (ident, MacroVarConst::IDENT);
	else if (val == Keys::MACRO_EXPR)
	    return new (Z0) IMacroVar (ident, MacroVarConst::EXPR);
	else if (val == Keys::MACRO_BLOCK)
	    return new (Z0) IMacroVar (ident, MacroVarConst::BLOCK);
	else 
	    return new (Z0) IMacroVar (ident, MacroVarConst::TOKEN);
	
    }

    MacroToken Visitor::visitMacroToken () {
	auto begin = this-> lex.rewind ().next ();
	auto val = visitString (begin);
	std::string value;
	if (auto str = val-> to <IString> ()) {
	    value = str-> getStr (); 	    
	} else if (auto ch = val-> to<IChar> ()) {
	    value = std::string (1, ch-> toChar ()); 
	} else  Ymir::Error::assert ("!!");
	
	return new (Z0) IMacroToken (begin, value);
    }
    
    ModDecl Visitor::visitModule () {
	auto ident = visitIdentifiant ();
	auto word = this-> lex.next ({Token::DOT, Token::SEMI_COLON, Token::LACC, Token::LPAR});
	if (word != Token::LACC && word != Token::LPAR) {
	    while (word == Token::DOT) {
		if (word == Token::DOT)
		    ident.setStr (ident.getStr () + ".");
		auto nx = this-> lex.next ();
		if (nx == Keys::UNDER) {
		    ident.setStr (ident.getStr () + nx.getStr ());
		    word = this-> lex.next ({Token::SEMI_COLON});
		} else {
		    this-> lex.rewind ();
		    auto name = visitIdentifiant ();
		    ident.setStr (ident.getStr () + name.getStr ());
		    word = this-> lex.next ({Token::DOT, Token::SEMI_COLON});
		}
	    }		
	    return new (Z0) IModDecl (ident);
	} else {
	    std::vector <Expression> templates;
	    if (word == Token::LPAR) {
		templates = visitTemplateStruct ();
		word = this-> lex.next ({Token::LACC});
	    }
	    
	    std::vector <Declaration> decls;
	    while (true) {
		auto decl = visitDeclaration (false);
		if (decl) decls.push_back (decl);
		else {
		    auto tok = this-> lex.next ({Token::RACC, Keys::PRIVATE, Keys::PUBLIC, Keys::VERSION, Keys::EXTERN});
		    this-> lex.rewind ();
		    if (tok == Keys::PRIVATE) {
			auto prv_decls = visitPrivateBlock ();
			for (auto it : prv_decls) decls.push_back (it);
		    } else if (tok == Keys::PUBLIC) {
			auto pub_decls = visitPublicBlock ();
			for (auto it : pub_decls) decls.push_back (it);
		    } else if (tok == Keys::VERSION) {			
			auto ver_decls = visitVersionGlob ();
			for (auto it : ver_decls) decls.push_back (it);
		    } else if (tok == Keys::EXTERN) {
			auto ext_decls = visitExtern ();
			for (auto it : ext_decls) decls.push_back (it);
		    }  else { this-> lex.next (); break; }
		}
	    }
	    auto ret = new (Z0) IModDecl (ident, decls);
	    ret-> getTemplates () = templates;
	    return ret;
	}
    }
    
    /**
       self := 'self' '(' ')' block
    */
    Self Visitor::visitSelf () {
	auto begin = this->lex.rewind ().next ();
	this-> lex.next ({Token::LPAR});
	this-> lex.next ({Token::RPAR});
	return new (Z0)  ISelf (begin, visitBlock ());
    }

    DestSelf Visitor::visitDestSelf () {
	auto begin = this-> lex.next ();
	this-> lex.next ({Token::LPAR});
	this-> lex.next ({Token::RPAR});
	return new (Z0)  IDestSelf (begin, visitBlock ());
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
	    return new (Z0)  IGlobal (ident, expr);
	} else {
	    Expression type;
	    next = this-> lex.next ();
	    if (next == Keys::FUNCTION || next == Keys::DELEGATE) type = visitFuncPtrSimple (next);
	    else {
		this-> lex.rewind ();
		type = visitType ();
	    }
	    this-> lex.next ({Token::SEMI_COLON});
	    return new (Z0)  IGlobal (ident, NULL, type);
	}
    }

    Global Visitor::visitGlobalImut () {
	auto begin = this-> lex.rewind ().next ();
	auto ident = visitIdentifiant ();
	auto next = this-> lex.next ({Token::EQUAL});

	auto expr = visitExpression ();
	this-> lex.next ({Token::SEMI_COLON});
	auto ret = new (Z0)  IGlobal (ident, expr);
	ret-> isImut () = true;
	return ret;
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

    Use Visitor::visitUse () {
	auto begin = this-> lex.rewind ().next ();
	auto mod = visitExpression ();
	this-> lex.next ({Token::SEMI_COLON});
	return new (Z0) IUse (begin, mod);
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
	return new (Z0)  IImport (begin, idents);
    }

    std::vector <Expression> Visitor::visitTemplateStruct () {
	std::vector<Expression> expr;
	auto next = this-> lex.next ();
	bool ig = false, cont = true;
	if (next != Token::RPAR) {
	    this-> lex.rewind ();
	    while (next != Token::RPAR) {
		expr.push_back (visitOf (ig, cont));
		next = this-> lex.next ({Token::RPAR, Token::COMA});
	    }
	}
	return expr;
    }

    Struct Visitor::visitStruct (bool isUnion) {
	Word word = this-> lex.next (), ident;
	std::vector <Var> exps;
	std::vector <Expression> temps;
	std::vector <Word> udas;
	if (word == Token::AT) {
	    udas = visitAttributes ();
	    word = this-> lex.next ({Token::LPAR, Token::PIPE, Token::ARROW});
	}
	
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
 	} else {
	    ident = visitIdentifiant ();
	    this-> lex.next ({Token::SEMI_COLON});    
	}
	return new (Z0)  IStruct (ident, temps, exps, udas, isUnion);
    }

    Enum Visitor::visitEnum () {
	std::vector<Word> names;
	std::vector<Expression> values;
	auto word = this-> lex.next ();
	Word ident;
	Var type = NULL;
	if (word == Token::COLON) type = visitType ();
	else this-> lex.rewind ();
	word = this-> lex.next ({Token::PIPE});
	while (true) {
	    names.push_back (visitIdentifiant ());
	    auto next = this-> lex.next ({Token::COLON});
	    values.push_back (visitPth ());
	    next = this-> lex.next ({Token::ARROW, Token::PIPE});
	    if (next == Token::ARROW || next.isEof ()) break;
	}
	
	ident = visitIdentifiant ();       	    
	word = this-> lex.next ({Token::SEMI_COLON});	
	return new (Z0)  IEnum (ident, type, names, values);
    }

    TypeCreator Visitor::visitTypeCreator () {
	std::vector <Expression> temps;
	auto ident = visitIdentifiant ();
	auto word = this-> lex.next ({Token::LPAR, Keys::OVER, Keys::IMPL});
	TypeForm form = TypeForm::OVER; 
	if (word == Token::LPAR) {
	    temps = visitTemplateStruct ();
	    word = this-> lex.next ({Keys::OVER, Keys::IMPL});
	    form = (word == Keys::OVER) ? TypeForm::OVER : TypeForm::IMPL;
	} else {	    
	    form = (word == Keys::OVER) ? TypeForm::OVER : TypeForm::IMPL;
	}

	auto save = this-> lambdaPossible;
	this-> lambdaPossible = false;

	bool isUnion = false;
	std::vector <Expression> who;
	if (form == TypeForm::IMPL || form == TypeForm::OVER) {
	    who.push_back (visitExpression ());	    
	} 
	
	this-> lambdaPossible = save;
	auto type = new (Z0) ITypeCreator (ident, form, who, temps, isUnion);
	
	this-> lex.next ({Token::LACC});
	while (true) {
	    auto next = this-> lex.next ({Token::RACC, Keys::SELF, Keys::DEF, Keys::OVER, Token::TILDE, Keys::PRIVATE, Keys::PROTECTED, Keys::LET});
	    if (next == Keys::SELF) type-> getConstructors ().push_back (visitTypeConstructor ());
	    else if (next == Keys::OVER || next == Keys::DEF)
		type-> getMethods ().push_back (visitTypeMethod ());
	    else if (next == Keys::LET)
		type-> getAlias ().push_back (visitTypeAlias ());
	    else if (next == Token::TILDE) type-> getDestructors ().push_back (visitTypeDestructor ());
	    else if (next == Keys::PRIVATE) visitTypePrivate (type);		
	    else if (next == Keys::PROTECTED) visitTypeProtected (type);
	    else break;
	}
	return type;
    }

    TypeConstructor Visitor::visitTypeConstructor () {
	std::vector <Var> params;
	auto begin = this-> lex.rewind ().next ();
	auto word = this-> lex.next ({Token::LPAR});
	word = this-> lex.next ();
	if (word != Token::RPAR) {
	    if (word == Keys::SELF) {
		auto name = visitIdentifiant ();
		this-> lex.next ({Token::RPAR});
		params.push_back (new (Z0) IVar (name));
		return new (Z0) ITypeConstructor (begin, params, visitBlock (), true);
	    } else {
		this-> lex.rewind ();
		while (true) {
		    params.push_back (visitVarDeclaration ());
		    word = this-> lex.next ({Token::RPAR, Token::COMA});
		    if (word == Token::RPAR) break;
		}
	    }
	}
	return new (Z0) ITypeConstructor (begin, params, visitBlock (), false);
    }

    TypeDestructor Visitor::visitTypeDestructor () {	
	Word begin = this-> lex.next ({Keys::SELF});
	this-> lex.next ({Token::LPAR});
	this-> lex.next ({Token::RPAR});
	return new (Z0) ITypeDestructor (begin, visitBlock ());
    }

    TypeAlias Visitor::visitTypeAlias () {
	auto next = this-> lex.next ();
	bool isConst = false;
	if (next == Keys::CONST) isConst = true;
	else this-> lex.rewind ();
	
	auto name = visitIdentifiant ();
	this-> lex.next ({Token::COLON});
						  
	this-> lex.next ({Keys::SELF});
	this-> lex.next ({Token::DOT});
	this-> lex.rewind (2);
	auto value = visitExpression ();
	this-> lex.next ({Token::SEMI_COLON});
	
	return new (Z0) ITypeAlias (name, value, isConst);
    }
    
    TypeMethod Visitor::visitTypeMethod () {
	auto over = (this-> lex.rewind ().next () == Keys::OVER);
	auto function = visitFunction ();
	return new (Z0) ITypeMethod (function, over);
    }

    void Visitor::visitTypePrivate (TypeCreator type) {
	bool mult = false;
	auto next = this-> lex.next ();
	if (next == Token::LACC) mult = true;
	else this-> lex.rewind ();
	while (true) {
	    Word next;
	    if (mult) next = this-> lex.next ({Token::RACC, Keys::SELF, Keys::DEF, Keys::OVER, Keys::LET});
	    else next = this-> lex.next ({Keys::SELF, Keys::DEF, Keys::OVER, Keys::LET});
	    
	    if (next == Keys::SELF) {
		auto cst = visitTypeConstructor ();
		cst-> getProtection () = InnerProtection::PRIVATE;
		type-> getConstructors ().push_back (cst);
	    } else if (next == Keys::OVER || next == Keys::DEF) {
		auto cst = visitTypeMethod ();
		cst-> getProtection () = InnerProtection::PRIVATE;
		type-> getMethods ().push_back (cst);
	    } else if (next == Keys::LET) {
		auto cst = visitTypeAlias ();
		cst-> getProtection () = InnerProtection::PRIVATE;
		type-> getAlias ().push_back (cst);
	    } else break;
	    if (!mult) break;
	}
    }
    
    void Visitor::visitTypeProtected (TypeCreator type) {
	bool mult = false;

	auto next = this-> lex.next ();
	if (next == Token::LACC) mult = true;
	else this-> lex.rewind ();
	
	while (true) {
	    Word next;
	    if (mult) next = this-> lex.next ({Token::RACC, Keys::SELF, Keys::DEF, Keys::OVER, Keys::LET});
	    else next = this-> lex.next ({Keys::SELF, Keys::DEF, Keys::OVER, Keys::LET});
	    if (next == Keys::SELF) {
		auto cst = visitTypeConstructor ();
		cst-> getProtection () = InnerProtection::PROTECTED;
		type-> getConstructors ().push_back (cst);
	    } else if (next == Keys::OVER || next == Keys::DEF) {
		auto cst = visitTypeMethod ();
		cst-> getProtection () = InnerProtection::PROTECTED;
		type-> getMethods ().push_back (cst);
	    } else if (next == Keys::LET) {
		auto cst = visitTypeAlias ();
		cst-> getProtection () = InnerProtection::PROTECTED;
		type-> getAlias ().push_back (cst);
	    } else break;
	    if (!mult) break;
	}
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

    std::vector <Word> Visitor::visitAttributes () {
	std::vector <Word> attrs;
	auto beg = this-> lex.next ();
	if (beg == Token::LACC) {
	    while (true) {
		attrs.push_back (visitIdentifiant ());
		beg = this-> lex.next ({Token::RACC, Token::COMA});
		if (beg == Token::RACC) break;
	    }	    
	} else {
	    this-> lex.rewind ();
	    attrs.push_back (visitIdentifiant ());
	}
	return attrs;
    }
    
    /**
       function := 'def' Identifiant ('(' var (',') var)* ')' )? '(' (var (',' var)*)? ')' (':' type)? '{' block '}'
    */
    Function Visitor::visitFunction () {
	std::vector <Word> attrs;
	auto next = this-> lex.next ();
	if (next == Token::AT) attrs = visitAttributes ();
	else this-> lex.rewind ();
	
	auto ident = visitIdentifiant ();
	bool templates = false, cont = true;
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
		auto constante = visitConstanteSimple ();
		if (constante == NULL)
		    temps.push_back (visitOf (templates, cont));
		else {
		    templates = true;
		    temps.push_back (constante);
		}
		word = this-> lex.next ({Token::RPAR, Token::COMA});		
		if (word == Token::RPAR) break;
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

	Expression type = NULL;
	Word retDeco;
	if (word == Token::ARROW) {
	    retDeco = this-> lex.next ();
	    if (retDeco != Keys::REF && retDeco != Keys::MUTABLE) {
		retDeco = Word::eof ();
		this-> lex.rewind ();
	    } 
	    type = visitLeftOpSimple ();
	} else this-> lex.rewind ();
	
	next = this-> lex.next ();
	if (next == Token::LACC) {
	    next = this-> lex.next ();
	    this-> lex.rewind ();
	    if (next == Keys::PRE || next == Keys::POST || next == Keys::BODY) {
		return visitContract (ident, attrs, type, retDeco, exps, temps, test);
	    } 
	}
	this-> lex.rewind ();
	return new (Z0)  IFunction (ident, attrs, type, retDeco, exps, temps, test, visitBlock ());
    }

    Function Visitor::visitContract (Word ident, std::vector<Word> & attrs, Expression type, Word retDeco, std::vector<Var> & exps, std::vector <Expression> & temps, Expression test) {
	Function func = NULL;
	Block pre = NULL, post = NULL;
	Var var = NULL;
	bool dones [] = {false, false, false};
	while (true) {
	    auto next = this-> lex.next ({Keys::PRE, Keys::BODY, Keys::POST, Token::RACC});
	    if (next == Keys::PRE && !dones [0]) {
		dones [0] = true;
		pre = visitBlock ();
	    } else if (next == Keys::BODY && !dones [1]) {
		dones [1] = true;
		func = new (Z0) IFunction (ident, attrs, type, retDeco, exps, temps, test, visitBlock ());
	    } else if (next == Keys::POST && !dones [2]) {
		dones [2] = true;
		next = this-> lex.next ({Token::LPAR});
		var = visitVar ();
		next = this-> lex.next ({Token::RPAR});
		post = visitBlock ();
	    } else if (next == Token::RACC) {
		if (!func) Ymir::Error::syntaxError (next);
		break;
	    } else {
		Ymir::Error::syntaxError (next);
		break;
	    }
	}
	
	func-> postVar () = var;
	func-> pre () = pre;
	func-> post () = post;
	return func;
    }

    Declaration Visitor::visitProto (Word from, std::string space) {
	bool isVariadic = false;
	auto ident = visitIdentifiant ();
	std::vector <Var> exps;
	auto word = this-> lex.next ({Token::LPAR, Token::COLON});
	if (word == Token::COLON) {
	    auto type = visitLeftOpSimple ();
	    this-> lex.next ({Token::SEMI_COLON});
	    auto ret = new (Z0) IGlobal (ident, type, true);
	    ret-> from = from.getStr ();
	    ret-> space = space;
	    return ret;
	} 
	word = this-> lex.next ();
	if (word != Token::RPAR) {
	    this-> lex.rewind ();
	    while (true) {
		word = this-> lex.next ();
		if (word == Token::TDOT) {
		    isVariadic = true;
		    word = this-> lex.next ({Token::RPAR});
		    break;
		} else this-> lex.rewind ();
		exps.push_back (visitVarDeclaration ());
		word = this-> lex.next ({Token::COMA, Token::RPAR});
		if (word == Token::RPAR) break;
	    }
	}
	word = this-> lex.next ({Token::ARROW, Token::SEMI_COLON});
	Expression type = NULL; Word deco;
	if (word == Token::ARROW) {
	    deco = this-> lex.next ();
	    if (deco != Keys::REF && deco != Keys::MUTABLE) {
		deco = Word::eof ();
		this-> lex.rewind ();
	    } 
	    type = visitLeftOpSimple ();
	    word = this-> lex.next ({Token::SEMI_COLON});
	}
	
	auto ret = new (Z0)  IProto (ident, type, deco, exps, space, isVariadic);
	ret-> from = from.getStr ();
	return ret;
    }

    Word Visitor::visitFromExtern () {
	Word begin = this-> lex.next ();
	while (true) {
	    auto word = this-> lex.next ();
	    if (word == Token::RPAR || word == Token::COMA) { this-> lex.rewind (); break; }	    
	    else begin.setStr (begin.getStr () + word.getStr ());
	}
	if (begin != Keys::CPPLANG && begin != Keys::DLANG && begin != Keys::CLANG) {
	    syntaxError (begin, {Keys::CPPLANG, Keys::DLANG, Keys::CLANG});
	}
	return begin;
    }
    
    std::vector <Declaration> Visitor::visitExternBlock (Word from, std::string space, bool onlyProto) {
	std::vector <Declaration> decls;
	while (true) {	   
	    auto next = this-> lex.next ({Keys::EXTERN, Keys::DEF, Token::RACC});
	    if (next == Keys::EXTERN) decls.push_back (visitProto (from, space));
	    else if (next == Keys::DEF && !onlyProto) {
		auto func = this-> visitFunction ()-> to<IFunction> ();
		func-> externLang () = from.getStr ();
		func-> externLangSpace () = space;
		decls.push_back (func);
	    } else if (next == Token::RACC) break;	    
	    else {
		if (onlyProto)
		    syntaxError (next, {Keys::EXTERN, Token::RACC});
		else syntaxError (next, {Keys::EXTERN, Token::RACC, Keys::DEF});
	    }
	}
	return decls;
    }

    std::vector <Declaration> Visitor::visitExtern () {
	auto begin = this-> lex.next ();
	auto word = this-> lex.next ();
	Word from = Word::eof ();
	std::string space = "";
	if (word == Token::LPAR) {
	    from = this-> visitFromExtern ();
	    word = this-> lex.next ({Token::COMA, Token::RPAR});
	    if (word == Token::COMA) {
		if (from != Keys::DLANG && from != Keys::CPPLANG)
		    syntaxError (word, {Token::RPAR});
		space = visitSpace ();
		this-> lex.next ({Token::RPAR});
	    }
	} else this-> lex.rewind ();
	auto next = this-> lex.next ();
	
	if (next == Token::LACC) {
	    return visitExternBlock (from, space, from != Keys::CLANG);
	} else {
	    this-> lex.rewind ();
	    return {visitProto (from, space)};
	}
    }    
    
    Declaration Visitor::visitExternBl () {
	auto word = this-> lex.next ();
	Word from = Word::eof ();
	std::string space = "";
	if (word == Token::LPAR) {
	    from = visitFromExtern ();
	    word = this-> lex.next ({Token::COMA, Token::RPAR});
	    if (word == Token::COMA) {
		if (from != Keys::DLANG && from != Keys::CPPLANG)
		    syntaxError (word, {Token::RPAR});
		space = visitSpace ();
		this-> lex.next ({Token::RPAR});
	    }
	} else this-> lex.rewind ();
	return visitProto (from, space);
    }

    /**
       var := type; 
    */
    Var Visitor::visitVar () {
	return visitType ();
    }

    Expression Visitor::visitOf (bool & temps, bool & cont) {
	auto deco = this-> lex.next ();
	if (deco != Keys::CONST && deco != Keys::REF) {
	    this-> lex.rewind ();
	    deco = Word::eof ();
	}
	
	auto ident = visitIdentifiant ();
	Word next = this-> lex.next ();
	if (next == Token::COLON) {
	    next = this-> lex.next ();
	    if (next == Token::TDOT) {
		temps = true;
		cont = false;
		return new (Z0) IVariadicVar (ident, true);
	    } else {
		this-> lex.rewind ();
		auto type = visitLeftOpSimple ();
		return new (Z0)  ITypedVar (ident, type, deco);
	    }
	} else if ((next == Keys::OF || next == Keys::TRAIT) && deco.isEof ()) {
	    temps = true;
	    auto type = visitLeftOpSimple ();
	    return new (Z0)  IOfVar (ident, type, next == Keys::TRAIT);	    
	} else if (next == Token::TDOT) {
	    cont = false;
	    temps = true;
	    return new (Z0)  IVariadicVar (ident);
	} else this-> lex.rewind ();
	return new (Z0)  IVar (ident, deco);
    }
        
    /**
       vardecl := var (':' type)?
    */
    Var Visitor::visitVarDeclaration () {
	auto deco = this-> lex.next ();
	if (deco != Keys::CONST && deco != Keys::REF) {
	    this-> lex.rewind ();
	    deco = Word::eof ();
	}
	auto ident = visitIdentifiant ();
	Word next = this-> lex.next ();
	if (next == Token::COLON) {
	    auto type = visitLeftOpSimple ();
	    return new (Z0)  ITypedVar (ident, type, deco);
	} else this-> lex.rewind ();
	return new (Z0)  IVar (ident, deco);
    }
    
    TypedVar Visitor::visitStructVarDeclaration () {
	auto ident = visitIdentifiant ();
	Word next  = this-> lex.next ({Token::COLON});
	auto type = visitLeftOpSimple ();
	return new (Z0)  ITypedVar (ident, type, Word::eof ());
    }

    
    TypedVar Visitor::visitTypedVarDeclaration () {
	auto deco = this-> lex.next ();
	if (deco != Keys::CONST && deco != Keys::REF) {
	    this-> lex.rewind ();
	    deco = Word::eof ();
	}

	auto ident = visitIdentifiant ();
	Word next = this-> lex.next ({Token::COLON});
	auto type = visitLeftOpSimple ();
	return new (Z0)  ITypedVar (ident, type, deco);
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

    Var Visitor::visitDecoType (const Word& begin) {
	auto next = this-> lex.next ();
	Expression type = NULL;
	if (next == Token::NOT) {
	    next = this-> lex.next ();
	    if (next == Token::LPAR) {
		type = visitLeftOpSimple ();
		next = this-> lex.next ({Token::RPAR, Token::COMA});
		if (next == Token::COMA) type = visitConstTuple (type);
	    } else {
		this-> lex.rewind ();
		type = visitType ();
	    }
	} else if (next == Token::LPAR) {
	    type = visitLeftOpSimple ();
	    next = this-> lex.next ({Token::RPAR, Token::COMA});
	    if (next == Token::COMA) type = visitConstTuple (type);
	} else {
	    this-> lex.rewind ();
	    type = visitType ();
	}
	return new (Z0)  IVar (begin, {type});
    }
    
    /**
       type := Identifiant ('!' (('(' expression (',' expression)* ')') | expression ) 
    */
    Var Visitor::visitType () {
	auto begin = this-> lex.next ();
	if (begin == Token::LCRO) {
	    auto next = this-> lex.next ();
	    Expression type;
	    if (next == Keys::FUNCTION || next == Keys::DELEGATE) 
		type = visitFuncPtrSimple (next);
	    else { 
		this-> lex.rewind ();
		type = visitType ();
	    }
	    
	    auto end = this-> lex.next ({Token::RCRO, Token::SEMI_COLON});
	    if (end == Token::SEMI_COLON) {
		auto len = visitNumericOrVar ();
		this-> lex.next ({Token::RCRO});
		return new (Z0) IArrayVar (begin, type, len);
	    } else return new (Z0)  IArrayVar (begin, type);
	} else if (begin == Keys::CONST || begin == Keys::MUTABLE)
	    return visitDecoType (begin);	
	else this-> lex.rewind ();
	
	auto ident = visitIdentifiant ();
	auto next = this-> lex.next ();
	if (next == Token::NOT) {
	    if (!(next == Token::NOT)) this-> lex.rewind ();
	    std::vector <Expression> params;
	    next = this-> lex.next ();
	    if (next == Token::LPAR) {
		while (true) {
		    auto ref = this-> lex.next ();
		    if (ref != Keys::REF) {
			this-> lex.rewind ();
			params.push_back (visitExpression ());
		    } else {
			auto type = visitType ();
			type-> deco = ref;
			params.push_back (type);
		    } 
		    next = this-> lex.next ();
		    if (next == Token::RPAR) break;
		    else if (next != Token::COMA) {
			syntaxError (next, {Token::RPAR, Token::COMA});
			break;
		    }
		}
	    } else if (next != Token::LCRO && next != Keys::IS) {
		this-> lex.rewind ();
		auto constante = visitConstanteSimple ();
		if (constante != NULL) 
		    params.push_back (constante);
		else {
		    auto ident_ = visitIdentifiant ();
		    params.push_back (new (Z0)  IVar (ident_));
		}
	    } else this-> lex.rewind (2);
	    return new (Z0)  IVar (ident, params);
	} else this-> lex.rewind ();
	return new (Z0)  IVar (ident);
    }

        
    /**
       Identifiant := ('_')* ([a-z]|[A-Z]) ([a-z]|[A-Z]|'_'|[0-9])*
    */
    Word Visitor::visitIdentifiant () {
	auto ident = this-> lex.next ();
	/*if (ident.isEof () && this-> lex.isMixinContext ())
	  return Word.eof ();*/

	if (ident.isToken ()) {
	    syntaxError (ident, {"'Identifier'"});
	    return Word::eof ();
	}
	
	if (std::find (this-> forbiddenIds.begin (), this-> forbiddenIds.end (), ident.getStr ()) != this-> forbiddenIds.end ()) {
	    syntaxError (ident, {"'Identifier'"});
	    return Word::eof ();
	}
	
	if (ident.getStr ().length () == 0) {
	    syntaxError (ident, {"'Identifier'"});
	    return Word::eof ();
	}
	
	auto i = 0;
	for (auto it : ident.getStr ()) {
	    if ((it >= 'a' && it <= 'z') || (it >= 'A' && it <= 'Z')) break;
	    else if (it != '_')  {
		syntaxError (ident, {"'identifier'"});
		return Word::eof ();
	    }
	    i++;
	}
	i++;
	if (((int) ident.getStr ().length ()) < i) {
	    syntaxError (ident, {"'Identifiant'"});
	    return Word::eof ();
	}
	
	for (auto it : ident.getStr ().substr (i, ident.getStr ().length ())) {
	    if ((it < 'a' || it > 'z')
		&& (it < 'A' || it > 'Z')
		&& (it != '_')
		&& (it < '0' || it > '9')) {
		syntaxError (ident, {"'Identifiant'"});
		return Word::eof ();
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
		if (next == Keys::IMPORT) decls.push_back (visitImport ());
		else if (next == Keys::USE) decls.push_back (visitUse ());
		else if (next == Keys::EXTERN) decls.push_back (visitExternBl ());
		else if (next == Keys::STRUCT) decls.push_back (visitStruct ());
		else if (next == Keys::UNION) decls.push_back (visitStruct (true));
		else if (next == Keys::ALIAS) decls.push_back (visitAlias ());
		else if (next == Keys::TRAIT) decls.push_back (visitTrait ());
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
	    return new (Z0)  IBlock (begin, decls, insts);
	} else this-> lex.rewind ();
	return new (Z0)  IBlock (begin, {}, {visitInstruction ()});
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
	else if (tok == Keys::PRAGMA) return visitPragma ();
	else if (tok == Keys::SCOPE) return visitScope ();
	else if (tok == Keys::VERSION) return visitVersion ();
	else if (tok == Keys::IMMUTABLE) {
	    tok = this-> lex.next ();
	    Instruction inst;
	    if (tok == Keys::IF) inst = visitIf ();
	    else if (tok == Keys::FOR) inst = visitFor ();
	    else if (tok == Keys::ASSERT) inst = visitAssert ();
	    else {
		syntaxError (tok, {Keys::IF, Keys::ASSERT, Keys::FOR});
		return NULL;
	    }
	    inst-> setStatic (true);
	    return inst;
	}
	else if (tok == Token::SEMI_COLON) {
	    Ymir::Error::lineInstructionWarn (tok);
	    return new (Z0)  INone (tok);
	} else {
	    this-> lex.rewind ();
	    auto retour = (Instruction) visitExpressionUlt ();
	    if (!retour-> is<IMatch> ()) {
		this-> lex.next ({Token::SEMI_COLON});	    
	    }
	    return retour;
	}	
    }

    Instruction Visitor::visitVersion () {
	auto ver = this-> visitIdentifiant ();
	if (Version::isOn (ver.getStr ())) {
	    auto bl = this-> visitBlock ();
	    auto next = this-> lex.next ();
	    if (next == Keys::ELSE) {
		this-> lex.next ({Token::LACC});
		auto nb = 1;
		while (nb != 0) {
		    auto next = this-> lex.next ();
		    if (next == Token::RACC) nb --;
		    else if (next == Token::LACC) nb ++;		
		    else if (next.isEof ())
			syntaxError (next);		    
		}
	    } else this-> lex.rewind ();
	    return bl;
	} else {
	    this-> lex.next ({Token::LACC});
	    auto nb = 1;
	    while (nb != 0) {
		auto next = this-> lex.next ();
		if (next == Token::RACC) nb --;
		else if (next == Token::LACC) nb ++;		
		else if (next.isEof ()) syntaxError (next);
	    }

	    auto next = this-> lex.next ();
	    if (next == Keys::ELSE) {
		return this-> visitBlock ();
	    } else {
		this-> lex.rewind ();
		return {};
	    }
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
	return new (Z0)  IVarDecl (tok, decos, decls, insts);
    }
   
    Instruction Visitor::visitLetDestruct (const Word& begin) {
	bool isVariadic = false;
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
	return new (Z0)  ITupleDest (begin, isVariadic, decls, right);
    }    
    
    Expression Visitor::visitExpressionOutSide () {
	return visitExpressionUlt ();
    }

    Expression Visitor::visitExpressionUlt () {
	auto left = visitExpression ();
	auto tok = this-> lex.next ();
	if (find (ultimeOp, tok)) {
	    auto right = visitExpressionUlt ();
	    return visitExpressionUlt (new (Z0)  IBinary (tok, left, right));
	} else this-> lex.rewind ();
	return left;
    }    

    Expression Visitor::visitExpressionUlt (Expression left) {
	auto tok = this-> lex.next ();
	if (find (ultimeOp, tok)) {
	    auto right = visitExpressionUlt ();
	    return visitExpressionUlt (new (Z0)  IBinary (tok, left, right));
	} else this-> lex.rewind ();
	return left;
    }
    
    Expression Visitor::visitExpression () {
	auto left = visitUlow ();
	auto tok = this-> lex.next ();
	if (find (expOp, tok)) {
	    auto ctype = visitAutoCaster ();
	    auto right = visitUlow ();
	    return visitExpression (new (Z0)  IBinary (tok, left, right, ctype));
	} else this-> lex.rewind ();
	return left;
    }

    Expression Visitor::visitExpression (Expression left) {
	auto tok = this-> lex.next ();
	if (find (expOp, tok)) {
	    auto ctype = visitAutoCaster ();	    
	    auto right = visitUlow ();
	    return visitExpression (new (Z0)  IBinary (tok, left, right, ctype));
	} else this-> lex.rewind ();
	return left;
    }
    
    Expression Visitor::visitUlow () {
	auto left = visitLow ();
	auto tok = this-> lex.next ();
	if (find (ulowOp, tok) || tok == Keys::IS) {
	    auto ctype = visitAutoCaster ();
	    auto right = visitLow ();
	    return visitUlow (new (Z0)  IBinary (tok, left, right, ctype));
	} else {
	    if (tok == Token::NOT) {
		auto suite = this-> lex.next ();
		if (suite == Keys::IS) {
		    auto ctype = visitAutoCaster ();
		    auto right = visitLow ();
		    tok.setStr (Keys::NOT_IS);
		    return visitUlow (new (Z0)  IBinary (tok, left, right, ctype));
		} else this-> lex.rewind ();
	    }
	    this-> lex.rewind ();
	}
	return left;
    }

    Expression Visitor::visitUlow (Expression left) {
	auto tok = this-> lex.next ();
	if (find (ulowOp, tok) || tok == Keys::IS) {
	    auto ctype = visitAutoCaster ();
	    auto right = visitLow ();	    
	    return visitUlow (new (Z0)  IBinary (tok, left, right, ctype));
	} else {
	    if (tok == Token::NOT) {
		auto suite = this-> lex.next ();
		if (suite == Keys::IS) {
		    auto ctype = visitAutoCaster ();
		    auto right = visitLow ();
		    tok.setStr (Keys::NOT_IS);
		    return visitUlow (new (Z0)  IBinary (tok, left, right, ctype));
		} else this-> lex.rewind ();
	    }
	    this-> lex.rewind ();
	}
	return left;
    }

    Expression Visitor::visitLow () {
	auto left = visitHigh ();
	auto tok = this-> lex.next ();
	if (find (lowOp, tok)) {
	    auto ctype = visitAutoCaster ();
	    auto right = visitHigh ();
	    return visitLow (new (Z0)  IBinary (tok, left, right, ctype));
	} else this-> lex.rewind ();
	return left;
    }

    Expression Visitor::visitLow (Expression left) {
	auto tok = this-> lex.next ();
	if (find (lowOp, tok)) {
	    auto ctype = visitAutoCaster ();
	    auto right = visitHigh ();
	    return visitLow (new (Z0)  IBinary (tok, left, right, ctype));
	} else this-> lex.rewind ();
	return left;
    }

    Expression Visitor::visitHigh () {
    	auto left = visitPth ();
    	auto tok = this-> lex.next ();
    	if (find (highOp, tok)) {
	    auto ctype = visitAutoCaster ();
    	    auto right = visitPth ();
    	    return visitHigh (new (Z0)  IBinary (tok, left, right, ctype));
    	} else if (tok == Keys::IN) {
	    auto ctype = visitAutoCaster ();
	    auto right = visitPth ();
	    return visitHigh (new (Z0)  IBinary (tok, left, right, ctype));
	} else this-> lex.rewind ();
    	return left;
    }

    Expression Visitor::visitHigh (Expression left) {
	auto tok = this-> lex.next ();
	if (find (highOp, tok)) {
	    auto ctype = visitAutoCaster ();
	    auto right = visitPth ();
	    return visitHigh (new (Z0)  IBinary (tok, left, right, ctype));
	} else if (tok == Keys::IN) {
	    auto ctype = visitAutoCaster ();
	    auto right = visitPth ();
	    return visitHigh (new (Z0)  IBinary (tok, left, right, ctype));
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
	    else if (tok == Token::LACC) {
		this-> lex.rewind ();
		return visitBlock ();
	    } else return visitPthWPar (tok);
	}
    }

    Expression Visitor::visitPthPar (const Word& token) {
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
		    auto save = this-> lambdaPossible;
		    this-> lambdaPossible = true;
		    params.push_back (visitExpressionUlt ());
		    this-> lambdaPossible = save;
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
	    if ((next == Token::DARROW || next == Token::LACC) && this-> lambdaPossible) {
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
			return new (Z0)  ILambdaFunc (tok, realParams, visitExpressionUlt ());
		    } else {
			this-> lex.rewind ();
			return new (Z0)  ILambdaFunc (tok, realParams, visitBlock ());
		    }
		} else this-> lex.rewind ();
	    } else if (isLambda) {
		syntaxError (next, {Token::LACC, Token::DARROW});
	    } else this-> lex.rewind ();
	}
		
	if (params.size () != 1 || isTuple) exp = new (Z0)  IConstTuple (token, tok, params);
	else exp = params [0];
	
	tok = this-> lex.next ();
	if (find (suiteElem, tok)) {
	    return visitSuite (tok, exp);
	} else if (find (afUnary, tok)) {
	    return visitAfter (tok, exp);
	} else this-> lex.rewind ();
	return exp;
    }
    
    Expression Visitor::visitConstTuple () {
	auto begin = this-> lex.rewind ().next ();
	std::vector <Expression> params;
	bool isTuple = false;
	Word tok;
	while (true) {
	    params.push_back (visitExpressionUlt ());
	    tok = this-> lex.next ({Token::RPAR, Token::COMA});
	    if (tok == Token::RPAR) break;
	    else {
		isTuple = true;
		tok = this-> lex.next ();
		if (tok == Token::RPAR) break;
		else this-> lex.rewind ();
	    }	    
	}
	
	if (!isTuple) return params [0];
	else return new (Z0) IConstTuple (begin, tok, params);
    }

    Expression Visitor::visitConstTuple (Expression fst) {
	auto begin = this-> lex.rewind ().next ();
	std::vector <Expression> params;
	params.push_back (fst);
	Word tok;
	while (true) {
	    params.push_back (visitExpressionUlt ());
	    tok = this-> lex.next ({Token::RPAR, Token::COMA});
	    if (tok == Token::RPAR) break;
	}
	
	return new (Z0) IConstTuple (begin, tok, params);
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
	    return new (Z0)  IBool (tok);
	else if (tok == Keys::NULL_)
	    return new (Z0)  INull (tok);
	else if (tok == Keys::EXPAND)
	    return visitExpand ();
	else if (tok == Keys::IS) 
	    return visitIs ();
	else if (tok == Keys::TYPEOF)
	    return visitTypeOf ();
	else if (tok == Keys::UNDER)	    
	    return new (Z0)  IIgnore (tok);
	else if (tok == Keys::STRINGOF) 
	    return visitStringOf ();
	else if (tok == Keys::REF) {
	    return visitLambdaMove ();
	} else this-> lex.rewind ();
	return NULL;
    }

    Expression Visitor::visitConstanteSimple () {       
	auto tok = this-> lex.next ();
	if (tok.isEof ()) return NULL;
	if (tok.getStr () [0] >= '0'&& tok.getStr () [0] <= '9')
	    return visitNumeric (tok, false);
	else if (tok == Token::APOS || tok == Token::GUILL || tok == Token::BSTRING)
	    return visitString (tok);
	else if (tok == Keys::TRUE_ || tok == Keys::FALSE_)
	    return new (Z0)  IBool (tok);
	else if (tok == Keys::NULL_)
	    return new (Z0)  INull (tok);
	else if (tok == Keys::EXPAND)
	    return visitExpand ();
	else if (tok == Keys::IS) 
	    return visitIs ();
	else if (tok == Keys::TYPEOF)
	    return visitTypeOf ();
	else if (tok == Keys::STRINGOF)
	    return visitStringOf ();
	else this-> lex.rewind ();
	return NULL;
    }
        
    Expression Visitor::visitExpand () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ({Token::LPAR});
	auto expr = visitExpression ();
	next = this-> lex.next ({Token::RPAR});
	return new (Z0)  IExpand (begin, expr);
    }

    Expression Visitor::visitTypeOf () {
	bool mut = false;
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ({Token::LPAR});
	next = this-> lex.next ();
	if (next == Keys::MUTABLE) mut = true;
	else this-> lex.rewind ();
	auto expr = visitExpression ();
	next = this-> lex.next ({Token::RPAR});
	return new (Z0)  ITypeOf (begin, expr, mut);
    }

    Expression Visitor::visitStringOf () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ({Token::LPAR});
	auto expr = visitExpression ();
	next = this-> lex.next ({Token::RPAR});
	return new (Z0) IStringOf (begin, expr);
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
	if (next == Keys::FUNCTION || next == Keys::STRUCT || next == Keys::TUPLE || next == Keys::DELEGATE) {
	    auto expType = next;
	    next = this-> lex.next ({Token::RPAR});
	    return new (Z0)  IIs (begin, expr, expType);
	} else {
	    this-> lex.rewind ();
	    auto type = visitLeftOpSimple ();
	    next = this-> lex.next ({Token::COMA,Token::RPAR});
	    if (next == Token::COMA) {
		std::vector <Expression> temps;
		while (true) {
		    bool templates = true, cont = true;
		    temps.push_back (visitOf (templates, cont));
		    next = this-> lex.next ({Token::RPAR, Token::COMA});
		    if (next == Token::RPAR) break;
		}
		return new (Z0) IIs (begin, expr, type, temps);
	    }
	    return new (Z0)  IIs (begin, expr, type, {});
	}
    }

    Expression Visitor::visitNumericOrVar () {
	auto tok = this-> lex.next ();
	if (tok.isEof ()) return NULL;
	if (tok.getStr () [0] >= '0'&& tok.getStr () [0] <= '9')
	    return visitNumeric (tok, false);
	else this-> lex.rewind ();
	return visitVar ();
    }

    Expression Visitor::visitNumericHexa (const Word & begin) {
	auto val = begin.getStr ().substr (2);
	for (auto it : Ymir::r (0, val.size ())) {
	    if ((val [it] < '0' || val [it] > '9') && (val [it] < 'A' || val [it] > 'F') && (val [it] < 'a' || val [it] > 'f') && val [it] != Keys::UNDER [0]) {
		if (val.substr (it) == "ub" || val.substr (it) == "UB")
		    return new (Z0) IFixed ({begin.getLocus (), val.substr (0, it)}, FixedConst::UBYTE, FixedMode::HEXA);
		else if (val.substr (it) == "us" || val.substr (it) == "US")
		    return new (Z0) IFixed ({begin.getLocus (), val.substr (0, it)}, FixedConst::USHORT, FixedMode::HEXA);
		else if (val.substr (it) == "s" || val.substr (it) == "S")
		    return new (Z0) IFixed ({begin.getLocus (), val.substr (0, it)}, FixedConst::SHORT, FixedMode::HEXA);
		else if (val.substr (it) == "u" || val.substr (it) == "U")
		    return new (Z0) IFixed ({begin.getLocus (), val.substr (0, it)}, FixedConst::UINT, FixedMode::HEXA);
		else if (val.substr (it) == "uL" || val.substr (it) == "UL")
		    return new (Z0) IFixed ({begin.getLocus (), val.substr (0, it)}, FixedConst::ULONG, FixedMode::HEXA);
		else if (val.substr (it) == "l" || val.substr (it) == "L")
		    return new (Z0) IFixed ({begin.getLocus (), val.substr (0, it)}, FixedConst::LONG, FixedMode::HEXA);
		else {
		    syntaxError (begin);
		    return NULL;
		}
	    } else if ((val.substr (it) == "_b" || val.substr (it) == "_B") && it == (int) val.size () - 2)
		return new (Z0) IFixed ({begin.getLocus (), val.substr (0, it)}, FixedConst::BYTE, FixedMode::HEXA);
	}
	return new (Z0) IFixed ({begin.getLocus (), val}, FixedConst::INT, FixedMode::HEXA);
    }
    
    Expression Visitor::visitNumeric (const Word& begin, bool abrev) {
	if (begin.getStr ().size () > 2 && begin.getStr () [0] == '0' && begin.getStr () [1] == Keys::LX [0]) {
	    return visitNumericHexa (begin);
	}
	
	for (int it = 0 ; it < (int) begin.getStr ().length (); it++) {
	    if ((begin.getStr () [it] < '0' || begin.getStr() [it] > '9') && begin.getStr ()[it] != Keys::UNDER [0]) {		
		if (begin.getStr () .substr (it, begin.getStr ().length () - it) == "ub" || begin.getStr () .substr (it, begin.getStr ().length () - it) == "UB")
		    return new (Z0)  IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::UBYTE, FixedMode::DECIMAL);
		else if (begin.getStr () .substr (it, begin.getStr ().length () - it) == "b" || begin.getStr () .substr (it, begin.getStr ().length () - it) == "B")
		    return new (Z0)  IFixed ({begin.getLocus (), begin.getStr ().substr (0, it)}, FixedConst::BYTE, FixedMode::DECIMAL);
		else if (begin.getStr () .substr (it, begin.getStr ().length () - it) == "s" || begin.getStr () .substr (it, begin.getStr ().length () - it) == "S")
		    return new (Z0)  IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::SHORT, FixedMode::DECIMAL);
		else if (begin.getStr () .substr (it, begin.getStr ().length () - it) == "us" || begin.getStr () .substr (it, begin.getStr ().length () - it) == "US")
		    return new (Z0)  IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::USHORT, FixedMode::DECIMAL);
		else if (begin.getStr () .substr (it, begin.getStr ().length () - it) == "u" || begin.getStr () .substr (it, begin.getStr ().length () - it) == "U")
		    return new (Z0)  IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::UINT, FixedMode::DECIMAL);
		else if (begin.getStr () .substr (it, begin.getStr ().length () - it) == "ul" || begin.getStr () .substr (it, begin.getStr ().length () - it) == "UL")
		    return new (Z0)  IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::ULONG, FixedMode::DECIMAL);
		else if (begin.getStr () .substr (it, begin.getStr ().length () - it) == "l" || begin.getStr () .substr (it, begin.getStr ().length () - it) == "L")
		    return new (Z0)  IFixed ({begin.getLocus (), begin.getStr () .substr (0, it)}, FixedConst::LONG, FixedMode::DECIMAL);
		else {
		    syntaxError (begin);
		    return NULL;
		}
	    }
	}
	
	if (abrev) {
	    auto next = this->lex.next ();
	    if (next == Token::DOT) {
		next = this-> lex.next ();
		auto suite = next.getStr ();
		FloatConst type = FloatConst::DOUBLE;
		for (auto it : Ymir::r (0, next.getStr ().length ())) {		
		    if ((next.getStr () [it] < '0' || next.getStr () [it] > '9') && next.getStr ()[it] != Keys::UNDER [0]) {
			if (next.getStr ().substr (it, next.getStr ().length () - it) == "f" ||
			    next.getStr ().substr (it, next.getStr ().length () - it) == "F") {
			    type = FloatConst::FLOAT;
			    return new (Z0) IFloat (begin, next.getStr ().substr (0, it), type);
			    break;
			} else {
			    suite = "0";
			    this-> lex.rewind ();
			    break;
			}
		    }		    
		}
		return new (Z0)  IFloat (begin, suite, type);
	    } else this-> lex.rewind ();
	}
	return new (Z0)  IFixed (begin, FixedConst::INT, FixedMode::DECIMAL);
    }    
    
    Expression Visitor::visitFloat (const Word&) {
	auto next = this-> lex.next ();
	FloatConst type = FloatConst::DOUBLE;
	for (auto it : Ymir::r (0, next.getStr ().length ())) {
	    if ((next.getStr () [it] < '0' || next.getStr () [it] > '9') && next.getStr () [it] != Keys::UNDER [0]) {
		if (next.getStr ().substr (it, next.getStr ().length () - it) == "f" ||
		    next.getStr ().substr (it, next.getStr ().length () - it) == "F") {
		    type = FloatConst::FLOAT;
		    return new (Z0) IFloat ({next.getLocus (), next.getStr ().substr (0, it)}, type);
		
		} else {
		    syntaxError (next);
		}
	    }
	}
	return new (Z0)  IFloat (next, type);
    }

    Expression Visitor::visitString (Word& word) {
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
		return new (Z0)  IChar (word, (ubyte) res [0]);
	}
	
	return new (Z0)  IString (word, res);
    }

    Expression Visitor::visitPthWPar (Word& tok) {
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
    

    //ICI
    Expression Visitor::visitLeftOp () {
	auto word = this-> lex.next ();
	if (word == Keys::CAST) {
	    return visitCast ();
	} else if (word == Token::LCRO) {
	    return visitConstArray ();
	} else if (word == Keys::FUNCTION || word == Keys::DELEGATE) {
	    return visitFuncPtr (word);
	} else if (word == Keys::MIXIN) {
	    return visitMixin ();
	} else  if (word == Keys::MATCH) {
	    return visitMatch ();
	} else if (word == Keys::PRAGMA) {
	    return visitPragma ();
	} else this-> lex.rewind ();
	auto var = visitVar ();
	auto next = this-> lex.next ();
	if (find (suiteElem, next)) 
	    return visitSuite (next, var);
	else this-> lex.rewind ();
	return var;
    }

    Expression Visitor::visitLeftOpSimple () {

	struct Inner {
	    Visitor * self;
	    bool last, lastM;
	    
	    Inner (Visitor* visit) : self (visit) {
		last = self-> lambdaPossible;
		lastM = self-> isInMatch;
		self-> lambdaPossible = false;
		self-> isInMatch = false;
	    }
	    
	    Expression call () {
		auto word = self-> lex.next ();
		if (word == Token::LCRO) {
		    return self-> visitConstArray ();
		} else if (word == Token::LPAR) {
		    return self-> visitConstTuple ();
		} else if (word == Keys::FUNCTION || word == Keys::DELEGATE) {
		    return self-> visitFuncPtr (word);
		} else self-> lex.rewind ();
		auto var = self-> visitVar ();
		auto next = self-> lex.next ();
		if (find (self-> suiteElem, next)) 
		    return self-> visitSuite (next, var);
		else self-> lex.rewind ();
		return var;
	    }

	    ~Inner () {
		self-> lambdaPossible = last;
		self-> isInMatch = lastM;
	    }
	};
	
	Inner in (this);
	return in.call ();
    }
    
    Expression Visitor::visitStructCst (Expression left) {
	this-> lex.rewind ();
	auto beg = this-> lex.next (), next = this-> lex.next ();
	auto suite = next;
	std::vector <Expression> params;
	if (next != Token::RACC) {
	    this-> lex.rewind ();
	    while (true) {
		if (this-> isInMatch) 
		    params.push_back (visitMatchExpression ());
		else
		    params.push_back (visitExpression ());
		
		next = this-> lex.next ({Token::RACC, Token::COMA});
		if (next == Token::RACC) break;
	    }
	}
	
	auto retour = new (Z0) IStructCst (beg, next, left, new (Z0)  IParamList (suite, params));
	next = this-> lex.next ();
	
	if (find (suiteElem, next))
	    return visitSuite (next, retour);
	else if (find (afUnary, next))
	    return visitAfter (next, retour);
	this-> lex.rewind ();
	return retour;
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
		bool isImmutable = true;
		next = this-> lex.next ();
		if (next == Keys::NEW) isImmutable = false;
		else this-> lex.rewind ();
		auto size = visitExpression ();
		next = this-> lex.next ({Token::RCRO});
		return new (Z0)  IArrayAlloc (begin, fst, size, isImmutable);		
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
	
	auto retour = new (Z0)  IConstArray (begin, params);
	auto next = this-> lex.next ();
	if (find (suiteElem, next))
	    return visitSuite (next, retour);
	else if (find (afUnary, next))
	    return visitAfter (next, retour);
	this-> lex.rewind ();
	return retour;
    }

        
    /**
     cast := 'cast' ':' type '(' expression ')'
     */
    Expression Visitor::visitCast () {
	this-> lex.rewind ();
	Expression type;
	auto begin = this-> lex.next ();
	auto word = this-> lex.next ({Token::NOT});
	auto next = this-> lex.next ();
	if (next == Keys::FUNCTION || next == Keys::DELEGATE) {
	    type = visitFuncPtrSimple (next);	    
	} else {
	    bool needClose = false;
	    if (next == Token::LPAR) 
		needClose = true;
	    else 
		this-> lex.rewind ();
	    
	    type = visitType ();
	    if (needClose) this-> lex.next ({Token::RPAR});
	}
	
	word = this-> lex.next ({Token::LPAR});
	auto expr = visitExpression ();
	word = this-> lex.next ({Token::RPAR});
	return new (Z0)  ICast (begin, type, expr);	
    }
    

    Expression Visitor::visitFuncPtrSimple (const Word & type) {
	std::vector <Expression> params;
	auto word = this-> lex.next ({Token::LPAR});
	word = this-> lex.next ();
	if (word != Token::RPAR) {
	    this-> lex.rewind ();
	    while (true) {
		bool ref = false;
		word = this-> lex.next ();
		if (word == Keys::REF) ref = true;
		else this-> lex.rewind ();
		Expression type;
		if (ref) {
		    auto var = visitType ();
		    var-> deco = word;
		    type = var;		    
		} else type = visitLeftOpSimple ();
		
		params.push_back (type);
		word = this-> lex.next ({Token::RPAR, Token::COMA});
		if (word == Token::RPAR) break;
	    }	    
	}
	word = this-> lex.next ({Token::ARROW});
	auto ret = visitLeftOpSimple ();
	return new (Z0)  IFuncPtr (type, params, ret);
    }

    Expression Visitor::visitFuncPtr (const Word & begin) {
	std::vector <Expression> params;
	auto word = this-> lex.next ({Token::LPAR});
	word = this-> lex.next ();
	if (word != Token::RPAR) {
	    this-> lex.rewind ();
	    while (true) {
		bool ref = false;
		word = this-> lex.next ();
		if (word == Keys::REF) ref = true;
		else this-> lex.rewind ();
		Expression type;
		if (ref) {
		    auto var = visitType ();
		    var-> deco = word;
		    type = var;		    
		} else type = visitLeftOpSimple ();
		
		params.push_back (type);
		word = this-> lex.next ({Token::RPAR, Token::COMA});
		if (word == Token::RPAR) break;
	    }	    
	}
	
	word = this-> lex.next ({Token::ARROW});
	auto ret = visitLeftOpSimple ();
	// word = this-> lex.next ();
	// if (word == Token::LACC) {
	//     auto expr = visitExpression ();
	//     word = this-> lex.next ({Token::RACC});
	//     return new (Z0)  IFuncPtr (begin, params, ret, expr);
	// } else this-> lex.rewind ();
	return new (Z0)  IFuncPtr (begin, params, ret);
    }

    Expression Visitor::visitLambdaEmpty () {
	auto next = this-> lex.next ({Token::DARROW, Token::LACC});
	if (next == Token::DARROW) {
	    auto expr = visitExpressionUlt ();
	    return new (Z0)  ILambdaFunc (next, {}, expr);
	} else if (next == Token::LACC) {
	    this-> lex.rewind ();
	    return new (Z0)  ILambdaFunc (next, {}, visitBlock ());
	} else return NULL;
    }
    
    Expression Visitor::visitLambdaMove () {
	std::vector <Var> params;
	auto begin = this-> lex.next ({Token::LPAR});
	auto next = this-> lex.next ();
	if (next != Token::RPAR) {
	    this-> lex.rewind ();
	    while (true) {
		params.push_back (visitVarDeclaration ());
		auto next = this-> lex.next ({Token::RPAR, Token::COMA});
		if (next == Token::RPAR) break;
	    }
	}
	
	LambdaFunc lambda;
	next = this-> lex.next ({Token::DARROW, Token::LACC});
	if (next == Token::DARROW) {
	    auto expr = visitExpressionUlt ();
	    lambda = new (Z0)  ILambdaFunc (begin, params, expr);
	} else if (next == Token::LACC) {
	    this-> lex.rewind ();
	    auto block = visitBlock ();
	    lambda = new (Z0)  ILambdaFunc (begin, params, block);
	} else return NULL;
	
	lambda-> isMoved () = false;
	return lambda;
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
	    return new (Z0)  ILambdaFunc (begin, params, expr);
	} else if (next == Token::LACC) {
	    this-> lex.rewind ();
	    auto block = visitBlock ();
	    return new (Z0)  ILambdaFunc (begin, params, block);
	} else return NULL;
    }    
    
    
    Expression Visitor::visitSuite (const Word& token, Expression left) {
	if (token == Token::LPAR) return visitPar (left);
	else if (token == Token::LCRO) return visitAccess (left);
	else if (token == Token::DOT) return visitDot (left);
	else if (token == Token::DCOLON) return visitDColon (left);
	else if (token == Token::COLON) {
	    if (this-> lambdaPossible || this-> isInMatch) {
		return visitMacroCall (left);
	    } else {
		this-> lex.rewind ();
		return left;
	    }
	} else if (token == Token::LACC) {
	    if (this-> lambdaPossible || this-> isInMatch) {
		return visitStructCst (left);
	    } else {
		this-> lex.rewind ();
		return left;
	    }
	}
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
	auto retour = new (Z0)  IPar (beg, next, left, new (Z0)  IParamList (suite, params));
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
	
	auto retour = new (Z0)  IAccess (beg, next, left, new (Z0)  IParamList (suite, params));
	next = this-> lex.next ();
	if (find (suiteElem, next))
	    return visitSuite (next, retour);
	else if (find (afUnary, next))
	    return visitAfter (next, retour);
	this-> lex.rewind ();
	return retour;
    }

    Expression Visitor::visitMacroCall (Expression left) {
	auto beg = this-> lex.next ({Token::LCRO, Token::LACC, Token::LPAR});
	std::string tokEnd = beg == Token::LCRO ? Token::RCRO : (beg == Token::LACC ? Token::RACC : Token::RPAR);
	this-> lex.commentEnable (false);
	this-> lex.skipEnable ({Token::SPACE, Token::RETURN, Token::RRETURN, Token::TAB}, false);       

	auto open = 1;
	std::vector <Word> tok;
	while (open != 0) {
	    tok.push_back (this-> lex.next ());
	    if (tok.back () == tokEnd) open--;
	    else if (tok.back () == beg) open ++;
	    else if (tok.back ().isEof ()) unterminated (beg);	    
	}
	
	this-> lex.commentEnable (true);
	this-> lex.skipEnable ({Token::SPACE, Token::RETURN, Token::RRETURN, Token::TAB}, true);       
	auto end = tok.back ();
	tok.pop_back ();
	auto retour = new (Z0) IMacroCall (beg, end, left, tok);
	
	auto next = this-> lex.next ();	
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
	Expression right = visitConstanteSimple ();
	if (right == NULL) right = visitVar ();
	auto retour = new (Z0)  IDot (begin, left, right);
	auto next = this-> lex.next ();
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
	auto next = this-> lex.next ();
	Expression retour;
	if (next == Keys::EXPAND) {
	    retour = new (Z0)  IExpand (next, left);
	} else if (next == Keys::TYPEOF) {
	    retour = new (Z0)  ITypeOf (next, left);
	} else if (next == Keys::STRINGOF) {
	    retour = new (Z0) IStringOf (next, left);
	} else {
	    this-> lex.rewind ();
	    auto right = visitVar ();
	    retour = new (Z0)  IDColon (begin, left, right);
	}
	
	next = this-> lex.next ();	
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
	return new (Z0)  IMixin (begin, expr);
    }

    Expression Visitor::visitMatch () {
	std::vector <Expression> values;
	std::vector <Block> insts;

	this-> lambdaPossible = false;
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto expr = visitExpression ();
	
	auto next = this-> lex.next ({Token::LACC});
	
	this-> isInMatch = true;
	while (true) {
	    values.push_back (visitMatchExpression ());
	    next = this-> lex.next ({Token::DARROW});
	    
	    this-> lambdaPossible = true;
	    this-> isInMatch = false;
	    insts.push_back (visitBlock ());
	    this-> lambdaPossible = false;
	    this-> isInMatch = true;
	    next = this-> lex.next ();
	    if (next == Token::RACC) break;
	    else this-> lex.rewind ();
	}
	
	this-> isInMatch = false;
	this-> lambdaPossible = true;
	return new (Z0)  IMatch (begin, expr, values, insts);
    }

    Expression Visitor::visitMatchExpression () {
	auto begin = this-> lex.next ();
	if (begin == Token::LPAR) {
	    Word tok;
	    std::vector <Expression> params;
	    while (true) {
		params.push_back (visitMatchExpression ());
		auto next = this-> lex.next ({Token::RPAR, Token::COMA});
		if (next == Token::RPAR) { tok = next; break; }
	    }
	    return new (Z0) IConstTuple (begin, tok, params);
	} else {
	    this-> lex.rewind ();
	    if (canVisitVarDeclaration ()) {
		return visitVarDeclaration ();
	    } else {
		if (begin == Keys::REF) {
		    this-> lex.next ();
		    auto var = new (Z0) IVar (visitIdentifiant ());
		    var-> deco = begin;
		    return var;
		}

		auto expr = visitExpression ();
		auto next = this-> lex.next ();
		if (next == Token::TDOT) {
		    auto res = visitExpression ();
		    return new (Z0) IMatchPair (next, expr, res);
		} else this-> lex.rewind ();
		return expr;
	    }
	}
    }    
    
    Expression Visitor::visitAfter (const Word& word, Expression left) {
	return new (Z0)  IUnary (word, left);
    }
    
    Expression Visitor::visitBeforePth (const Word& word) {
	auto elem = visitPth ();
	return new (Z0)  IUnary (word, elem);
    }
        
    Instruction Visitor::visitIf () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	this-> lambdaPossible = false;
	bool needPar = false;
	auto next = this-> lex.next  ();
	if (next == Token::LPAR) needPar = true;	
	else this-> lex.rewind ();
	auto test = visitExpression ();
	this-> lambdaPossible = true;
	if (needPar) this-> lex.next ({Token::RPAR});
	auto block = visitBlock ();
	next = this-> lex.next ();
	if (next == Keys::ELSE) {
	    return new (Z0)  IIf (begin, test, block, visitElse ()); 
	} else this-> lex.rewind ();
	return new (Z0)  IIf (begin, test, block);
    }
        
    If Visitor::visitElse () {
	this-> lex.rewind ();
	auto begin = this-> lex.next (), next = this-> lex.next ();
	if (next == Keys::IF) {
	    this-> lambdaPossible = false;
	    bool needPar = false;
	    auto next = this-> lex.next  ();
	    if (next == Token::LPAR) needPar = true;
	    else this-> lex.rewind ();
	    auto test = visitExpression ();
	    this-> lambdaPossible = true;
	    if (needPar) this-> lex.next ({Token::RPAR});
	    auto block = visitBlock ();
	    next = this-> lex.next ();
	    if (next == Keys::ELSE) {
		return new (Z0)  IIf (begin, test, block, visitElse ());
	    } else this-> lex.rewind ();
	    return new (Z0)  IIf (begin, test, block);
	} else this-> lex.rewind ();
	return new (Z0)  IIf (begin, NULL, visitBlock ());
    }

    Scope Visitor::visitScope () {
	auto begin = this->lex.next ();
	auto next = this-> lex.next ({Token::LACC, Token::DARROW});
	if (next == Token::LACC) this-> lex.rewind ();
	return new (Z0) IScope (begin, visitBlock ());
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
	return new (Z0)  IAssert (begin, expr, msg);
    }

    Pragma Visitor::visitPragma () {
	auto next = this-> lex.next ({Token::LPAR});
	auto id = visitIdentifiant ();
	auto suite = this-> lex.next ();
	std::vector <Expression> params;
	if (suite == Token::COMA) {
	    while (true) {
		params.push_back (visitExpression ());
		next = this-> lex.next ({Token::RPAR, Token::COMA});
		if (next == Token::RPAR) break;
	    }
	}
	
	return new (Z0) IPragma (id, new (Z0) IParamList (suite, params));
    }    
    
    Instruction Visitor::visitReturn () {
	this-> lex.rewind ();
	auto begin = this-> lex.next (), next = this-> lex.next ();
	if (next == Token::SEMI_COLON) 
	    return new (Z0)  IReturn (begin);
	else this-> lex.rewind ();
	auto exp = visitExpression ();
	next = this-> lex.next ({Token::SEMI_COLON});
	return new (Z0)  IReturn (begin, exp);	
    }
    
    
    Instruction Visitor::visitBreak () {
	this-> lex.rewind ();
	auto begin = this-> lex.next (), next = this-> lex.next ();
	if (next == Token::SEMI_COLON) {
	    return new (Z0)  IBreak (begin);
	} else this-> lex.rewind ();
	auto id = visitIdentifiant ();
	next = this-> lex.next ({Token::SEMI_COLON});
	return new (Z0)  IBreak (begin, id);
    }

    Instruction Visitor::visitWhile () {
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ();
	if (next == Token::COLON) {
	    auto id = visitIdentifiant ();
	    next = this-> lex.next ({Token::LPAR});
	    this-> lambdaPossible = false;
	    bool needPar = false;
	    auto next = this-> lex.next ();
	    if (next == Token::LPAR) needPar = true;
	    else this-> lex.rewind ();
	    auto test = visitExpression ();
	    if (needPar) this-> lex.next ({Token::RPAR});
	    this-> lambdaPossible = true;
	    next = this-> lex.next ({Token::RPAR});
	    return new (Z0)  IWhile (begin, id, test, visitBlock ());
	} else {
	    this-> lex.rewind ();
	    this-> lambdaPossible = false;
	    bool needPar = false;
	    auto next = this-> lex.next ();
	    if (next == Token::LPAR) needPar = true;
	    else this-> lex.rewind ();
	    auto test = visitExpression ();
	    if (needPar) this-> lex.next ({Token::RPAR});
	    this-> lambdaPossible = true;
	    return new (Z0)  IWhile (begin, test, visitBlock ());
	}
    }

    Instruction Visitor::visitFor () {	
	this-> lex.rewind ();
	auto begin = this-> lex.next ();
	auto next = this-> lex.next ();
	Word id = Word::eof ();
	this-> lambdaPossible = false;
	bool need = false;
	if (next == Token::COLON) {
	    id = visitIdentifiant ();
	    next = this-> lex.next ({Token::LPAR});
	    need = true;
	} else if (next == Token::LPAR) {
	    need = true;
	} else this-> lex.rewind ();
	std::vector <Var> vars;
	std::vector <bool> _const;
	while (true) {
	    auto next = this-> lex.next ();
	    if (next == Keys::CONST) _const.push_back (true);
	    else {
		_const.push_back (false);
		this-> lex.rewind ();
	    }
	    
	    vars.push_back (visitVar ());	    
	    next = this-> lex.next ({Keys::IN, Token::COMA});
	    if (next == Keys::IN) break;
	}

	auto iter = visitExpression ();
	if (need) {
	    next = this-> lex.next ({Token::RPAR});
	}
	this-> lambdaPossible = true;
	
	return new (Z0)  IFor (begin, id, vars, iter, visitBlock (), _const);
    }       

    Alias Visitor::visitAlias () {
	auto ident = visitIdentifiant ();
	this-> lex.next ({Token::EQUAL});
	auto value = this-> visitExpression ();
	this-> lex.next ({Token::SEMI_COLON});
	return new (Z0) IAlias (ident, value);	
    }

    Expression Visitor::visitAutoCaster () {
	auto next = this-> lex.next ();
	if (next == Token::COLON) {
	    bool need_par = true;
	    next = this-> lex.next ();
	    if (next != Token::LPAR) {
		need_par = false;
		this-> lex.rewind ();
	    }
	    auto expr = this-> visitLeftOpSimple ();
	    if (need_par) this-> lex.next ({Token::RPAR});
	    return expr;
	}
	this-> lex.rewind ();
	return NULL;
    }
    
    TraitProto Visitor::visitTraitProto () {
	auto ident = visitIdentifiant ();
	bool isSelf = false;
	std::vector <Expression> params;
	this-> lex.next ({Token::LPAR});
	auto next = this-> lex.next ();
	if (next == Keys::SELF) {
	    isSelf = true;
	    next = this-> lex.next ({Token::RPAR, Token::COMA});
	}
	if (next != Token::RPAR) {
	    while (true) {
		params.push_back (visitLeftOpSimple ());
		if (this-> lex.next ({Token::COMA, Token::RPAR}) == Token::RPAR) break;
	    }
	}

	Expression type = NULL;
	next = this-> lex.next ({Token::ARROW, Token::SEMI_COLON});
	if (next == Token::ARROW) {
	    type = visitLeftOpSimple ();
	    this-> lex.next ({Token::SEMI_COLON});
	}
	
	return TraitProto {ident, params, isSelf, type};
    }
    
    Trait Visitor::visitTrait () {
	auto ident = visitIdentifiant ();
	this-> lex.next ({Token::LACC});
	std::vector <TypedVar> vars;
	std::vector <TraitProto> protos;
	while (true) {
	    auto next = this-> lex.next ({Token::RACC, Keys::LET, Keys::DEF});
	    if (next == Keys::LET) {
		vars.push_back (visitTypedVarDeclaration ());
		this-> lex.next ({Token::SEMI_COLON});
	    } else if (next == Keys::DEF) protos.push_back (visitTraitProto ());
	    else break;
	}
	return new (Z0) ITrait (ident, protos, vars);
    }
    
};
