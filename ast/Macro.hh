#pragma once

#include "Declaration.hh"
#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace semantic {
    struct MacroSolution;
}

namespace syntax {
    
    /**
     * \enum MacroVarConst
     * Different kind of macro var     
     */
    enum class MacroVarConst {
	EXPR,
	IDENT,
	BLOCK,
	TOKEN
    };

    /**
     * \struct IMacroElement
     * Parent of all Macro element, used to create a macro expression
     */
    class IMacroElement : public IExpression {
    public: 

	/** 
	 * \param ident the location 
	 */
	IMacroElement (Word ident) : IExpression (ident) {}

	virtual Ymir::json generateDocs () = 0;
	
	/**
	 * \brief Convert this element to a succession of tokens
	 * \param success return by reference, the success of this function
	 * \return a list of token, (can be empty if success is returned false)
	 */
	virtual std::vector <Word> toTokens (bool& success);

	/**
	 * \return a independant clone of the element
	 */
	virtual IMacroElement* clone () = 0;

	static const char* id ();
	
	std::vector <std::string> getIds () override;
	
    };
    
    /**
     * \struct IMacroExpr
     * A macro expression is a suite of macro element
     */
    class IMacroExpr {

	/** The list of element composing the expression*/
	std::vector <IMacroElement*> _elements;
	
    public:
	/** 
	 * \param begin the location of the expression
	 * \param the end location of the expression
	 * \param elements the elements composing the expression
	 */
	IMacroExpr (Word begin, Word end, std::vector <IMacroElement*> elements);

	const std::vector <IMacroElement*> & getElements ();

	Ymir::json generateDocs ();
	
    };

    /** 
     * \struct IMacroToken
     * A macro token is a string that must be retreive at macro resolution
     */
    class IMacroToken : public IMacroElement {

	/** The value of the token */
	std::string _value;
	
    public:
	
	/**
	 * \param tok the location 
	 * \param value the value of the token
	 */
	IMacroToken (Word tok, std::string value);
	
	IMacroToken* clone () override;

	Ymir::json generateDocs () override;
	
	std::vector <Word> toTokens (bool& success) override;
	
	const std::string & getValue ();

	Expression expression () override;
	
	Expression templateExpReplace (const std::map <std::string, Expression> &) override;

	std::string prettyPrint () override;
	
	static const char* id ();
	
	std::vector <std::string> getIds () override;
    };

    
    /**
     * \struct IMacroRepeat
     * A macro repeat is a macro expression, that can be repeated multiple times
     * \verbatim
     macro_repeat := '(' macro_expr ',' macro_token ')' ('*' | '+')
     \endverbatim
     */
    class IMacroRepeat : public IMacroElement {

	/** The content */
	IMacroExpr* _content;

	/** 
	 *  This token is the one meaning the repeat must be done\n
	 * Example : 
	 * \verbatim
	 // for the macro_repeat : 
	 // (expression, ",")* 
	 // This will pass the resolution :
	 
	 1, 2, 3
	 \endverbatim
	*/
	IMacroToken * _pass;

	/** Is this expression a '+' or a '*', '+' means we must have this expression a least one time */
	bool _oneTime;

	/** The solution, found at semantic time */
	std::vector <semantic::MacroSolution> _soluce;
	
    public :
	
	/** The identifier of the expression */
	Word ident;

	/**
	 * \param ident the location and identifier of the expression
	 * \param content the content expression 
	 * \param pass the token at each step (cf this-> _pass)
	 * \param atLeastOneTime (cf this-> _oneTime)
	 */
	IMacroRepeat (Word ident, IMacroExpr * content, IMacroToken * pass, bool atLeastOneTime);

	Ymir::json generateDocs () override;
	
	IMacroRepeat* clone () override;

	/** 
	 * \return this-> _pass
	 */
	IMacroToken* getClose ();

	/**
	 * \return the content
	 */
	IMacroExpr* getExpr ();

	std::vector <Word> toTokens (bool& success) override;
	
	void addSolution (semantic::MacroSolution soluce);

	Ymir::Tree toGeneric () override;
	
	Expression expression () override;
	
	std::string prettyPrint () override;
	
	const std::vector <semantic::MacroSolution>& getSolution ();

	Expression templateExpReplace (const std::map <std::string, Expression> &) override;
	
	bool isOneTime ();
	
	static const char* id ();

	std::vector <std::string> getIds () override;
       
    };

    /**
     * \struct IMacroVar
     * The syntaxic node representation of a macro var
     * \verbatim 
     macro_var := Identifier ':' ('expr' | 'block' | 'ident' | 'token')
     \endverbatim
     */
    class IMacroVar : public IMacroElement {

	/** The type of var */
	MacroVarConst _type;

