#pragma once

#include "Expression.hh"


namespace syntax {

    class ITreeExpression : public IExpression {
	
	Ymir::Tree _content;
	
    public:

	ITreeExpression (Word locus, semantic::InfoType info, Ymir::Tree content);

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Expression expression ();
	
	Ymir::Tree toGeneric () override;
	
	void print (int id = 0) override;		

    };

    
}
