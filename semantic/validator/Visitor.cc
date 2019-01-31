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
		
		of (syntax::Float, f,
		    return validateFloat (f);
		);

		of (syntax::Char, c,
		    return validateChar (c);
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

	Generator Visitor::validateFloat (const syntax::Float & f) {
	    Generator type (Generator::empty ());
	    if (f.getSuffix () == Keys::FLOAT_S) type = Float::init (f.getLocation (), 32);
	    else {
		type = Float::init (f.getLocation (), 64);
	    }

	    return FloatValue::init (f.getLocation (), type, f.getValue ());
	}

	Generator Visitor::validateChar (const syntax::Char & c) {
	    struct Anonymous {
		
		static size_t utf8_codepoint_size(uint8_t text) {
		    // According to utf-8 documentation, a continuous char begin with 10xxxxxx,
		    // Meaning 01101000 10111111 is 1 in length, and 0111111 11010100 is 2 in length
		    
		    if((text & 0b10000000) == 0) {
			return 1;
		    }

		    if((text & 0b11100000) == 0b11000000) {
			return 2;
		    }

		    if((text & 0b11110000) == 0b11100000) {
			return 3;
		    }
		    
		    return 4;
		}

		static std::vector <uint> utf8_to_utf32(const std::string & text) {
		    std::vector <uint> res;
		    size_t i = 0;

		    for (size_t n = 0; i < text.length (); n++) {
			size_t byte_count = utf8_codepoint_size(text[i]);
			
			uint a = 0, b = 0, c = 0, d = 0;
			uint a_mask, b_mask, c_mask, d_mask;
			a_mask = b_mask = c_mask = d_mask = 0b00111111;
			
			switch(byte_count) {
			case 4 : {
			    a = text [i]; b = text [i + 1]; c = text [i + 2]; d = text [i + 3];
			    a_mask = 0b00000111;
			} break;
			case 3 : {
			    b = text [i]; c = text [i + 1]; d = text [i + 2];
			    b_mask = 0b00001111;
			} break;
			case 2 : {
			    c = text [i]; d = text [i + 1];
			    c_mask = 0b00011111;
			} break;
			
			case 1 : {
			    d = text [i];
			    d_mask = 0b01111111;
			} break;
			}
			
			uint b0 = a & a_mask;
			uint b1 = b & b_mask;
			uint b2 = c & c_mask;
			uint b3 = d & d_mask;
			res.push_back ((b0 << 18) | (b1 << 12) | (b2 << 6) | b3);

			i += byte_count;
		    }

		    return res;
		}
	       		
		static std::string escapeChar (const lexing::Word & loc, const std::string & content) {
		    OutBuffer buf;
		    int it = 0;
		    static std::vector <char> escape = {'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '\'', '\"', '"', '?', '\0'};
		    static std::vector <uint> values = {7, 8, 12, 10, 13, 9, 11, 92, 39, 34, 63};
		    
		    while (it < (int) content.size ()) {
			if (it == '\\') {
			    if (it + 1 < (int) content.size ()) {
				it += 1;				
				auto pos = std::find (escape.begin (), escape.end (), content [it]) - escape.begin ();
				if (pos >= (int) escape.size ()) {
				    auto real_loc = loc;
				    real_loc.column += it;
				    Error::occur (real_loc, ExternalError::get (UNDEFINED_ESCAPE));
				}
				
				buf.write ((char) values [pos]);				
			    } else {
				auto real_loc = loc;
				real_loc.column += it;
				Error::occur (real_loc, ExternalError::get (UNTERMINATED_SEQUENCE));
			    }
			} else buf.write (content [it]);
			it ++;
		    }
		    return buf.str ();
		}
		
		static uint convert (const lexing::Word & loc, const lexing::Word & content, int size) {
		    auto str = // escapeChar (loc, 
					   content.str;
		    if (size == 32) {
			std::vector <uint> utf_32 = utf8_to_utf32 (str);
			if (utf_32.size () != 1) {		    
			    Ymir::Error::occur (loc, ExternalError::get (MALFORMED_CHAR), "c32", utf_32.size ());
			}
			return utf_32 [0];
		    } else if (size == 8) {
			if (str.length () != 1)
			    Ymir::Error::occur (loc, ExternalError::get (MALFORMED_CHAR), "c8", str.length ());
			return str [0] & 0b01111111;
		    }
		    
		    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		    return 0;
		}
		    
	    };
	    
	    Generator type (Generator::empty ());	    
	    if (c.getSuffix () == Keys::C8) type = Char::init (c.getLocation (), 8);
	    if (c.getSuffix () == "") type = Char::init (c.getLocation (), 32);
	    
	    uint value = Anonymous::convert (c.getLocation (), c.getSequence (), type.to<Char> ().getSize ());	   
	    return CharValue::init (c.getLocation (), type, value);
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
	    if (std::find (Integer::NAMES.begin (), Integer::NAMES.end (), var.getName ().str) != Integer::NAMES.end ()) {
		auto size = var.getName ().str.substr (1);
		
		// According to c++ documentation atoi return 0, if the conversion failed
		return Integer::init (var.getName (), std::atoi (size.c_str ()), var.getName ().str[0] == 'i');
	    } else if (var.getName ().str == Void::NAME) {
		return Void::init (var.getName ());
	    } else if (var.getName ().str == Bool::NAME) {
		return Bool::init (var.getName ());
	    } else if (std::find (Float::NAMES.begin (), Float::NAMES.end (), var.getName ().str) != Float::NAMES.end ()) {
		auto size = var.getName ().str.substr (1);
		return Float::init (var.getName (), std::atoi (size.c_str ())); 
	    } else if (std::find (Char::NAMES.begin (), Char::NAMES.end (), var.getName ().str) != Char::NAMES.end ()) {
		return Char::init (var.getName (), std::atoi (var.getName ().str.substr (1).c_str ()));
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
