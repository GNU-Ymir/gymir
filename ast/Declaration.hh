#pragma once

#include <ymir/utils/memory.hh>

namespace semantic {
    class IModule;
    typedef IModule* Module;
}

namespace syntax {

    class IExpression;
    typedef IExpression* Expression;
    
    class IDeclaration  {
    protected :
	
	bool isPublic;

    public:

	bool is_public () {
	    return this-> isPublic;
	}

	void is_public (bool isPublic) {
	    this-> isPublic = isPublic;
	}

	virtual void declare () = 0;
	
	virtual void declare (semantic::Module);
	
	virtual void declareAsExtern (semantic::Module);	

	virtual void declareAsInternal () {
	    return this-> declare ();
	}

	virtual IDeclaration* templateDeclReplace (const std::map <std::string, Expression>&);

	virtual std::vector <std::string> getIds () {
	    return {TYPEID (IDeclaration)};
	}
	
	template <typename T>
	bool is () {
	    return this-> to<T> () != NULL;
	}

	template <typename T>
	T* to () {	    
	    auto ids = this-> getIds ();
	    if (std::find (ids.begin (), ids.end (), T::id ()) != ids.end ())
		return (T*) this;
	    return NULL;
	}
	
	virtual void print (int) {}

	virtual ~IDeclaration () {}
	
    };
        
    typedef IDeclaration* Declaration;

}
