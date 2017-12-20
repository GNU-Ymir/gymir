#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/utils/StringUtils.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>

namespace semantic {

    bool IStringInfo::__initStringTypeNode__ = false;
    
    IStringInfo::IStringInfo (bool isConst) :
	IInfoType (isConst)
    {}

    bool IStringInfo::isSame (InfoType other) {
	return other-> is<IStringInfo> ();
    }

    std::string IStringInfo::innerTypeString () {
	return "string";
    }

    std::string IStringInfo::simpleTypeString () {
	return "s";
    }
    
    InfoType IStringInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) return AffectRight (left);
	return NULL;
    }
    
    InfoType IStringInfo::clone () {
	return new IStringInfo (this-> isConst ());
    }

    InfoType IStringInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "ptr") return Ptr ();
	if (var-> token == "len") return Length ();
	return NULL;
    }

    InfoType IStringInfo::CompOp (InfoType other) {
	if (this-> isSame (other)) {
	    return this-> clone ();
	}
	return NULL;
    }
    
    InfoType IStringInfo::Ptr () {	
	auto ret = new IPtrInfo (this-> isConst (), new ICharInfo (this-> isConst ()));
	ret-> unopFoo = &StringUtils::InstPtr;
	return ret;
    }

    InfoType IStringInfo::Length () {
	auto ret = new IFixedInfo (true, FixedConst::ULONG);
	return ret;
    }

    InfoType IStringInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    auto i = this-> clone ();
	    i-> binopFoo = &StringUtils::InstAff;
	    return i;
	}
	return NULL;
    }
    
    const char* IStringInfo::getId () {
	return IStringInfo::id ();
    }

    InfoType IStringInfo::ConstVerif (InfoType other) {
	if (this-> isConst () && !other-> isConst ()) return NULL;
	else if (!this-> isConst () && other-> isConst ()) {
	    this-> isConst () = true;
	}
	return this;	
    }

    Ymir::Tree IStringInfo::toGeneric () {
	return toGenericStatic ();
    }
    
    Ymir::Tree IStringInfo::toGenericStatic () {
	Ymir::Tree string_type_node = Ymir::makeStructType ("string", 2,
						 get_identifier ("len"),
						 (new IFixedInfo (true, FixedConst::ULONG))-> toGeneric ().getTree (),
						 get_identifier ("ptr"),
						 (new IPtrInfo (true, new ICharInfo (true)))-> toGeneric ().getTree ()
	).getTree ();
		
	IFinalFrame::declareType ("string", string_type_node);
	return string_type_node;
    }

    namespace StringUtils {
	using namespace syntax;
	
	Ymir::Tree InstAff (Word word, Expression left, Expression right) {
	    location_t loc = word.getLocus ();	   
	    auto lexp = left-> toGeneric ();
	    Ymir::TreeStmtList list;

	    Ymir::Tree lenl = Ymir::getField (loc, lexp, "len");
	    Ymir::Tree ptrl = Ymir::getField (loc, lexp, "ptr");	
	    Ymir::Tree lenr;
	    Ymir::Tree ptrr;
	    
	    if (auto str = right-> to<IString> ()) {
		std::string value = str-> getStr ();
		auto intExpr = new IFixed (word, FixedConst::ULONG);
		intExpr-> setUValue (value.length ());
		auto lenExpr = intExpr-> expression ();		

		lenr = lenExpr-> toGeneric ();
		ptrr = str-> toGeneric ();

	    } else {
		auto rexp = right-> toGeneric ();
		lenr = Ymir::getField (loc, rexp, "len");
		ptrr = Ymir::getField (loc, rexp, "ptr");
	    }
	    		
	    list.append (Ymir::buildTree (
		MODIFY_EXPR, loc,
		void_type_node,
		lenl.getTree (),
		lenr.getTree ()
	    ).getTree ());
		
	    list.append (Ymir::buildTree (
		MODIFY_EXPR, loc,
		void_type_node,
		ptrl.getTree (),
		ptrr.getTree ()
	    ).getTree ());
			     
	    Ymir::getStackStmtList ().back ().append (list.getTree ());
		
	    return lexp;			    
	}
	
	Ymir::Tree InstPtr (Word locus, Expression expr) {
	    location_t loc = locus.getLocus ();
	    Ymir::TreeStmtList list;

	    if (expr-> is<IString> ()) {
		return expr-> toGeneric ();
	    } else {
		auto lexp = expr-> toGeneric ();
		return Ymir::getField (loc, lexp, "ptr");
	    }
	}

    }

    
}
