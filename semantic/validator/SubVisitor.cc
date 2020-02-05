#include <ymir/semantic/validator/SubVisitor.hh>
#include <ymir/syntax/expression/Var.hh>
#include <ymir/semantic/generator/value/ModuleAccess.hh>
#include <cfloat>
#include <climits>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;

	std::string SubVisitor::__SIZEOF__ = "sizeof";
	std::string SubVisitor::__INIT__ = "init";
	std::string SubVisitor::__TYPEID__ = "typeid";
	std::string SubVisitor::__TYPEINFO__ = "typeinfo";
	
	SubVisitor::SubVisitor (Visitor & context) :
	    _context (context) 
	{}

	SubVisitor SubVisitor::init (Visitor & context) {
	    return SubVisitor {context};
	}

	generator::Generator SubVisitor::validate (const syntax::Binary & expression) {
	    std::vector <std::string> errors;
	    Generator left (Generator::empty ());
	    {
		TRY (
		    left = this-> _context.validateValue (expression.getLeft ());
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;
	    }

	    if (left.isEmpty ()) {
		TRY (
		    left = this-> _context.validateType (expression.getLeft (), true);		    
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		} FINALLY;

		if (left.isEmpty ())
		    THROW (ErrorCode::EXTERNAL, errors);
		else errors = {};
	    }

	    Generator gen (Generator::empty ());
	    match (left) {
		of (MultSym, mult, gen = validateMultSym (expression, mult))		    
		else of (ModuleAccess, acc, gen = validateModuleAccess (expression, acc))
		else of (generator::Enum, en, gen = validateEnum (expression, en))	     
		else of (generator::Struct, str ATTRIBUTE_UNUSED, gen = validateStruct(expression, left))
		else of (generator::Class, cl ATTRIBUTE_UNUSED, gen = validateClass (expression, left, errors))			 
		else of (ClassRef,  cl, gen = validateClass (expression, cl.getRef ().to <semantic::Class> ().getGenerator (), errors))
		else of (Type, te ATTRIBUTE_UNUSED, gen = validateType (expression, left));
	    }
	    	    
	    if (left.is<Value> () && gen.isEmpty ()) {
		match (left.to <Value> ().getType ()) {
		    of (StructRef, str ATTRIBUTE_UNUSED, gen = validateStructValue (expression, left));
		    of (ClassRef, cl ATTRIBUTE_UNUSED, gen = validateClassValue (expression, left));
		}
	    }
	    
	    if (gen.isEmpty ())
		this-> error (expression, left, expression.getRight (), errors);
	    
	    return gen;
	}

	Generator SubVisitor::validateMultSym (const syntax::Binary &expression, const MultSym & mult) {
	    auto right = expression.getRight ().to <syntax::Var> ().getName ().str;
	    std::vector <Symbol> syms;
	    std::vector <std::string> errors;
	    std::vector <Generator> gens;
	    for (auto & gen : mult.getGenerators ()) {
		match (gen) {
		    of (ModuleAccess, md ATTRIBUTE_UNUSED, {
			    if (this-> _context.getModuleContext (gen.to <ModuleAccess> ().getModRef ())) {
				auto elems = gen.to <ModuleAccess> ().getLocal (right);		    
				syms.insert (syms.end (), elems.begin (), elems.end ());
			    } else {
				auto elems = gen.to <ModuleAccess> ().getLocalPublic (right);		    
				if (elems.size () == 0) {
				    elems = gen.to <ModuleAccess> ().getLocal (right);
				    for (auto & it : elems)
					errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), it.getName (), right));
				}
				syms.insert (syms.end (), elems.begin (), elems.end ());
			    }
			}
		    ) else of (generator::Enum, en,  {
			    gens.push_back (validateEnum (expression, en));
			}
		    ) else of (generator::Struct, str ATTRIBUTE_UNUSED, {
			    gens.push_back (validateStruct (expression, gen));
			}
		    ) else of (generator::Class, cl ATTRIBUTE_UNUSED, {
			    gens.push_back (validateClass (expression, gen, errors));
			}
		    ) else of (ClassRef, cl, {
			    gens.push_back (validateClass (expression, cl.getRef ().to <semantic::Class> ().getGenerator (), errors));
			}
		    ) else of (Type, te ATTRIBUTE_UNUSED, {
			    gens.push_back (validateType (expression, gen));
			}
		    ) else of (Value, v, {
			    if (v.getType ().is <StructRef> ())
				gens.push_back (validateStructValue (expression, gen));
			    else if (v.getType ().is <ClassRef> ())
				gens.push_back (validateClassValue (expression, gen));
				
			}
		    )
		}
	    }

	    if (syms.size () == 0 && gens.size () == 0) {
		this-> error (expression, mult.clone (), expression.getRight (), errors);
	    }
	    
	    if (syms.size () != 0) {
		auto s = this-> _context.validateMultSym (expression.getLocation (), syms);
		if (s.is <MultSym> ()) {
		    gens.insert (gens.end (), s.to <MultSym> ().getGenerators ().begin (), s.to <MultSym> ().getGenerators ().end ());
		} else
		    gens.push_back (s);
	    }

	    if (gens.size () == 1) return gens [0];
	    else return MultSym::init (expression.getLocation (), gens);
	}

	Generator SubVisitor::validateModuleAccess (const syntax::Binary &expression, const ModuleAccess & acc) {
	    auto right = expression.getRight ().to <syntax::Var> ().getName ().str;
	    std::vector <Symbol> syms;
	    std::vector <std::string> errors;
	    if (this-> _context.getModuleContext (acc.getModRef ())) {
		syms = acc.getLocal (right);
	    } else {
		syms = acc.getLocalPublic (right);
		if (syms.size () == 0) {
		    auto elems = acc.getLocal (right);
		    for (auto & it : elems)
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), it.getName (), right));
		}
	    }
	    
	    if (syms.size () == 0) {
		this-> error (expression, acc.clone (), expression.getRight (), errors);
	    }
	    
	    return this-> _context.validateMultSym (expression.getLocation (), syms);
	}

	Generator SubVisitor::validateEnum (const syntax::Binary & expression, const generator::Enum & en) {
	    auto right = expression.getRight ().to <syntax::Var> ().getName ().str;
	    auto val = en.getFieldValue (right);
	    if (val.isEmpty ()) {
		this-> error (expression, en.clone (), expression.getRight ());
	    }

	    auto prox = EnumRef::init (en.getLocation (), en.getRef ());
	    auto type = val.to <Value> ().getType ();
	    type.to <Type> ().setProxy (prox);
	    val.to <Value> ().setType (type);
	    
	    return val;
	}	

	Generator SubVisitor::validateType (const syntax::Binary & expression, const Generator & type) {
	    Generator ret (Generator::empty ());
	    match (type) {
		of (Array, ar ATTRIBUTE_UNUSED, ret = validateArray (expression, type))
		else of (Bool, bl ATTRIBUTE_UNUSED, ret = validateBool (expression, type))
		else of (Char, ch ATTRIBUTE_UNUSED, ret = validateChar (expression, type))
		else of (Float, fl ATTRIBUTE_UNUSED, ret = validateFloat (expression, type))
		else of (Integer, it ATTRIBUTE_UNUSED, ret = validateInteger (expression, type))
		else of (Pointer, pt ATTRIBUTE_UNUSED, ret = validatePointer (expression, type))
		else of (Slice, sl ATTRIBUTE_UNUSED, ret = validateSlice (expression, type))
		else of (Tuple, tl ATTRIBUTE_UNUSED, ret = validateTuple (expression, type));
	    }
	    
	    if (ret.isEmpty ()) {
		if (expression.getRight ().is <syntax::Var> ()) {
		    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;
		    if (name == __TYPEID__) {		    
			auto stringLit = syntax::String::init (
			    expression.getLocation (),
			    expression.getLocation (),
			    lexing::Word {expression.getLocation (), type.prettyString ()},
			    lexing::Word::eof ()
			);
		
			return this-> _context.validateValue (stringLit);
		    } else if (name == __TYPEINFO__) {
			return this-> _context.validateTypeInfo (expression.getRight ().getLocation (), type);
		    }
		}

		this-> error (expression, type, expression.getRight ());
	    }
	    
	    return ret;
	}

	    
	Generator SubVisitor::validateArray (const syntax::Binary & expression, const Generator & b) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;

	    if (name == Array::INNER_NAME) {
		return b.to <Type> ().getInners () [0];
	    } else if (name == Array::LEN_NAME) {
		auto type = Integer::init (b.getLocation (), 64, false);
		Fixed::UI value;
		value.u = b.to <Array> ().getSize ();
		return Fixed::init (b.getLocation (), type, value);	       		
	    } else if (name == Array::INIT) {
		std::vector <Generator> params;
		std::vector <std::string> errors;
		TRY (
		    auto bin = syntax::Binary::init (
			b.to <Type> ().getInners ()[0].getLocation (),
			expression.getLeft (),
			expression.getRight (),
			syntax::Expression::empty ()
		    );
		    
		    auto inner = validateType (bin.to <syntax::Binary> (), b.to <Type> ().getInners ()[0]);
		    for (auto it ATTRIBUTE_UNUSED : Ymir::r (0, b.to <Array> ().getSize ()))
			params.push_back (inner);
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;

		if (errors.size () != 0) {
		    this-> error (expression, b, expression.getRight (), errors);
		}
		return ArrayValue::init (expression.getLocation (), b, params);
	    }	    
	    
	    return Generator::empty ();	    
	}

	Generator SubVisitor::validateBool (const syntax::Binary & expression, const Generator & b) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;

	    if (name == __INIT__) {
		return BoolValue::init (b.getLocation (), Bool::init (b.getLocation ()), Bool::INIT);
	    }
	    
	    return Generator::empty ();
	}

	Generator SubVisitor::validateChar (const syntax::Binary & expression, const generator::Generator & c) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;
	    
	    if (name == __INIT__) {
		return CharValue::init (c.getLocation (), c, Char::INIT);
	    }
	    
	    return Generator::empty ();
	}

	Generator SubVisitor::validateFloat (const syntax::Binary & expression, const generator::Generator & f) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;
	    
	    if (name == __INIT__) {
		return FloatValue::init (f.getLocation (), f, Float::INIT);
	    } else if (name == Float::INF_NAME) {
		return FloatValue::init (f.getLocation (), f, Float::INF);
	    } else if (name == Float::MIN_NAME) {
		if (f.to<Float> ().getSize () == 32) {
		    return FloatValue::init (f.getLocation (), f, FLT_MIN);
		} else {
		    return FloatValue::init (f.getLocation (), f, DBL_MIN);
		}
	    } else if (name == Float::MAX_NAME) {
		if (f.to<Float> ().getSize () == 32) {
		    return FloatValue::init (f.getLocation (), f, FLT_MAX);
		} else {
		    return FloatValue::init (f.getLocation (), f, DBL_MAX);
		}
	    } else if (name == Float::NAN_NAME) {
		return FloatValue::init (f.getLocation (), f, Float::NOT_A_NUMBER);
	    } else if (name == Float::DIG_NAME) {
		auto type = Integer::init (f.getLocation (), 32, false);
		Fixed::UI value;
		if (f.to<Float> ().getSize () == 32) 
		    value.u = FLT_DIG;
		else value.u = DBL_DIG;
		
		return Fixed::init (f.getLocation (), type, value);	       		
	    } else if (name == Float::MANT_DIG_NAME) {
		auto type = Integer::init (f.getLocation (), 32, false);
		Fixed::UI value;
		if (f.to<Float> ().getSize () == 32) 
		    value.u = FLT_MANT_DIG;
		else value.u = DBL_MANT_DIG;
		
		return Fixed::init (f.getLocation (), type, value);	       		
	    } else if (name == Float::EPSILON_NAME) {
		if (f.to<Float> ().getSize () == 32) {
		    return FloatValue::init (f.getLocation (), f, FLT_EPSILON);
		} else {
		    return FloatValue::init (f.getLocation (), f, DBL_EPSILON);
		}
	    } else if (name == Float::MAX_10_EXP_NAME) {
		auto type = Integer::init (f.getLocation (), 32, false);
		Fixed::UI value;
		if (f.to<Float> ().getSize () == 32) 
		    value.u = FLT_MAX_10_EXP;
		else value.u = DBL_MAX_10_EXP;
		
		return Fixed::init (f.getLocation (), type, value);	       			
	    } else if (name == Float::MAX_EXP_NAME) {
		auto type = Integer::init (f.getLocation (), 32, false);
		Fixed::UI value;
		if (f.to<Float> ().getSize () == 32) 
		    value.u = FLT_MAX_EXP;
		else value.u = DBL_MAX_EXP;
		
		return Fixed::init (f.getLocation (), type, value);	       			
	    } else if (name == Float::MIN_10_EXP_NAME) {
		auto type = Integer::init (f.getLocation (), 32, false);
		Fixed::UI value;
		if (f.to<Float> ().getSize () == 32) 
		    value.u = FLT_MIN_10_EXP;
		else value.u = DBL_MIN_10_EXP;
		
		return Fixed::init (f.getLocation (), type, value);	       			
	    } else if (name == Float::MIN_EXP_NAME) {
		auto type = Integer::init (f.getLocation (), 32, false);
		Fixed::UI value;
		if (f.to<Float> ().getSize () == 32) 
		    value.u = FLT_MIN_EXP;
		else value.u = DBL_MIN_EXP;
		
		return Fixed::init (f.getLocation (), type, value);	       			
	    }
	    
	    return Generator::empty ();
	}

	Generator SubVisitor::validateInteger (const syntax::Binary & expression, const generator::Generator & i) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;

	    if (name == Integer::INIT_NAME) {
		Fixed::UI value;
		value.i = Integer::INIT;
		return Fixed::init (i.getLocation (), i, value);
	    } else if (name == Integer::MAX_NAME) {
		Fixed::UI value;
		if (i.to <Integer> ().isSigned ()) {		    
		    switch (i.to<Integer> ().getSize ()) {
		    case 8 : value.i = SCHAR_MAX; break;
		    case 16 : value.i = SHRT_MAX; break;
		    case 32 : value.i = INT_MAX; break;
		    case 64 : value.i = LONG_MAX; break;
		    default :
			Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
			break;
		    }
		} else {
		    switch (i.to<Integer> ().getSize ()) {
		    case 8 : value.u = UCHAR_MAX; break;
		    case 16 : value.u = USHRT_MAX; break;
		    case 32 : value.u = UINT_MAX; break;
		    case 64 : value.u = ULONG_MAX; break;
		    default :
			Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
			break;
		    }
		}
		return Fixed::init (i.getLocation (), i, value);
	    } else if (name == Integer::MIN_NAME) {
		Fixed::UI value;
		if (i.to <Integer> ().isSigned ()) {		    
		    switch (i.to<Integer> ().getSize ()) {
		    case 8 : value.i = SCHAR_MIN; break;
		    case 16 : value.i = SHRT_MIN; break;
		    case 32 : value.i = INT_MIN; break;
		    case 64 : value.i = LONG_MIN; break;
		    default :
			Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
			break;
		    }
		} else {
		    value.u = 0;
		}
		return Fixed::init (i.getLocation (), i, value);
	    }

	    return Generator::empty ();
	}

	Generator SubVisitor::validatePointer (const syntax::Binary & expression, const generator::Generator & p) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;

	    // if (name == Pointer::INIT_NAME) {
	    // TODO
	    // } else
	    if (name == Pointer::INNER_NAME) {
		return p.to <Type> ().getInners () [0];
	    }

	    return Generator::empty ();
	}

	Generator SubVisitor::validateSlice (const syntax::Binary & expression, const generator::Generator & s) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;

	    if (name == Slice::INNER_NAME) {
		return s.to <Type> ().getInners () [0];
	    } else if (name == Slice::INIT) {
		return ArrayValue::init (expression.getLocation (), s, {});
	    }

	    return Generator::empty ();
	}

	Generator SubVisitor::validateTuple (const syntax::Binary & expression, const generator::Generator & t) {
	    if (!expression.getRight ().is <syntax::Var> ()) {
		auto value = this-> _context.retreiveValue (this-> _context.validateValue (expression.getRight ()));
		if (!value.is <Fixed> ()) return Generator::empty ();
		if (value.to <Fixed> ().getUI ().i < 0 || value.to <Fixed> ().getUI ().u > t.to<Type> ().getInners ().size ()) return Generator::empty ();
		else {
		    return t.to <Type> ().getInners () [value.to <Fixed> ().getUI ().i];
		}
	    } else {
		auto name = expression.getRight ().to <syntax::Var> ().getName ().str;
		if (name == Tuple::ARITY_NAME) {
		    auto type = Integer::init (t.getLocation (), 32, false);
		    Fixed::UI value;
		    value.u = t.to<Tuple> ().getInners ().size ();
		
		    return Fixed::init (t.getLocation (), type, value);	       			
		} else if (name == Tuple::INIT_NAME) {
		    auto & fields = t.to <Type> ().getInners ();
		    std::vector <std::string> errors;
		    std::vector <Generator> params;		    
		    TRY (
			for (auto & field : fields) {
			    auto bin = syntax::Binary::init (field.getLocation (), expression.getLeft (), expression.getRight (), syntax::Expression::empty ());
			    params.push_back (validateType (bin.to <syntax::Binary> (), field));
			}
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
		    } FINALLY;

		    if (errors.size () != 0) {
			this-> error (expression, t, expression.getRight (), errors);
		    }
		    return TupleValue::init (expression.getLocation (), t, params);
		}
	    }
	    return Generator::empty ();
	}

	Generator SubVisitor::validateStruct (const syntax::Binary & expression, const generator::Generator & t) {
	    if (expression.getRight ().is <syntax::Var> ()) {
		auto name = expression.getRight ().to <syntax::Var> ().getName ().str;
		if (name == StructRef::INIT_NAME) {
		    auto & fields = t.to <generator::Struct> ().getFields ();
		    std::vector <Generator> params;
		    std::vector <Generator> types;
		    std::vector <std::string> errors;
		    TRY (
			for (auto & field : fields) {
			    auto type = field.to <generator::VarDecl> ().getVarType ();
			    types.push_back (type);
			    auto bin = syntax::Binary::init (type.getLocation (), expression.getLeft (), expression.getRight (), syntax::Expression::empty ());
			    params.push_back (validateType (bin.to<syntax::Binary> (), type));
			}
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
		    } FINALLY;

		    if (errors.size () != 0) {
			this-> error (expression, t, expression.getRight (), errors);
		    }
		    
		    return StructCst::init (
			expression.getLocation (),
			StructRef::init (expression.getLocation (), t.to<generator::Struct> ().getRef ()),
			t.clone (),
			types,
			params
		    );
		}
		
		if (name == __TYPEID__) {		    
		    auto stringLit = syntax::String::init (
			expression.getLocation (),
			expression.getLocation (),
			lexing::Word {expression.getLocation (), t.prettyString ()},
			lexing::Word::eof ()
		    );
		
		    return this-> _context.validateValue (stringLit);
		} else if (name == __TYPEINFO__) {
		    return this-> _context.validateTypeInfo (
			expression.getRight ().getLocation (),
			StructRef::init (t.getLocation (), t.to <generator::Struct> ().getRef ())
		    );
		} 
		
	    }
	    return Generator::empty ();
	}

	Generator SubVisitor::validateClass (const syntax::Binary & expression, const generator::Generator & t, std::vector <std::string> &) {	    
	    if (expression.getRight ().is <syntax::Var> ()) {
		auto name = expression.getRight ().to <syntax::Var> ().getName ().str;
		if (name == ClassRef::INIT_NAME) {
		    if (t.to <generator::Class> ().getRef ().to <semantic::Class> ().isAbs ()) {
			Ymir::Error::occur (expression.getLocation (), ExternalError::get (ALLOC_ABSTRACT_CLASS), t.prettyString ());
			
		    }
		    
		    return this-> _context.getClassConstructors (expression.getLocation (), t);
		}


		if (name == __TYPEID__) {
		    auto stringLit = syntax::String::init (
			expression.getLocation (),
			expression.getLocation (),
			lexing::Word {expression.getLocation (), t.prettyString ()},
			lexing::Word::eof ()
		    );
		
		    return this-> _context.validateValue (stringLit);   
		} else if (name == __TYPEINFO__) {
		    Generator cl = this-> _context.validateClass (t.to <generator::Class> ().getRef ());
		    return this-> _context.validateTypeInfo (
			expression.getRight ().getLocation (),
			cl
		    );
		} 
	    }

	    return Generator::empty ();
	}

	Generator SubVisitor::validateClassValue (const syntax::Binary & expression, const generator::Generator & value) {
	    if (expression.getRight ().is <syntax::Var> ()) {
		auto opName = expression.getRight ().to <syntax::Var> ().getName ().str;
		if (opName == SubVisitor::__TYPEINFO__) {		    
		    auto loc = expression.getLocation ();
		    auto typeInfoValue = this-> _context.validateTypeInfo (expression.getLocation (), value.to <Value> ().getType ());
		    auto typeInfo = typeInfoValue.to<Value> ().getType ();
		    typeInfo.to <Type> ().isRef (true); // It is a pointer in the vtable, we need to unref it
		    typeInfo.to <Type> ().isMutable (false);
		    
		    return VtableAccess::init (loc,
					       typeInfo,
					       value,
					       0
		    );
		}
	    }
	    
	    return Generator::empty ();
	}

	
	Generator SubVisitor::validateStructValue (const syntax::Binary & expression, const generator::Generator & value) {
	    auto & t = value.to <Value> ().getType ().to <StructRef> ().getRef ().to <semantic::Struct> ().getGenerator ();
	    if (expression.getRight ().is <syntax::Var> ()) {
		auto opName = expression.getRight ().to <syntax::Var> ().getName ().str;
		if (opName == StructRef::TUPLEOF) {
		    auto & fields = t.to <generator::Struct> ().getFields ();
		    std::vector <Generator> types;
		    std::vector <Generator> params;
		    std::vector <std::string> errors;
		    TRY (
			for (auto & field : fields) {
			    auto type = field.to <generator::VarDecl> ().getVarType ();
			    auto name = field.to <generator::VarDecl> ().getName ();
			    type.to <Type> ().isMutable (false);
			    params.push_back (StructAccess::init (expression.getLocation (), type, value, name));
			    types.push_back (type);
			}
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
		    } FINALLY;

		    if (errors.size () != 0) {
			this-> error (expression, t, expression.getRight (), errors);
		    }
		    auto tuple = Tuple::init (expression.getLocation (), types);
		    tuple.to <Type> ().isMutable (false); // Impossible to modify a struct via its tupleof
		    tuple.to <Type> ().isLocal (false);
		    
		    return TupleValue::init (expression.getLocation (), tuple, params); 
		}
	    }	    
	    return Generator::empty ();
	}

	
	void SubVisitor::error (const syntax::Binary & expression, const generator::Generator & left, const syntax::Expression & right) {
	    std::string leftName;
	    std::string rightName = "";
	    {
		match (left) {
		    of (FrameProto, proto, leftName = proto.getName ())
		    else of (ModuleAccess, acc, leftName = acc.prettyString ())
		    else of (generator::Struct, str, leftName = str.getName ())
		    else of  (generator::Enum, en, leftName = en.getName ())
		    else of (MultSym,    sym,   leftName = sym.getLocation ().str)
		    else of (generator::Class, cl, leftName = cl.getName ())
		    else of (ClassRef, cl, leftName = cl.getName ())
		    else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ())
		    else of (Type,       type,  leftName = type.getTypeName ());
		}
	    }
	    {		
		match (right) {
		    of (syntax::Var, var, rightName = var.getName ().str);
		}
	    }
	    
	    if (rightName == "") {
		auto val = this-> _context.retreiveValue (this-> _context.validateValue (right));
		rightName = val.prettyString ();
	    }

	    
	    Ymir::Error::occur (
		expression.getLocation (),
		ExternalError::get (UNDEFINED_SUB_PART_FOR),
		rightName,
		leftName
	    );
	}

	void SubVisitor::error (const syntax::Binary & expression, const generator::Generator & left, const syntax::Expression & right, std::vector <std::string> & errors) {
	    std::string leftName;
	    std::string rightName = "";
	    {
		match (left) {
		    of (FrameProto, proto, leftName = proto.getName ())
		    else of (generator::Struct, str, leftName = str.getName ())
		    else of  (generator::Enum, en, leftName = en.getName ())
		    else of (MultSym,    sym,   leftName = sym.getLocation ().str)
		    else of (generator::Class, cl, leftName = cl.getName ())
		    else of (ClassRef, cl, leftName = cl.getName ())
		    else of (ModuleAccess, acc, leftName = acc.prettyString ())
		    else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ())
		    else of (Type,       type,  leftName = type.getTypeName ());
		}
	    }
	    {
		match (right) {
		    of (syntax::Var, var, rightName = var.getName ().str);
		}
	    }
	    
	    if (rightName == "") {
		auto val = this-> _context.retreiveValue (this-> _context.validateValue (right));
		rightName = val.prettyString ();
	    }

	    
	    errors.insert (errors.begin (), Ymir::Error::makeOccur (
		expression.getLocation (),
		ExternalError::get (UNDEFINED_SUB_PART_FOR),
		rightName,
		leftName
	    ));
	    
	    THROW (ErrorCode::EXTERNAL, errors);
	}
	
	
    }    

}
