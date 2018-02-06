#include <ymir/semantic/pack/FrameProto.hh>
#include <ymir/errors/Error.hh>
#include <ymir/utils/Range.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/pack/Symbol.hh>
#include <ymir/ast/Var.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/value/_.hh>

namespace semantic {

    IFrameProto::IFrameProto (std::string name, Namespace space, Symbol type, const std::vector <syntax::Var>& vars, const std::vector <syntax::Expression>& tmps) :
	_space (space),
	_name (name),
	_type (type),
	_vars (vars),
	_tmps (tmps),
	_extern (""),
	_fn ()
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

    std::vector <syntax::Expression>& IFrameProto::tmps () {
	return this-> _tmps;
    }

    std::string& IFrameProto::externName () {
	return this-> _extern;
    }

    bool& IFrameProto::isCVariadic () {
	return this-> _isCVariadic;
    }
    
    bool IFrameProto::equals (IFrameProto* scd) {
	if (scd) {
	    if (this-> _space != scd-> _space) return false;
	    if (this-> _name != scd-> _name) return false;
	    if (this-> _tmps.size () != scd-> _tmps.size () ||
		this-> _vars.size () != scd-> _vars.size ()) return false;
	    
	    for (auto it : Ymir::r (0, this-> _tmps.size ())) {
		if (this-> _tmps [it]-> info-> isImmutable () != scd-> _tmps [it]-> info-> isImmutable ()) {
		    return false;
		} else if (this-> _tmps [it]-> info-> isImmutable ()) {
		    if (!this-> _tmps [it]-> info-> value ()-> equals (scd-> _tmps [it]-> info-> value ())) {
			return false;
		    }
		} else if (this-> _tmps [it]-> info-> type-> simpleTypeString () != scd-> _tmps [it]-> info-> type-> simpleTypeString ())
		    return false;
	    }
	    
	    for (auto it : Ymir::r (0, this-> _vars.size ())) {
		if (this-> _vars [it]-> info-> type-> simpleTypeString () != scd-> _vars  [it]-> info-> type-> simpleTypeString () ||
		    this-> _vars [it]-> info-> isConst () != scd-> _vars  [it]-> info-> type-> isConst ())
		    return false;
	    }
	    return true;
	}
	return false;
    }
    
    Ymir::Tree IFrameProto::toGeneric () {
	if (this-> _fn.isNull ()) {
	    std::vector <tree> fndecl_type_params (this-> _vars.size ());
	    for (uint i = 0 ; i < this-> _vars.size () ; i++) {
		fndecl_type_params [i] = this-> _vars [i]-> info-> type-> toGeneric ().getTree ();
	    }

	    std::string ident;		
	    tree ret = this-> _type-> type-> toGeneric ().getTree ();
	    tree fndecl_type;
	    if (this-> isCVariadic ()) {
		ident = this-> _name;
		fndecl_type = build_varargs_function_type_array (ret, 0, fndecl_type_params.data ());
	    } else {
		if (this-> _extern == "C") ident = this-> _name;		
		else if (this-> _extern == "") ident = Mangler::mangle_function (this-> _name, this);
		else Ymir::Error::assert ("TODO");
		
		fndecl_type = build_function_type_array (ret, 0, fndecl_type_params.data ());
	    }

	    tree fndecl = build_fn_decl (ident.c_str (), fndecl_type);

	    if (this->_extern != "") {
		DECL_EXTERNAL (fndecl) = 1;
	    }
	
	    this-> _fn =  build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}
	return this-> _fn;
    }        

}
