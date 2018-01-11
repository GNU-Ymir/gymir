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

	static const char * id () {
	    return TYPEID (IVar);
	}
	
	std::vector <std::string> getIds () override;
	
	bool hasTemplate ();

	Expression expression () override;

	Expression templateExpReplace (std::map <std::string, Expression>) override;	
	
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

	Expression templateExpReplace (std::map <std::string, Expression>) override;	
	
	Expression contentExp ();
	
	Type asType () override;

	bool isType () override;

	std::string prettyPrint () override;

	static const char * id () {
	    return TYPEID (IArrayVar);
	}
	
	std::vector <std::string> getIds () override;
	
	void print (int nb = 0) override;	
    };
    
    typedef IVar* Var;    
    typedef IArrayVar* ArrayVar;
}
