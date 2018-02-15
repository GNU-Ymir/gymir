#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/utils/StringUtils.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/semantic/value/_.hh>
#include <ymir/semantic/utils/ArrayUtils.hh>

namespace semantic {
        
    IStringInfo::IStringInfo (bool isConst) :
	IArrayInfo (isConst, new (Z0) ICharInfo (false))
    {
	this-> isText () = isConst;
    }
    
    InfoType IStringInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) {
	    if (left-> info-> type-> is <IUndefInfo> ()) {
		auto arr = this-> clone ();
		arr-> binopFoo = &ArrayUtils::InstAffect;
		return arr;
	    }
	}
	return IArrayInfo::BinaryOpRight (op, left);
    }

    InfoType IStringInfo::onClone () {
	auto aux = new IStringInfo (this-> isConst ());
	aux-> value () = this-> value ();
	aux-> isText () = this-> isConst ();
	return aux;
    }
   
}
