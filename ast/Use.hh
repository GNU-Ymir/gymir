#pragma once

#include <ymir/ast/Declaration.hh>
#include <ymir/ast/Expression.hh>

namespace syntax {

    class IUse : public IDeclaration {

	Word loc;
	Expression mod;
	
    public :

	IUse (Word loc, Expression mod) :
	    loc (loc),
	    mod (mod)
	{
	    if (mod) mod-> inside = new (Z0) INone (loc);
	}

	Declaration templateDeclReplace (const std::map <std::string, Expression>& tmps);
	
	void declare () override;

	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module) override;

	void execute ();
	
	void print (int) override {}

	
	static const char * id () {
	    return TYPEID (IUse);
	}
	
	std::vector <std::string> getIds () override {
	    return {TYPEID (IUse)};
	}

	
    };

    typedef IUse* Use;

}
