#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/validator/BinaryVisitor.hh>
#include <ymir/semantic/validator/CompileTime.hh>
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

		    for (auto & deco : var.getDecorators ()) {
			switch (deco.getValue ()) {
			case syntax::Decorator::REF : type.to <Type> ().isRef (true); break;
			case syntax::Decorator::MUT : type.to <Type> ().isMutable (true); break;
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
	    else retType = Void::init (func.getName ());
	    
	    
	    if (!function.getBody ().getBody ().isEmpty ()) {
		if (!function.getBody ().getInner ().isEmpty () ||
		    !function.getBody ().getOuter ().isEmpty ()
		)
		    Ymir::Error::halt ("%(r) - TODO contract", "Critical");
		
		auto body = validateValue (function.getBody ().getBody ());

		if (!body.to<Value> ().isReturner ()) {
		    verifyMemoryOwner (retType, body);
		    
		    if (retType.to <Type> ().isRef () && (!body.to <Value> ().getType ().to <Type> ().isRef () || !body.to <Value> ().isLocal ())) {
			auto note = Ymir::Error::createNote (retType.getLocation (), ExternalError::get (BORROWED_HERE));
			Ymir::Error::occurAndNote (body.getLocation (), note, ExternalError::get (RETURN_LOCAL_REFERENCE));
		    }
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

		of (syntax::DecoratedExpression, dec_expr,
		    return validateDecoratedExpression (dec_expr);
		);

		of (syntax::If, _if,
		    return validateIfExpression (_if);
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
		    type.to <Type> ().isRef (false);
		    type.to <Type> ().isMutable (false);
		    
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

	    if (!type.is<Void> ()) verifyMemoryOwner (type, values.back());
	    
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
	    if (!var.getType ().isEmpty ()) {
		type = validateType (var.getType ());
	    } else {
		type = value.to <Value> ().getType ();
		type.to<Type> ().isRef (false);
	    }

	    for (auto & deco : var.getDecorators ()) {
		switch (deco.getValue ()) {
		case syntax::Decorator::REF : type.to <Type> ().isRef (true); break;
		case syntax::Decorator::MUT : type.to <Type> ().isMutable (true); break;
		default :
		    Ymir::Error::occur (deco.getLocation (),
					ExternalError::get (DECO_OUT_OF_CONTEXT),
					deco.getLocation ().str
		    );
		}
	    }

	    if (!value.isEmpty ()) {
		verifyMemoryOwner (type, value);
		if (type.to <Type> ().isRef () && !value.is <Referencer> ()) {
		    if (value.to<Value> ().isLvalue ()) {
			Ymir::Error::occur (value.getLocation (), ExternalError::get (IMPLICIT_REFERENCE),
					    value.to <Value> ().getType ().to <Type> ().typeName ()
			);
		    } else {
			Ymir::Error::occur (value.getLocation (), ExternalError::get (NOT_A_LVALUE));	    
		    }
		}
	    }

	    if (type.is<Void> ()) {
		Ymir::Error::occur (type.getLocation (), ExternalError::get (VOID_VAR));
	    } else if (type.to <Type> ().isRef () && value.isEmpty ()) {
		Ymir::Error::occur (var.getName (), ExternalError::get (REF_NO_VALUE), var.getName ().str);
	    }
	    
	    auto ret = generator::VarDecl::init (var.getLocation (), var.getName ().str, type, value);
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
					test.to <Value> ().getType ().to <Type> ().typeName (),
					Bool::NAME
		    );
		}
	    }

	    auto content = validateValue (_if.getContent ());
	    auto type = content.to <Value> ().getType ();

	    if (!_if.getElsePart ().isEmpty ()) {
		auto _else = validateValue (_if.getElsePart ());
		if (!_else.to<Value> ().getType ().equals (type)) type = Void::init (_if.getLocation ());
		return Conditional::init (_if.getLocation (), type, test, content, _else);	    
	    } else
		return Conditional::init (_if.getLocation (), Void::init (_if.getLocation ()), test, content, Generator::empty ());
	}

	Generator Visitor::validateList (const syntax::List & list) {
	    if (list.isArray ()) return validateArray (list);

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}
	
	Generator Visitor::validateArray (const syntax::List & list) {
	    std::vector <Generator> params;
	    for (auto it : list.getParameters ()) {
		params.push_back (validateValue (it));		
		// if (params.size () != 1 && !params.back ().to<Value> ().getType ().equals (params [0].to <Value> ().getType ())) {
		//     auto note = Ymir::Error::createNote (list.getLocation ());
		//     Ymir::Error::occurAndNote (params.back ().getLocation (), note, ExternalError::get (INCOMPATIBLE_TYPES),
		// 			       params.back ().to <Value> ().getType ().to <Type> ().typeName (),
		// 			       params [0].to <Value> ().getType ().to <Type> ().typeName ()
		//     );		   
		// }
		verifyMemoryOwner (params [0].to <Value> ().getType (), params.back ());
	    }

	    Generator innerType (Void::init (list.getLocation ()));
	    if (params.size () != 0)
		innerType = params [0].to <Value> ().getType ();

	    // An array literal is always static
	    auto type = Array::init (list.getLocation (), innerType, params.size ());	    
	    return ArrayValue::init (list.getLocation (), type, params);
	}	

	Generator Visitor::validateIntrinsics (const syntax::Intrinsics & intr) {
	    if (intr.isCopy ()) return validateCopy (intr);

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator Visitor::validateCopy (const syntax::Intrinsics & intr) {
	    auto content = validateValue (intr.getContent ());

	    if (content.to <Value> ().getType ().is <Array> ()) {
		return Copier::init (intr.getLocation (), content.to <Value> ().getType (), content);
	    } else {
		Ymir::Error::occur (
		    intr.getLocation (),
		    ExternalError::get (NO_COPY_EXIST),
		    content.to<Value> ().getType ().to <Type> ().typeName ()
		);
		
		return Generator::empty ();		
	    }
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

	    if (type.to <Type> ().isMutable () && !type.to<Type> ().isRef ()) {
		Ymir::Error::occur (expr.getDecorator (syntax::Decorator::MUT).getLocation (),
				    ExternalError::get (MUTABLE_CONST_RETURN)
		);
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
				    value.to <Value> ().getType ().to <Type> ().typeName (),
				    (Integer::init (lexing::Word::eof (), 64, false)).to<Type> ().typeName ()
		);
	    }
	    
	    return Array::init (alloc.getLocation (), type, value.to <Fixed> ().getUI ().u);	    
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

	void Visitor::verifyMemoryOwner (const Generator & type, const Generator & gen) {
	    if (!type.to <Type> ().isCompatible (gen.to <Value> ().getType ()))
		Ymir::Error::occur (gen.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    type.to <Type> ().typeName (),
				    gen.to <Value> ().getType ().to <Type> ().typeName ()
		);	    

	    // We can asset that, a block, and an arrayvalue have already perform the copy, it is not mandatory for them to force it, as well as conditional
	    if (type.to<Type> ().isComplex () && !(gen.is <Referencer> () || gen.is <Copier> () || gen.is<ArrayValue> () || gen.is <Block> () || gen.is <Conditional> ())) {
		Ymir::Error::occur (gen.getLocation (), ExternalError::get (IMPLICIT_COPY),
				    gen.to <Value> ().getType ().to <Type> ().typeName ());
	    }

	    if (!type.to<Type> ().isRef () && type.is<Array> () && type.to <Array> ().isStatic () && gen.is <Referencer> ())
		Ymir::Error::occur (gen.getLocation (), ExternalError::get (STATIC_ARRAY_REF));
	}


	Generator Visitor::retreiveValue (const Generator & gen) {
	    auto compile_time = CompileTime::init (*this);
	    return compile_time.execute (gen);
	}
	
    }
    
}