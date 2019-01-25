#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <string>
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
			Ymir::Error::halt ("%(r) - TODO var decoration", "Critical");
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
	    
	    if (!function.getBody ().getBody ().isEmpty ()) {
		if (!function.getBody ().getInner ().isEmpty () ||
		    !function.getBody ().getOuter ().isEmpty ()
		)
		    Ymir::Error::halt ("%(r) - TODO contract", "Critical");
		
		auto body = validateValue (function.getBody ().getBody ()); 
		quitBlock ();
		insertNewGenerator (Frame::init (function.getName (), function.getName ().str, params, retType, body));
		
	    } else quitBlock ();
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

	Generator Visitor::validateValue (const syntax::Expression & value) {
	    match (value) {
		of (syntax::Block, block,
		    return validateBlock (block);
		);

		of (syntax::Fixed, fixed,
		    return validateFixed (fixed);
		);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator Visitor::validateBlock (const syntax::Block & block) {
	    std::vector <Generator> values;
	    Generator type (Void::init (block.getLocation ()));
	    bool breaker = false, returner = false;
	    enterBlock ();
	    
	    for (auto & expr : block.getContent ()) {
		if (returner || breaker)
		    Error::occur (expr.getLocation (), ExternalError::get (UNREACHBLE_STATEMENT));

		auto value = validateValue (expr);
		if (value.to <Value> ().isReturner ()) returner = true;
		if (value.to <Value> ().isBreaker ()) breaker = true;
		type = value.to <Value> ().getType ();

		values.push_back (value);
	    }
	    
	    quitBlock ();
	    return Block::init (type, values);
	}	

	Generator Visitor::validateFixed (const syntax::Fixed & fixed) {
	    struct Anonymous {

		static std::string removeUnder (const std::string & value) {
		    auto aux = value;
		    aux.erase (std::remove (aux.begin (), aux.end (), '_'), aux.end ());
		    return aux;
		}
		
		static ulong convertU (const lexing::Word & loc, const Integer & type) { 
		    char * temp = nullptr; errno = 0; // errno !!
		    auto val = removeUnder (loc.str);
		    ulong value = std::strtoul (val.c_str (), &temp, 0);
		    bool overflow = false;
		    if (temp == val.c_str () || *temp != '\0' ||
			((value == 0 || value == ULONG_MAX) && errno == ERANGE)) {
			overflow = true;
		    }
		    
		    if (overflow || value > getMaxU (type))
			Error::occur (loc, ExternalError::get (OVERFLOW), type.typeName ());
		    
		    return value;
		}
		
		static long convertS (const lexing::Word & loc, const Integer & type) {
		    char * temp = nullptr; errno = 0; // errno !!
		    auto val = removeUnder (loc.str);
		    ulong value = std::strtol (val.c_str (), &temp, 0);
		    bool overflow = false;
		    if (temp == val.c_str () || *temp != '\0' ||
			((value == 0 || value == ULONG_MAX) && errno == ERANGE)) {
			overflow = true;
		    }
		    
		    if (overflow || value > getMaxS (type))
			Error::occur (loc, ExternalError::get (OVERFLOW), type.typeName ());
		    
		    return value;
		}

		static ulong getMaxU (const Integer & type) {
		    switch (type.getSize ()) {
		    case 8 : return UCHAR_MAX;
		    case 16 : return USHRT_MAX;
		    case 32 : return UINT_MAX;
		    case 64 : return ULONG_MAX;
		    }
		    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		    return 0;
		}
		
		static ulong getMaxS (const Integer & type) {
		    switch (type.getSize ()) {
		    case 8 : return SCHAR_MAX;
		    case 16 : return SHRT_MAX;
		    case 32 : return INT_MAX;
		    case 64 : return LONG_MAX;
		    }
		    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		    return 0;
		}
		
	    };

	    Generator type (Generator::empty ());
	    if (fixed.getSuffix () == Keys::U8) type = Integer::init (fixed.getLocation (), 8, false);
	    if (fixed.getSuffix () == Keys::U16) type = Integer::init (fixed.getLocation (), 16, false);
	    if (fixed.getSuffix () == Keys::U32) type = Integer::init (fixed.getLocation (), 32, false);
	    if (fixed.getSuffix () == Keys::U64) type = Integer::init (fixed.getLocation (), 64, false);
	    
	    if (fixed.getSuffix () == Keys::I8) type = Integer::init (fixed.getLocation (), 8, true);
	    if (fixed.getSuffix () == Keys::I16) type = Integer::init (fixed.getLocation (), 16, true);
	    if (fixed.getSuffix () == "") type = Integer::init (fixed.getLocation (), 32, true);
	    if (fixed.getSuffix () == Keys::I64) type = Integer::init (fixed.getLocation (), 64, true);

	    auto integer = type.to<Integer> ();
	    Fixed::UI value;
	    
	    if (integer.isSigned ()) value.i = Anonymous::convertS (fixed.getLocation (), integer);
	    else value.u = Anonymous::convertU (fixed.getLocation (), integer);	    
	    
	    return Fixed::init (type, value);
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
