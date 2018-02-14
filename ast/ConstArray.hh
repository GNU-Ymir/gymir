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
	bool _isMut;
	
    public:

	IConstArray (Word token, const std::vector <Expression> &params);

	Expression expression () override;
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;	
	int nbParams ();
	
	bool & isMut ();
	
	static const char * id () {
	    return TYPEID (IConstArray);
	}
	
	std::vector <std::string> getIds () override;

	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual ~IConstArray ();
	
    private:

	semantic::InfoType validate ();
	
    };
    
    typedef IConstArray* ConstArray;

}
