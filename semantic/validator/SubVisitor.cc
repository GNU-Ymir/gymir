#include <ymir/semantic/validator/SubVisitor.hh>
#include <ymir/semantic/validator/DotVisitor.hh>
#include <ymir/syntax/expression/Var.hh>
#include <ymir/semantic/generator/value/ModuleAccess.hh>
#include <ymir/syntax/declaration/Class.hh>
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
	    std::list <Ymir::Error::ErrorMsg> errors;
	    Generator left (Generator::empty ());
	    bool failed = false;

	    {
		try {
		    left = this-> _context.validateValue (expression.getLeft ());
		} catch (Error::ErrorList list) {
		    
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    failed = true;
		} 
	    }

	    if (failed) {
		failed = false;
	    	try {
	    	    left = this-> _context.validateType (expression.getLeft (), true);		    
	    	} catch (Error::ErrorList list) {
	    	    
		    failed = true;
	    	} 
	    }

	    if (failed) {
		throw Error::ErrorList {errors};
	    } else errors = {};		   
	    
	    Generator gen (Generator::empty ());
	    match (left) {
		of (MultSym, mult, gen = validateMultSym (expression, mult))		    
		else of (ModuleAccess, acc, gen = validateModuleAccess (expression, acc))
		else of (generator::Enum, en, gen = validateEnum (expression, en))	     
		else of (generator::Struct, str ATTRIBUTE_UNUSED, gen = validateStruct(expression, left))
		else of (generator::Class, cl ATTRIBUTE_UNUSED, gen = validateClass (expression, left, errors))
		else of (TemplateRef, rf ATTRIBUTE_UNUSED, gen = validateTemplate (expression, left, errors))
		else of (MacroRef, rf ATTRIBUTE_UNUSED, gen = validateMacro (expression, left, errors))
		else of (generator::StructRef, str_ ATTRIBUTE_UNUSED, gen = validateStruct (expression, left))
		else of (ClassRef,  cl, gen = validateClass (expression, cl.getRef ().to <semantic::Class> ().getGenerator (), errors))
		else of (Pointer, ptr ATTRIBUTE_UNUSED, gen = validateType (expression, left))
		else of (ClassPtr, ptr, gen = validateClass (expression, ptr.getClassRef ().getRef ().to <semantic::Class> ().getGenerator (), errors))
		else of (Option, o ATTRIBUTE_UNUSED, gen = validateOption (expression, left, errors))
		else of (Type, te ATTRIBUTE_UNUSED, gen = validateType (expression, left));			 
	    }
	    	    
	    if (left.is<Value> () && gen.isEmpty ()) {
		match (left.to <Value> ().getType ()) {		    
		    of (ClassRef, cl ATTRIBUTE_UNUSED, gen = validateClassValue (expression, left))
		    else of (TemplateRef, rf ATTRIBUTE_UNUSED, gen = validateTemplate (expression, left, errors))
		    else of (ClassPtr, ptr ATTRIBUTE_UNUSED, gen = validateClassValue (expression, left));
		}
	    }
	    

	    
	    if (gen.isEmpty ()) {
		this-> error (expression, left, expression.getRight (), errors);
	    }
	    
	    return gen;
	}

	Generator SubVisitor::validateMultSym (const syntax::Binary &expression, const MultSym & mult) {
	    auto right = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    std::vector <Symbol> syms;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector <Generator> gens;

	    for (auto gen : mult.getGenerators ()) {
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
			    auto res = validateEnum (expression, en);
			    if (!res.isEmpty ())
				gens.push_back (res);
			}
		    ) else of (generator::Struct, str ATTRIBUTE_UNUSED, {
			    auto res = validateStruct (expression, gen);
			    if (!res.isEmpty ())
				gens.push_back (res);
			}
		    ) else of (generator::Class, cl ATTRIBUTE_UNUSED, {
			    auto res = validateClass (expression, gen, errors);
			    if (!res.isEmpty ())
				gens.push_back (res);
			}
		    ) else of (ClassRef, cl, {
			    auto res = validateClass (expression, cl.getRef ().to <semantic::Class> ().getGenerator (), errors);
			    if (!res.isEmpty ())
				gens.push_back (res);
			}
		    ) else of (Type, te ATTRIBUTE_UNUSED, {
			    auto res = validateType (expression, gen);
			    if (!res.isEmpty ())
				gens.push_back (res);
			}
		    ) else of (TemplateRef, te ATTRIBUTE_UNUSED, {
			    auto res = validateTemplate (expression, gen, errors);
			    if (res.is <MultSym> ()) {
				if (!res.isEmpty ()) {
				    gens.insert (gens.end (), res.to <MultSym> ().getGenerators ().begin (),
						 res.to <MultSym> ().getGenerators ().end ());
				}
			    } else {
				gens.push_back (res);
			    }
			}
		    ) else of (Value, v, {
			    if (v.getType ().is <ClassPtr> ()) {
				auto res = validateClassValue (expression, gen);
				if (!res.isEmpty ()) gens.push_back (res);
			    }			       
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
	    auto right = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    std::vector <Symbol> syms;
	    std::list <Ymir::Error::ErrorMsg> errors;
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
		this-> error (expression, Generator {acc.clone ()}, expression.getRight (), errors);
	    }

	    return this-> _context.validateMultSym (expression.getLocation (), syms);
	}

	Generator SubVisitor::validateEnum (const syntax::Binary & expression, const generator::Enum & en) {
	    auto right = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    auto val = en.getFieldValue (right);
	    if (val.isEmpty ()) {
		right = removeUnders (right);		
		if (right == EnumRef::MEMBERS) {
		    auto inners = en.getFields ();
		    std::vector <Generator> values;
		    for (auto & it : inners) values.push_back (it.to <generator::VarDecl> ().getVarValue ());
		    
		    auto prox = EnumRef::init (en.getLocation (), en.getRef ());
		    auto innerType = values [0].to <Value> ().getType (); // there is always at least one element in tuple
		    innerType = Type::init (innerType.to <Type> (), prox);
		    
		    auto type = Array::init (expression.getLocation (), innerType, values.size ());
		    type = Type::init (type.to <Type> (), true);
		    innerType = Type::init (innerType.to <Type> (), true);
		    
		    auto slc = Slice::init (expression.getLocation (), innerType);
		    slc = Type::init (slc.to <Type> (), true);
		    auto ret = Copier::init (expression.getLocation (),
					 slc,
					 Aliaser::init (expression.getLocation (), slc,
							ArrayValue::init (expression.getLocation (), type.to <Type> ().toDeeplyMutable (), values)
					     )
			);

		    return ret;
		} else if (right == __TYPEID__) {
		    auto prox = EnumRef::init (en.getLocation (), en.getRef ());
		    auto stringLit = syntax::String::init (
			expression.getLocation (),
			expression.getLocation (),
			lexing::Word::init (expression.getLocation (), prox.prettyString ()),
			lexing::Word::eof ()
		     );
		
		    return this-> _context.validateValue (stringLit);

		} else if (right == __TYPEINFO__) {
		    auto inners = en.getFields ();
		    auto type = inners [0].to <generator::VarDecl> ().getVarValue ().to <Value> ().getType ();	    
		    return this-> _context.validateTypeInfo (expression.getRight ().getLocation (), type);
		} else if (right == EnumRef::INNER_NAME) {
		    auto inners = en.getFields ();
		    auto type = inners [0].to <generator::VarDecl> ().getVarValue ().to <Value> ().getType ();
		    return type;
		}
		
		this-> error (expression, en.clone (), expression.getRight ());
	    }

	    auto prox = EnumRef::init (en.getLocation (), en.getRef ());
	    auto type = val.to <Value> ().getType ();
	    type = Type::init (type.to <Type> (), prox);
	    val = Value::init (expression.getLocation (), val.to <Value> (), type);
	    
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
		else of (Tuple, tl ATTRIBUTE_UNUSED, ret = validateTuple (expression, type))
		else of (StructRef, st ATTRIBUTE_UNUSED, ret = validateStruct (expression, type));
	    }
	    
	    if (ret.isEmpty ()) {
		if (expression.getRight ().is <syntax::Var> ()) {
		    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
		    name = removeUnders (name);		
		    if (name == __TYPEID__) {		    
			auto stringLit = syntax::String::init (
			    expression.getLocation (),
			    expression.getLocation (),
			    lexing::Word::init (expression.getLocation (), type.prettyString ()),
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
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    name = removeUnders (name);
	    
	    if (name == Array::INNER_NAME) {
		return b.to <Type> ().getInners () [0];
	    } else if (name == Array::LEN_NAME) {
		auto type = Integer::init (b.getLocation (), 64, false);
		Fixed::UI value;
		value.u = b.to <Array> ().getSize ();
		return Fixed::init (b.getLocation (), type, value);	       		
	    } else if (name == Array::INIT) {
		std::vector <Generator> params;
		std::list <Ymir::Error::ErrorMsg> errors;
		try {
		    auto bin = syntax::Binary::init (
			b.to <Type> ().getInners ()[0].getLocation (),
			expression.getLeft (),
			expression.getRight (),
			syntax::Expression::empty ()
		    );
		    
		    auto inner = validateType (bin.to <syntax::Binary> (), b.to <Type> ().getInners ()[0]);
		    for (auto it ATTRIBUTE_UNUSED : Ymir::r (0, b.to <Array> ().getSize ()))
			params.push_back (inner);
		} catch (Error::ErrorList list) {
		    
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 

		if (errors.size () != 0) {
		    this-> error (expression, b, expression.getRight (), errors);
		}
		return ArrayValue::init (expression.getLocation (), b, params);
	    }	    
	    
	    return Generator::empty ();	    
	}

	Generator SubVisitor::validateBool (const syntax::Binary & expression, const Generator & b) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    name = removeUnders (name);
	    
	    if (name == __INIT__) {
		return BoolValue::init (b.getLocation (), Bool::init (b.getLocation ()), Bool::INIT);
	    }
	    
	    return Generator::empty ();
	}

	Generator SubVisitor::validateChar (const syntax::Binary & expression, const generator::Generator & c) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    name = removeUnders (name);
	    
	    if (name == __INIT__) {
		return CharValue::init (c.getLocation (), c, Char::INIT);
	    }
	    
	    return Generator::empty ();
	}

	Generator SubVisitor::validateFloat (const syntax::Binary & expression, const generator::Generator & f) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    name = removeUnders (name);
	    
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
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    name = removeUnders (name);
	    
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
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    name = removeUnders (name);
	    
	    if (name == Pointer::INNER_NAME) {
		return p.to <Type> ().getInners () [0];
	    }

	    return Generator::empty ();
	}

	Generator SubVisitor::validateSlice (const syntax::Binary & expression, const generator::Generator & s) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    name = removeUnders (name);
	    
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
		auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
		name = removeUnders (name);
		
		if (name == Tuple::ARITY_NAME) {
		    auto type = Integer::init (t.getLocation (), 32, false);
		    Fixed::UI value;
		    value.u = t.to<Tuple> ().getInners ().size ();
		
		    return Fixed::init (t.getLocation (), type, value);	       			
		} else if (name == Tuple::INIT_NAME) {
		    auto & fields = t.to <Type> ().getInners ();
		    std::list <Ymir::Error::ErrorMsg> errors;
		    std::vector <Generator> params;		    
		    try {
			for (auto & field : fields) {
			    auto bin = syntax::Binary::init (field.getLocation (), expression.getLeft (), expression.getRight (), syntax::Expression::empty ());
			    params.push_back (validateType (bin.to <syntax::Binary> (), field));
			}
		    } catch (Error::ErrorList list) {
			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 

		    if (errors.size () != 0) {
			this-> error (expression, t, expression.getRight (), errors);
		    }
		    return TupleValue::init (expression.getLocation (), t, params);
		}
	    }
	    return Generator::empty ();
	}

	Generator SubVisitor::validateStruct (const syntax::Binary & expression, const generator::Generator & t_) {
	    if (expression.getRight ().is <syntax::Var> ()) {
		auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
		name = removeUnders (name);

		Generator t = t_;
		if (t_.is <StructRef> ()) {
		    t = t_.to <StructRef> ().getRef ().to <semantic::Struct> ().getGenerator ();
		}
		
		if (name == StructRef::INIT_NAME) {
		    auto & fields = t.to <generator::Struct> ().getFields ();
		    std::vector <Generator> params;
		    std::vector <Generator> types;
		    std::list <Ymir::Error::ErrorMsg> errors;
		    
		    try {
			for (auto & field : fields) {
			    auto type = field.to <generator::VarDecl> ().getVarType ();
			    types.push_back (type);
			    if (field.to <generator::VarDecl> ().getVarValue ().isEmpty ()) {
				auto bin = syntax::Binary::init (type.getLocation (), expression.getLeft (), expression.getRight (), syntax::Expression::empty ());
				params.push_back (validateType (bin.to<syntax::Binary> (), type));
			    } else {
				params.push_back (field.to <generator::VarDecl> ().getVarValue ());
			    }
			}
		    } catch (Error::ErrorList list) {			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
			    
		    if (errors.size () != 0) {
			this-> error (expression, t, expression.getRight (), errors);
		    }
		    
		    return StructCst::init (
			expression.getLocation (),
			StructRef::init (expression.getLocation (), t.to<generator::Struct> ().getRef ()),
			t,
			types,
			params
			);
		}
	
		if (name == __TYPEID__) {		    
		    auto stringLit = syntax::String::init (
			expression.getLocation (),
			expression.getLocation (),
			lexing::Word::init (expression.getLocation (), t.to<generator::Struct> ().getName ()),
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

	Generator SubVisitor::validateOption (const syntax::Binary & expression, const generator::Generator & t, std::list <Ymir::Error::ErrorMsg> & errors) {
	    if (expression.getRight ().is <syntax::Var> ()) {
		auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
		name = removeUnders (name);
		
		if (name == Option::INIT_NAME) {
		    auto ptrType = Pointer::init (expression.getLocation (), Void::init (expression.getLocation ()));
		    auto val = Aliaser::init (
			expression.getLocation (),
			t,
			OptionValue::init (expression.getLocation (),
					   t,
					   NullValue::init (expression.getLocation (), ptrType),
					   false
			    )
			);
		    return val;
		}
	    }
	    return Generator::empty ();
	}
	
	
	Generator SubVisitor::validateClass (const syntax::Binary & expression, const generator::Generator & t, std::list <Ymir::Error::ErrorMsg> & errors) {	    
	    if (expression.getRight ().is <syntax::Var> ()) {
		auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
		auto aux_name = removeUnders (name);
		
		if (aux_name == ClassRef::INIT_NAME) {
		    if (t.to <generator::Class> ().getRef ().to <semantic::Class> ().isAbs ()) {
			errors.push_back (Ymir::Error::makeOccur (expression.getLocation (), ExternalError::get (ALLOC_ABSTRACT_CLASS), t.prettyString ()));			
		    }
		    
		    bool succeed = true;
		    Generator gen (Generator::empty ());
		    try {			
			gen = this-> _context.getClassConstructors (expression.getLocation (), t, lexing::Word::eof ());
		    } catch (Error::ErrorList list) {			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			succeed = false;
		    } 
		    
		    if (!succeed) return Generator::empty ();
		    else return gen;
		}
		
		if (aux_name == __TYPEID__) {
		    auto stringLit = syntax::String::init (
			expression.getLocation (),
			expression.getLocation (),
			lexing::Word::init (expression.getLocation (), t.prettyString ()),
			lexing::Word::eof ()
		    );
		
		    return this-> _context.validateValue (stringLit);   
		} else if (aux_name == __TYPEINFO__) {
		    Generator cl = this-> _context.validateClass (t.to <generator::Class> ().getRef ());
		    return this-> _context.validateTypeInfo (
			expression.getRight ().getLocation (),
			cl
			);
		} else if (name == ClassRef::SUPER) {
		    auto ancestor = t.to<generator::Class> ().getClassRef ().to <ClassRef> ().getAncestor ();
		    if (!ancestor.isEmpty ()) {
			auto inner = Type::init (ancestor.to <Type> (), false); 
			auto type = Type::init (ClassPtr::init (expression.getLocation (), inner).to <Type> (), false);
			return type;
		    }
		} else {
		    if (t.to <generator::Class> ().getRef ().to <semantic::Class> ().isAbs ()) {
			errors.push_back (Ymir::Error::makeOccur (expression.getLocation (), ExternalError::get (ALLOC_ABSTRACT_CLASS), t.prettyString ()));			
		    }
		    
		    bool succeed = true;
		    Generator gen (Generator::empty ());
		    try {			
			gen = this-> _context.getClassConstructors (expression.getLocation (), t, expression.getRight ().to <syntax::Var> ().getName ());
		    } catch (Error::ErrorList list) {			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			succeed = false;
		    } 
		    
		    if (!succeed) return Generator::empty ();
		    else return gen;
		}	       
	    }

	    return Generator::empty ();
	}


	generator::Generator SubVisitor::validateMacro (const syntax::Binary & expression, const generator::Generator & t, std::list <Ymir::Error::ErrorMsg> & errors) {
	    if (expression.getRight ().is <syntax::Var> ()) {
		auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
		auto syms = this-> _context.getMacroRules (expression.getLocation (), t.to <generator::MacroRef> (), name);
		std::vector <Generator> gens;
		for (auto & it : syms) {
		    gens.push_back (MacroRuleRef::init (it.getName (), it));
		}
		
		if (gens.size () == 1) return gens [0];
		else if (gens.size () != 0) {
		    return MultSym::init (expression.getLocation (), gens);
		}
	    }
	    return Generator::empty ();
	}
	
	Generator SubVisitor::validateTemplate (const syntax::Binary & expression, const generator::Generator & t, std::list <Ymir::Error::ErrorMsg> & errors) {
	    if (expression.getRight ().is <syntax::Var> ()) {
		auto tmp = t.to <TemplateRef> ().getTemplateRef ().to <semantic::Template> ().getDeclaration ();
		auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();

		if (!tmp.is <syntax::Class> ()) return Generator::empty ();

		if (removeUnders (name) == ClassRef::INIT_NAME) {
		    if (tmp.to <syntax::Class> ().isAbstract ()) {
			errors.push_back (Ymir::Error::makeOccur (expression.getLocation (), ExternalError::get (ALLOC_ABSTRACT_CLASS), t.prettyString ()));			
		    }

		    auto constructors = this-> _context.getAllConstructors (tmp.to <syntax::Class> ().getDeclarations (), lexing::Word::eof ());
		    std::vector <syntax::Function::Prototype> csts;
		    for (auto & it : constructors) {
			csts.push_back (it.to <syntax::Constructor> ().getPrototype ());
		    }
		    
		    if (csts.size () != 0) {
			return TemplateClassCst::init (expression.getLocation (), t.to<TemplateRef> ().getTemplateRef (), csts); 
		    } else
			return Generator::empty ();
		} else if (name != __TYPEID__ && name != __TYPEINFO__ && name != ClassRef::SUPER) {
		    if (tmp.to <syntax::Class> ().isAbstract ()) {
			errors.push_back (Ymir::Error::makeOccur (expression.getLocation (), ExternalError::get (ALLOC_ABSTRACT_CLASS), t.prettyString ()));			
		    }

		    auto constructors = this-> _context.getAllConstructors (tmp.to <syntax::Class> ().getDeclarations (), expression.getRight ().to <syntax::Var> ().getName ());
		    std::vector <syntax::Function::Prototype> csts;
		    for (auto & it : constructors) {
			csts.push_back (it.to <syntax::Constructor> ().getPrototype ());
		    }
		    
		    if (csts.size () != 0) {
			return TemplateClassCst::init (expression.getLocation (), t.to<TemplateRef> ().getTemplateRef (), csts); 
		    } else
			return Generator::empty ();
		}
	    }

	    return Generator::empty ();
   
	}	

	Generator SubVisitor::validateClassValue (const syntax::Binary & expression, const generator::Generator & value) {
	    if (expression.getRight ().is <syntax::Var> ()) {
		auto opName = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
		auto ancOpName = opName;
		opName = removeUnders (opName);	
		if (opName == SubVisitor::__TYPEINFO__) {		    
		    auto loc = expression.getLocation ();
		    auto type = value.to <Value> ().getType ();
		    if (type.is <ClassPtr> ()) type = type.to<ClassPtr> ().getInners ()[0];
		    auto typeInfoValue = this-> _context.validateTypeInfo (expression.getLocation (), type);

		    // It is a pointer in the vtable, we need to unref it
		    auto typeInfo = Type::init (typeInfoValue.to<Value> ().getType ().to <Type> (), false, true);
		    
		    return VtableAccess::init (loc,
					       typeInfo,
					       value,
					       0,
					       SubVisitor::__TYPEINFO__
		    );
		} else if (ancOpName == ClassRef::SUPER) {
		    bool prv = false, prot = false;
		    if (!value.to <Value> ().getType ().is <ClassProxy> ()) {
			this-> _context.getClassContext (value.to <Value> ().getType ().to <ClassPtr> ().getClassRef ().getRef (), prv, prot);
			if (prv) {
			    auto ancestor = value.to <Value>().getType ().to <ClassPtr> ().getInners ()[0].to <ClassRef> ().getAncestor ();
			    if (!ancestor.isEmpty ()) {
				auto clRef = value.to<Value> ().getType ().to <Type> ().getInners () [0];
				auto inner = Type::init (ancestor.to <Type> (), clRef.to <Type> ().isMutable ()); // mutability of the inner ref
				// mutability of ref
				auto proxyType = Type::init (ClassProxy::init (expression.getLocation (), inner, clRef).to <Type> (), value.to <Value> ().getType ().to <Type> ().isMutable ());
				return Value::init (value.to <Value> (), proxyType);			
			    }
			}
		    }
		}
	    }

	    bool prv = false, prot = false;
	    if (!value.to <Value> ().getType ().is <ClassProxy> ()) {
		this-> _context.getClassContext (value.to <Value> ().getType ().to <ClassPtr> ().getClassRef ().getRef (), prv, prot);
	    
		if (prv) { // can Class proxy
		    auto ancestor = this-> _context.validateValue (expression.getRight ());
		    if (ancestor.is <generator::Class> ()) {
			auto ancClRef = ancestor.to <generator::Class> ().getClassRef ();
			this-> _context.verifyCompatibleType (expression.getLocation (), value.getLocation (), ClassPtr::init (expression.getLocation (), ancClRef), value.to <Value> ().getType ());

			
			auto clRef = value.to<Value> ().getType ().to <Type> ().getInners () [0];
			auto inner = Type::init (ancClRef.to <Type> (), clRef.to <Type> ().isMutable ()); // mutability of the inner ref
			// mutability of ref
			auto proxyType = Type::init (ClassProxy::init (expression.getLocation (), inner, clRef).to <Type> (), value.to <Value> ().getType ().to <Type> ().isMutable ());
			return Value::init (value.to <Value> (), proxyType);			
		    }
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
		    else of (MultSym,    sym,   leftName = sym.prettyString ())
		    else of (generator::Class, cl, leftName = cl.getName ())
		    else of (ClassRef, cl, leftName = cl.getName ())
		    else of (TemplateRef, rf, leftName = rf.prettyString ())
		    else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ())
		    else of (Type,       type,  leftName = type.getTypeName ())
		    else of (MacroRef,   rf,    leftName = rf.prettyString ());
		}
	    }
	    {		
		match (right) {
		    of (syntax::Var, var, rightName = var.getName ().getStr ());
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

	void SubVisitor::error (const syntax::Binary & expression, const generator::Generator & left, const syntax::Expression & right, std::list <Ymir::Error::ErrorMsg> & errors) {
	    std::string leftName;
	    std::string rightName = "";
	    {
		match (left) {
		    of (FrameProto, proto, leftName = proto.getName ())
		    else of (generator::Struct, str, leftName = str.getName ())
		    else of (generator::Enum, en, leftName = en.getName ())
		    else of (MultSym,    sym,   leftName = sym.prettyString ())
		    else of (generator::Class, cl, leftName = cl.getName ())
		    else of (ClassRef, cl, leftName = cl.getName ())
		    else of (ModuleAccess, acc, leftName = acc.prettyString ())
		    else of (TemplateRef, rf, leftName = rf.prettyString ())
		    else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ())
		    else of (Type,       type,  leftName = type.getTypeName ())
		    else of (MacroRef,   rf,    leftName = rf.prettyString ());
		}
	    }
	    {
		match (right) {
		    of (syntax::Var, var, rightName = var.getName ().getStr ());
		}
	    }
	    
	    if (rightName == "") {
		auto val = this-> _context.retreiveValue (this-> _context.validateValue (right));
		rightName = val.prettyString ();
	    }

	    Ymir::Error::occurAndNote (
		expression.getLocation (),
		expression.getRight ().getLocation (),
		errors,
		ExternalError::get (UNDEFINED_SUB_PART_FOR),
		rightName,
		leftName
	    );
	    
	    //throw Error::ErrorList {errors};
	}

	std::string SubVisitor::removeUnders (const std::string & name) const {
	    Ymir::OutBuffer buf;
	    int nb = 0;
	    bool started = false;
	    for (unsigned int i = 0 ; i < name.length (); i ++) {
		if (name[i] != '_') {
		    if (started) for (auto j ATTRIBUTE_UNUSED : Ymir::r (0, nb)) buf.write ('_');
		    buf.write (name [i]);
		    started = true;
		    nb = 0;
		} else {
		    nb += 1;
		}
	    }	    
	    return buf.str ();
	}
	
    }    

}
