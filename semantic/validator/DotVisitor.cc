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
	    std::list <std::string> errors;
	    auto left = this-> _context.validateValue (expression.getLeft ());
	    
	    Generator ret (Generator::empty ());
	    match (left.to <Value> ().getType ()) {
		of (Tuple, tu ATTRIBUTE_UNUSED,
		    ret = validateTuple (expression, left);
		)
		    
		else of (StructRef, st ATTRIBUTE_UNUSED,
		    ret = validateStruct (expression, left);
		)
			 
		else of (Array, arr ATTRIBUTE_UNUSED,
		     ret = validateArray (expression, left);
		)

		else of (Slice, sl ATTRIBUTE_UNUSED,
		     ret = validateSlice (expression, left);
		)
			 
		else of (Range, rng ATTRIBUTE_UNUSED,
		    ret = validateRange (expression, left);
		)

		else of (ClassRef, cl ATTRIBUTE_UNUSED,
			 ret = validateClass (expression, left, errors);
		); 		
	    }

	    if (!ret.isEmpty ()) return ret;
	    else if (expression.getRight ().is<syntax::TemplateCall> () && !isFromCall) {
		return validateDotTemplateCall (expression, left);
	    }
	    
	    if (expression.getRight ().is <syntax::Var> ()) 
		this-> error (expression, left, expression.getRight ().to <syntax::Var> ().getName ().str, errors);
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
		Ymir::Error::occur (index.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    index.to <Value> ().getType ().to <Type> ().getTypeName (),
				    Integer::init (index.getLocation (), 64, false). to <Type> ().getTypeName ()
		);
	    }
	    
	    if (!index.is <Fixed> ()) {
		Ymir::Error::occur (index.getLocation (), ExternalError::get (COMPILE_TIME_UNKNOWN));
	    }
	    
	    auto index_val = index.to <Fixed> ().getUI ().u;
	    auto & tu_inners = left.to <Value> ().getType ().to <Tuple> ().getInners ();
	    if (index_val >= tu_inners.size ()) {
		Ymir::Error::occur (index.getLocation (), ExternalError::get (OVERFLOW_ARITY), index_val, tu_inners.size ());
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
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;

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
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;

	    if (name == Array::LEN_NAME) {
		return ufixed (left.to <Value> ().getType ().to <Array> ().getSize ());		
	    }
	    
	    return Generator::empty ();
	}

	Generator DotVisitor::validateSlice (const syntax::Binary & expression, const Generator & left) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;

	    if (name == Slice::LEN_NAME) {
		return StructAccess::init (expression.getLocation (),
					   Integer::init (expression.getLocation (), 64, false),
					   left, name);
	    }

	    if (name == Slice::PTR_NAME) {
		auto inner = left.to<Value> ().getType ().to <Slice> ().getInners ()[0];
		return StructAccess::init (expression.getLocation (),
					   Pointer::init (expression.getLocation (),
							  inner
					   ),
					   left, name);
	    }
	    
	    return Generator::empty ();
	}


	Generator DotVisitor::validateRange (const syntax::Binary & expression, const Generator & left) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;

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

	Generator DotVisitor::validateClass (const syntax::Binary & expression, const Generator & left, std::list <std::string> & errors) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;
	    auto cl = left.to <Value> ().getType ().to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ();
	    bool prv = false, prot = false;
	    
	    this-> _context.getClassContext (left.to <Value> ().getType ().to <ClassRef> ().getRef (), prv, prot);
	    int i = 0;
	    
	    // Field
	    while (!cl.isEmpty ()) {
		Generator type (Generator::empty ());
		if (i == 0 && prv) {
		    type = cl.to <generator::Class> ().getFieldType (name);		
		} else if (prv) {
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
			left.to <Value> ().isLvalue () &&
			left.to <Value> ().getType ().to <Type> ().isMutable () &&
			type.to <Type> ().isMutable ()
		    )
			type = Type::init (type.to <Type> (), true);
		    else
			type = Type::init (type.to<Type> (), false);
		    
		    return StructAccess::init (expression.getLocation (),
					       type,
					       left, name);
		}
		
		auto ancestor = cl.to <generator::Class> ().getClassRef ().to <ClassRef> ().getAncestor ();
		if (!ancestor.isEmpty ())
		    cl = ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ();
		else cl = Generator::empty ();
		i += 1;
	    }
	    
	    // Method
	    cl = left.to <Value> ().getType ().to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ();
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
			syms.push_back (
			    DelegateValue::init ({expression.getLocation (), name}, delType,
						 vtable [i].to <MethodProto> ().getClassType (),
						 left,
						 VtableAccess::init (expression.getLocation (),
								     FuncPtr::init (expression.getLocation (),
										    vtable [i].to <FrameProto> ().getReturnType (),
										    types),
								     left,
								     i + 1 // + 1 to ignore the typeinfo
						 )
			    )
			);
		    } else {
			errors.push_back (
			    Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), vtable [i].getLocation (), vtable[i].prettyString ())
			);
		    }
		}
	    }

	    cl = left.to <Value> ().getType ().to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ();
	    // Template methods
	    while (!cl.isEmpty ()) {
		auto clRef = Type::init (cl.to <generator::Class> ().getClassRef ().to <Type> (), true);
		
		for (auto & it : cl.to <generator::Class> ().getRef ().to <semantic::Class> ().getAllInner ()) {
		    match (it) {
			of (Template, tl, {
				if (tl.getName () == name) {
				    if (prv || (prot && it.isProtected ()) || it.isPublic ()) {
					// We cast it, because the parent method must be validated with parent class ref					    
					auto castedRef = Cast::init (left.getLocation (), clRef, left);
					if (left.is <Aliaser> ()) {
					    castedRef = Aliaser::init (castedRef.getLocation (), clRef, left);
					}
					
					syms.push_back (
					    MethodTemplateRef::init ({expression.getLocation (), tl.getName ().str}, it, castedRef)
					);					
				    } else {
					errors.push_back (
					    Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), it.getName (), tl.prettyString ())
					);			    
				    }
				}

			    });			
		    }
		}
		auto ancestor = cl.to <generator::Class> ().getClassRef ().to <ClassRef> ().getAncestor ();
		if (!ancestor.isEmpty ())
		    cl = ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ();
		else cl = Generator::empty ();		
	    }
	    
	    if (syms.size () != 0) 
		return MultSym::init ({expression.getLocation (), name}, syms);
	    	    
	    return Generator::empty ();
	}
	

	generator::Generator DotVisitor::validateDotTemplateCall (const syntax::Binary & expression, const generator::Generator & left) {
	    auto call = CallVisitor::init (this-> _context);
	    // We throw the errors of the call visitor,
	    // if we are here, it is the only possible meaning of the expression
	    // The user can't have mean something else, unless they made a syntax mistake
	    
	    auto elem = this-> _context.validateValue (expression.getRight (), false, true); // We are in a call finfine

	    int score;
	    std::list <std::string> errors;
	    auto ret = call.validate (expression.getLocation (), elem, {left}, score, errors);
	    if (ret.isEmpty ())
		call.error (expression.getLocation (), elem, {left}, errors);
	    return ret;
	}
	
	void DotVisitor::error (const syntax::Binary & expression, const generator::Generator & left, const std::string & right, std::list <std::string> & errors) {
	    std::string leftName;
	    match (left) {
		of (FrameProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())
		    else of (MultSym,    sym,   leftName = sym.getLocation ().str)
			else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ());
	    }

	    OutBuffer buf;
	    for (auto & it : errors) {
		buf.write (it, "\n");
	    }
	    
	    Ymir::Error::occurAndNote (
		expression.getLocation (),
		buf.str (),
		ExternalError::get (UNDEFINED_FIELD_FOR),
		right,
		leftName
	    );
	    
	    //THROW (ErrorCode::EXTERNAL, errors);		
	}

	
    }

}
