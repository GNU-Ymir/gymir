#pragma once

#include <gc/gc_cpp.h>

namespace semantic {
    class IModule;
    typedef IModule* Module;
}

namespace syntax {

    class IExpression;
    typedef IExpression* Expression;
    
    class IDeclaration : public gc {
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

	virtual void declareAsExtern (semantic::Module);	

	virtual void declareAsInternal () {
	    return this-> declare ();
	}

	virtual IDeclaration* templateDeclReplace (std::map <std::string, Expression>);
	
	virtual void print (int nb = 0) = 0;	
	
    };
        
    typedef IDeclaration* Declaration;

}
