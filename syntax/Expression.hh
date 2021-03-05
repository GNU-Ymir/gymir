#pragma once

#include <ymir/utils/macros.hh>
#include <ymir/utils/Ref.hh>
#include <ymir/errors/Error.hh>
#include <ymir/lexing/Word.hh>

namespace syntax {

    class Expression;
    class Declaration;
    
    class IExpression {

	lexing::Word _location;

	std::set <std::string> _sub_var_names;

	bool _set_sub_var_names = false;
	
    protected :

	IExpression (const lexing::Word & location);
	
    public:

	friend Expression;
       
	/**
	 * \brief Print the expression into the buffer 
	 * \brief Debugging purpose only
	 */
	virtual void treePrint (Ymir::OutBuffer & stream, int i = 0) const;

	/**
	 * \brief Create a pretty print of the expression
	 */
	virtual std::string prettyString () const = 0;
	
	/**
	 * \return the location of the expression
	 */
	const lexing::Word & getLocation () const;

	/**
	 * @return the names of the variable that are contained in the expression
	 */
	const std::set <std::string> & getSubVarNames () const ;
	
	virtual ~IExpression ();

    protected :
	
	virtual const std::set <std::string> & computeSubVarNames () = 0;
	
	void setSubVarNames (const std::set <std::string> & subVarNames);
	
    };


    /**
     * \struct Expression Proxy of all Expression
     */
    class Expression : public RefProxy <IExpression, Expression> {

	static std::set <std::string> __empty_sub_names__;
	
    public:

	Expression (IExpression * expr);

	static Expression empty ();

	bool isEmpty () const;
	
	/**
	 * Transform the expression into a declaration
	 */
	static Declaration toDeclaration (const Expression & expr, const std::string & comments);

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
#ifdef TO_DEBUG
	    if (dynamic_cast <T*> (this-> _value.get ()) == nullptr)
	    	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
#endif
	    return *((T*) this-> _value.get ());	    
	}
	
	/**
	 * \brief Cast the content pointer the type (if possible)
	 * Raise an internal error if that impossible
	 */
	template <typename T>
	const T& to () const {
#ifdef TO_DEBUG
	    if (dynamic_cast <T*> (this-> _value.get ()) == nullptr)
	    	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
#endif
	    return *((const T*) this-> _value.get ());	    
	}
	
	/**
	 * \brief Tell if the inner type inside the proxy is of type T
	 */
	template <typename T>
	bool is () const {	    
	    return dynamic_cast <T*> (this-> _value.get ()) != nullptr;
	}
	
	void treePrint (Ymir::OutBuffer & stream, int i = 0) const;

	const std::set <std::string> & getSubVarNames () const ;
	
	std::string prettyString () const;

    private :
	
	const std::set <std::string> & computeSubVarNames ();
	
    };       

}
