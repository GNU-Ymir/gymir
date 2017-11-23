#include <ymir/semantic/pack/FrameProto.hh>
#include <ymir/errors/Error.hh>
#include <ymir/utils/Range.hh>
#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    IFrameProto::IFrameProto (std::string name, Namespace space, Symbol type, std::vector <syntax::Var> vars, std::vector <syntax::Expression> tmps) :
	_space (space),
	_name (name),
	_type (type),
	_vars (vars),
	_tmps (tmps),
	_extern ("")
    {}

    std::string& IFrameProto::name () {
	return this-> _name;
    }

    Namespace IFrameProto::space () {
	return this-> _space;
    }

    Symbol& IFrameProto::type () {
	return this-> _type;
    }

    std::vector <syntax::Var> & IFrameProto::vars () {
	return this-> _vars;
    }

    std::string& IFrameProto::externName () {
	return this-> _extern;
    }

    bool IFrameProto::equals (IFrameProto* scd) {
	if (scd) {
	    if (this-> _space != scd-> _space) return false;
	    if (this-> _name != scd-> _name) return false;
	    if (this-> _tmps.size () != scd-> _tmps.size () ||
		this-> _vars.size () != scd-> _vars.size ()) return false;
	    
	    for (auto it : Ymir::r (0, this-> _tmps.size ())) {
		Ymir::Error::assert ("TODO");
	    }

	    for (auto it : Ymir::r (0, this-> _vars.size ())) {
		if (this-> _vars [it]-> info-> type-> simpleTypeString () !=
		    scd-> _vars  [it]-> info-> type-> simpleTypeString ())
		    return false;
	    }
	    return true;
	}
	return false;
    }
    

    

}
