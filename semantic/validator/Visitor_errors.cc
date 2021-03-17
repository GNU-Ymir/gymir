#include <ymir/semantic/validator/_.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/utils/map.hh>
#include <ymir/global/Core.hh>
#include <ymir/utils/Path.hh>
#include <ymir/global/State.hh>
#include <string>
#include <algorithm>

using namespace global;

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;       

	void Visitor::throwIncompatibleTypes (const lexing::Word & loc, const lexing::Word & rightLoc, const Generator & left, const Generator & right, const std::list <Error::ErrorMsg> & notes) {
	    std::string leftName = left.prettyString ();	    
	    if (!left.to <Type> ().getProxy ().isEmpty () && !left.to <Type> ().getProxy ().to <Type> ().isCompatible (right.to <Type> ().getProxy ())) {
		leftName = left.to<Type> ().getProxy ().to <Type> ().getTypeName ();
	    }

	    if (loc.getLine () == rightLoc.getLine ()) {
		Ymir::Error::occurAndNote (loc, notes, ExternalError::get (INCOMPATIBLE_TYPES),
					   leftName, 
					   right.to <Type> ().getTypeName ()
		    );
	    } else {
		std::list <Error::ErrorMsg> auxNotes = {Ymir::Error::createNote (rightLoc)};
		auxNotes.insert (auxNotes.end (), notes.begin (), notes.end ());
		
		Ymir::Error::occurAndNote (loc, auxNotes, ExternalError::get (INCOMPATIBLE_TYPES),
					   leftName,
					   right.to <Type> ().getTypeName ()
		    );
	    }
	}

	
    }

}
