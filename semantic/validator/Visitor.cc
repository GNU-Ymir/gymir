#include <ymir/semantic/validator/Visitor.hh>
#include <algorithm>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;       
	
	Visitor::Visitor ()
	{}

	Visitor Visitor::init () {
	    return Visitor ();
	}

	void Visitor::validate (const semantic::Symbol & sym) {	    
	    match (sym) {
		of (semantic::Module, mod, {
			validateModule (mod);
			return;
		    }
		);

		of (semantic::Function, func, {
			validateFunction (func);
			return;
		    }
		);

		of (semantic::VarDecl, decl, {
			validateVarDecl (decl);
			return;
		    }
		);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	}

	void Visitor::validateModule (const semantic::Module & mod) {
	    const std::vector <Symbol> & syms = mod.getAllLocal ();
	    std::vector <Generator> ret;

	    for (auto & it : syms) {
		validate (it);
	    }
	}

	void Visitor::validateFunction (const semantic::Function & func) {
	    auto & function = func.getContent ();
	    std::vector <Generator> params;

	    enterBlock ();
	    for (auto & param : function.getPrototype ().getParameters ()) {
		auto var = param.to <syntax::VarDecl> ();
		if (!var.getType ().isEmpty ()) {
		    auto type = validateType (var.getType ());
		    if (!var.getDecos ().empty ()) {
			Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		    }

		    params.push_back (ParamVar::init (var.getName (), var.getName ().str, type));
		    insertLocal (var.getName ().str, params.back ());
		} else {
		    quitBlock ();
		    return; // This function is uncomplete, we can't validate it
		}
	    }
	    
	    Generator retType (Generator::empty ());
	    if (!function.getPrototype ().getType ().isEmpty ())
		retType = validateType (function.getPrototype ().getType ());
	    
	    quitBlock ();
	    insertNewGenerator (Frame::init (function.getName (), function.getName ().str, params, retType, Generator::empty ()));
	}

	void Visitor::validateVarDecl (const semantic::VarDecl & var) {
	    Generator type (Generator::empty ());
	    Generator value (Generator::empty ());
	    
	    if (!var.getType ().isEmpty ()) {
		type = validateType (var.getType ());
	    }

	    if (!var.getValue ().isEmpty ()) {
		value = validateValue (var.getValue ());
	    }

	    if (type.isEmpty () && value.isEmpty ()) {
		Error::occur (var.getName (), ExternalError::get (VAR_DECL_WITH_NOTHING), var.getName ().str);
	    }

	    if (!value.isEmpty () && !type.isEmpty ()) {
		if (!type.equals (value.to<Value> ().getType ())) {
		    Error::occur (var.getName (), ExternalError::get (INCOMPATIBLE_TYPES), type.to<Type> ().typeName (), value.to<Value> ().getType ().to<Type> ().typeName ());
		}
	    }

	    insertNewGenerator (GlobalVar::init (var.getName (), var.getName ().str, type, value));
	}

	Generator Visitor::validateValue (const syntax::Expression &) {
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator Visitor::validateType (const syntax::Expression & type) {
	    match (type) {
		of (syntax::Var, var,
		    return validateTypeVar (var);
		);
	    }
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator Visitor::validateTypeVar (const syntax::Var & var) {
	    auto intName = {"i8", "i16", "i32", "i64",
			    "u8", "u16", "u32", "u64"};
	    if (std::find (intName.begin (), intName.end (), var.getName ().str) != intName.end ()) {
		auto size = var.getName ().str.substr (1);
		if (size == "8") return Integer::init (var.getName (), 8, var.getName ().str[0] == 'i');
		if (size == "16") return Integer::init (var.getName (), 16, var.getName ().str[0] == 'i');
		if (size == "32") return Integer::init (var.getName (), 32, var.getName ().str[0] == 'i');
		if (size == "64") return Integer::init (var.getName (), 64, var.getName ().str[0] == 'i');
	    }
	    
	    Error::occur (var.getName (), ExternalError::get (UNDEF_TYPE), var.getName ().str);
	    return Generator::empty ();
	}


	const std::vector <generator::Generator> & Visitor::getGenerators () const {
	    return this-> _list;
	}
	
	void Visitor::insertNewGenerator (const Generator & generator) {
	    this-> _list.push_back (generator);
	}

	void Visitor::enterBlock () {
	    this-> _symbols.push_back ({});
	}

	void Visitor::quitBlock () {
	    if (this-> _symbols.empty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    this-> _symbols.pop_back ();
	}

	void Visitor::insertLocal (const std::string & name, const Generator & gen) {
	    if (this-> _symbols.empty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    this-> _symbols.back ().emplace (name, gen);
	}       

    }
    
}
