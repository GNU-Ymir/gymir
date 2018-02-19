#pragma once

#include "Function.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IConstructor : public IFunction {
    public:

	IConstructor (Word token, std::vector <Var> params, Block bl);
	
	void print (int nb = 0) override;
    };

    typedef IConstructor* Constructor;
}
