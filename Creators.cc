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
	return templates [0]-> info-> type-> cloneConst ();	
    }
    
    Creators::Creators () {
	this-> creators ["int"] = (void*)&IFixedInfo::create;
	this-> creators ["uint"] = (void*)&IFixedInfo::create;
	this-> creators ["short"] = (void*)&IFixedInfo::create;
	this-> creators ["ushort"] = (void*) &IFixedInfo::create;
	this-> creators ["byte"] = (void*) &IFixedInfo::create;
	this-> creators ["ubyte"] = (void*) &IFixedInfo::create;
	this-> creators ["long"] = (void*) &IFixedInfo::create;
	this-> creators ["ulong"] = (void*) &IFixedInfo::create;
	this-> creators ["char"] = (void*) &ICharInfo::create;
	this-> creators ["float"] = (void*) &IFloatInfo::create;
	this-> creators ["double"] = (void*) &IFloatInfo::create;
	this-> creators ["string"] = (void*) &IStringInfo::create;
	this-> creators ["p"] = (void*) &IPtrInfo::create;
	this-> creators ["r"] = (void*) &IRangeInfo::create;
	this-> creators ["t"] = (void*) &ITupleInfo::create;
	this-> creators ["const"] = (void*) &createConst;
	this-> creators ["ref"] = (void*) &IRefInfo::create;
	this-> creators ["void"] = (void*) &IVoidInfo::create;
	this-> creators ["bool"] = (void*) IBoolInfo::create;
	this-> creators ["fn"] = (void*) IPtrFuncInfo::create;
    }

    void* Creators::find (const std::string & elem) {
	auto it = this-> creators.find (elem);
	if (it != this-> creators.end ()) return it-> second;
	else return NULL;
    }    

}
