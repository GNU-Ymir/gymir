#include <ymir/semantic/validator/DotVisitor.hh>
#include <ymir/semantic/validator/CompileTime.hh>
#include <ymir/semantic/generator/value/_.hh>

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

	Generator DotVisitor::validate (const syntax::Binary & expression) {	    
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
		);
	    }

	    if (!ret.isEmpty ()) return ret;
	    
	    if (expression.getRight ().is <syntax::Var> ()) 
		this-> error (expression, left, expression.getRight ().to <syntax::Var> ().getName ().str);
	    else {
		auto right = this-> _context.validateValue (expression.getRight ());
		this-> error (expression, left, right.to <Value> ().prettyString ());
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
		type.to <Type> ().isMutable (true);
	    else
		type.to<Type> ().isMutable (false);
	    
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
		    field_type.to <Type> ().isMutable (true);
		else
		    field_type.to<Type> ().isMutable (false);
		
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
		innerType.to<Type> ().isMutable (false);
		return StructAccess::init (expression.getLocation (),
					   innerType,
					   left, name);					   
	    }

	    if (name == Range::STEP_NAME) {
		auto step_type = left.to<Value> ().getType ().to <Range> ().getStepType ();
		step_type.to <Type> ().isMutable (left.to <Value> ().getType ().to <Type> ().isMutable ());
		return StructAccess::init (expression.getLocation (),
					   step_type,
					   left, name);	
	    }

	    if (name == Range::FULL_NAME) {
		auto full_type = Bool::init (expression.getLocation ());
		full_type.to <Type> ().isMutable (false);
		return StructAccess::init (expression.getLocation (),
					   full_type,
					   left, name);	
		
	    }
	    
	    return Generator::empty ();	    
	}
	
	void DotVisitor::error (const syntax::Binary & expression, const generator::Generator & left, const std::string & right) {
	    std::string leftName;
	    match (left) {
		of (FrameProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())
		    else of (MultSym,    sym,   leftName = sym.getLocation ().str)
			else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ());
	    }

	    Ymir::Error::occur (
		expression.getLocation (),
		ExternalError::get (UNDEFINED_FIELD_FOR),
		right,
		leftName
	    );
	}

	
    }

}
