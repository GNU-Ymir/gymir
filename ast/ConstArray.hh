#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace semantic {
    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace syntax {

    class IConstArray : public IExpression {
	std::vector <Expression> params;
	std::vector <semantic::InfoType> casters;

    public:

	IConstArray (Word token, std::vector <Expression> params);

	Expression expression () override;
	
	void print (int nb = 0) override;
    };
    
    typedef IConstArray* ConstArray;

}
