#include <ymir/semantic/validator/PragmaVisitor.hh>
#include <ymir/semantic/validator/BinaryVisitor.hh>
#include <ymir/semantic/validator/UnaryVisitor.hh>
#include <ymir/semantic/validator/DotVisitor.hh>
#include <ymir/semantic/validator/UtfVisitor.hh>
#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/global/Core.hh>
#include <ymir/errors/_.hh>


namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	using namespace global;

	const std::string PragmaVisitor::COMPILE = "compile";
	const std::string PragmaVisitor::MANGLE = "mangle";
	const std::string PragmaVisitor::OPERATOR = "operator";
	
	const std::string PragmaVisitor::FIELD_NAMES = "field_names";
	const std::string PragmaVisitor::FIELD_OFFSETS = "field_offsets";
	const std::string PragmaVisitor::HAS_DEFAULT = "field_has_value";
	const std::string PragmaVisitor::DEFAULT_VALUE = "field_value";
	const std::string PragmaVisitor::FIELD_TYPE = "field_type";
	const std::string PragmaVisitor::HAS_FIELD  = "has_field";
	
	const std::string PragmaVisitor::TUPLEOF = "tupleof";
	const std::string PragmaVisitor::LOCAL_TUPLEOF = "local_tupleof";
	const std::string PragmaVisitor::TRUSTED = "trusted";
	const std::string PragmaVisitor::PANIC = "panic";
	const std::string PragmaVisitor::FAKE_THROW = "fake_throw";
	
	
	PragmaVisitor::PragmaVisitor (Visitor & context) :
	    _context (context)
	{}

	PragmaVisitor PragmaVisitor::init (Visitor & context) {	    
	    return PragmaVisitor (context);
	}

	Generator PragmaVisitor::validate (const syntax::Pragma & prg) {
	    auto ret = Generator::empty ();
	    if (prg.getLocation ().getStr () == PragmaVisitor::COMPILE) {
		ret = this-> validateCompile (prg);	       		
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::MANGLE) {
		ret = this-> validateMangle (prg);		
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::OPERATOR) {
		ret = this-> validateOperator (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::FIELD_NAMES) {
		ret = this-> validateFieldNames (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::TUPLEOF) {
		ret = this-> validateTupleOf (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::LOCAL_TUPLEOF) {
		ret = this-> validateLocalTupleOf (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::HAS_DEFAULT) {
		ret = this-> validateHasDefault (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::DEFAULT_VALUE) {
		ret = this-> validateDefaultValue (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::FIELD_OFFSETS) {
		ret = this-> validateFieldOffsets (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::FIELD_TYPE) {
		ret = this-> validateFieldType (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::TRUSTED) {
		ret = this-> validateTrusted (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::PANIC) {
		ret = this-> validatePanic (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::FAKE_THROW) {
		ret = this-> validateFakeThrow (prg);
	    } else if (prg.getLocation ().getStr () == PragmaVisitor::HAS_FIELD) {
		ret = this-> validateHasField (prg);
	    } else {
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (UNKOWN_PRAGMA), prg.getLocation ().getStr ());
	    }

	    return ret;
	}


	/**
	 * ========================================================
	 *                       COMPILE
	 * ========================================================
	 */

	Generator PragmaVisitor::validateCompile (const syntax::Pragma & prg) {
	    try {
		for (auto & it : prg.getContent ()) {
		    this-> _context.validateValue (it);
		}
		    
		return BoolValue::init (prg.getLocation (), Bool::init (prg.getLocation ()), true);
	    } catch (Error::ErrorList list) {
		return BoolValue::init (prg.getLocation (), Bool::init (prg.getLocation ()), false);
	    }
	}

	
	/**
	 * ========================================================
	 *                       TRUSTED
	 * ========================================================
	 */

	Generator PragmaVisitor::validateTrusted (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 1) {
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }
	    
	    if (this-> _context.isInTrusted ()) {
		auto bl = this-> _context.validateValue (prg.getContent ()[0]);
		auto jmp_buf_type = this-> _context.validateType (syntax::Var::init (lexing::Word::init (prg.getLocation (), global::CoreNames::get (global::JMP_BUF_TYPE))));	
		auto ex = ExitScope::init (bl.getLocation (), bl.to <Value> ().getType (), jmp_buf_type, bl, {}, {
			Panic::init (bl.getLocation ())
		    }, Generator::empty (), Generator::empty (), Generator::empty ());
		ex.setThrowers ({});
		return ex;
	    } else {
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (UNTRUSTED_CONTEXT));
	    }
	    
	    return Generator::empty ();
	}	
	
	/**
	 * ========================================================
	 *                       PANIC
	 * ========================================================
	 */

	Generator PragmaVisitor::validatePanic (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 0) {
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }
	    
	    if (this-> _context.isInTrusted ()) {		
		return Value::initBrRet (Panic::init (prg.getLocation ()).to <Value> (), true, true, prg.getLocation (), prg.getLocation ());
	    } else {
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (UNTRUSTED_CONTEXT));
	    }
	    
	    return Generator::empty ();
	}	

	/**
	 * ========================================================
	 *                       PANIC
	 * ========================================================
	 */

	Generator PragmaVisitor::validateFakeThrow (const syntax::Pragma & prg) {
	    auto syntaxType = this-> _context.createClassTypeFromPath (prg.getLocation (), {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (EXCEPTION_TYPE)});
	    auto ancType = this-> _context.validateType (syntaxType);
	    
	    std::vector <Generator> throwers;
	    for (auto & it : prg.getContent ()) {
		auto type = this-> _context.validateType (it);
		this-> _context.verifyCompatibleType (prg.getLocation (), type.getLocation (), ancType, type);
		throwers.push_back (type);
	    }

	    auto val = FakeValue::init (prg.getLocation (), Void::init (prg.getLocation ()));
	    val.setThrowers (throwers);
	    return val;
	}
	
	/**
	 * ========================================================
	 *                       MANGLE
	 * ========================================================
	 */
	
	Generator PragmaVisitor::validateMangle (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 1) {
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }
		
	    std::string res;
	    auto mangler = Mangler::init (false);
	    try {
		auto val = this-> _context.validateValue (prg.getContent ()[0], true, false);
		if (val.is <StringValue> ()) { // assumed to be a path
		    res = mangler.manglePath (Ymir::format ("%", val.to <StringValue> ().getValue ()));
		} else {
		    res = mangler.mangle (val);
		}


	    } catch (Error::ErrorList list) {
		try {
		    auto val = this-> _context.validateType (prg.getContent ()[0], true);
		    res = mangler.mangle (val);
		} catch (Error::ErrorList list) {
		    Ymir::Error::occurAndNote (prg.getLocation (), list.errors, ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
		}
	    }

	    return this-> _context.validateValue (
		syntax::String::init (
		    prg.getLocation (),
		    prg.getLocation (),
		    lexing::Word::init (prg.getLocation (), res),
		    lexing::Word::eof ()
		    )
		);		    		   
	}

	/**
	 * ========================================================
	 *                       OPERATOR 
	 * ========================================================
	 */

	Generator PragmaVisitor::validateOperator (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 2 && prg.getContent ().size () != 3) {		    
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }
		
	    auto op = prg.getContent ()[0];
	    if (!op.is <syntax::String> ())
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());

	    syntax::Expression syntOp (syntax::Expression::empty ());
		
	    if (prg.getContent ().size () == 2) {
		if (UnaryVisitor::toOperator (op.to<syntax::String> ().getSequence ()) == generator::Unary::Operator::LAST_OP) {
		    Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
		}
		auto type = this-> _context.validateType (prg.getContent () [1]);
		auto value = FakeValue::init (prg.getLocation (), type);
		syntOp = syntax::Unary::init (op.to <syntax::String> ().getSequence (),
					      TemplateSyntaxWrapper::init (value.getLocation (), value));
	    } else if (prg.getContent ().size () == 3) {
		bool af = false;
		if (BinaryVisitor::toOperator (op.to<syntax::String> ().getSequence (), af) == generator::Binary::Operator::LAST_OP) {
		    Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
		}
		auto left = this-> _context.validateType (prg.getContent () [1]);
		auto right = this-> _context.validateType (prg.getContent () [2]);
		auto lValue = FakeValue::init (prg.getLocation (), left);
		auto rValue = FakeValue::init (prg.getLocation (), right);
		syntOp = syntax::Binary::init (op.to <syntax::String> ().getSequence (),
					       TemplateSyntaxWrapper::init (lValue.getLocation (), lValue),
					       TemplateSyntaxWrapper::init (rValue.getLocation (), rValue),
					       syntax::Expression::empty ());
	    }
		
	    try {
		this-> _context.validateValue (syntOp);
		return BoolValue::init (prg.getLocation (), Bool::init (prg.getLocation ()), true);
	    } catch (Error::ErrorList list) {
		return BoolValue::init (prg.getLocation (), Bool::init (prg.getLocation ()), false);
	    }
	}

	/**
	 * ========================================================
	 *                       FIELD NAMES
	 * ========================================================
	 */
	
	Generator PragmaVisitor::validateFieldNames (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 1) {		    
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }

	    auto type = this-> _context.validateType (prg.getContent ()[0]);
	    match (type) {
		s_of (generator::Enum, en) return validateEnumFieldNames (prg, en);
		s_of (generator::Struct, str) return validateStructFieldNames (prg, str);
		s_of (generator::Class, cl) return validateClassFieldNames (prg, cl);
		s_of (generator::EnumRef, eref) return validateEnumFieldNames (prg, eref.getRef ().to <semantic::Enum> ().getGenerator ().to <generator::Enum> ());
		s_of (generator::StructRef, sref) return validateStructFieldNames (prg, sref.getRef ().to <semantic::Struct> ().getGenerator ().to <generator::Struct> ());
		s_of (generator::ClassRef, cref) return validateClassFieldNames (prg, cref.getRef ().to <semantic::Class> ().getGenerator ().to <generator::Class> ());
		s_of (generator::ClassPtr, cptr) return validateClassFieldNames (prg, cptr.getClassRef ().getRef ().to <semantic::Class> ().getGenerator ().to <generator::Class> ());
		s_of (generator::Type, t) {
		    if (t.getProxy ().is <generator::Enum> ()) return validateEnumFieldNames (prg, t.getProxy ().to <generator::Enum> ());
		    if (t.getProxy ().is <generator::EnumRef> ()) return validateEnumFieldNames (prg, t.getProxy ().to <generator::EnumRef> ().getRef ().to <semantic::Enum> ().getGenerator ().to <generator::Enum> ());
		}		
	    }
	    
	    Ymir::Error::occur (prg.getLocation (), ExternalError::get (TYPE_NO_FIELD), type.prettyString ());
	    return Generator::empty ();
	}

	Generator PragmaVisitor::validateEnumFieldNames (const syntax::Pragma & expression, const generator::Enum & en) {
	    auto inners = en.getFields ();
	    std::vector <Generator> values;
	    for (auto & it : inners) values.push_back (
		this-> _context.validateValue (syntax::String::init (it.getLocation (),
								     it.getLocation (),
								     it.getLocation (),
								     lexing::Word::eof ()))
		);
		    
	    auto innerType = values [0].to <Value> ().getType (); // there is always at least one element in tuple
		    
	    auto type = Array::init (expression.getLocation (), innerType, values.size ());	    
	    return ArrayValue::init (expression.getLocation (), type, values);
	}

	Generator PragmaVisitor::validateStructFieldNames (const syntax::Pragma & expression, const generator::Struct & str) {
	    auto & fields = str.getFields ();
	    std::vector <Generator> params;
	    Generator innerType (Void::init (expression.getLocation ()));
	    std::list <Ymir::Error::ErrorMsg> errors;
		    		    
	    for (auto & field : fields) {
		auto name = field.to <generator::VarDecl> ().getLocation ();
		params.push_back (this-> _context.validateValue (syntax::String::init (
								     name, 
								     name,
								     name,
								     lexing::Word::eof ())));
		innerType = params.back ().to <Value> ().getType ();
	    }			

	    auto arrType = Array::init (expression.getLocation (), innerType, params.size ());		    
	    return ArrayValue::init (expression.getLocation (), arrType, params);
	}

	Generator PragmaVisitor::validateClassFieldNames (const syntax::Pragma & expression, const generator::Class & cl) {
	    bool prv = false, prot = false;	    
	    this-> _context.getClassContext (cl.getRef (), prv, prot);

	    auto value = FakeValue::init (expression.getLocation (), ClassPtr::init (expression.getLocation (), cl.getClassRef ()));
	    std::vector <Generator> params;	    
	    Generator innerType (Void::init (expression.getLocation ()));
	    
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto dot_visit = DotVisitor::init (this-> _context);
	    for (auto & field : cl.getLocalFields ()) {			
		auto name = field.to <generator::VarDecl> ().getName ();
		auto field_access = dot_visit.validateClassFieldAccess (expression.getLocation (), value, name, prv, prot, errors);
		if (!field_access.isEmpty ()) {
		    auto name = field.to <generator::VarDecl> ().getLocation ();
		    params.push_back (this-> _context.validateValue (syntax::String::init (
									 name, 
									 name,
									 name,
									 lexing::Word::eof ())));
		    innerType = params.back ().to <Value> ().getType ();
		}
	    }
	    
	    auto arrType = Array::init (expression.getLocation (), innerType, params.size ());		    
	    return ArrayValue::init (expression.getLocation (), arrType, params);
	}

	/**
	 * ========================================================
	 *                       FIELD OFFSETS
	 * ========================================================
	 */

	Generator PragmaVisitor::validateFieldOffsets (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 1) {		    
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }

	    auto type = this-> _context.validateType (prg.getContent ()[0]);
	    match (type) {
		s_of (generator::Struct, str) return validateStructFieldOffsets (prg, str);
		s_of (generator::StructRef, sref) return validateStructFieldOffsets (prg, sref.getRef ().to <semantic::Struct> ().getGenerator ().to <generator::Struct> ());
	    }

	    Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    return Generator::empty ();	    
	}

	
	Generator PragmaVisitor::validateStructFieldOffsets (const syntax::Pragma & expression, const generator::Struct & str) {
	    auto & fields = str.getFields ();
	    std::vector <Generator> params;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto type = Integer::init (expression.getLocation (), 0, false);
	    auto strType = StructRef::init (expression.getLocation (), str.getRef ());
	    
	    for (auto & field : fields) {
		auto field_type = field.to <generator::VarDecl> ().getVarType ();			    
		auto name = field.to <generator::VarDecl> ().getName ();
			    
		params.push_back (FieldOffset::init (expression.getLocation (), type, strType, name));
	    }

	    auto arrType = Array::init (expression.getLocation (), type, params.size ());	    
	    return ArrayValue::init (expression.getLocation (), arrType, params); 
	}
       
	/**
	 * ========================================================
	 *                       TUPLEOF
	 * ========================================================
	 */

	Generator PragmaVisitor::validateTupleOf (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 1) {		    
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }

	    auto value = this-> _context.validateValue (prg.getContent ()[0]);
	    match (value.to <Value> ().getType ()) {
		s_of_u (StructRef) return validateStructTupleOf (prg, value);
		s_of_u (ClassPtr) return validateClassTupleOf (prg, value);
		s_of_u (ClassRef) return validateClassTupleOf (prg, value);
		s_of_u (Array) return validateArrayTupleOf (prg, value);
	    }

	    Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    return Generator::empty ();
	}
	
	Generator PragmaVisitor::validateStructTupleOf (const syntax::Pragma & expression, const Generator & value) {
	    auto & t = value.to <Value> ().getType ().to <StructRef> ().getRef ().to <semantic::Struct> ().getGenerator ();
	    auto & fields = t.to <generator::Struct> ().getFields ();
	    std::vector <Generator> types;
	    std::vector <Generator> params;
	    std::list <Ymir::Error::ErrorMsg> errors;

	    for (auto & field : fields) {
		auto type = field.to <generator::VarDecl> ().getVarType ();
		auto name = field.to <generator::VarDecl> ().getName ();
		type = Type::init (type.to <Type> (), false);
			    
		params.push_back (StructAccess::init (expression.getLocation (), type, value, name));
		types.push_back (type);
	    }

	    auto tuple = Tuple::init (expression.getLocation (), types);
	    tuple = Type::init (tuple.to <Type> (), false); // Impossible to modify a struct via its tupleof
		    
	    return TupleValue::init (expression.getLocation (), tuple, params);        
	}

	Generator PragmaVisitor::validateClassTupleOf (const syntax::Pragma & expression, const Generator & value) {
	    auto cl = value.to <Value> ().getType ().to <ClassPtr> ().getClassRef ().getRef ().to <semantic::Class> ().getGenerator ();
	    bool prv = false, prot = false;

	    if (value.to<Value> ().getType ().is <ClassProxy> ()) {
		this-> _context.getClassContext (value.to <Value> ().getType ().to <ClassProxy> ().getProxyRef ().getRef (), prv, prot);
		prv = false;
	    } else
		this-> _context.getClassContext (value.to <Value> ().getType ().to <ClassPtr> ().getClassRef ().getRef (), prv, prot);
		    
	    std::vector <Generator> types;
	    std::vector <Generator> params;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto dot_visit = DotVisitor::init (this-> _context);
	    for (auto & field : cl.to <generator::Class> ().getFields ()) {			
		auto name = field.to <generator::VarDecl> ().getName ();
		auto field_access = dot_visit.validateClassFieldAccess (expression.getLocation (), value, name, prv, prot, errors);
		if (!field_access.isEmpty ()) {
		    auto field_type = field_access.to <Value> ().getType ();
		    auto type = Type::init (field_type.to <Type> (), false);			    
		    params.push_back (field_access);
		    types.push_back (type);
		}
	    }

	    auto tuple = Tuple::init (expression.getLocation (), types);
	    tuple = Type::init (tuple.to <Type> (), false);
		    
	    return TupleValue::init (expression.getLocation (), tuple, params);
	}

	Generator PragmaVisitor::validateArrayTupleOf (const syntax::Pragma & expression, const Generator & value) {
	    std::vector <Generator> params;
	    std::vector <Generator> types;
	    auto inner = value.to <Value> ().getType ().to <Array> ().getInners ()[0];
	    inner = Type::init (inner.to <Type> (), false);
	    for (auto it : Ymir::r (0, value.to <Value> ().getType ().to <Array> ().getSize ())) {
		params.push_back (ArrayAccess::init (expression.getLocation (), inner, value, ufixed (it)));
		types.push_back (inner);
	    }

	    auto tuple = Tuple::init (expression.getLocation (), types);
	    tuple = Type::init (tuple.to <Type> (), false);
	    
	    return TupleValue::init (expression.getLocation (), tuple, params);
	}

	/**
	 * ========================================================
	 *                       LOCAL TUPLEOF
	 * ========================================================
	 */
	
	Generator PragmaVisitor::validateLocalTupleOf (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 1) {		    
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }

	    auto value = this-> _context.validateValue (prg.getContent ()[0]);
	    match (value.to <Value> ().getType ()) {
		s_of_u (ClassPtr) return validateClassLocalTupleOf (prg, value);
		s_of_u (ClassRef) return validateClassLocalTupleOf (prg, value);
	    }

	    Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    return Generator::empty ();
	}

	Generator PragmaVisitor::validateClassLocalTupleOf (const syntax::Pragma & expression, const Generator & value) {
	    auto cl = value.to <Value> ().getType ().to <ClassPtr> ().getClassRef ().getRef ().to <semantic::Class> ().getGenerator ();
	    bool prv = false, prot = false;

	    if (value.to<Value> ().getType ().is <ClassProxy> ()) {
		this-> _context.getClassContext (value.to <Value> ().getType ().to <ClassProxy> ().getProxyRef ().getRef (), prv, prot);
		prv = false;
	    } else
		this-> _context.getClassContext (value.to <Value> ().getType ().to <ClassPtr> ().getClassRef ().getRef (), prv, prot);
	    if (prv) { // local tupleof exists only inside class definition
		std::vector <Generator> types;
		std::vector <Generator> params;
		std::list <Ymir::Error::ErrorMsg> errors;
		auto dot_visit = DotVisitor::init (this-> _context);
		for (auto & field : cl.to <generator::Class> ().getLocalFields ()) {			
		    auto name = field.to <generator::VarDecl> ().getName ();
		    auto field_access = dot_visit.validateClassFieldAccess (expression.getLocation (), value, name, prv, prot, errors);
		    if (!field_access.isEmpty ()) {
			auto field_type = field_access.to <Value> ().getType ();
			auto type = Type::init (field_type.to <Type> (), false);
			params.push_back (field_access);
			types.push_back (type);
		    }
		}

		auto tuple = Tuple::init (expression.getLocation (), types);
		tuple = Type::init (tuple.to <Type> (), false);
		    
		return TupleValue::init (expression.getLocation (), tuple, params);
	    }
	    
	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (LOCAL_TUPLEOF_OUT_CLASS));
	    return Generator::empty ();
	}

	/**
	 * ========================================================
	 *                       HAS DEFAULT
	 * ========================================================
	 */

	Generator PragmaVisitor::validateHasDefault (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 1 && prg.getContent ().size () != 2) {		    
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }

	    if (prg.getContent ().size () == 1) {
		auto type = this-> _context.validateType (prg.getContent ()[0]);
		match (type) {
		    s_of (generator::Struct, str) return validateStructHasDefault (prg, str);
		    s_of (generator::StructRef, sref) return validateStructHasDefault (prg, sref.getRef ().to <semantic::Struct> ().getGenerator ().to <generator::Struct> ());
		}
	    } else {
		auto type = this-> _context.validateType (prg.getContent ()[0]);
		auto name = this-> _context.retreiveValue (this-> _context.validateValue (prg.getContent ()[1]));
		match (type) {
		    s_of (generator::Struct, str) return validateStructHasDefaultNamed (prg, str, name);
		    s_of (generator::StructRef, sref) return validateStructHasDefaultNamed (prg, sref.getRef ().to <semantic::Struct> ().getGenerator ().to <generator::Struct> (), name);
		}
	    }

	    Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    return Generator::empty ();
	}

	Generator PragmaVisitor::validateStructHasDefault (const syntax::Pragma & expression, const generator::Struct & str) {
	    auto & fields = str.getFields ();
	    std::vector <Generator> params;
	    Generator innerType (Bool::init (expression.getLocation ()));
	    std::list <Ymir::Error::ErrorMsg> errors;
		    		    
	    for (auto & field : fields) {
		bool has = !(field.to <generator::VarDecl> ().getVarValue ().isEmpty ());
		params.push_back (BoolValue::init (expression.getLocation (), Bool::init (expression.getLocation ()), has));
	    }			

	    auto arrType = Array::init (expression.getLocation (), innerType, params.size ());		    
	    return ArrayValue::init (expression.getLocation (), arrType, params);
	}


	Generator PragmaVisitor::validateStructHasDefaultNamed (const syntax::Pragma & expression, const generator::Struct & str, const Generator & name_) {
	    auto name = name_;
	    if (name.is<Aliaser> ()) name = name.to <Aliaser> ().getWho ();
	    if (!name.is <StringValue> ()) {
		auto inner = Char::init (str.getLocation (), 32);
		auto sliceType = Slice::init (str.getLocation (), inner);
		Ymir::Error::occur (expression.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    name_.to <Value> ().getType ().to <Type> ().getTypeName (),
				    sliceType.prettyString ()
		    );
	    }

	    auto str_content = name.to <StringValue> ().getValue ();
	    if (name.to <StringValue> ().getType ().to <Type> ().getInners () [0].to <Char> ().getSize () == 32) {
		str_content = UtfVisitor::utf32_to_utf8 (str_content);
	    }

	    Ymir::OutBuffer buf;
	    for (auto i : str_content) {
		if (i != '\0') buf.write (i);
	    }
	    
	    auto str_name = buf.str ();
	    
	    auto & fields = str.getFields ();
	    std::vector <Generator> params;
	    Generator innerType (Bool::init (expression.getLocation ()));
	    std::list <Ymir::Error::ErrorMsg> errors;
		    		    
	    for (auto & field : fields) {
		if (field.to <generator::VarDecl> ().getName () == str_name) {
		    bool has = !(field.to <generator::VarDecl> ().getVarValue ().isEmpty ());
		    return BoolValue::init (expression.getLocation (), Bool::init (expression.getLocation ()), has);
		}
	    }
	    
	    auto note = Ymir::Error::createNote (expression.getLocation ());	    	    
	    Ymir::Error::occurAndNote (
		name_.getLocation (),
		note,
		ExternalError::get (UNDEFINED_FIELD_FOR),
		str_name,
		str.prettyString ()
		);
	    
	    return Generator::empty ();
	}

	/**
	 * ========================================================
	 *                       DEFAULT VALUE
	 * ========================================================
	 */

	Generator PragmaVisitor::validateDefaultValue (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 2) {
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }
	    
	    auto type = this-> _context.validateType (prg.getContent ()[0]);
	    auto name = this-> _context.retreiveValue (this-> _context.validateValue (prg.getContent ()[1]));
	    match (type) {
		s_of (generator::Struct, str) return validateStructDefaultValue (prg, str, name);
		s_of (generator::StructRef, sref) return validateStructDefaultValue (prg, sref.getRef ().to <semantic::Struct> ().getGenerator ().to <generator::Struct> (), name);
	    }

	    Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    return Generator::empty ();	    
	}

	Generator PragmaVisitor::validateStructDefaultValue (const syntax::Pragma & prg, const generator::Struct & str, const Generator & name_) {
	    auto name = name_;
	    if (name.is<Aliaser> ()) name = name.to <Aliaser> ().getWho ();
	    if (!name.is <StringValue> ()) {
		auto inner = Char::init (str.getLocation (), 32);
		auto sliceType = Slice::init (str.getLocation (), inner);
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    name_.to <Value> ().getType ().to <Type> ().getTypeName (),
				    sliceType.prettyString ()
		    );
	    }

	    auto str_content = name.to <StringValue> ().getValue ();
	    if (name.to <StringValue> ().getType ().to <Type> ().getInners () [0].to <Char> ().getSize () == 32) {
		str_content = UtfVisitor::utf32_to_utf8 (str_content);
	    }

	    Ymir::OutBuffer buf;
	    for (auto i : str_content) {
		if (i != '\0') buf.write (i);
	    }
	    auto str_name = buf.str ();
	    auto & fields = str.getFields ();
	    
	    for (auto & field : fields) {
		if (field.to <generator::VarDecl> ().getName () == str_name) {
		    if (field.to <generator::VarDecl> ().getVarValue ().isEmpty ()) {
			auto note = Ymir::Error::createNote (prg.getLocation ());
			Ymir::Error::occurAndNote (name_.getLocation (), note, ExternalError::get (FIELD_NO_DEFAULT), name_.prettyString (), str.prettyString ());
		    } else {
			return field.to <generator::VarDecl> ().getVarValue ();
		    }
		}
	    }
	    
	    auto note = Ymir::Error::createNote (prg.getLocation ());
	    	    
	    Ymir::Error::occurAndNote (
		name_.getLocation (),
		note,
		ExternalError::get (UNDEFINED_FIELD_FOR),
		str_name,
		str.prettyString ()
	    );
	    
	    return Generator::empty ();
	}	

	/**
	 * ========================================================
	 *                       FIELD TYPE
	 * ========================================================
	 */

	Generator PragmaVisitor::validateFieldType (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 2) {
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }
	    
	    auto type = this-> _context.validateType (prg.getContent ()[0]);
	    auto name = this-> _context.retreiveValue (this-> _context.validateValue (prg.getContent ()[1]));
	    match (type) {
		s_of (generator::Struct, str) return validateStructFieldType (prg, str, name);
		s_of (generator::StructRef, sref) return validateStructFieldType (prg, sref.getRef ().to <semantic::Struct> ().getGenerator ().to <generator::Struct> (), name);
	    }

	    Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    return Generator::empty ();	    
	}

	Generator PragmaVisitor::validateStructFieldType (const syntax::Pragma & prg, const generator::Struct & str, const Generator & name_) {
	    auto name = name_;
	    if (name.is<Aliaser> ()) name = name.to <Aliaser> ().getWho ();
	    if (!name.is <StringValue> ()) {
		auto inner = Char::init (str.getLocation (), 32);
		auto sliceType = Slice::init (str.getLocation (), inner);
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    name_.to <Value> ().getType ().to <Type> ().getTypeName (),
				    sliceType.prettyString ()
		    );
	    }

	    auto str_content = name.to <StringValue> ().getValue ();
	    if (name.to <StringValue> ().getType ().to <Type> ().getInners () [0].to <Char> ().getSize () == 32) {
		str_content = UtfVisitor::utf32_to_utf8 (str_content);
	    }

	    Ymir::OutBuffer buf;
	    for (auto i : str_content) {
		if (i != '\0') buf.write (i);
	    }
	    
	    auto str_name = buf.str ();
	    auto & fields = str.getFields ();
	    
	    for (auto & field : fields) {
		if (field.to <generator::VarDecl> ().getName () == str_name) {
		    return field.to <generator::VarDecl> ().getVarType ();		
		}
	    }
	    
	    auto note = Ymir::Error::createNote (prg.getLocation ());	    	    
	    Ymir::Error::occurAndNote (
		name_.getLocation (),
		note,
		ExternalError::get (UNDEFINED_FIELD_FOR),
		str_name,
		str.prettyString ()
	    );
	    
	    return Generator::empty ();
	}
	
	/**
	 * ========================================================
	 *                       HAS FIELD 
	 * ========================================================
	 */

	Generator PragmaVisitor::validateHasField (const syntax::Pragma & prg) {
	    if (prg.getContent ().size () != 2) {
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    }
	    
	    auto type = this-> _context.validateType (prg.getContent ()[0]);
	    auto name = this-> _context.retreiveValue (this-> _context.validateValue (prg.getContent ()[1]));
	    match (type) {
		s_of (generator::Struct, str) return validateStructHasField (prg, str, name);
		s_of (generator::StructRef, sref) return validateStructHasField (prg, sref.getRef ().to <semantic::Struct> ().getGenerator ().to <generator::Struct> (), name);
	    }

	    Ymir::Error::occur (prg.getLocation (), ExternalError::get (MALFORMED_PRAGMA), prg.getLocation ().getStr ());
	    return Generator::empty ();	    
	}

	Generator PragmaVisitor::validateStructHasField (const syntax::Pragma & prg, const generator::Struct & str, const Generator & name_) {
	    auto name = name_;
	    if (name.is<Aliaser> ()) name = name.to <Aliaser> ().getWho ();
	    if (!name.is <StringValue> ()) {
		auto inner = Char::init (str.getLocation (), 32);
		auto sliceType = Slice::init (str.getLocation (), inner);
		Ymir::Error::occur (prg.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    name_.to <Value> ().getType ().to <Type> ().getTypeName (),
				    sliceType.prettyString ()
		    );
	    }

	    auto str_content = name.to <StringValue> ().getValue ();
	    if (name.to <StringValue> ().getType ().to <Type> ().getInners () [0].to <Char> ().getSize () == 32) {
		str_content = UtfVisitor::utf32_to_utf8 (str_content);
	    }

	    Ymir::OutBuffer buf;
	    for (auto i : str_content) {
		if (i != '\0') buf.write (i);
	    }
	    
	    auto str_name = buf.str ();
	    auto & fields = str.getFields ();
	    
	    for (auto & field : fields) {
		if (field.to <generator::VarDecl> ().getName () == str_name) {
		    return BoolValue::init (prg.getLocation (), Bool::init (prg.getLocation ()), true);
		}
	    }

	    return BoolValue::init (prg.getLocation (), Bool::init (prg.getLocation ()), false);
	}

	

	
    }

    
}
