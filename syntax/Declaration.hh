#pragma once

#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Proxy.hh>
#include <ymir/errors/Error.hh>

namespace syntax {

    class Declaration;
    
    class IDeclaration {
    public:
	
	virtual Declaration clone () const = 0;

	/**
	 * \brief It is mandatory to override this function C++ does not implement mixin system
	 * \brief The simplest way is as follow : 
	 * \verbatim 
	 class Heir : public IDeclaration {
	     // ...
	     bool isOf (const IDeclaration & type) {
	         auto vtable = ((void**) &type) [0];
		 Heir thisType; // That's why we cannot implement it for all class
		 if (((void**) &thisType) [0] == vtable) return true;
		 return IDeclaration::isOf (type);
	     }
	     //...
	 };
	 \endverbatim
	 * \return true if type is the same as this, or is an ancestor of this
	 */
	virtual bool isOf (const IDeclaration * type) const = 0;

	/**
	 * \brief Print the declaration into the buffer 
	 * \brief Debugging purpose only
	 */
	virtual void treePrint (Ymir::OutBuffer & stream, int i = 0) const;
	
	virtual ~IDeclaration ();
	
    };

    /**
     * \struct Declaration Proxy of all declaration
     * 
     */
    class Declaration : public Proxy <IDeclaration, Declaration> {	
    public:
	
	Declaration (IDeclaration * decl); 

	/**
	 * \brief Create an empty declaration (ptr to null)
	 */
	static Declaration empty ();

	/**
	 * \brief Does this proxy refer to nothing ?
	 */
	bool isEmpty () const;

	/**
	 * \brief Cast the content pointer into the type (if possible)
	 * Raise an internal error if that impossible
	 */
	template <typename T>
	T& to () {	    
	    if (this-> _value == NULL)
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "nullptr");	    

	    T t;
	    if (!this-> _value-> isOf (&t))
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
	    return *((T*) this-> _value);	    
	}

	/**
	 * \brief Cast the content pointer into the type (if possible)
	 * Raise an internal error if that impossible
	 */
	template <typename T>
	const T& to () const {	    
	    if (this-> _value == NULL)
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "nullptr");	    

	    T t;
	    if (!this-> _value-> isOf (&t))
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
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
