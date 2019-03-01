#include <ymir/semantic/validator/DotVisitor.hh>
#include <ymir/semantic/validator/CompileTime.hh>

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
	    
	    match (left.to <Value> ().getType ()) {
		of (Tuple, tu ATTRIBUTE_UNUSED,
		    return validateTuple (expression, left);
		);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
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

	
    }

}
