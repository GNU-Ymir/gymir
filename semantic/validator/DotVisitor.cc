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
		);
	    }

	    if (!ret.isEmpty ()) return ret;
	    // std::vector <std::string> errors;
	    // TRY (
	    // TODO DotCall
	    // 	auto right = this-> _context.validateValue (expression.getRight ());
	    // ) CATCH (ErrorCode::EXTERNAL) {
	    if (expression.getRight ().is <syntax::Var> ()) 
		this-> error (expression, left, expression.getRight ().to <syntax::Var> ().getName ().str);
	    else {
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    }
	    //} FINALLY;

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
	    
	    return TupleAccess::init (expression.getLocation (), type, left, index_val);
	}

	Generator DotVisitor::validateStruct (const syntax::Binary & expression, const Generator & left) {
	    if (!expression.getRight ().is <syntax::Var> ()) return Generator::empty ();
	    auto name = expression.getRight ().to <syntax::Var> ().getName ().str;

	    auto & str = left.to <Value> ().getType ().to <StructRef> ().getRef ().to <semantic::Struct> ().getGenerator ();
	    auto field_type = str.to <generator::Struct> ().getFieldType (name);
	    if (field_type.isEmpty ()) return Generator::empty ();
	    else {
		return StructAccess::init (expression.getLocation (), field_type, left, name);
	    }	    
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
