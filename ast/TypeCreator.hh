#pragma once

#include "Declaration.hh"
#include "Var.hh"
#include "Expression.hh"
#include "Block.hh"
#include <vector>
#include "../syntax/Word.hh"

namespace syntax {

    enum class TypeForm {
	OVER,
	IMPL
    };

    class IFunction;
    class ITypeCreator;
    class ITypeConstructor;
    class ITypeDestructor;
    class ITypeMethod;

    typedef IFunction* Function;
    typedef ITypeCreator* TypeCreator;
    typedef ITypeConstructor* TypeConstructor;
    typedef ITypeDestructor* TypeDestructor;
    typedef ITypeMethod* TypeMethod; 
        
    class ITypeCreator : public IDeclaration {
    protected:

	Word _ident;
	TypeForm _form;
	Expression _who;
	std::vector <Expression> _tmps;
	std::vector <TypeConstructor> _constr;
	std::vector <TypeDestructor> _destr;
	std::vector <TypeMethod> _methods;

    public:

	ITypeCreator (Word ident, TypeForm form, Expression who, const std::vector <Expression> & tmps);

	std::vector <TypeConstructor> & getConstructors ();

	std::vector <TypeDestructor> & getDestructors ();

	std::vector <TypeMethod> & getMethods ();		

	void declare () override {}

	void declare (semantic::Module) override {}

	void declareAsExtern (semantic::Module) override {}

	void print (int) override {}
	
    };

    class ITypeConstructor : public IFunction {	
    public :

	ITypeConstructor (Word ident, const std::vector <Var> & params, Block block);

	void declare () override {}
	
	void declare (semantic::Module) override {}
	
	void declareAsExtern (semantic::Module) override {}
	
	void print (int) override {}
	
    };

    class ITypeDestructor : public IFunction {	
    public :

	ITypeDestructor (Word ident, Block block);

	void declare () override {}
	
	void declare (semantic::Module) override {}
	
	void declareAsExtern (semantic::Module) override {}
	
	void print (int) override {}
	
    };

    class ITypeMethod : public IFunction {

	bool _isOver;
	
    public :

	ITypeMethod (Function func, bool over);

	void declare () override {}
	
	void declare (semantic::Module) override {}
	
	void declareAsExtern (semantic::Module) override {}
	
	void print (int) override {}
	
    };
    
}
