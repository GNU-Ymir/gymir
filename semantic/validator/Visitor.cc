#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/validator/BinaryVisitor.hh>
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

		if (!body.to<Value> ().isReturner () && (!retType.isEmpty () && !retType.is<Void> ())) {
		    if (!body.to <Value> ().getType ().equals (retType))
			Ymir::Error::occur (body.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    body.to <Value> ().getType ().to <Type> ().typeName (),
					    retType.to <Type> ().typeName ()
			);
		}

		if (retType.isEmpty ()) {
		    retType = body.to<Value> ().getType ();
		}		
		
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

		of (syntax::Bool, b,
		    return validateBool (b);
		);
		
		of (syntax::Binary, binary,
		    return validateBinary (binary);
		);

		of (syntax::Var, var,
		    return validateVar (var);
		);

		of (syntax::VarDecl, var,
		    return validateVarDeclValue (var);
		);

		of (syntax::Set, set,
		    return validateSet (set);
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

	    std::vector <std::string> errors;
	    for (int i = 0 ; i < (int) block.getContent ().size () ; i ++) {
		TRY (
		    if (returner || breaker) {			
			Error::occur (block.getContent () [i].getLocation (), ExternalError::get (UNREACHBLE_STATEMENT));
		    }
		    
		    auto value = validateValue (block.getContent () [i]);
		    
		    if (value.to <Value> ().isReturner ()) returner = true;
		    if (value.to <Value> ().isBreaker ()) breaker = true;
		    type = value.to <Value> ().getType ();
		    
		    values.push_back (value);		    
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());		    
		} FINALLY;
	    }

	    TRY (
		quitBlock ();
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors.insert (errors.end (), msgs.begin (), msgs.end ());
	    } FINALLY;
	    
	    if (errors.size () != 0) {
		THROW (ErrorCode::EXTERNAL, errors);
	    }
	    
	    return Block::init (block.getLocation (), type, values);
	}	

	Generator Visitor::validateSet (const syntax::Set & set) {
	    std::vector <Generator> values;
	    Generator type (Void::init (set.getLocation ()));
	    bool breaker = false, returner = false;

	    std::vector <std::string> errors;
	    for (int i = 0 ; i < (int) set.getContent ().size () ; i ++) {
		TRY (
		    if (returner || breaker) {			
			Error::occur (set.getContent () [i].getLocation (), ExternalError::get (UNREACHBLE_STATEMENT));
		    }
		    
		    auto value = validateValue (set.getContent () [i]);
		    
		    if (value.to <Value> ().isReturner ()) returner = true;
		    if (value.to <Value> ().isBreaker ()) breaker = true;
		    type = value.to <Value> ().getType ();
		    
		    values.push_back (value);		    
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());		    
		} FINALLY;
	    }

	    if (errors.size () != 0) {
		THROW (ErrorCode::EXTERNAL, errors);
	    }

	    return Set::init (set.getLocation (), type, values);
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
	    
	    return Fixed::init (fixed.getLocation (), type, value);
	}       

	Generator Visitor::validateBool (const syntax::Bool & b) {
	    return BoolValue::init (b.getLocation (), Bool::init (b.getLocation ()), b.getLocation () == Keys::TRUE_);
	}
	
	Generator Visitor::validateBinary (const syntax::Binary & bin) {
	    auto binVisitor = BinaryVisitor::init (*this);
	    return binVisitor.validate (bin);
	}
	
	Generator Visitor::validateVar (const syntax::Var & var) {
	    auto & gen = getLocal (var.getName ().str);
	    if (gen.isEmpty ()) {
		Error::occur (var.getLocation (), ExternalError::get (UNDEF_VAR), var.getName ().str);
	    }

	    // The gen that we got can be either a param decl or a var decl
	    if (gen.is <ParamVar> ()) {
		return VarRef::init (var.getLocation (), var.getName ().str, gen.to<Value> ().getType (), gen.getUniqId ());
	    } else if (gen.is <generator::VarDecl> ()) {
		return VarRef::init (var.getLocation (), var.getName ().str, gen.to<generator::VarDecl> ().getVarType (), gen.getUniqId ());		
	    } 

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator Visitor::validateVarDeclValue (const syntax::VarDecl & var) {
	    auto & gen = getLocal (var.getName ().str);
	    if (!gen.isEmpty ()) {
		auto note = Ymir::Error::createNote (gen.getLocation ());
		Error::occurAndNote (var.getLocation (), note, ExternalError::get (SHADOWING_DECL), var.getName ().str);
	    }

	    if (var.getValue ().isEmpty () && var.getType ().isEmpty ()) {
		Error::occur (var.getLocation (), ExternalError::get (VAR_DECL_WITH_NOTHING));
	    }

	    Generator value (Generator::empty ());
	    if (!var.getValue ().isEmpty ()) value = validateValue (var.getValue ());

	    Generator type (Generator::empty ());
	    if (!var.getType ().isEmpty ()) type = validateType (var.getType ());

	    if (!type.isEmpty () && !value.isEmpty ()) {
		if (!type.equals (value.to <Value> ().getType ())) {
		    Ymir::Error::occur (value.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					value.to <Value> ().getType ().to <Type> ().typeName (),
					type.to <Type> ().typeName ()
		    );
		}
	    } else if (type.isEmpty ()) {
		type = value.to <Value> ().getType ();
	    }

	    if (type.is<Void> ()) {
		Ymir::Error::occur (type.getLocation (), ExternalError::get (VOID_VAR));
	    }
	    
	    auto ret = generator::VarDecl::init (var.getLocation (), var.getName ().str, type, value);
	    insertLocal (var.getName ().str, ret);
	    return ret;
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
	    } else if (var.getName ().str == "void") {
		return Void::init (var.getName ());
	    } else if (var.getName ().str == "bool")
		return Bool::init (var.getName ());
	    
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
	    this-> _usedSyms.push_back ({});
	    this-> _symbols.push_back ({});
	}

	void Visitor::quitBlock () {
	    if (this-> _symbols.empty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");

	    std::vector <std::string> errors;
	    for (auto & sym : this-> _symbols.back ()) {
		if (this-> _usedSyms.back ().find (sym.first) == this-> _usedSyms.back ().end ()) {
		    errors.push_back (Error::makeWarn (sym.second.getLocation (), ExternalError::get (NEVER_USED), sym.second.getName ()));
		}
	    }

	    this-> _usedSyms.pop_back ();
	    this-> _symbols.pop_back ();

	    if (errors.size () != 0) {
		THROW (ErrorCode::EXTERNAL, errors);
	    }
	}

	void Visitor::insertLocal (const std::string & name, const Generator & gen) {
	    if (this-> _symbols.empty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    this-> _symbols.back ().emplace (name, gen);
	}       

	const Generator & Visitor::getLocal (const std::string & name) {
	    for (auto it : Ymir::r (0, this-> _symbols.size ())) {
		auto ptr = this-> _symbols [it].find (name); 		    
		if (ptr != this-> _symbols [it].end ()) {
		    this-> _usedSyms [it].insert (name);
		    return ptr-> second;
		}		
	    }
	    
	    return Generator::__empty__;
	}
	
    }
    
}
