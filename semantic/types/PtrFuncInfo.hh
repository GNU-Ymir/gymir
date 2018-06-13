#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace PtrFuncUtils {
	Ymir::Tree InstAffectComp (Word loc, InfoType type, syntax::Expression left, syntax::Expression right);
    }
    
    class IPtrFuncInfo : public IInfoType {

	std::vector <InfoType> params;
	InfoType ret;
	ApplicationScore score;
	bool _isDelegate, _forcedDelegate = false;
	
    public:

	IPtrFuncInfo (bool isConst);

	bool passingConst (InfoType) override;
	
	bool isSame (InfoType) override;
	
	bool isSameNoDTest (InfoType);

	static InfoType create (Word tok, const std::vector<syntax::Expression> & templates) {
	    if (templates.size () < 1) 
		Ymir::Error::takeATypeAsTemplate (tok);
	    else {
		auto ptr = new (Z0) IPtrFuncInfo (false);
		ptr-> ret = templates [0]-> info-> type ();
		if (templates.size () > 1) {
		    for (auto it : Ymir::r (1, templates.size ())) {
			ptr-> params.push_back (templates [it]-> info-> type ());
		    }
		}
		return ptr;
	    }
	    return NULL;
	}

	static InfoType createDeg (Word tok, const std::vector<syntax::Expression> & templates) {
	    if (templates.size () < 1) 
		Ymir::Error::takeATypeAsTemplate (tok);
	    else {
		auto ptr = new (Z0) IPtrFuncInfo (false);
		ptr-> ret = templates [0]-> info-> type ();
		if (templates.size () > 1) {
		    for (auto it : Ymir::r (1, templates.size ())) {
			ptr-> params.push_back (templates [it]-> info-> type ());
		    }
		}
		ptr-> isDelegate () = true;
		return ptr;
	    }
	    return NULL;
	}
	
	InfoType BinaryOp (Word token, syntax::Expression right) override;

	InfoType BinaryOpRight (Word token, syntax::Expression right) override;
	
	InfoType onClone () override;

	InfoType DotOp (syntax::Var) override;
	
	InfoType DColonOp (syntax::Var) override;

	ApplicationScore CallType (Word token, syntax::ParamList params) override;
	
	ApplicationScore CallOp (Word token, syntax::ParamList params);

	InfoType CompOp (InfoType other) override;
	
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;
	
	Ymir::Tree toGeneric () override;

	ulong nbTemplates () override;
	
	InfoType getTemplate (ulong i);

	std::vector <InfoType> getTemplate (ulong i, ulong af) override;
	
	InfoType& getType ();

	std::vector <InfoType> & getParams ();

	ApplicationScore& getScore ();	 
	
	bool & isDelegate ();

	bool & forcedDelegate ();
	
	static const char* id () {
	    return "IPtrFuncInfo";
	}

	const char* getId () override;

    private:

	InfoType Affect (syntax::Expression right);

	InfoType AffectRight (syntax::Expression right);

	InfoType Is (syntax::Expression right);

	InfoType NotIs (syntax::Expression right);
	
    };

    typedef IPtrFuncInfo* PtrFuncInfo;
    
}
