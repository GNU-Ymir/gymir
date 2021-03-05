#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {
        
    /**
     * \struct Function
     * Declaration of a function
     * \verbatim
     function := 'def' ('if' expression)? attributes Identifier templates prototype body
     body := block |
             'in' block 'body' block 'out' '(' Identifier ')' block
     \endverbatim
     */
    class Function : public IDeclaration {
    public: 
	/**
	 * The function prototype is the parameters and return type of a function
	 */
	class Prototype {

	    /** A set of var declaration (VarDecl) representing */
	    std::vector <Expression> _parameters;

	    /** The return type of the function (a VarDecl) */
	    Expression _retType;

	    /** Some C function can be variadic, a Ymir function cannot */
	    bool _isVariadic;

	    std::set <std::string> _sub_var_names;

	    bool _set_sub_var_names = false;
	    
	private:
	    
	    Prototype ();

	    Prototype (const std::vector <Expression> & vars, const Expression & retType, bool isVariadic);
	    
	public:

	    static Prototype empty ();
	    
	    /**
	     * \brief Create a new prototype
	     * \param names the name of the parameters of the function 
	     * \param types the types of the parameters of the function
	     * \param decos the decorator of each parameters
	     * \param retType the return type of the function 
	     * \param retDeco the decorators of the return type
	     */
	    static Prototype init (const std::vector <Expression> & vars, const Expression & retType, bool isVariadic);


	    void treePrint (Ymir::OutBuffer & stream, int i = 0) const;
	    
	    /**
	     * \return the list of parameters of the prototype
	     */
	    const std::vector <Expression> & getParameters () const;

	    /**
	     * \return the type of the prototype
	     */
	    const Expression & getType () const;	    

	    /**
	     * \return is this prototype variadic ?
	     */
	    bool isVariadic () const;

	    const std::set <std::string> & getSubVarNames ();

	protected:
	    
	    const std::set <std::string> & computeSubVarNames ();
	   
	};


    private :
	
	/** The prototype of the function */
	Prototype _proto;

	/** The body of the function */
	Expression _body;

	/** The CAs (custom attributes) of the functions */
	std::vector <lexing::Word> _cas;

	std::vector <syntax::Expression> _throwers;
	
	/** Some method can be marked over */
	bool _isOver;
	
    private :

	friend Declaration; // Needed for dynamic casting 
	
	/** \brief Does nothing special, just to ensure that the function cannot be created without init */
	Function ();

	Function (const lexing::Word & name,  const std::string & comment, const Prototype & proto, const Expression & body, const std::vector <lexing::Word> & cas, const std::vector <Expression> & throwers, bool isOver);
	
    public:

	/**
	 * \brief Create a new function
	 * \param name the name of the function
	 * \param proto the prototype of the function
	 * \param body the content of the function
	 */
	static Declaration init (const lexing::Word & name, const std::string & comment, const Prototype & proto, const Expression & body, const std::vector <lexing::Word> & cas, const std::vector <Expression> & throwers, bool isOver);

	/**
	 * \return an encapsulation into a Declaration
	 */
	static Declaration init (const Function & other);
	
	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;
	
	
	/**
	 * \return the list of custom attributs
	 */
	const std::vector <lexing::Word> & getCustomAttributes () const;
	

	/**
	 * \return the prototype of the function
	 */
	const Prototype & getPrototype () const;

	/**
	 * \return the body of the function
	 */
	const Expression & getBody () const;

	/**
	 * does this function override another function
	 */
	bool isOver () const;

	const std::vector <syntax::Expression> & getThrowers () const;
	
    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };
    
}
