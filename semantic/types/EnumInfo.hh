#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <vector>
#include <ymir/utils/Array.hh>

namespace semantic {

    class IEnumCstInfo : public IInfoType {

	std::string _name;

    public:

	std::vector <InfoType> comps;
	std::vector <std::string> names;
	std::vector <syntax::Expression> values;
	InfoType type;
	
	IEnumCstInfo (std::string, InfoType);

	std::string name ();

	void addAttrib (std::string, syntax::Expression, InfoType comp = NULL);

	InfoType DColonOp (syntax::Var) override;

	InfoType DotOp (syntax::Var) override;

	InfoType create ();

	std::string simpleTypeString () override;

	std::string innerTypeString () override;

	bool isSame (InfoType) override;

	InfoType clone () override;

	static const char* id () {
	    return "IEnumCstInfo";
	}

	const char* getId () override;
	
    private:

	InfoType GetAttrib (ulong nb);
	
    };


    class IEnumInfo : public IInfoType {

	std::string _name;
	InfoType _content;

    public:

	IEnumInfo (bool, std::string, InfoType);

	std::string name ();

	InfoType BinaryOp (Word, syntax::Expression) override;

	InfoType BinaryOpRight (Word, syntax::Expression) override;

	InfoType AccessOp (Word, syntax::ParamList) override;

	InfoType DotOp (syntax::Var) override;

	InfoType DotExpOp (syntax::Expression) override;

	InfoType DColonOp (syntax::Var) override;

	InfoType UnaryOp (Word) override;

	InfoType CompOp (InfoType) override;

	InfoType CastOp (InfoType) override;

	InfoType ApplyOp (std::vector <syntax::Var>) override;

	std::string simpleTypeString () override;

	std::string innerTypeString () override;

	static const char* id () {
	    return "IEnumInfo";
	}

	const char* getId () override;

	bool isSame (InfoType) override;

	InfoType clone () override;
	
	InfoType content ();
	
    };

    

    typedef IEnumCstInfo* EnumCstInfo;
    
}
