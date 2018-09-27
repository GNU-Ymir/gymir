#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace syntax {
    class IExpression;
    typedef IExpression* Expression;
}

namespace semantic {

    class IAliasCstInfo : public IInfoType {

	Namespace _space;

	Word _ident;

	syntax::Expression _value;

    public:

	IAliasCstInfo (Word ident, Namespace space, syntax::Expression value);

	bool isSame (InfoType other) override;

	InfoType onClone () override;

	InfoType TempOp (const std::vector <::syntax::Expression> &) override;

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	Word getIdent ();
	
	bool isType () override;

	syntax::Expression expression ();
	
	static const char* id () {
	    return "IAliasCstInfo";
	}

	const char* getId () override;
	
    };

    typedef IAliasCstInfo* AliasCstInfo;
    
}
