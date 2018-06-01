#include <ymir/semantic/object/AggregateInfo.hh>
#include <ymir/semantic/types/FunctionInfo.hh>
#include <ymir/ast/Var.hh>
#include <ymir/semantic/utils/StructUtils.hh>
#include <ymir/semantic/types/UndefInfo.hh>
#include <ymir/ast/ParamList.hh>

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

    std::vector <FunctionInfo> & IAggregateCstInfo::getConstructors () {
	return this-> _contrs;
    }

    FunctionInfo & IAggregateCstInfo::getDestructor () {
	return this-> _destr;
    }

    std::vector <FunctionInfo> & IAggregateCstInfo::getMethods () {
	return this-> _methods;
    }

    std::vector <FunctionInfo> & IAggregateCstInfo::getStaticMethods () {
	return this-> _staticMeth;
    }

    Namespace IAggregateCstInfo::space () {
	return this-> _space;
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
    
    
    InfoType IAggregateCstInfo::TempOp (const std::vector <Expression> & exprs) {
	auto expr = this-> _impl [0]-> expression ();
	if (expr-> info-> type-> is <IStructCstInfo> ())  {
	    auto str = expr-> info-> type-> TempOp (exprs);
	    if (str) {
		auto ret = new (Z0) IAggregateInfo (this, this-> _space, this-> _name, {});
		ret-> _impl = str-> to <IStructInfo> ();
		if (this-> _destr)
		    ret-> _destr = this-> _destr-> frame ();
		return ret;
	    } return NULL;
	} else {
	    //TODO
	    return NULL;
	}
    }

    InfoType IAggregateCstInfo::DColonOp (Var var) {
	if (var-> token == "init") return Init ();	
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

    std::string IAggregateCstInfo::name () {
	return this-> _name;
    }
    
    const char* IAggregateCstInfo::getId () {
	return IAggregateCstInfo::id ();
    }

    Word IAggregateCstInfo::getLocId () {
	return this-> _locId;	
    }

    InfoType IAggregateCstInfo::Init () {
	if (this-> _contrs.size () == 0) {
	    return NULL;
	} else {
	    std::vector <Frame> frames;
	    for (auto it : this-> _contrs)
		frames.push_back (it-> frame ());
	    auto ret = new (Z0) IFunctionInfo (this-> _space, "init", frames);
	    ret-> isConstr () = true;
	    return ret;
	}
    }

    IAggregateInfo::IAggregateInfo (AggregateCstInfo from, Namespace space, std::string name, const std::vector <syntax::Expression> & tmpsDone) :
	IInfoType (false),
	_space (space),
	_name (name),
	tmpsDone (tmpsDone),
	_id (from)
    {}

    bool IAggregateInfo::isSame (InfoType other) {
	if (auto ot = other-> to <IAggregateInfo> ()) {
	    if (this-> _space == ot-> _space && this-> _name == ot-> _name && this-> _id-> getLocId ().isSame (ot-> _id-> getLocId ())) {
		if (this-> tmpsDone.size () != ot-> tmpsDone.size ()) {
		    return false;		    
		}

		for (auto it : Ymir::r (0, this-> tmpsDone.size ())) {
		    if (auto ps = this-> tmpsDone [it]-> to <IParamList> ()) {
			if (auto ps2 = ot-> tmpsDone [it]-> to <IParamList> ()) {
			    if (ps-> getParams ().size () != ps2-> getParams ().size ()) {
				return false;
			    }
			    for (auto it_ : Ymir::r (0, ps-> getParams ().size ())) {
				if (!ps-> getParams ()[it_]-> info-> type-> isSame (ps2-> getParams () [it_]-> info-> type)) {
				    return false;
				}
			    }
			} else {
			    return false;
			}
		    } else {
			if (ot-> tmpsDone [it]-> is <IParamList> ()) {
			    return false;
			}
			if (!this-> tmpsDone [it]-> info-> type-> isSame (ot-> tmpsDone [it]-> info-> type)) {
			    return false;
			}
		    }
		}		    
		return true;
	    }
	}
	return false;
    }

    Frame IAggregateInfo::getDestructor () {
	return this-> _destr;
    }   
    
    InfoType IAggregateInfo::ConstVerif (InfoType type) {
	if (auto aggr = type-> to <IAggregateInfo> ())
	    if (this-> _impl-> ConstVerif (aggr-> _impl))
		return this;
	return NULL;
    }

    InfoType IAggregateInfo::onClone () {
	auto ret = new (Z0) IAggregateInfo (this-> _id, this-> _space, this-> _name, this-> tmpsDone);
	ret-> _impl = this-> _impl-> clone ()-> to <IStructInfo> ();
	ret-> _destr = this-> _destr;	
	
	return ret;
    }

    InfoType IAggregateInfo::BinaryOp (Word op, Expression right) {
	if (op == Token::EQUAL && right-> info-> type-> isSame (this)) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }

    InfoType IAggregateInfo::BinaryOpRight (Word op, Expression left) {
	if (op == Token::EQUAL && left-> info-> type-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IAggregateInfo::DotOp (Var var) {
	auto ret = this-> _impl-> DotOp (var);
	return ret;
    }

    InfoType IAggregateInfo::DColonOp (Var) {
	return NULL;
    }

    InfoType IAggregateInfo::CompOp (InfoType type) {
	return type-> clone ();
    }

    std::string IAggregateInfo::getName () {
	return this-> _name;
    }

    Namespace & IAggregateInfo::getSpace () {
	return this-> _space;
    }

    std::string IAggregateInfo::innerTypeString () {
	return this-> _name;	
    }

    std::string IAggregateInfo::innerSimpleTypeString () {
	return this-> _name;
    }

    Ymir::Tree IAggregateInfo::toGeneric () {
	return this-> _impl-> toGeneric ();
    }

    void IAggregateInfo::setTmps (const std::vector <Expression> & tmps) {
	this-> tmpsDone = tmps;
    }

    InfoType IAggregateInfo::getTemplate (ulong) {
	return NULL;
    }
    
    std::vector <InfoType> IAggregateInfo::getTemplate (ulong, ulong) {
	return {};
    }

    const char * IAggregateInfo::getId () {
	return IAggregateInfo::id ();
    }
    
    
}
