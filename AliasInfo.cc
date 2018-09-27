#include <ymir/semantic/types/_.hh>
#include <ymir/ast/_.hh>

namespace semantic {

    using namespace syntax;
    using namespace std;

    IAliasCstInfo::IAliasCstInfo (Word ident, Namespace space, Expression value) :
	IInfoType (true),
	_space (space),
	_ident (ident),
	_value (value)    
    {}

    bool IAliasCstInfo::isSame (InfoType) {
	return false;
    }

    InfoType IAliasCstInfo::onClone () {
	return this;
    }

    InfoType IAliasCstInfo::TempOp (const std::vector<::syntax::Expression> &) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }

    Expression IAliasCstInfo::expression () {
	auto globSpace = Table::instance ().space ();
	Table::instance ().setCurrentSpace (this-> _space);
	this-> _value-> inside = new (Z0) syntax::IExpression (this-> _ident);
	auto value = this-> _value-> expression ();
	Table::instance ().setCurrentSpace (globSpace);
	return value;
    }
    
    std::string IAliasCstInfo::innerTypeString () {
	return "alias(" + this-> _ident.getStr () + ")";
    }

    std::string IAliasCstInfo::innerSimpleTypeString () {
	return "alias(" + this-> _ident.getStr () + ")";
    }

    Word IAliasCstInfo::getIdent () {
	return this-> _ident;
    }

    bool IAliasCstInfo::isType () {
	return false;
    }

    const char* IAliasCstInfo::getId () {
	return IAliasCstInfo::id ();
    }
    
    
}
