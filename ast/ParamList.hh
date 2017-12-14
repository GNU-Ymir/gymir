#pragma once

#include "Expression.hh"
#include <vector>
#include "../errors/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IParamList : public IExpression {

	std::vector <Expression> params;

    public :
	IParamList (Word ident, std::vector <Expression> params);
	
	std::vector <Expression>& getParams ();

	std::vector <semantic::InfoType> getParamTypes ();

	Expression expression () override;
	
	std::vector <tree> toGenericParams (std::vector <semantic::InfoType>);

	void print (int nb = 0) override;

	const char* getId () override;
	
	static const char * id ();
    };

    typedef IParamList* ParamList;
    
}
