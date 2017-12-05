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

	Expression expression () override;
	
	void print (int nb = 0) override;


    private:

	bool simpleVerif (IPar*&);
	
    };

    typedef IPar* Par;    
}
