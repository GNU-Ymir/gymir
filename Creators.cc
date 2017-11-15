#include "semantic/types/Creators.hh"
#include "semantic/types/InfoType.hh"

namespace semantic {

    Creators Creators::__instance__;
    
    Creators::Creators () {
	// this-> creators ["int"] = FixedInfo::create;
	// this-> creators ["uint"] = FixedInfo::create;
	// this-> creators ["short"] = FixedInfo::create;
	// this-> creators ["ushort"] = FixedInfo::create;
	// this-> creators ["byte"] = FixedInfo::create;
	// this-> creators ["ubyte"] = FixedInfo::create;
	// this-> creators ["long"] = FixedInfo::create;
	// this-> creators ["ulong"] = FixedInfo::create;
    }

    void* Creators::find (const std::string & elem) {
	auto it = this-> creators.find (elem);
	if (it != this-> creators.end ()) return it-> second;
	else return NULL;
    }    

}
