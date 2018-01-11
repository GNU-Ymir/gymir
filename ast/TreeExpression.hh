#pragma once

#include "Expression.hh"


namespace syntax {

    class ITreeExpression : public IExpression {
	
	Ymir::Tree _content;
	
    public:

	ITreeExpression (Word locus, semantic::InfoType info, Ymir::Tree content);
	
	Ymir::Tree toGeneric () override;
	
	void print (int id = 0) override;		

    };

    
}
