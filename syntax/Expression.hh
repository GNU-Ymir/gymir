#pragma once

#include <ymir/utils/Proxy.hh>
#include <ymir/errors/Error.hh>
#include <ymir/lexing/Word.hh>

namespace syntax {

    class Expression;
    class Declaration;
    
    class IExpression {

	lexing::Word _location;
	
    protected :

	IExpression (const lexing::Word & location);
	
    public:	
	
	virtual Expression clone () const = 0;

	virtual bool isOf (const IExpression * type) const = 0;

	/**
	 * \brief Print the expression into the buffer 
	 * \brief Debugging purpose only
	 */
	virtual void treePrint (Ymir::OutBuffer & stream, int i = 0) const;

	/**
	 * \return the location of the expression
	 */
	const lexing::Word & getLocation () const;
	
	virtual ~IExpression ();
	
    };


    /**
     * \struct Expression Proxy of all Expression
     */
    class Expression : public Proxy <IExpression, Expression> {
    public:

	Expression (IExpression * expr);

	static Expression empty ();

	bool isEmpty () const;
	
	/**
	 * Transform the expression into a declaration
	 */
	static Declaration toDeclaration (const Expression & expr);

	/**
	 * Proxy method
	 */
	const lexing::Word & getLocation () const;
	
	/**
	 * \brief Cast the content pointer the type (if possible)
	 * Raise an internal error if that impossible
	 */
	template <typename T>
	T& to () {	    
	    if (this-> _value == NULL) 
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "nullptr");
	    else {
		T t;
		if (!this-> _value-> isOf (&t))
		    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
	    }
	    return *((T*) this-> _value);	    
	}
	
	/**
	 * \brief Cast the content pointer the type (if possible)
	 * Raise an internal error if that impossible
	 */
	template <typename T>
	const T& to () const {	    
	    if (this-> _value == NULL) 
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "nullptr");
	    else {
		T t;
		if (!this-> _value-> isOf (&t))
		    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
	    }
	    return *((const T*) this-> _value);	    
	}
	
	/**
	 * \brief Tell if the inner type inside the proxy is of type T
	 */
	template <typename T>
	bool is () const {	    
	    if (this-> _value == NULL)
		return false;

	    T t;
	    return this-> _value-> isOf (&t); 			    
	}
	
	void treePrint (Ymir::OutBuffer & stream, int i = 0) const;
	
    };       

}
