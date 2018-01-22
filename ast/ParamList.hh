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

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	std::vector <tree> toGenericParams (std::vector <semantic::InfoType>);

	void print (int nb = 0) override;

	static const char * id () {
	    return TYPEID (IParamList);
	}
	
	std::vector <std::string> getIds () override;
	
    };

    typedef IParamList* ParamList;
    
}
