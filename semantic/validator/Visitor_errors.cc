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
	    // This function is used to throw the error about type incompatibility, it chooses how the notes will be added, and what are the names of the types that are used
	    std::string leftName = left.prettyString ();	    
	    if (!left.to <Type> ().getProxy ().isEmpty () && !left.to <Type> ().getProxy ().to <Type> ().isCompatible (right.to <Type> ().getProxy ())) { 
		leftName = left.to<Type> ().getProxy ().to <Type> ().getTypeName (); // If the type is proxied, then the problem comes from the proxy, not the proxied type
	    }

	    if (loc.getLine () == rightLoc.getLine ()) { // If the two locations are on the same line, we throw a single line error
		Ymir::Error::occurAndNote (loc, rightLoc, notes, ExternalError::INCOMPATIBLE_TYPES,
					   leftName, 
					   right.to <Type> ().getTypeName ()
		    );
	    } else {
		std::list <Error::ErrorMsg> auxNotes = {Ymir::Error::createNote (rightLoc)};
		auxNotes.insert (auxNotes.end (), notes.begin (), notes.end ()); // If they are on two different lines, we add a note
		
		Ymir::Error::occurAndNote (loc, auxNotes, ExternalError::INCOMPATIBLE_TYPES,
					   leftName,
					   right.to <Type> ().getTypeName ()
		    );
	    }
	}

	
    }

}
