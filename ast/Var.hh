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
	bool _fromClosure = false;
	semantic::Symbol _lastInfo;
	
    public :

	Word deco;

	IVar (Word ident);
	IVar (Word ident, Word deco);
	IVar (Word ident, const std::vector <Expression>& tmps);

	static const char * id () {
	    return TYPEID (IVar);
	}
	
	std::vector <std::string> getIds () override;
	
	bool hasTemplate ();
	
	Expression expression () override;

	Expression expression (semantic::Symbol sym);

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;	
	
	Ymir::Tree toGeneric () override;

	Ymir::Tree lastInfoDecl ();
	
	semantic::Symbol& lastInfo ();

	Expression onClone () override;
	
	virtual IVar* var ();
		
	virtual Type asType ();

	virtual bool isTypeV ();

	bool fromClosure ();
	
	bool isLvalue () override;
	
	std::vector<Expression>& getTemplates ();

	Word& getDeco ();
	
	TypedVar setType (::semantic::Symbol info);

	TypedVar setType (::semantic::InfoType info);

	void print (int nb = 0) override;

	virtual std::string prettyPrint ();

	virtual ~IVar ();
	
    };

    class IArrayVar : public IVar {
	Expression content;
	Expression len;
	
    public: 
	IArrayVar (Word token, Expression content);

	IArrayVar (Word token, Expression content, Expression len);
	
	IVar* var () override;
		
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;	
	
	Expression contentExp ();

	Expression getLen ();
	
	Type asType () override;

	bool isTypeV () override;

	std::string prettyPrint () override;

	static const char * id () {
	    return TYPEID (IArrayVar);
	}
	
	std::vector <std::string> getIds () override;
	
	void print (int nb = 0) override;

	virtual ~IArrayVar ();
	
    };
    
    typedef IVar* Var;    
    typedef IArrayVar* ArrayVar;
}
