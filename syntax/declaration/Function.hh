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

	private:

	    Prototype ();
	    
	public:

	    /**
	     * \brief Create en empty prototype
	     * There is no proxy over prototype, but to gain in clarity all the object are initialized the same way
	     */
	    static Prototype init ();

	    /**
	     * \brief Create a copy of another prototype
	     * \param proto the other prototype
	     */
	    static Prototype init (const Prototype & proto);
	    
	    /**
	     * \brief Create a new prototype
	     * \param names the name of the parameters of the function 
	     * \param types the types of the parameters of the function
	     * \param decos the decorator of each parameters
	     * \param retType the return type of the function 
	     * \param retDeco the decorators of the return type
	     */
	    static Prototype init (const std::vector <Expression> & vars, const Expression & retType);


	    /**
	     * Not mandatory since Prototype is not inside a proxy, 
	     * but it is implemented to produce the same result on all the object types
	     */
	    Prototype clone () const; 

	    void treePrint (Ymir::OutBuffer & stream, int i = 0) const;
	    
	    /**
	     * \brief Add a new parameter to the prototype
	     * \param name the name of the parameter
	     * \param type the type of the parameter
	     * \param decos the decorator associated to the parameter
	     */
	    void addParameter (const Expression & var); 
	    /**
	     * \brief Change the type of the prototype
	     * \param type the new type
	     * \param decos the decorator of the return type
	     */
	    void setType (const Expression & type);
	    
	};

	/**
	 * A function body can reprensent either a block or a contract block
	 */
	class Body {
	    
	    /** The body of the function */
	    Expression _body;

	    /** The in expression of a contract programming (empty if unused) */
	    Expression _inner;

	    /** The out expression of a contract programming (empty if unused) */
	    Expression _outer;

	    /** The name of the variable of the outer block (used only if !_outer.empty ()) */
	    lexing::Word _outerName;

	private:
	    
	    Body ();

	public:

	    /**
	     * \brief Create an empty body	     
	     * There is no proxy over body, but to gain in clarity all the object are initialized the same way
	     */
	    static Body init ();

	    /**
	     * \brief Create a body from another one
	     */
	    static Body init (const Body& body);
	    
	    /**
	     * \brief Create a normal body (without contract programming)
	     * \param body the content of the body
	     */
	    static Body init (const Expression & body); 

	    /**
	     * \brief Create a contract programming body
	     * \param inner the in part of the contract
	     * \param body the body of the function
	     * \param outer the out part of the contract
	     * \param name the name of the variable inside the out part
	     */
	    static Body init (const Expression & inner, const Expression & body, const Expression & outer, const lexing::Word & name);

	    Body clone () const;

	    void treePrint (Ymir::OutBuffer & stream, int i = 0) const;
	    
	    /**
	     * \brief Set the body of the expression
	     * \param body the content of this body
	     */
	    void setBody (const Expression & body);

	    /**
	     * \brief Set the inner contract part 
	     * \param inner 
	     */
	    void setInner (const Expression & inner);


	    /**
	     * \brief Set the outer contract part 
	     * \param outer the content of the out part
	     * \param name the name of the variable in the out block
	     */
	    void setOuter (const Expression & outer, const lexing::Word & name);
	};

    private :

	/** The test of the definition if there is templates */
	Expression _test;
	
	/** The prototype of the function */
	Prototype _proto;

	/** The body of the function */
	Body _body;

	/** The CAs (custom attributes) of the functions */
	std::vector <lexing::Word> _cas;

	/** The name of the function */
	lexing::Word _name;

    private :

	friend Declaration; // Needed for dynamic casting 
	
	/** \brief Does nothing special, just to ensure that the function cannot be created without init */
	Function ();

    public:

	/** 
	 * \brief Create a new empty function 
	 */
	static Declaration init ();

	/**
	 * \brief Create a function from another one
	 * \param func the function to copy
	 */
	static Declaration init (const Function& func);

	/**
	 * \brief Create a new function
	 * \param name the name of the function
	 * \param proto the prototype of the function
	 * \param body the content of the function
	 */
	static Declaration init (const lexing::Word & name, const Prototype & proto, const Body & body);

	/**
	 * Mandatory function used for proxy polymoprhism system
	 */
	Declaration clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;
	
	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;
	
	/**
	 * \brief add a custom attribute to the function
	 * \param ca the custom attr
	 */
	void addCustomAttribute (const lexing::Word & ca);
	
	/**
	 * \brief Set the set of custom attributes 
	 */
	void setCustomAttributes (const std::vector <lexing::Word> & cas);

	/**
	 * \return the list of custom attributs
	 */
	const std::vector <lexing::Word> & getCustomAttributes () const;
	
	/**
	 * \return the name and location of the function
	 */
	const lexing::Word & getName () const;

	/**
	 * \brief Change the name of the function
	 * \param name the name of the function
	 */
	void setName (const lexing::Word & name);

	/**
	 * \brief Set the prototype of the function 
	 * \param proto the prototype of the function
	 */
	void setPrototype (const Prototype & proto);

	/**
	 * \brief Set the body of the function
	 * \param body the body of the function
	 */
	void setBody (const Body & body);
	
    };
    
}
