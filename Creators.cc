#include "semantic/types/Creators.hh"
#include "semantic/types/InfoType.hh"
#include <semantic/types/_.hh>

namespace semantic {

    Creators Creators::__instance__;
    
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
    }

    void* Creators::find (const std::string & elem) {
	auto it = this-> creators.find (elem);
	if (it != this-> creators.end ()) return it-> second;
	else return NULL;
    }    

}
