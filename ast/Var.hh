#pragma once

#include "Expression.hh"
#include "../syntax/Word.hh"

#include <vector>

namespace syntax {

    class ITypedVar;
    typedef ITypedVar* TypedVar;

    class IType;
    typedef IType* Type;
    
    class IVar : public IExpression {
    protected :
	
	std::vector <Expression> templates;

    public :

	Word deco;

	IVar (Word ident);
	IVar (Word ident, Word deco);
	IVar (Word ident, std::vector <Expression> tmps);

	static const char* id ();
	
	const char* getId () override;
	
	bool hasTemplate ();

	Expression expression () override;

	Ymir::Tree toGeneric () override;
	
	virtual IVar* var ();
		
	virtual Type asType ();

	virtual bool isType ();

	std::vector<Expression>& getTemplates ();

	Word& getDeco ();
	
	TypedVar setType (::semantic::Symbol info);

	TypedVar setType (::semantic::InfoType info);

	void print (int nb = 0) override;

	virtual std::string prettyPrint ();
    };

    class IArrayVar : public IVar {
	Expression content;

    public: 
	IArrayVar (Word token, Expression content);

	IVar* var () override;

	Expression expression () override;

	Type asType () override;

	bool isType () override;

	std::string prettyPrint () override;
	
	void print (int nb = 0) override;	
    };
    
    typedef IVar* Var;    
    typedef IArrayVar* ArrayVar;
}
