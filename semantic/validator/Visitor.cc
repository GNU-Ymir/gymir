#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/validator/BinaryVisitor.hh>
#include <ymir/semantic/validator/BracketVisitor.hh>
#include <ymir/semantic/validator/CallVisitor.hh>
#include <ymir/semantic/validator/CompileTime.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/semantic/validator/UnaryVisitor.hh>
#include <ymir/semantic/validator/SubVisitor.hh>
#include <ymir/semantic/validator/DotVisitor.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <string>
#include <algorithm>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;       
	
	Visitor::Visitor ()
	{
	    enterForeign ();
	}

	Visitor Visitor::init () {
	    return Visitor ();
	}

	void Visitor::validate (const semantic::Symbol & sym) {	    
	    match (sym) {
		of (semantic::Module, mod, {
			std::vector <std::string> errors;
			this-> _referent.push_back (sym);			
			TRY (
			    validateModule (mod);
			) CATCH (ErrorCode::EXTERNAL) {
			    GET_ERRORS_AND_CLEAR (msgs);
			    errors.insert (errors.end (), msgs.begin (), msgs.end ());
			} FINALLY;
			
			this-> _referent.pop_back ();
			
			if (errors.size () != 0) {
			    THROW (ErrorCode::EXTERNAL, errors);
			}
			return;
		    }
		);

		of (semantic::Function, func, {
			std::vector <std::string> errors;
			this-> _referent.push_back (sym);			
			TRY (
			    validateFunction (func);
			) CATCH (ErrorCode::EXTERNAL) {
			    GET_ERRORS_AND_CLEAR (msgs);
			    errors.insert (errors.end (), msgs.begin (), msgs.end ());
			} FINALLY;
			
			this-> _referent.pop_back ();
			
			if (errors.size () != 0) {
			    THROW (ErrorCode::EXTERNAL, errors);
			}
			return;
		    }
		);

		of (semantic::VarDecl, decl, {
			std::vector <std::string> errors;
			this-> _referent.push_back (sym);			
			TRY (
			    validateVarDecl (decl);
			) CATCH (ErrorCode::EXTERNAL) {
			    GET_ERRORS_AND_CLEAR (msgs);
			    errors.insert (errors.end (), msgs.begin (), msgs.end ());
			} FINALLY;
			
			this-> _referent.pop_back ();
			
			if (errors.size () != 0) {
			    THROW (ErrorCode::EXTERNAL, errors);
			}

			return;
		    }
		);

		of (semantic::ModRef, x ATTRIBUTE_UNUSED, return);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	}      
	
	void Visitor::validateModule (const semantic::Module & mod) {
	    if (!mod.isExtern ()) {
		const std::vector <Symbol> & syms = mod.getAllLocal ();
		std::vector <Generator> ret;

		for (auto & it : syms) {
		    validate (it);
		}
	    }
	}

	void Visitor::validateFunction (const semantic::Function & func) {
	    auto & function = func.getContent ();
	    std::vector <Generator> params;
	    
	    enterBlock ();
	    for (auto & param : function.getPrototype ().getParameters ()) {
		auto var = param.to <syntax::VarDecl> ();
		Generator type (Generator::empty ()), value (Generator::empty ());
		if (!var.getType ().isEmpty ()) {
		    type = validateType (var.getType ());
		}
		
		if (!var.getValue ().isEmpty ()) {
		    value = validateValue (var.getValue ());
		    if (!type.isEmpty () && !type.equals (value.to <Value> ().getType ()))
			Ymir::Error::occur (type.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    type.to <Type> ().getTypeName (),
					    value.to <Value> ().getType ().to<Type> ().getTypeName ()
			);
		    else {
			type = value.to <Value> ().getType ();
			type.to <Type> ().isMutable (false);
		    }
		}
		
		if (type.isEmpty ()) {
		    quitBlock ();
		    return; // This function is uncomplete, we can't validate it
		}
		
		bool isMutable = false;
		for (auto & deco : var.getDecorators ()) {
		    switch (deco.getValue ()) {
		    case syntax::Decorator::REF : type.to <Type> ().isRef (true); break;
		    case syntax::Decorator::MUT : { type.to <Type> ().isMutable (true); isMutable = true; } break;
		    default :
			Ymir::Error::occur (deco.getLocation (),
					    ExternalError::get (DECO_OUT_OF_CONTEXT),
					    deco.getLocation ().str
			);
		    }
		}
			
		if (type.to <Type> ().isMutable () && !type.to<Type> ().isRef ()) {
		    Ymir::Error::occur (var.getDecorator (syntax::Decorator::MUT).getLocation (),
					ExternalError::get (MUTABLE_CONST_PARAM)
		    );
		}

		if (!value.isEmpty ()) {		    
		    verifyMemoryOwner (value.getLocation (), type, value, true);
		}
		
		params.push_back (ParamVar::init (var.getName (), type, isMutable));
		insertLocal (var.getName ().str, params.back ());
	    }	    
	    
	    Generator retType (Generator::empty ());
	    if (!function.getPrototype ().getType ().isEmpty ()) 
		retType = validateType (function.getPrototype ().getType ());
	    else retType = Void::init (func.getName ());
	    
	    
	    if (!function.getBody ().getBody ().isEmpty ()) {
		if (!function.getBody ().getInner ().isEmpty () ||
		    !function.getBody ().getOuter ().isEmpty ()
		)
		    Ymir::Error::halt ("%(r) - TODO contract", "Critical");
		
		auto body = validateValue (function.getBody ().getBody ());
		bool needFinalReturn = false;
		
		if (!body.to<Value> ().isReturner ()) {
		    verifyMemoryOwner (body.getLocation (), retType, body, true);
		    
		    if (retType.to <Type> ().isRef () && (!body.to <Value> ().getType ().to <Type> ().isRef () || !body.to <Value> ().isLocal ())) {
			auto note = Ymir::Error::createNote (retType.getLocation (), ExternalError::get (BORROWED_HERE));
			Ymir::Error::occurAndNote (body.getLocation (), note, ExternalError::get (RETURN_LOCAL_REFERENCE));
		    }
		    
		    needFinalReturn = !retType.is<Void> ();
		}
		
		quitBlock ();
		insertNewGenerator (Frame::init (function.getName (), func.getRealName (), params, retType, body, needFinalReturn));
		
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
		if (!type.to <Type> ().isCompatible (value.to<Value> ().getType ())) {
		    Error::occur (var.getName (), ExternalError::get (INCOMPATIBLE_TYPES), type.to<Type> ().getTypeName (), value.to<Value> ().getType ().to<Type> ().getTypeName ());
		}
	    }

	    insertNewGenerator (GlobalVar::init (var.getName (), var.getName ().str, type, value));
	}

	Generator Visitor::validateValue (const syntax::Expression & expr) {
	    auto value = validateValueNoReachable (expr);
	    if (value.to <Value> ().isBreaker ())
		Ymir::Error::occur (value.getLocation (), ExternalError::get (BREAK_INSIDE_EXPR));
	    else if (value.to <Value> ().isReturner ())
		Ymir::Error::occur (value.getLocation (), ExternalError::get (RETURN_INSIDE_EXPR));
	    
	    return value;
	}
	
	Generator Visitor::validateValueNoReachable (const syntax::Expression & value) {
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

		of (syntax::DecoratedExpression, dec_expr,
		    return validateDecoratedExpression (dec_expr);
		);

		of (syntax::If, _if,
		    return validateIfExpression (_if);
		);

		of (syntax::While, _while,
		    return validateWhileExpression (_while);
		);

		of (syntax::Break, _break,
		    return validateBreak (_break);
		);

		of (syntax::List, list,
		    return validateList (list);
		);

		of (syntax::Intrinsics, intr,
		    return validateIntrinsics (intr);
		);

		of (syntax::Unit, u,
		    return None::init (u.getLocation ());
		);

		of (syntax::MultOperator, mult,
		    return validateMultOperator (mult);
		);

		of (syntax::Unary, un,
		    return validateUnary (un);
		);

		of (syntax::NamedExpression, named,
		    auto inner = validateValue (named.getContent ());
		    return NamedGenerator::init (named.getLocation (), inner);
		);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator Visitor::validateBlock (const syntax::Block & block) {
	    std::vector <Generator> values;
	    
	    Generator type (Void::init (block.getLocation ()));
	    bool breaker = false, returner = false;
	    std::vector <std::string> errors;
	    Symbol decl (Symbol::empty ());
	    { // We enter a sub scope to prevent TRY catch buffer shadow at compile time
		TRY (
		    enterBlock ();
		    decl = validateInnerModule (block.getDeclModule ());
		    if (!decl.isEmpty ()) {
			this-> _referent.push_back (decl);
		    }		   
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		    decl = Symbol::empty ();
		} FINALLY;
	    }

	    // i is not really volatile, but the compiler seems to want it to be (due to TRY CATCHS)
	    for (volatile int i = 0 ; i < (int) block.getContent ().size () ; i ++) {
		TRY (
		    if ((returner || breaker) && !block.getContent ()[i].is <syntax::Unit> ()) {			
			Error::occur (block.getContent () [i].getLocation (), ExternalError::get (UNREACHBLE_STATEMENT));
		    }
		    
		    auto value = validateValueNoReachable (block.getContent () [i]);
		    
		    if (value.to <Value> ().isReturner ()) returner = true;
		    if (value.to <Value> ().isBreaker ()) breaker = true;
		    type = value.to <Value> ().getType ();
		    type.to <Type> ().isRef (false);
		    type.to <Type> ().isMutable (false);
		    
		    values.push_back (value);		    
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());		    
		} FINALLY;
	    }

	    {
		TRY (
		    if (!decl.isEmpty ()) {
			this-> _referent.pop_back ();
		    }
		    quitBlock ();
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;
	    }
	    
	    if (errors.size () != 0) {
		THROW (ErrorCode::EXTERNAL, errors);
	    }

	    if (!type.is<Void> ()) verifyMemoryOwner (block.getEnd (), type, values.back(), false);	    
	    auto ret = Block::init (block.getLocation (), type, values);
	    ret.to <Value> ().isBreaker (breaker);
	    ret.to <Value> ().isReturner (returner);
	    return ret;
	}	

	Symbol Visitor::validateInnerModule (const syntax::Declaration & decl) {
	    if (decl.isEmpty ()) return Symbol::empty ();
	    auto sym = declarator::Visitor::init ().visit (decl);
	    if (!sym.isEmpty ()) {		
		this-> _referent.back ().insert (sym);
		std::vector <std::string> errors;
		enterForeign ();
		TRY (
		    this-> validate (sym);
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;   
		exitForeign ();

		if (errors.size () != 0) {
		    THROW (ErrorCode::EXTERNAL, errors);
		}
		
	    }
	    return sym;
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
			Error::occur (loc, ExternalError::get (OVERFLOW), type.getTypeName (), val);
		    
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
			Error::occur (loc, ExternalError::get (OVERFLOW), type.getTypeName (), val);
		    
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
	    type.to <Type> ().isMutable (true);
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
	    if (bin.getLocation () == Token::DCOLON) {
		auto subVisitor = SubVisitor::init (*this);
		return subVisitor.validate (bin);
	    } else if (bin.getLocation () == Token::DOT) {
		auto dotVisitor = DotVisitor::init (*this);
		return dotVisitor.validate (bin);
	    } else {
		auto binVisitor = BinaryVisitor::init (*this);
		return binVisitor.validate (bin);
	    }
	}

	Generator Visitor::validateUnary (const syntax::Unary & un) {
	    auto unVisitor = UnaryVisitor::init (*this);
	    return unVisitor.validate (un);
	}
	
	Generator Visitor::validateVar (const syntax::Var & var) {
	    auto & gen = getLocal (var.getName ().str);
	    if (gen.isEmpty ()) {
		auto sym = getGlobal (var.getName ().str);
		if (sym.empty ()) 
		    Error::occur (var.getLocation (), ExternalError::get (UNDEF_VAR), var.getName ().str);
		return validateMultSym (var.getLocation (), sym);
	    }

	    // The gen that we got can be either a param decl or a var decl
	    if (gen.is <ParamVar> ()) {
		return VarRef::init (var.getLocation (), var.getName ().str, gen.to<Value> ().getType (), gen.getUniqId (), gen.to<ParamVar> ().isMutable ());
	    } else if (gen.is <generator::VarDecl> ()) {
		return VarRef::init (var.getLocation (), var.getName ().str, gen.to<generator::VarDecl> ().getVarType (), gen.getUniqId (), gen.to<generator::VarDecl> ().isMutable ());		
	    } 

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}
	
	Generator Visitor::validateMultSym (const lexing::Word & loc, const std::vector <Symbol> & multSym) {	    
	    std::vector <Generator> gens;
	    for (auto & sym : multSym) {
		match (sym) {
		    of (semantic::Function, func, {
			    this-> _referent.push_back (sym);
			    gens.push_back (validateFunctionProto (func));
			    this-> _referent.pop_back ();
			    continue;
			}
		    );

		    of (semantic::ModRef, r ATTRIBUTE_UNUSED, {
			    gens.push_back (ModuleAccess::init (loc, sym));
			    continue;
			}
		    );
		    
		    of (semantic::Module, mod ATTRIBUTE_UNUSED, {
			    gens.push_back (ModuleAccess::init (loc, sym));
			    continue;
			}
		    );
		}
		
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    }

	    if (gens.size () == 1) return gens [0];
	    else return MultSym::init (loc, gens);
	}
	
	Generator Visitor::validateFunctionProto (const semantic::Function & func) {
	    enterForeign ();
	    std::vector <Generator> params;
	    static std::list <lexing::Word> __validating__; 
	    auto & function = func.getContent ();
	    std::vector <std::string> errors;
	    bool no_value = false;
	    for (auto func_loc : __validating__) {
		// If there is a foward reference, we can't validate the values
		if (func_loc.isSame (func.getName ())) no_value = true;		    
	    }

	    __validating__.push_back (func.getName ());
	    for (auto & param : function.getPrototype ().getParameters ()) {
		TRY (
		    auto var = param.to <syntax::VarDecl> ();
		    Generator type (Generator::empty ()); // C++ macros are a mystery to me.
		    // You just can't declare two vars in a row !!

		    Generator value (Generator::empty ());
		    
		    if (!var.getType ().isEmpty ()) {
		    	type = validateType (var.getType ());
		    }
		    		
		    if (!var.getValue ().isEmpty () && !no_value) {
			value = validateValue (var.getValue ());
			if (!type.isEmpty () && !type.equals (value.to <Value> ().getType ()))
		    	    Ymir::Error::occur (type.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
		    				type.to <Type> ().getTypeName (),
		    				value.to <Value> ().getType ().to<Type> ().getTypeName ()
		    	    );
		    	else {
		    	    type = value.to <Value> ().getType ();
		    	    type.to <Type> ().isMutable (false);
		    	}
		    }

		    if (var.getType ().isEmpty () && no_value && !var.getValue ().isEmpty ()) {
			Ymir::Error::occur (var.getLocation (), ExternalError::get (FORWARD_REFERENCE_VAR));
		    }		    
		
		    if (type.isEmpty ()) {
			// TODO, create frame prototype for uncomplete functions
		    	Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		    }
		
		    bool isMutable = false;
		    for (auto & deco : var.getDecorators ()) {
		    	switch (deco.getValue ()) {
		    	case syntax::Decorator::REF : type.to <Type> ().isRef (true); break;
		    	case syntax::Decorator::MUT : { type.to <Type> ().isMutable (true); isMutable = true; } break;
		    	default :
		    	    Ymir::Error::occur (deco.getLocation (),
		    				ExternalError::get (DECO_OUT_OF_CONTEXT),
		    				deco.getLocation ().str
		    	    );
		    	}
		    }
		
		    if (type.to <Type> ().isMutable () && !type.to<Type> ().isRef ()) {
		    	Ymir::Error::occur (var.getDecorator (syntax::Decorator::MUT).getLocation (),
		    			    ExternalError::get (MUTABLE_CONST_PARAM)
		    	);
		    }
		
		    if (!value.isEmpty ()) {		    
		    	verifyMemoryOwner (value.getLocation (), type, value, true);
		    }
		
		    params.push_back (ProtoVar::init (var.getName (), type, value, isMutable));
		    
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		    errors.push_back (Ymir::Error::createNote (param.getLocation ()));
		} FINALLY;
	    }	    
	    
	    Generator retType (Generator::empty ());
	    TRY (
		if (!function.getPrototype ().getType ().isEmpty ())
		    retType = validateType (function.getPrototype ().getType ());
		else retType = Void::init (func.getName ());
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors.insert (errors.end (), msgs.begin (), msgs.end ());
	    } FINALLY;

	    __validating__.pop_back ();
	    exitForeign ();

	    if (errors.size () != 0) {
		THROW (ErrorCode::EXTERNAL, errors);		
	    }
	    
	    return FrameProto::init (function.getName (), func.getRealName (), retType, params);	    
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
	    if (!var.getType ().isEmpty ()) {
		type = validateType (var.getType ());
	    } else {
		type = value.to <Value> ().getType ();
		type.to<Type> ().isRef (false);
	    }

	    bool isMutable = false;
	    for (auto & deco : var.getDecorators ()) {
		switch (deco.getValue ()) {
		case syntax::Decorator::REF : type.to <Type> ().isRef (true); break;
		case syntax::Decorator::MUT : { type.to <Type> ().isMutable (true); isMutable = true; } break;
		default :
		    Ymir::Error::occur (deco.getLocation (),
					ExternalError::get (DECO_OUT_OF_CONTEXT),
					deco.getLocation ().str
		    );
		}
	    }

	    if (!isMutable) type.to <Type> ().isMutable (false);	    
	    type.to <Type> ().isLocal (true);
	    if (!value.isEmpty ()) {
		verifyMemoryOwner (value.getLocation (), type, value, true);
	    }

	    if (type.is<Void> ()) {
		Ymir::Error::occur (var.getLocation (), ExternalError::get (VOID_VAR));
	    } else if (type.to <Type> ().isRef () && value.isEmpty ()) {
		Ymir::Error::occur (var.getName (), ExternalError::get (REF_NO_VALUE), var.getName ().str);
	    } 
	    
	    auto ret = generator::VarDecl::init (var.getLocation (), var.getName ().str, type, value, isMutable);
	    insertLocal (var.getName ().str, ret);
	    return ret;
	}

	Generator Visitor::validateDecoratedExpression (const syntax::DecoratedExpression & dec_expr) {
	    auto inner = validateValue (dec_expr.getContent ());
	    if (dec_expr.hasDecorator (syntax::Decorator::MUT)) {
		if (!inner.to<Value> ().getType ().to<Type> ().isMutable ()) 
		    Ymir::Error::occur (dec_expr.getDecorator (syntax::Decorator::MUT).getLocation (),
					ExternalError::get (DISCARD_CONST)
		    );
		else
		    Ymir::Error::warn (dec_expr.getDecorator (syntax::Decorator::MUT).getLocation (),
				       ExternalError::get (USELESS_DECORATOR)
		    );
	    } 

	    if (dec_expr.hasDecorator (syntax::Decorator::REF)) {
		if (inner.to <Value> ().isLvalue ()) {
		    if (!inner.to <Value> ().getType ().to <Type> ().isMutable ())
			Ymir::Error::occur (inner.getLocation (), ExternalError::get (IMMUTABLE_LVALUE));
		    
		    auto type = inner.to <Value> ().getType ();
		    type.to <Type> ().isRef (true);
		    return Referencer::init (dec_expr.getLocation (), type, inner);
		} else {
		    Ymir::Error::occur (inner.getLocation (),
					ExternalError::get (NOT_A_LVALUE)
		    );
		}
	    }
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator Visitor::validateIfExpression (const syntax::If & _if) {
	    Generator test (Generator::empty ());
	    if (!_if.getTest ().isEmpty ()) { // Has a test if it is not an else
		test = validateValue (_if.getTest ());
		
		if (!test.to<Value> ().getType ().is <Bool> ()) {
		    Ymir::Error::occur (test.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					test.to <Value> ().getType ().to <Type> ().getTypeName (),
					Bool::NAME
		    );
		}
	    }

	    auto content = validateValueNoReachable (_if.getContent ());
	    auto type = content.to <Value> ().getType ();

	    if (!_if.getElsePart ().isEmpty ()) {
		auto _else = validateValueNoReachable (_if.getElsePart ());
		if (!_else.to<Value> ().getType ().equals (type)) type = Void::init (_if.getLocation ());
		return Conditional::init (_if.getLocation (), type, test, content, _else);	    
	    } else
		return Conditional::init (_if.getLocation (), Void::init (_if.getLocation ()), test, content, Generator::empty ());
	}

	Generator Visitor::validateWhileExpression (const syntax::While & _wh) {
	    Generator test (Generator::empty ());
	    if (!_wh.getTest ().isEmpty ()) {
		test = validateValue (_wh.getTest ());
		if (!test.to <Value> ().getType ().is <Bool> ()) {
		    Ymir::Error::occur (test.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					test.to <Value> ().getType ().to <Type> ().getTypeName (),
					Bool::NAME
		    );
		}
	    }
	    
	    enterLoop ();
	    auto content = validateValueNoReachable (_wh.getContent ());
	    auto breakType = quitLoop ();
	    Generator type (Generator::empty ());
	    if (!test.isEmpty ()) {
		type = content.to <Value> ().getType ();

		if (!breakType.isEmpty () && !content.to <Value> ().isBreaker () && !type.equals (breakType)) {
		    auto note = Ymir::Error::createNote (breakType.getLocation ());
		    Ymir::Error::occurAndNote (content.getLocation (), note, ExternalError::get (INCOMPATIBLE_TYPES),
					       type.to <Type> ().getTypeName (),
					       breakType.to <Type> ().getTypeName ()
		    );				    
		} else if (content.to <Value> ().isBreaker ()) {
		    type = breakType;
		}
	    } else {
		if (breakType.isEmpty ()) type = Void::init (_wh.getLocation ());
		else type = breakType;
	    }

	    return Loop::init (_wh.getLocation (), type, test, content, _wh.isDo ());	    
	}	

	Generator Visitor::validateBreak (const syntax::Break & _break) {
	    if (!this-> isInLoop ())
		Ymir::Error::occur (_break.getLocation (), ExternalError::get (BREAK_NO_LOOP));
				    
	    Generator value = Generator::empty ();
	    Generator type = Generator::empty ();
	    if (!_break.getValue ().isEmpty ()) {
		value = validateValue (_break.getValue ());
		type = value.to <Value> ().getType ();
	    } else type = Void::init (_break.getLocation ());

	    auto loop_type = getCurrentLoopType ();
	    if (loop_type.isEmpty ()) setCurrentLoopType (type);
	    else if (!loop_type.equals (type)) {
		auto note = Ymir::Error::createNote (loop_type.getLocation ());
		Ymir::Error::occurAndNote (value.getLocation (), note, ExternalError::get (INCOMPATIBLE_TYPES),
					   type.to <Type> ().getTypeName (),
					   loop_type.to <Type> ().getTypeName ()
		);				    
	    }
	    
	    return Break::init (_break.getLocation (), Void::init (_break.getLocation ()), value);
	}
	
	Generator Visitor::validateList (const syntax::List & list) {
	    if (list.isArray ()) return validateArray (list);
	    if (list.isTuple ()) return validateTuple (list);
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}
	
	Generator Visitor::validateArray (const syntax::List & list) {
	    std::vector <Generator> params;
	    for (auto it : list.getParameters ()) {
		params.push_back (validateValue (it));
		verifyMemoryOwner (params.back ().getLocation (), params [0].to <Value> ().getType (), params.back (), false);
	    }

	    Generator innerType (Void::init (list.getLocation ()));
	    if (params.size () != 0)
		innerType = params [0].to <Value> ().getType ();	    
	    innerType.to <Type> ().isRef (false);
	    
	    // An array literal is always static
	    auto type = Array::init (list.getLocation (), innerType, params.size ());
	    type.to <Type> ().isMutable (true); // Array constant are mutable by default (not lvalue), to ease simple affectation
	    type.to <Type> ().isLocal (true); // Array constant are declared in the stack, so they are local
	    return ArrayValue::init (list.getLocation (), type, params);
	}	

	Generator Visitor::validateTuple (const syntax::List & list) {
	    std::vector <Generator> params;
	    std::vector <Generator> types;
	    for (auto it : list.getParameters ()) {
		params.push_back (validateValue (it));
		auto type = params.back ().to <Value> ().getType ();
		type.to <Type> ().isRef (false);
		types.push_back (type);
		verifyMemoryOwner (params.back ().getLocation (), type, params.back (), false);
	    }
	    
	    auto type = Tuple::init (list.getLocation (), types);
	    type.to <Type> ().isMutable (true); // Tuple are mutable by default (not lvalue)
	    type.to <Type> ().isLocal (true); //
	    return TupleValue::init (list.getLocation (), type, params);	    
	}
	
	Generator Visitor::validateIntrinsics (const syntax::Intrinsics & intr) {
	    if (intr.isCopy ()) return validateCopy (intr);
	    if (intr.isAlias ()) return validateAlias (intr);

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}
       	
	Generator Visitor::validateCopy (const syntax::Intrinsics & intr) {
	    auto content = validateValue (intr.getContent ());

	    if (content.to <Value> ().getType ().is <Array> () || content.to <Value> ().getType ().is <Slice> ()) {
		auto type = content.to <Value> ().getType ().to <Type> ().toMutable ();
		type.to<Type> ().isLocal (false);
		
		// The copy is done on the first level, so we don't have the right to change the mutability of inner data
		return Copier::init (intr.getLocation (), type, content);
	    } else {
		Ymir::Error::occur (
		    intr.getLocation (),
		    ExternalError::get (NO_COPY_EXIST),
		    content.to<Value> ().getType ().to <Type> ().getTypeName ()
		);
		
		return Generator::empty ();		
	    }	    
	}

	Generator Visitor::validateAlias (const syntax::Intrinsics & intr) {
	    auto content = validateValue (intr.getContent ());

	    if (content.to <Value> ().getType ().is <Array> () || content.to <Value> ().getType ().is <Slice> ()) {
		auto type = content.to <Value> ().getType ();
		if (type.is <Array> ()) {
		    type = Slice::init (intr.getLocation (), type.to<Array> ().getInners () [0]);
		    type.to<Type> ().isMutable (content.to <Value> ().getType ().to <Type> ().isMutable ());
		}
		return Aliaser::init (intr.getLocation (), type, content);
	    } else {
		Ymir::Error::occur (
		    intr.getLocation (),
		    ExternalError::get (NO_ALIAS_EXIST),
		    content.to <Value> ().getType ().to <Type> ().getTypeName ()
		);
	    }
	    
	    return Generator::empty ();
	}

	Generator Visitor::validateMultOperator (const syntax::MultOperator & op) {
	    if (op.getEnd () == Token::RCRO) {
		auto bracketVisitor = BracketVisitor::init (*this);
		return bracketVisitor.validate (op);
	    } else if (op.getEnd () == Token::RPAR) {
		auto callVisitor = CallVisitor::init (*this);
		return callVisitor.validate (op);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}
	
	Generator Visitor::validateType (const syntax::Expression & type) {
	    match (type) {
		of (syntax::Var, var,
		    return validateTypeVar (var);
		);

		of (syntax::DecoratedExpression, dec_expr,
		    return validateTypeDecorated (dec_expr);
		);

		of (syntax::ArrayAlloc, array,
		    return validateTypeArrayAlloc (array);
		);

		of (syntax::List, list,
		    if (list.isArray ())
			return validateTypeSlice (list);
		    if (list.isTuple ())
			return validateTypeTuple (list);
		);
	    }
	    
	    Ymir::Error::occur (type.getLocation (), ExternalError::get (USE_AS_TYPE));
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

	Generator Visitor::validateTypeDecorated (const syntax::DecoratedExpression & expr) {
	    auto type = validateType (expr.getContent ());
	    
	    lexing::Word gotConstOrMut (lexing::Word::eof ());
	    for (auto & deco : expr.getDecorators ()) {
		switch (deco.getValue ()) {
		case syntax::Decorator::REF : type.to<Type> ().isRef (true); break;
		case syntax::Decorator::CONST : {
		    if (!gotConstOrMut.isEof ()) {
			auto note = Ymir::Error::createNote (gotConstOrMut);
			Ymir::Error::occurAndNote (expr.getDecorator (syntax::Decorator::CONST).getLocation (), note, ExternalError::get (CONFLICT_DECORATOR));
		    }
		    gotConstOrMut = expr.getDecorator (syntax::Decorator::CONST).getLocation ();
		    type.to<Type> ().isMutable (false); break;
		}
		case syntax::Decorator::MUT : {
		    if (!gotConstOrMut.isEof ()) {
			auto note = Ymir::Error::createNote (gotConstOrMut);
			Ymir::Error::occur (expr.getDecorator (syntax::Decorator::MUT).getLocation (), ExternalError::get (CONFLICT_DECORATOR));
		    }
		    gotConstOrMut = expr.getDecorator (syntax::Decorator::MUT).getLocation ();
		    type.to<Type> ().isMutable (true); break;
		}		    
		default :
		    Ymir::Error::occur (deco.getLocation (),
					ExternalError::get (DECO_OUT_OF_CONTEXT),
					deco.getLocation ().str
		    );		
		}
	    }
	    	   	    
	    return type;
	}

	Generator Visitor::validateTypeArrayAlloc (const syntax::ArrayAlloc & alloc) {
	    if (alloc.isDynamic ())
		Ymir::Error::occur (alloc.getLocation (), ExternalError::get (USE_AS_TYPE));

	    auto type = validateType (alloc.getLeft ());
	    auto size = validateValue (alloc.getSize ());

	    Generator value = retreiveValue (size);
	    if (!value.is <Fixed> ()) {
		Ymir::Error::occur (alloc.getSize ().getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    value.to <Value> ().getType ().to <Type> ().getTypeName (),
				    (Integer::init (lexing::Word::eof (), 64, false)).to<Type> ().getTypeName ()
		);
	    }
	    
	    return Array::init (alloc.getLocation (), type, value.to <Fixed> ().getUI ().u);	    
	}

	Generator Visitor::validateTypeSlice (const syntax::List & list) {
	    if (list.getParameters ().size () != 1)
		Ymir::Error::occur (list.getLocation (), ExternalError::get (USE_AS_TYPE));

	    auto type = validateType (list.getParameters () [0]);
	    return Slice::init (list.getLocation (), type);	    
	}
	
	Generator Visitor::validateTypeTuple (const syntax::List & list) {
	    std::vector <Generator> inners;
	    for (auto & it : list.getParameters ())
		inners.push_back (validateType (it));

	    return Tuple::init (list.getLocation (), inners);
	}	

	const std::vector <generator::Generator> & Visitor::getGenerators () const {
	    return this-> _list;
	}
	
	void Visitor::insertNewGenerator (const Generator & generator) {
	    this-> _list.push_back (generator);
	}
	
	void Visitor::enterBlock () {
	    this-> _usedSyms.back ().push_back ({});
	    this-> _symbols.back ().push_back ({});
	}
	
	void Visitor::quitBlock () {	    
	    if (this-> _symbols.back ().empty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");

	    std::vector <std::string> errors;
	    for (auto & sym : this-> _symbols.back ().back ()) {
		if (this-> _usedSyms.back ().back ().find (sym.first) == this-> _usedSyms.back ().back ().end ()) {
		    errors.push_back (Error::makeWarn (sym.second.getLocation (), ExternalError::get (NEVER_USED), sym.second.getName ()));
		}
	    }

	    this-> _usedSyms.back ().pop_back ();
	    this-> _symbols.back ().pop_back ();

	    if (errors.size () != 0) {
		THROW (ErrorCode::EXTERNAL, errors);
	    }
	}

	void Visitor::enterForeign () {
	    this-> _usedSyms.push_back ({});
	    this-> _symbols.push_back ({});
	}

	void Visitor::exitForeign () {
	    this-> _usedSyms.pop_back ();
	    this-> _symbols.pop_back ();
	}
	
	void Visitor::enterLoop () {
	    this-> _loopBreakTypes.push_back (Generator::empty ());	    
	}

	Generator Visitor::quitLoop () {
	    auto last = this-> _loopBreakTypes.back ();
	    this-> _loopBreakTypes.pop_back ();
	    return last;
	}

	const Generator & Visitor::getCurrentLoopType () const {
	    return this-> _loopBreakTypes.back ();
	}

	void Visitor::setCurrentLoopType (const Generator & type) {
	    this-> _loopBreakTypes.back () = type;
	}

	bool Visitor::isInLoop () const {
	    return !this-> _loopBreakTypes.empty ();
	}
	
	void Visitor::insertLocal (const std::string & name, const Generator & gen) {
	    if (this-> _symbols.back ().empty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    this-> _symbols.back ().back ().emplace (name, gen);
	}       

	const Generator & Visitor::getLocal (const std::string & name) {
	    for (auto it : Ymir::r (0, this-> _symbols.back ().size ())) {
		auto ptr = this-> _symbols.back () [it].find (name); 		    
		if (ptr != this-> _symbols.back () [it].end ()) {
		    this-> _usedSyms.back () [it].insert (name);
		    return ptr-> second;
		}		
	    }
	    
	    return Generator::__empty__;
	}

	void Visitor::verifyMemoryOwner (const lexing::Word & loc, const Generator & type, const Generator & gen, bool construct) {
	    if (!type.to <Type> ().isCompatible (gen.to <Value> ().getType ()))
		Ymir::Error::occur (loc, ExternalError::get (INCOMPATIBLE_TYPES),
				    type.to <Type> ().getTypeName (),
				    gen.to <Value> ().getType ().to <Type> ().getTypeName ()
		);	    


	    if (!construct && gen.is<Referencer> ()) {
		Ymir::Error::warn (gen.getLocation (), ExternalError::get (REF_NO_EFFECT));
	    } else {
		if (type.to <Type> ().isRef ()) {
		    if (!type.equals (gen.to<Value> ().getType ()))
			Ymir::Error::occur (gen.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    type.to<Type> ().getTypeName (),
					    gen.to <Value> ().getType ().to <Type> ().getTypeName ()
			);
		    		
		    if (!gen.is<Referencer> ()) {
			if (gen.to<Value> ().isLvalue ()) {
			    Ymir::Error::occur (gen.getLocation (), ExternalError::get (IMPLICIT_REFERENCE),
						gen.to<Value> ().getType ().to <Type> ().getTypeName ()
			    );
			} else {
			    Ymir::Error::occur (gen.getLocation (), ExternalError::get (NOT_A_LVALUE));
			}
		    }
		}
	    }

	    // Tuple copy is by default, as we cannot alias a tuple
	    if (gen.to <Value> ().getType ().is <Tuple> ()) {
		auto tu = gen.to<Value> ().getType ().to <Type> ().toMutable ();
		auto llevel = type.to <Type> ().mutabilityLevel ();
		auto rlevel = tu.to <Type> ().mutabilityLevel ();
		if (llevel > rlevel) {
		    Ymir::Error::occur (loc, ExternalError::get (DISCARD_CONST_LEVEL),
					llevel, rlevel
		    );
		}
	    } else {	    
		// Verify copy ownership
		// We can asset that, a block, and an arrayvalue (and some others ...) have already perform the copy, it is not mandatory for them to force it, as well as conditional
		if (type.to<Type> ().isComplex () && !(gen.is <Copier> () || gen.is<ArrayValue> () || gen.is <Block> () || gen.is <Conditional> () || gen.is <Aliaser> ())) {
		    if (!(construct && gen.is<Referencer> () && type.to<Type> ().isRef ()))
			Ymir::Error::occur (loc, ExternalError::get (IMPLICIT_COPY),
					    gen.to <Value> ().getType ().to <Type> ().getTypeName ());
		}

		// Verify mutability
		if (type.to<Type> ().isComplex ()) {
		    auto llevel = type.to <Type> ().mutabilityLevel ();
		    auto rlevel = gen.to <Value> ().getType ().to <Type> ().mutabilityLevel ();
		    if (llevel > rlevel) {
			Ymir::Error::occur (loc, ExternalError::get (DISCARD_CONST_LEVEL),
					    llevel, rlevel
			);
		    }
		}
	    }

	    // TODO Verify locality
	    // if (!type.to<Type> ().isLocal () && gen.to <Value> ().getType ().to <Type> ().isLocal ()) {
	    // 	Ymir::Error::occur (loc, ExternalError::get (DISCARD_LOCALITY));				    
	    // }
	}


	Generator Visitor::retreiveValue (const Generator & gen) {
	    auto compile_time = CompileTime::init (*this);
	    return compile_time.execute (gen);
	}

	std::vector <Symbol> Visitor::getGlobal (const std::string & name) {
	    return this-> _referent.back ().get (name);
	}
	
    }
    
}
