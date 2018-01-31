#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IStruct : public IDeclaration {
	
	Word ident;
	std::vector<Var> params;
	std::vector<Expression> tmps;

    public:

	IStruct (Word ident, std::vector <Expression> tmps, std::vector <Var> params);
	void declare () override;
	
	void declareAsExtern (semantic::Module) override {}
	
	void print (int nb = 0) override ;
	
	virtual ~IStruct ();
    };

    typedef IStruct* Struct;

}
