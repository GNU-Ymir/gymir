#include <ymir/semantic/validator/CastVisitor.hh>
#include <ymir/global/Core.hh>

using namespace global;

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
	    auto type = this-> _context.validateType (expression.getType (), true);
	    auto value = this-> _context.validateValue (expression.getContent ());
	    match (type) {
		of (Integer, in, {
			if (value.to <Value> ().getType ().is<Integer> ()) {
			    type = Type::init (type.to <Type> (), value.to <Value> ().getType ().to <Type> ().isMutable ());
			    return generator::Cast::init (expression.getLocation (), type, value);
			} else if (value.to <Value> ().getType ().is<Char> () && !in.isSigned ()) {
			    type = Type::init (type.to <Type> (), false);
			    return generator::Cast::init (expression.getLocation (), type, value);
			}
		    }
		) else of (Float, flt ATTRIBUTE_UNUSED, {
			if (value.to <Value> ().getType ().is<Float> ()) {
			    type = Type::init (type.to <Type> (), value.to <Value> ().getType ().to <Type> ().isMutable ());
			    return generator::Cast::init (expression.getLocation (), type, value);
			}
		    }
		) else of (Char, chr ATTRIBUTE_UNUSED, {
			if (value.to <Value> ().getType ().is<Char> ()) {
			    type = Type::init (type.to <Type> (), value.to <Value> ().getType ().to <Type> ().isMutable ());
			    return generator::Cast::init (expression.getLocation (), type, value);
			}
		    }
		) else of (Pointer, ptr , {
			if (value.to <Value> ().getType ().is <Pointer> () && value.to <Value> ().getType ().to <Type> ().getInners ()[0].is <Void> ()) { // One can cast &(void) in anything
			    auto inner = Type::init (type.to <Type> ().getInners ()[0].to <Type> (), value.to <Value> ().getType ().to <Type> ().getInners ()[0].to <Type> ().isMutable ());
			    type = Type::init (Pointer::init (expression.getLocation (), inner).to <Type> (), value.to <Value> ().getType ().to <Type> ().isMutable ());
			
			    return generator::Cast::init (expression.getLocation (), type, value);
			} if (value.to <Value> ().getType ().is <Pointer> () && ptr.getInners ()[0].is <Void> ()) {// One can cast anything to &(void)
			    auto inner = Type::init (type.to <Type> ().getInners ()[0].to <Type> (), value.to <Value> ().getType ().to <Type> ().getInners ()[0].to <Type> ().isMutable ());
			    type = Type::init (Pointer::init (expression.getLocation (), inner).to <Type> (), value.to <Value> ().getType ().to <Type> ().isMutable ());
			    
			    return generator::Cast::init (expression.getLocation (), type, value);
			}
		    }
		) else of (ClassPtr, ptr ATTRIBUTE_UNUSED, {
			if (value.to <Value> ().getType ().is <ClassPtr> () && this-> _context.isAncestor (type, value.to <Value> ().getType ())) {
			    auto inner = Type::init (type.to <Type> ().getInners ()[0].to <Type> (), value.to <Value> ().getType ().to <Type> ().getInners ()[0].to <Type> ().isMutable ());
			    type = Type::init (ClassPtr::init (expression.getLocation (), inner).to <Type> (), value.to <Value> ().getType ().to <Type> ().isMutable ());
			    
			    return generator::Cast::init (expression.getLocation (), type, value);
			}
			
			if (value.to <Value> ().getType ().is <ClassPtr> ()) {
			    auto syntaxType = this-> _context.createClassTypeFromPath (expression.getLocation (), {CoreNames::get (CORE_MODULE), CoreNames::get (OBJECT_MODULE), CoreNames::get (OBJECT_TYPE)});
			    auto objectType = this-> _context.validateType (syntaxType);
			    if (objectType.to <Type> ().isCompatible (type)) {
				auto inner = Type::init (type.to <Type> ().getInners ()[0].to <Type> (), value.to <Value> ().getType ().to <Type> ().getInners ()[0].to <Type> ().isMutable ());
				type = Type::init (ClassPtr::init (expression.getLocation (), inner).to <Type> (), value.to <Value> ().getType ().to <Type> ().isMutable ());
				
				return generator::Cast::init (expression.getLocation (), type, value);
			    }
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
