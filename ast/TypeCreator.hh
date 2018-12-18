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
    class ITypeAttr;
    
    typedef IFunction* Function;
    typedef ITypeCreator* TypeCreator;
    typedef ITypeConstructor* TypeConstructor;
    typedef ITypeDestructor* TypeDestructor;
    typedef ITypeMethod* TypeMethod; 
    typedef ITypeAttr* TypeAttr;
    
    class ITypeCreator : public IDeclaration {
    protected:

	Word _ident;
	
	Expression _who;
	std::vector <Expression> _tmps;
	std::vector <TypeConstructor> _constr;
	std::vector <TypeDestructor> _destr;
	std::vector <Block> _staticConstruct;
	std::vector <TypeMethod> _methods;
	std::vector <TypeAttr> _attrs;
		
    public:

	ITypeCreator (Word ident, const std::string & docs, Expression who, const std::vector <Expression> & tmps);

	std::vector <TypeConstructor> & getConstructors ();

	std::vector <TypeDestructor> & getDestructors ();

	std::vector <Block> & getStaticConstructs ();
	
	std::vector <TypeMethod> & getMethods ();		

	std::vector <TypeAttr> & getAttrs ();

	semantic::InfoType declare (semantic::Namespace, const std::vector <Expression> &tmps);

	Declaration templateDeclReplace (const std::map <std::string, Expression> &) override;
	
	void declare () override;

	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module) override;

	void print (int) override {}

	static const char* id () {
	    return "ITypeCreator";
	}

	std::vector<std::string> getIds () override {
	    auto vec = IDeclaration::getIds ();
	    vec.push_back (ITypeCreator::id ());
	    return vec;
	}
	
    };

    class ITypeConstructor {

	InnerProtection _prot = InnerProtection::PUBLIC;
	Word _ident;
	std::vector <Var> _params;
	Block _block;
	bool _isCopy;
	
    public :

	ITypeConstructor (Word ident, const std::vector <Var> & params, Block block, bool isCopy);

	InnerProtection & getProtection ();

	std::vector <Var> & getParams ();

	Block& getBlock ();

	ITypeConstructor* templateDeclReplace (const std::map <std::string, Expression>& tmps);
	
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

	ITypeDestructor* templateDeclReplace (const std::map <std::string, Expression>& tmps);
	
    };

    class ITypeMethod : public IFunction {

	bool _isOver;
	InnerProtection _prot = InnerProtection::PUBLIC;
	
    public :

	ITypeMethod (Function func, bool over);
	
	InnerProtection & getProtection ();
	
	semantic::InfoType declare (semantic::AggregateCstInfo info, bool & isStatic, bool isExternal = false, bool fromTemplate = false);

	ITypeMethod* templateDeclReplace (const std::map <std::string, Expression>& tmps) override;

	ITypeMethod* templateReplace (const std::map <std::string, Expression> & tmps) override;
	
	void declare () override;

	void declare (semantic::Module) override;

	void declareAsExtern (semantic::Module) override;

    private :

	bool verifUdasMeth ();
	
    };

    class ITypeAttr : public IDeclaration {

	Word _ident;
	
	Expression _type;
	
	bool _isConst;

	bool _isStatic;

	InnerProtection _prot = InnerProtection::PUBLIC;

	semantic::Namespace _space;
	
    public :

	ITypeAttr (Word ident, const std::string & docs, Expression type, bool isConst, bool isStatic);

	InnerProtection & getProtection ();

	bool isPrivate ();

	bool isProtected ();

	bool isStatic ();
	
	semantic::Namespace & space ();
	
	void declare () override;

	void declare (semantic::Module) override;

	void declareAsExtern (semantic::Module) override;

	ITypeAttr* templateDeclReplace (const std::map <std::string, Expression> &) override;
	
	Word getIdent ();

	Expression getType ();

	bool isConst ();
	
    };
    
}
