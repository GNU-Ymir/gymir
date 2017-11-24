#include <ymir/ast/ParamList.hh>
#include <ymir/semantic/pack/Symbol.hh>

namespace syntax {

    IParamList::IParamList (Word ident, std::vector <Expression> params) :
	IExpression (ident),
	params (params)
    {}

    std::vector <Expression> IParamList::getParams () {
	return this-> params;
    }

    std::vector <semantic::InfoType> IParamList::getParamTypes () {
	std::vector<semantic::InfoType> paramTypes;
	for (auto it : this-> params) {
	    paramTypes.push_back (it-> info-> type);
	}
	return paramTypes;
    }
	
    void IParamList::print (int nb) {
	printf ("\n%*c<ParamList> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);

	for (auto it : this-> params) {
	    it-> print (nb + 4);
	}	    
    }
  
}
