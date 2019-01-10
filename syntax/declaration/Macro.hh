#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Macro
     *  Declaration of a macro
     */
    class Macro : public IDeclaration {
    public :

	class MacroExpr : public IExpression {
	    
	    lexing::Word _location;
	    
	    std::vector <Expression> _content;
	    

	private :

	    friend Macro;
	    friend Expression;

	    MacroExpr ();
	    
	public :

	    static Expression init (const MacroExpr & var);
	    
	    static Expression init (const lexing::Word & token, const std::vector <Expression> & content);

	    Expression clone () const override;

	    bool isOf (const IExpression * type) const override;	    
	   
	};
	
	class MacroToken : public IExpression {

	    lexing::Word _location;
	    
	    /** The value of the token */ 
	    std::string _value;

	private :

	    friend Macro;
	    friend Expression;

	    MacroToken ();
	    
	public :

	    static Expression init (const MacroToken & var);
	    
	    static Expression init (const lexing::Word & token, const std::string & value);

	    Expression clone () const override;

	    bool isOf (const IExpression * type) const override;	    
	    
	};

	/**
	 * \struct MacroRepeat
	 * A macro repeat is a macro expression, that can be repeated multiple times
	 * \verbatim
	 macro_repeat := '(' macro_expr ',' macro_token ')' ('*' | '+')
	 \endverbatim
	*/
	class MacroRepeat : public IExpression {

	    lexing::Word _location;
	    
	    Expression _content;
	    
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
	    Expression _pass;

	    /** Is this expression a '+' or a '*', '+' means we must have this expression a least one time */
	    bool _oneTime;	    

	private :

	    friend Macro;
	    friend Expression;

	    MacroRepeat ();

	public :

	    static Expression init (const MacroRepeat & var);
	    
	    static Expression init (const lexing::Word & ident, const Expression & content, const Expression & pass, bool oneTime = false);

	    Expression clone () const override;

	    bool isOf (const IExpression * type) const override;
	    
	};

	/**
	 * \struct MacroVar
	 * The syntaxic node representation of a macro var
	 * \verbatim 
	 macro_var := Identifier ':' ('expr' | 'block' | 'ident' | 'token')
	 \endverbatim
	*/
	class MacroVar : public IExpression {

	    enum class MacroVarConst : int {
		EXPR,
		IDENT,
		BLOCK,
		TOKEN
	    };
	    
	    lexing::Word _name;
	    
	    /** The kind of var */
	    MacroVarConst _type;

	    /** The token if the var take a specifig token */
	    Expression _token;

	    /** The content of the var (the solution) */
	    Expression _content;

	private :

	    friend Macro;
	    friend Expression;

	    MacroVar ();

	public :

	    static Expression init (const MacroVar & var);
	    
	    static Expression init (const lexing::Word & name, const Expression & type, const Expression & token, const Expression & content);

	    Expression clone () const override;
	    
	    bool isOf (const IExpression * type) const override;
	    
	};

	/**
	 * \struct MacroEnum
	 * A list of possible expression, one of them is taken for solution 
	 * \verbatim
	 macro_enum := macro_expr ('|' macro_expr)+
	 \endverbatim
	*/
	class MacroEnum : public IExpression {

	    lexing::Word _location;
	    
	    std::vector <Expression> _elems;

	private :

	    friend Macro;
	    friend Expression;

	    MacroEnum ();

	public :

	    static Expression init (const MacroVar & var);
	    
	    static Expression init (const lexing::Word & location, const std::vector <Expression> & elems); 

	    Expression clone () const override;
	    
	    bool isOf (const IExpression * type) const override;	    
	    
	};
	
    private : 

	lexing::Word _name;

	std::vector <Expression> _exprs;

	std::vector <Expression> _blocks;

    private :

	friend Declaration;

	Macro ();

    public :

	static Declaration init (const Macro & macro);

	static Declaration init (const lexing::Word & name, const std::vector <Expression> & exprs, const std::vector <Expression> & blocks);

	Declaration clone () const override;

	bool isOf (const IDeclaration * type) const override;       	
	
    };

}
