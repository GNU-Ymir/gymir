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

    class IEvaluatedExpr : public IExpression {

	Expression _value;
	
    public :

	IEvaluatedExpr (Expression value);

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Expression expression ();

	void print (int id = 0) override;
	
	
    };
    
    class ISemanticConst : public IExpression {

	semantic::InfoType _type;
	semantic::InfoType _auxType;
	
    public :

	ISemanticConst (Word locus, semantic::InfoType type, semantic::InfoType auxType);

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Expression expression ();

	Ymir::Tree toGeneric ();
	
	void print (int id = 0) override;

    };

    
}
