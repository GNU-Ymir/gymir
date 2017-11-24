#pragma once

#include "Expression.hh"
#include <vector>
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IParamList : public IExpression {

	std::vector <Expression> params;

    public :
	IParamList (Word ident, std::vector <Expression> params);

	std::vector <Expression> getParams ();

	std::vector <semantic::InfoType> getParamTypes ();
	
	void print (int nb = 0) override;
    };

    typedef IParamList* ParamList;
    
}
