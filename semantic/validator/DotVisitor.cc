#include <ymir/semantic/validator/DotVisitor.hh>
#include <ymir/semantic/validator/CompileTime.hh>
#include <ymir/semantic/validator/CallVisitor.hh>
#include <ymir/semantic/generator/value/_.hh>
#include <ymir/utils/Path.hh>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	
	DotVisitor::DotVisitor (Visitor & context) :
	    _context (context)
	{}

	DotVisitor DotVisitor::init (Visitor & context) {
	    return DotVisitor (context);
	}

	Generator DotVisitor::validate (const syntax::Binary & expression, bool isFromCall) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto left = this-> _context.validateValue (expression.getLeft ());
	    
	    Generator ret (Generator::empty ());
	    match (left.to <Value> ().getType ()) {
		of_u (Tuple) {
		    ret = validateTuple (expression, left);
		}		    
		elof_u (StructRef) {
		    ret = validateStruct (expression, left);
		}			 
		elof_u (Array) {
		    ret = validateArray (expression, left);
		}
		elof_u (Slice) {
		    ret = validateSlice (expression, left);
		}			 
		elof_u (Range) {
		    ret = validateRange (expression, left);
	        }
		elof_u (ClassPtr) {			    
		    ret = validateClass (expression, left, errors);
		} fo;
	    }
	    
	    if (!ret.isEmpty ()) return ret;
	    else if (expression.getRight ().is<syntax::TemplateCall> () && !isFromCall) {
		return validateDotTemplateCall (expression, left);
	    }
	    
	    if (expression.getRight ().is <syntax::Var> ()) 
		this-> error (expression, left, expression.getRight ().to <syntax::Var> ().getName ().getStr (), errors);
	    else {
		auto right = this-> _context.validateValue (expression.getRight ());
		this-> error (expression, left, right.to <Value> ().prettyString (), errors);
	    }

	    return Generator::empty ();
	}

	Generator DotVisitor::validateTuple (const syntax::Binary & expression, const Generator & left) {
	    auto right = this-> _context.validateValue (expression.getRight ());
	    auto index = CompileTime::init (this-> _context).execute (right);

	    if (!index.to <Value> ().getType ().is<Integer> ()) {
		auto note = Ymir::Error::createNote (index.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						     index.to <Value> ().getType ().to <Type> ().getTypeName (),
						     Integer::init (index.getLocation (), 64, false). to <Type> ().getTypeName ()
		    );
		
		Ymir::Error::occurAndNote (expression.getLocation (), note, ExternalError::get (UNDEFINED_BIN_OP), expression.getLocation ().getStr (),
					   left.to <Value> ().getType ().prettyString (), index.to <Value> ().getType ().prettyString ());		
	    }
	    
	    if (!index.is <Fixed> ()) {
		auto note = Ymir::Error::createNote (expression.getLocation ());
		Ymir::Error::occurAndNote (index.getLocation (), note, ExternalError::get (COMPILE_TIME_UNKNOWN));
	    }
	    
	    auto index_val = index.to <Fixed> ().getUI ().u;
	    auto & tu_inners = left.to <Value> ().getType ().to <Tuple> ().getInners ();
	    if (index_val >= tu_inners.size ()) {
		auto note = Ymir::Error::createNote (expression.getLocation ());
		Ymir::Error::occurAndNote (index.getLocation (), note, ExternalError::get (OVERFLOW_ARITY), index_val, tu_inners.size ());
	    }
	    auto type = tu_inners [index_val];
	    if (
		left.to <Value> ().isLvalue () &&
		left.to <Value> ().getType ().to <Type> ().isMutable () &&
		type.to <Type> ().isMutable ()
	    )
		type = Type::init (type.to <Type> (), true);
	    else
		type = Type::init (type.to<Type> (), false);

	    return TupleAccess::init (expression.getLocation (), type, left, index_val);
	}

	Generator DotVisitor::validateStruct (const syntax::Binary & expression, const Generator & left) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();

	    auto & str = left.to <Value> ().getType ().to <StructRef> ().getRef ().to <semantic::Struct> ().getGenerator ();
	    auto field_type = str.to <generator::Struct> ().getFieldType (name);
	    if (field_type.isEmpty ()) return Generator::empty ();
	    else {
		if (
		    left.to <Value> ().isLvalue () &&
		    left.to <Value> ().getType ().to <Type> ().isMutable () &&
		    field_type.to <Type> ().isMutable ()
		)
		    field_type = Type::init (field_type.to <Type> (), true);
		else
		    field_type = Type::init (field_type.to<Type> (), false);
		
		return StructAccess::init (expression.getLocation (), field_type, left, name);
	    }	    
	}

	Generator DotVisitor::validateArray (const syntax::Binary & expression, const Generator & left) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();

	    if (name == Array::LEN_NAME) {
		return ufixed (left.to <Value> ().getType ().to <Array> ().getSize ());		
	    }
	    
	    return Generator::empty ();
	}

	Generator DotVisitor::validateSlice (const syntax::Binary & expression, const Generator & left) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();

	    if (name == Slice::LEN_NAME) {
		return StructAccess::init (expression.getLocation (),
					   Integer::init (expression.getLocation (), 64, false),
					   left, name);
	    }

	    if (name == Slice::PTR_NAME) {
		auto inner = left.to<Value> ().getType ().to <Slice> ().getInners ()[0];
		auto type =  Type::init (Pointer::init (expression.getLocation (), inner).to <Type> (), left.to <Value> ().getType ().to<Type> ().isMutable ());
		return StructAccess::init (expression.getLocation (),
					   type, left, name);
	    }
	    
	    return Generator::empty ();
	}


	Generator DotVisitor::validateRange (const syntax::Binary & expression, const Generator & left) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();

	    if (name == Range::FST_NAME || name == Range::SCD_NAME) {
		auto innerType = left.to <Value> ().getType ().to <Type> ().getInners ()[0];
		innerType = Type::init (innerType.to<Type> (), false);
		return StructAccess::init (expression.getLocation (),
					   innerType,
					   left, name);					   
	    }

	    if (name == Range::STEP_NAME) {
		auto step_type = left.to<Value> ().getType ().to <Range> ().getStepType ();
		step_type = Type::init (step_type.to <Type> (), left.to <Value> ().getType ().to <Type> ().isMutable ());
		return StructAccess::init (expression.getLocation (),
					   step_type,
					   left, name);	
	    }

	    if (name == Range::FULL_NAME) {
		auto full_type = Bool::init (expression.getLocation ());
		full_type = Type::init (full_type.to <Type> (), false);
		return StructAccess::init (expression.getLocation (),
					   full_type,
					   left, name);	
		
	    }
	    
	    return Generator::empty ();	    
	}

	Generator DotVisitor::validateClassFieldAccess (const lexing::Word & loc, const Generator & classValue, const std::string & name, bool prv, bool prot, std::list <Ymir::Error::ErrorMsg> & errors) {
	    auto cl = classValue.to <Value> ().getType ().to <ClassPtr> ().getClassRef ().getRef ().to <semantic::Class> ().getGenerator ();
	    int i = 0;
	    
	    // Field
	    while (!cl.isEmpty ()) {
		Generator type (Generator::empty ());
		if (i == 0 && prv) {
		    type = cl.to <generator::Class> ().getFieldType (name);		
		} else if (prot) {
		    type = cl.to <generator::Class> ().getFieldTypeProtected (name);
		    if (type.isEmpty ()) {
			type = cl.to <generator::Class> ().getFieldType (name);		    
			if (!type.isEmpty ()) {
			    errors.push_back (
				Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), type.getLocation (), name)
			    );
			    break;
			}
		    }
		} else {
		    type = cl.to <generator::Class> ().getFieldTypePublic (name);
		    if (type.isEmpty ()) {
			auto type = cl.to <generator::Class> ().getFieldType (name);
			if (!type.isEmpty ()) {
			    errors.push_back (
				Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), type.getLocation (), name)
			    );
			    break;
			}			
		    }
		}

		if (!type.isEmpty ()) {			     
		    if (
			classValue.to <Value> ().isLvalue () &&
			classValue.to <Value> ().getType ().to <Type> ().isMutable () &&
			type.to <Type> ().isMutable () &&
			classValue.to <Value> ().getType ().to <Type> ().getInners ()[0].to <Type> ().isMutable () // ClassRef must be mutable 
		    )
			type = Type::init (type.to <Type> (), true);
		    else
			type = Type::init (type.to<Type> (), false);
		    
		    return StructAccess::init (loc,
					       type,
					       classValue, name);
		}
		
		auto ancestor = cl.to <generator::Class> ().getClassRef ().to <ClassRef> ().getAncestor ();
		if (!ancestor.isEmpty ())
		    cl = ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ();
		else cl = Generator::empty ();
		i += 1;
	    }
	    
	    return Generator::empty ();
	}
	
	Generator DotVisitor::validateClass (const syntax::Binary & expression, const Generator & left, std::list <Ymir::Error::ErrorMsg> & errors) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().getStr ();
	    bool proxy = left.to <Value> ().getType ().is <ClassProxy> ();
	    bool prv = false, prot = false;
	    if (!proxy) 
		this-> _context.getClassContext (left.to <Value> ().getType ().to <ClassPtr> ().getClassRef ().getRef (), prv, prot);
	    else {
		this-> _context.getClassContext (left.to <Value> ().getType ().to <ClassProxy> ().getProxyRef ().getRef (), prv, prot);
		prv = false;
	    }

	    auto field_access = this-> validateClassFieldAccess (expression.getLocation (), left, name, prv, prot, errors);
	    if (!field_access.isEmpty ()) return field_access;	    

	    // Method
	    auto cl = left.to <Value> ().getType ().to <ClassPtr> ().getClassRef ().getRef ().to <semantic::Class> ().getGenerator ();

	    std::vector <Generator> syms;
	    auto & vtable = cl.to <generator::Class> ().getVtable ();
	    auto & protVtable = cl.to <generator::Class> ().getProtectionVtable ();

	    for (auto i : Ymir::r (0, vtable.size ())) {
		if (Ymir::Path (vtable [i].to <FrameProto> ().getName (), "::").fileName ().toString () == name) {
		    if (prv || (prot && protVtable [i] == generator::Class::MethodProtection::PROT) || protVtable [i] == generator::Class::MethodProtection::PUB) {			
			std::vector <Generator> types;
			for (auto & it : vtable [i].to <FrameProto> ().getParameters ())
			    types.push_back (it.to <ProtoVar> ().getType ());
		    
			auto delType = Delegate::init (vtable [i].getLocation (), vtable [i]);
			// If the class is final, then no need to access the vtable
			if (left.to <Value> ().getType ().is <ClassProxy> () || cl.to <generator::Class> ().getClassRef ().to <ClassRef> ().getRef ().to <semantic::Class> ().isFinal ()) {
			    syms.push_back (
				DelegateValue::init (lexing::Word::init (expression.getLocation (), name), delType,
						     vtable [i].to <MethodProto> ().getClassType (),
						     left,
						     vtable [i]
				    )
				);
			} else {
			    syms.push_back (
				DelegateValue::init (lexing::Word::init (expression.getLocation (), name), delType,
						     vtable [i].to <MethodProto> ().getClassType (),
						     left,
						     VtableAccess::init (expression.getLocation (),
									 FuncPtr::init (expression.getLocation (),
											vtable [i].to <FrameProto> ().getReturnType (),
											types),
									 left,
									 i + 1, // + 1 to ignore the typeinfo,
									 name
							 )
				    )
				);
			}
		    } else {
			errors.push_back (
			    Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), vtable [i].getLocation (), vtable[i].prettyString ())
			);
		    }
		}
	    }

	    cl = left.to <Value> ().getType ().to <ClassPtr> ().getClassRef ().getRef ().to <semantic::Class> ().getGenerator ();

	    int i = 0;
	    if (left.to <Value> ().getType ().is <ClassProxy> ()) i += 1; // can't access private of ancestor even through proxy
	    
	    // Template methods
	    while (!cl.isEmpty ()) {
		auto clRef = left.to <Value> ().getType ();//Type::init (cl.to <generator::Class> ().getClassRef ().to <Type> (), true);
		clRef = Type::init (clRef.to <Type> (), clRef.to<Type> ().isMutable (), false);
		
		for (auto & it : cl.to <generator::Class> ().getRef ().to <semantic::Class> ().getAllInner ()) {
		    match (it) {
			of (Template, tl) {
			    if (tl.getName () == name) {
				if ((i == 0 && prv) || (prot && it.isProtected ()) || it.isPublic ()) {
				    auto realRef = left.to <Value> ().getType ();
				    auto innerRealRef = realRef.to <ClassPtr> ().getInners ()[0];
					
				    auto parentClRef = cl.to <generator::Class> ().getClassRef ();
				    auto parentRef = ClassPtr::init (cl.getLocation (), Type::init (parentClRef.to <Type> (), innerRealRef.to <Type> ().isMutable (), false));
				    parentRef = Type::init (parentRef.to <Type> (), realRef.to <Type> ().isMutable (), false);
					
				    // We cast it, because the parent method must be validated with parent class ref					    
				    auto castedRef = Cast::init (left.getLocation (), parentRef, left);
				    if (left.is <Aliaser> ()) {
					castedRef = Aliaser::init (castedRef.getLocation (), parentRef, castedRef);
				    }
					
				    syms.push_back (
					MethodTemplateRef::init (lexing::Word::init (expression.getLocation (), tl.getName ().getStr ()), it, castedRef)
					);					
				} else {
				    errors.push_back (
					Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), it.getName (), tl.prettyString ())
					);			    
				}
			    }			    
			} fo;			
		    }
		}
		auto ancestor = cl.to <generator::Class> ().getClassRef ().to <ClassRef> ().getAncestor ();
		if (!ancestor.isEmpty ())
		    cl = ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ();
		else cl = Generator::empty ();
		i += 1;
	    }
	    
	    if (syms.size () != 0) 
		return MultSym::init (lexing::Word::init (expression.getLocation (), name), syms);
	    	    
	    return Generator::empty ();
	}
	

	generator::Generator DotVisitor::validateDotTemplateCall (const syntax::Binary & expression, const generator::Generator & left) {
	    auto call = CallVisitor::init (this-> _context);
	    // We throw the errors of the call visitor,
	    // if we are here, it is the only possible meaning of the expression
	    // The user can't have mean something else, unless they made a syntax mistake
	    
	    auto elem = this-> _context.validateValue (expression.getRight (), false, true); // We are in a call finfine

	    int score;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto ret = call.validate (expression.getLocation (), elem, {left}, score, errors);
	    if (ret.isEmpty ())
		call.error (expression.getLocation (), elem, {left}, errors);
	    return ret;
	}
	
	void DotVisitor::error (const syntax::Binary & expression, const generator::Generator & left, const std::string & right, std::list <Ymir::Error::ErrorMsg> & errors) {
	    std::string leftName;
	    match (left) {
		of (FrameProto, proto) leftName = proto.getName ();
		elof (generator::Struct, str) leftName = str.getName ();
		elof (MultSym,    sym) leftName = sym.getLocation ().getStr ();
		elof (Value,      val)  leftName = val.getType ().to <Type> ().getTypeName ();
		fo;
	    }
	    
	    Ymir::Error::occurAndNote (
		expression.getLocation (),
		expression.getRight ().getLocation (),
		errors,
		ExternalError::get (UNDEFINED_FIELD_FOR),
		right,
		leftName
	    );
	    
	    //THROW (ErrorCode::EXTERNAL, errors);		
	}

	
    }

}
