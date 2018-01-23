#include <ymir/semantic/types/_.hh>
using namespace syntax;

namespace semantic {

    namespace IgnoreUtils {

	Ymir::Tree InstAffect (Word, InfoType, Expression, Expression right) {
	    return right-> toGeneric ();
	}
	
    }
    
    
    IIgnoreInfo::IIgnoreInfo () :
	IInfoType (false)
    {}

    bool IIgnoreInfo::isSame (InfoType) {
	return false;
    }

    InfoType IIgnoreInfo::clone () {
	return new (GC) IIgnoreInfo ();
    }

    InfoType IIgnoreInfo::BinaryOp (Word op, syntax::Expression left) {
	if (op == Token::EQUAL && !left-> info-> type-> is<IIgnoreInfo> ()) {
	    auto ret = new (GC) IIgnoreInfo ();
	    ret-> binopFoo = &IgnoreUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    Ymir::Tree IIgnoreInfo::toGeneric () {
	return void_type_node;
    }    
    
    std::string IIgnoreInfo::typeString () {
	return "void";
    }

    std::string IIgnoreInfo::innerTypeString () {
	return "void";
    }

    std::string IIgnoreInfo::innerSimpleTypeString () {
	return "v";
    }

    const char * IIgnoreInfo::getId () {
	return IIgnoreInfo::id ();
    }
    
}
