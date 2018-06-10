#pragma once

#include "Declaration.hh"
#include "Var.hh"
#include "Expression.hh"
#include "Block.hh"
#include <vector>
#include "../syntax/Word.hh"

namespace semantic {
    class IAggregateCstInfo;
    typedef IAggregateCstInfo* AggregateCstInfo;

    class IInfoType;
    typedef IInfoType* InfoType;    
}

namespace syntax {

    enum class TypeForm {
	OVER,
	IMPL
    };

    enum class InnerProtection {
	PUBLIC,
	PRIVATE,
	PROTECTED
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
	
	std::vector <Expression> _who;
	std::vector <Expression> _tmps;
	std::vector <TypeConstructor> _constr;
	std::vector <TypeDestructor> _destr;
	std::vector <TypeMethod> _methods;

	bool _isUnion;
	
    public:

	ITypeCreator (Word ident, TypeForm form, const std::vector <Expression> & who, const std::vector <Expression> & tmps, bool isUnion);

	std::vector <TypeConstructor> & getConstructors ();

	std::vector <TypeDestructor> & getDestructors ();

	std::vector <TypeMethod> & getMethods ();		

	void declare () override;

	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module) override;

	void print (int) override {}
	
    };

    class ITypeConstructor {

	InnerProtection _prot = InnerProtection::PUBLIC;
	Word _ident;
	std::vector <Var> _params;
	Block _block;
	
    public :

	ITypeConstructor (Word ident, const std::vector <Var> & params, Block block);

	InnerProtection & getProtection ();

	std::vector <Var> & getParams ();

	Block& getBlock ();
	
	semantic::InfoType declare (semantic::AggregateCstInfo info, bool isExternal = false);

	Word getIdent ();
	
    };

    class ITypeDestructor {	

	InnerProtection _prot = InnerProtection::PUBLIC;
	Word _ident;
	Block _block;
	
    public :

	ITypeDestructor (Word ident, Block block);

	InnerProtection & getProtection ();

	Word & getIdent ();

	Block & getBlock ();
	
	semantic::InfoType declare (semantic::AggregateCstInfo info, bool isExternal = false);		
	
    };

    class ITypeMethod : public IFunction {

	bool _isOver;
	InnerProtection _prot = InnerProtection::PUBLIC;
	
    public :

	ITypeMethod (Function func, bool over);
	
	InnerProtection & getProtection ();
	
	semantic::InfoType declare (semantic::AggregateCstInfo info, bool & isStatic, bool isExternal = false);

	void declare () override;

	void declare (semantic::Module) override;

	void declareAsExtern (semantic::Module) override;
	
    };
    
}
