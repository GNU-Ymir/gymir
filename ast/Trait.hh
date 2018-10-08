#pragma once

#include <ymir/ast/Declaration.hh>
#include <ymir/ast/Var.hh>
#include <ymir/ast/Expression.hh>
#include <ymir/ast/Block.hh>
#include <vector>
#include <ymir/syntax/Word.hh>

namespace semantic {
    class IModule;
    typedef IModule* Module;
}

namespace syntax {

    struct TraitProto {
	Word _name;
	
	std::vector <Expression> _types;
	bool _isSelf;
	Expression _ret;
    };
    
    class ITrait : public IDeclaration {

	Word _ident;

	std::vector <TraitProto> _protos;
	std::vector <TypedVar> _attrs;

    public:

	ITrait (Word ident, std::vector <TraitProto> protos, std::vector <TypedVar> attrs);

	std::vector <TraitProto> & getProtos ();

	std::vector <TypedVar> & getAttrs ();

	Word & getIdent ();

	void declare () override;

	void declare (semantic::Module) override;

	void declareAsExtern (semantic::Module) override;

	void print (int) override {}

	static const char* id () {
	    return "ITrait";
	}

	std::vector <std::string> getIds () override {
	    auto vec = IDeclaration::getIds ();
	    vec.push_back (ITypeCreator::id ());
	    return vec;
	}	
    };


    typedef ITrait* Trait;
}
