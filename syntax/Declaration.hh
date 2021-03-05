#pragma once

#include <ymir/utils/macros.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Ref.hh>
#include <ymir/errors/Error.hh>

namespace syntax {

    class Declaration;
    
    class IDeclaration {
	
	lexing::Word _location;

	std::string _comments;

	std::set <std::string> _sub_var_names;

	bool _set_sub_var_names = false;
	
    protected : 
	
	IDeclaration (const lexing::Word & location, const std::string & comments);
	
    public:
	
	friend Declaration;
	
	/**
	 * \brief Print the declaration into the buffer 
	 * \brief Debugging purpose only
	 */
	virtual void treePrint (Ymir::OutBuffer & stream, int i = 0) const;

	/**
	 * \return the location of this declaration
	 */
	const lexing::Word & getLocation () const;
	
	/**
	 * \return the comments of the declaration
	 */
	const std::string& getComments () const;

	/**
	 * @return the names of the variable that are contained in the declaration
	 */
	const std::set <std::string> & getSubVarNames () const ;

	/**
	 * Change the comments of the declaration
	 */
	void setComments (const std::string & comments);
	
	virtual ~IDeclaration ();

    protected :
	
	virtual const std::set <std::string> & computeSubVarNames () = 0;
	
	void setSubVarNames (const std::set <std::string> & subVarNames);
	
    };

    /**
     * \struct Declaration Proxy of all declaration
     * 
     */
    class Declaration : public RefProxy <IDeclaration, Declaration> {

	static std::set <std::string> __empty_sub_names__;
	
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
	 * Proxy function
	 */
	const lexing::Word & getLocation () const;


	/**
	 * Proxy function
	 */
	const std::string & getComments () const;

	/**
	 * Proxy function
	 */
	void setComments (const std::string & comments);
	
	/**
	 * \brief Cast the content pointer into the type (if possible)
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
	 * \brief Cast the content pointer into the type (if possible)
	 * Raise an internal error if that impossible
	 */
	template <typename T>
	const T& to () const {
#ifdef TO_DEBUG
	    if (dynamic_cast <T*> (this-> _value.get ()) == nullptr) 
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
#endif
	    return * ((const T*) this-> _value.get ());	    
	}
	
	/**
	 * \brief Tell if the inner type inside the proxy is of type T
	 */
	template <typename T>
	bool is () const {	    
	    return dynamic_cast <T*> (this-> _value.get ()) != nullptr; 
	}


	const std::set <std::string> & getSubVarNames () const ;
	
	void treePrint (Ymir::OutBuffer & stream, int i = 0) const;
	
    private :
	
	const std::set <std::string> & computeSubVarNames ();
	
    };    

}
