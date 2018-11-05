#pragma once

#include <ymir/utils/memory.hh>
#include <ymir/utils/json.hpp>

namespace semantic {
    class IModule;
    typedef IModule* Module;
}

namespace syntax {

    class IExpression;
    typedef IExpression* Expression;

    /**
     * \struct IDeclaration
     * Parent class of all type of declaration
     * \verbatim
     declaration := alias    |
                    enum     |
		    function |
		    global   | 
		    import   |
		    macro    |
		    moddecl  |
		    proto    |
		    struct   |
		    trait    |
		    use 		    
                     
     \endverbatim
     */
    class IDeclaration  {
    protected :

	/**
	 * Is this declaration public ?
	 */
	bool _isPublic;
	
	/**
	 * The documentation related to the class (comments)
	 */
	std::string _docs;
	
    public:

	/**
	 * \param docs the comments related to the declaration
	 */
	IDeclaration (const std::string docs) :
	    _docs (docs)
	{}
	

	/**
	 * \return is this declaration public
	 */
	bool isPublic () {
	    return this-> _isPublic;
	}

	/**
	 * \param isPublic set the declaration privacy to isPublic
	 */
	void setPublic (bool isPublic) {
	    this-> _isPublic = isPublic;
	}

	/**
	 * \return get or set the comments related to the declaration
	 */
	std::string & docs ();


	/**
	 * \return the comments related to the declaration
	 */
	const std::string& getDocs ();	
	
	/**
	 * \return a json value representing the different meta information of the declaration, used for doc generation
	 */
	virtual Ymir::json generateDocs ();

	/**
	 * \brief declare the declaration inside the current context (cf Table)
	 */
	virtual void declare () = 0;

	/**
	 * \brief declare the declaration inside a module context
	 */
	virtual void declare (semantic::Module);

	/**
	 * \brief declare the declaration inside an external module context
	 */
	virtual void declareAsExtern (semantic::Module);	

	/**
	 * \brief declare this declaration inside the current context (cf. declare ())
	 */
	virtual void declareAsInternal () {
	    return this-> declare ();
	}

	/**
	 * \brief Replace the different occurence of the variable by template expression
	 * \brief Generally called after, template resolution
	 * \param values an associative array, string => Expression representing the template association
	 * \return an untyped declaration where all occurence found in values are replaced
	 */
	virtual IDeclaration* templateDeclReplace (const std::map <std::string, Expression>&);

	/**
	 * \brief Type information used for dynamic casting
	 */
	virtual std::vector <std::string> getIds () {
	    return {TYPEID (IDeclaration)};
	}

	/**
	 * \return true if this declaration is castable to type T
	 */
	template <typename T>
	bool is () {
	    return this-> to<T> () != NULL;
	}

	/**
	 * \return if this declaration is castable to type T, return a it, return NULL otherwise
	 */
	template <typename T>
	T* to () {	    
	    auto ids = this-> getIds ();
	    if (std::find (ids.begin (), ids.end (), T::id ()) != ids.end ())
		return (T*) this;
	    return NULL;
	}

	/**
	 * \brief print the declaration to the screen
	 * \deprecated Only used for debugging purpose
	 */
	virtual void print (int) {}

	virtual ~IDeclaration () {}
	
    };
        
    typedef IDeclaration* Declaration;

}
