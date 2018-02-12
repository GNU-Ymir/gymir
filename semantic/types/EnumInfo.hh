#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <vector>
#include <ymir/utils/Array.hh>

namespace semantic {

    class IEnumCstInfo : public IInfoType {

	std::string _name;
	Namespace _space;
	
    public:

	std::vector <InfoType> comps;
	std::vector <std::string> names;
	std::vector <syntax::Expression> values;
	InfoType type;
	
	IEnumCstInfo (Namespace space, std::string, InfoType);

	std::string name ();

	void addAttrib (std::string, syntax::Expression, InfoType comp = NULL);

	InfoType DColonOp (syntax::Var) override;

	InfoType DotOp (syntax::Var) override;

	InfoType create ();

	InfoType TempOp (const std::vector<::syntax::Expression> &) override;	
	
	std::string innerSimpleTypeString () override;

	std::string innerTypeString () override;

	std::vector <std::string> & getNames ();

	std::vector <syntax::Expression> & getValues ();

	std::vector <InfoType> & getComp ();
	
	bool isSame (InfoType) override;

	InfoType onClone () override;

	static const char* id () {
	    return "IEnumCstInfo";
	}

	const char* getId () override;
	
    private:

	InfoType GetAttrib (ulong nb);
	
    };


    class IEnumInfo : public IInfoType {

	std::string _name;
	Namespace _space;
	InfoType _content;

	InfoType comp;
	syntax::Expression value;
	
    public:

	IEnumInfo (bool, Namespace, std::string, InfoType);

	std::string& name ();

	InfoType & getComp ();

	syntax::Expression & getValue ();
	
	InfoType BinaryOp (Word, syntax::Expression) override;

	InfoType BinaryOpRight (Word, syntax::Expression) override;

	InfoType AccessOp (Word, syntax::ParamList, std::vector<InfoType>&) override;

	InfoType DotOp (syntax::Var) override;

	InfoType DotExpOp (syntax::Expression) override;

	InfoType DColonOp (syntax::Var) override;

	InfoType UnaryOp (Word) override;

	InfoType CompOp (InfoType) override;

	InfoType CastOp (InfoType) override;

	InfoType ApplyOp (const std::vector<syntax::Var> &) override;

	InfoType & getContent ();
	
	std::string innerSimpleTypeString () override;

	std::string innerTypeString () override;
	
	InfoType getTemplate (ulong) override;
	
	Ymir::Tree toGeneric ();
	
	static const char* id () {
	    return "IEnumInfo";
	}

	const char* getId () override;

	bool isSame (InfoType) override;

	InfoType onClone () override;
	
	InfoType content ();
	
    };

    

    typedef IEnumCstInfo* EnumCstInfo;
    typedef IEnumInfo* EnumInfo;
    
}
