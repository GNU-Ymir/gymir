#include <ymir/semantic/object/AggregateInfo.hh>

using namespace syntax;

namespace semantic {

    IAggregateCstInfo::IAggregateCstInfo (Word locId, Namespace space, std::string name, const std::vector <syntax::Expression> & tmps, const std::vector <syntax::Expression> & self, bool isUnion) :
	IInfoType (true),
	_space (space),
	_locId (locId),
	_name (name),
	_contrs ({}), _destr (NULL), _methods ({}), _staticMeth ({}),
	_tmps (tmps),
	_impl (self),
	_isUnion (isUnion)
	
    {}

    std::vector <InfoType> & IAggregateCstInfo::getConstructors () {
	return this-> _contrs;
    }

    InfoType & IAggregateCstInfo::getDestructor () {
	return this-> _destr;
    }

    std::vector <InfoType> & IAggregateCstInfo::getMethods () {
	return this-> _methods;
    }

    std::vector <InfoType> & IAggregateCstInfo::getStaticMethods () {
	return this-> _staticMeth;
    }

    bool IAggregateCstInfo::isSame (InfoType other) {
	if (other == this) return true;
	if (auto ot = other-> to <IAggregateCstInfo> ()) {
	    if (ot-> _name == this-> _name && ot-> _space == this-> _space) {
		return true;
	    }
	}
	return false;
    }


    InfoType IAggregateCstInfo::onClone () {
	return this;
    }

    InfoType IAggregateCstInfo::TempOp (const std::vector <Expression> &) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }

    InfoType IAggregateCstInfo::DColonOp (Var) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }
    
    std::string IAggregateCstInfo::typeString () {
	return this-> _name;
    }

    std::string IAggregateCstInfo::innerTypeString () {
	return this-> _name;
    }    
    
    std::string IAggregateCstInfo::innerSimpleTypeString () {
	return Ymir::OutBuffer (this-> _name.length (), this-> _name).str ();
    }

    Ymir::Tree IAggregateCstInfo::toGeneric () {
	Ymir::Error::assert ("TODO");
	return Ymir::Tree ();
    }

    bool IAggregateCstInfo::isType () {
	return true;
    }

    const char* IAggregateCstInfo::getId () {
	return IAggregateCstInfo::id ();
    }

    Word IAggregateCstInfo::getLocId () {
	return this-> _locId;
    }
    
}
