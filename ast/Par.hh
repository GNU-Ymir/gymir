#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IApplicationScore;
    typedef IApplicationScore* ApplicationScore;
}

namespace syntax {
    
    class IDotCall;
    class IParamList;
    typedef IParamList* ParamList;
    
    class IPar : public IExpression {
	Word end;
	ParamList params;
	Expression _left;
	IDotCall* _dotCall;
	bool _opCall = false;
	semantic::ApplicationScore _score;
	
    public :

	IPar (Word word, Word end);
	
	IPar (Word word, Word end, Expression left, ParamList params, bool fromOpCall = false);

	ParamList& paramList ();	

	Expression& left ();

	semantic::ApplicationScore& score ();
	
	IDotCall*& dotCall (); 

	bool isLvalue () override;
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Ymir::Tree toGeneric () override;

	static const char * id () {
	    return TYPEID (IPar);
	}

	std::string prettyPrint () override;
	
	std::vector <std::string> getIds () {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID(IPar));
	    return ret;
	}
	
	void print (int nb = 0) override;

	virtual ~IPar ();
	
    private:

	bool simpleVerif (IPar*&);

	void tuplingParams (semantic::ApplicationScore score, IPar* par);

	Ymir::Tree createClosureVar ();

	Ymir::Tree callInline (std::vector <tree> args);

	Expression findOpCall ();
	
    };

    typedef IPar* Par;    
}