	/** The token if the var take a specifig token */
	IMacroToken * _token;

	/** The content of the var (the solution) */
	Expression _content;
	
    public:
	/** The name of the var */
	Word name;	

	/**
	 * \param name the name and location of the var
	 * \param type the type of var
	 */
	IMacroVar (Word name, MacroVarConst type);

	/**
	 * \param name the name and location of the var
	 * \param token the token
	 */
	IMacroVar (Word name, IMacroToken* token);

	Ymir::json generateDocs () override;
	
	IMacroVar* clone () override;

	std::vector <Word> toTokens (bool& success) override;
	
	Expression templateExpReplace (const std::map <std::string, Expression> &) override;

	Expression expression ();
	
	MacroVarConst getType ();

	IMacroToken* getToken ();
	
	void setContent (Expression content);

	std::string prettyPrint () override;
	
	static const char* id ();

	std::vector <std::string> getIds () override;

    };

    /**
     * \struct IMacroEnum
     * A list of possible expression, one of them is taken for solution 
     * \verbatim
     macro_enum := macro_expr ('|' macro_expr)+
     \endverbatim
     */
    class IMacroEnum : public IMacroElement {
	
	/** The list of macro expression*/
	std::vector <IMacroExpr*> _elems;

	/** The solution */
	semantic::MacroSolution _soluce;
	
    public:

	/**
	 * \param name the location of the expression
	 * \param elems the list of expression
	 */
	IMacroEnum (Word name, std::vector <IMacroExpr*> elems);

	semantic::MacroSolution& getSoluce ();

	Ymir::json generateDocs () override;

	IMacroEnum* clone () override;
	
	std::vector <Word> toTokens (bool& success) override;

	const std::vector <IMacroExpr*> & getElems ();
	
	Expression templateExpReplace (const std::map <std::string, Expression> &) override;

	Expression expression ();

	std::string prettyPrint () override;

	static const char * id ();

	std::vector <std::string> getIds () override;	

    };
    
    /**
     * \struct IMacro
     * The syntaxic node representation of a macro declaration
     */
    class IMacro : public IDeclaration {
	
	/** The identifier of the macro */	
	Word _ident;

	/** The documentation related to the expressions and blocks */	
	std::vector <std::string> _innerDocs;

	/** The list of expression */
	std::vector <IMacroExpr*> _exprs;

	/** The list of blocks */
	std::vector <Block> _blocks;
	
    public:

	/**
	 * \param ident the location and name of the macro
	 * \param docs the comment related to the macro
	 * \param innerDocs the comments related to the inner expressions
	 * \param exprs the set of expression composing the macro
	 * \param block the set of blocks composing the macro
	 */
	IMacro (Word ident, const std::string & docs, std::vector <std::string> innerDocs, const std::vector <IMacroExpr*> &exprs, const std::vector <Block> & blocks);

	void declare () override;
	
	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module) override;

	Ymir::json generateDocs () override;
	
	const std::vector <IMacroExpr*>& getExprs ();
	
	const std::vector <Block>& getBlocks ();
	
    };

    /**
     * \struct IMacroCall
     * The syntaxic node representation of a macro call
     * \verbatim
     macro_call := expression ':' ('{' [.]* '}') | ('[' [.]* ']') | ('(' [.]* ')')
     \endverbatim
     */
    class IMacroCall : public IExpression {

	/** The closure token*/
	Word _end;

	/** The left operand */
	Expression _left;

	/** ?? */
	Expression _inner;

	/** The content token */
	std::vector <Word> _content;

	/** block set after macro resolution */
	Block _bl = NULL;

	/** The solution returned by semantic analyse */
	std::map <std::string, Expression> _soluce;
	
    public :

	/**
	 * \param begin the location of the call
	 * \param end the location of the closure token of the call
	 * \param left the left operand
	 * \param content the list of token 
	 */
	IMacroCall (Word begin, Word end, Expression left, std::vector<Word> content);
	
	Expression expression () override;
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	const std::vector <Word> & getTokens ();	
	
	void setSolution (std::map <std::string, Expression> soluce);

	std::string prettyPrint () override;
	
	static const char * id () {
	    return TYPEID (IMacroCall);
	}
	
	std::vector <std::string> getIds () override;	

    private:

	MacroCall solve (const std::map <std::string, Expression>&);
	
	std::vector <Word> expressionExpand ();
	
    };
	    
    typedef IMacro* Macro;
    typedef IMacroExpr* MacroExpr;
    typedef IMacroCall* MacroCall;
    typedef IMacroElement* MacroElement;
    typedef IMacroVar* MacroVar;
    typedef IMacroToken* MacroToken;
    typedef IMacroRepeat* MacroRepeat;
    typedef IMacroEnum* MacroEnum;
}


