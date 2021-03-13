#include <ymir/semantic/validator/ClassVisitor.hh>

namespace semantic {

    namespace validator {

	ClassVisitor::ClassVisitor (Visitor & context) :
	    _context (context)
	{}
	
	ClassVisitor ClassVisitor::init (Visitor & context) {
	    return ClassVisitor (context);
	}
	
	generator::Generator ClassVisitor::validate (const semantic::Class & cls, bool inModule) {
	    if (cls.to <semantic::Class> ().getGenerator ().isEmpty () || inModule) {
		
	    }
	}

	generator::Generator ClassVisitor::validateAncestor (const semantic::Class & cls) {
	    if (!cls.to <semantic::Class> ().getAncestor ().isEmpty ()) {
		auto ancestor = this-> validateValue (cls.to <semantic::Class> ().getAncestor (), true, false);
		if (ancestor.is <generator::Class> ()) ancestor = ancestor.to <generator::Class> ().getClassRef ();
		if (!ancestor.is <ClassRef> ()) {
		    Ymir::Error::occur (cls.to <semantic::Class> ().getAncestor ().getLocation (),
					ExternalError::get (INHERIT_NO_CLASS),
					ancestor.prettyString ()
			);
		} else if (ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().isFinal ()) {
		    Ymir::Error::occur (cls.to <semantic::Class> ().getAncestor ().getLocation (),
					ExternalError::get (INHERIT_FINAL_CLASS),
					ancestor.prettyString ()
			);
		}
		
		return ancestor;
	    } return Generator::empty ();
	}
	

    }    

}
