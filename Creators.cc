#include "semantic/types/Creators.hh"
#include "semantic/types/InfoType.hh"
#include <semantic/types/_.hh>
#include <syntax/Keys.hh>

namespace semantic {

    Creators Creators::__instance__;

    InfoType createConst (Word token, std::vector <syntax::Expression> templates) {
	if (templates.size () != 1 ||
	    (!templates [0]-> is <::syntax::IType> () && !templates [0]-> info-> type-> isType ())
	) {
	    Ymir::Error::takeATypeAsTemplate (token);
	    return NULL;	
	}
	
	if (templates [0]-> info-> type-> is <IStructCstInfo> ()) {
	    auto ret =  templates [0]-> info-> type-> TempOp ({});
	    ret-> isConst (true);
	    ret-> isType (true);
	    return ret;
	}
	return templates [0]-> info-> type-> cloneConst ();	
    }
    
    Creators::Creators () {
	this-> creators ["i32"] = (void*)&IFixedInfo::create;
	this-> creators ["u32"] = (void*)&IFixedInfo::create;
	this-> creators ["i16"] = (void*)&IFixedInfo::create;
	this-> creators ["u16"] = (void*) &IFixedInfo::create;
	this-> creators ["i8"] = (void*) &IFixedInfo::create;
	this-> creators ["u8"] = (void*) &IFixedInfo::create;
	this-> creators ["i64"] = (void*) &IFixedInfo::create;
	this-> creators ["u64"] = (void*) &IFixedInfo::create;
	this-> creators ["char"] = (void*) &ICharInfo::create;
	this-> creators ["f32"] = (void*) &IFloatInfo::create;
	this-> creators ["f64"] = (void*) &IFloatInfo::create;
	this-> creators ["string"] = (void*) &IStringInfo::create;
	this-> creators ["p"] = (void*) &IPtrInfo::create;
	this-> creators ["r"] = (void*) &IRangeInfo::create;
	this-> creators ["t"] = (void*) &ITupleInfo::create;
	this-> creators ["const"] = (void*) &createConst;
	this-> creators ["ref"] = (void*) &IRefInfo::create;
	this-> creators ["void"] = (void*) &IVoidInfo::create;
	this-> creators ["bool"] = (void*) IBoolInfo::create;
	this-> creators ["fn"] = (void*) IPtrFuncInfo::create;
	this-> creators ["dg"] = (void*) IPtrFuncInfo::createDeg;
    }

    void* Creators::find (const std::string & elem) {
	auto it = this-> creators.find (elem);
	if (it != this-> creators.end ()) return it-> second;
	else return NULL;
    }    

}
