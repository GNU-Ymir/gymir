#include <ymir/semantic/validator/CastVisitor.hh>


namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	
	CastVisitor::CastVisitor (Visitor & context) :
	    _context (context)
	{}

	CastVisitor CastVisitor::init (Visitor & context) {
	    return CastVisitor (context);
	}

	Generator CastVisitor::validate (const syntax::Cast & expression) {
	    auto type = this-> _context.validateType (expression.getType ());
	    auto value = this-> _context.validateValue (expression.getContent ());
	    match (type) {
		of (Integer, in ATTRIBUTE_UNUSED, {
			if (value.to <Value> ().getType ().is<Integer> ()) {
			    type.to <Type> ().isMutable (value.to <Value> ().getType ().to <Type> ().isMutable ());
			    return generator::Cast::init (expression.getLocation (), type, value);
			}
		    }
		) else of (Float, flt ATTRIBUTE_UNUSED, {
			if (value.to <Value> ().getType ().is<Float> ()) {
			    type.to <Type> ().isMutable (value.to <Value> ().getType ().to <Type> ().isMutable ());
			    return generator::Cast::init (expression.getLocation (), type, value);
			}
		    }
		) else of (Char, chr ATTRIBUTE_UNUSED, {
			if (value.to <Value> ().getType ().is<Char> ()) {
			    type.to <Type> ().isMutable (value.to <Value> ().getType ().to <Type> ().isMutable ());
			    return generator::Cast::init (expression.getLocation (), type, value);
			}
		    }
		);
	    }

	    error (expression, type, value);
	    return Generator::empty ();
	}

	void CastVisitor::error (const syntax::Cast & expression, const Generator & type, const Generator & value) {
	    Ymir::Error::occur (
		expression.getLocation (),
		ExternalError::get (UNDEFINED_CAST_OP),
		value.to <Value> ().getType ().to <Type> ().getTypeName (),
		type.to <Type> ().getTypeName ()
	    );
	}
	
    }
}
