#include <ymir/semantic/validator/BracketVisitor.hh>


namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	
	BracketVisitor::BracketVisitor (Visitor & context) :
	    _context (context)
	{}

	BracketVisitor BracketVisitor::init (Visitor & context) {
	    return BracketVisitor (context);
	}

	Generator BracketVisitor::validate (const syntax::MultOperator & expression) {
	    auto left = this-> _context.validateValue (expression.getLeft ());			
	    std::vector <Generator> rights;
	    for (auto & it : expression.getRights ()) 
		rights.push_back (this-> _context.validateValue (it));	    

	    if (left.to <Value> ().getType ().is <Array> ())
	    	return validateArray (expression, left, rights);

	    if (left.to <Value> ().getType ().is <Slice> ())
		return validateSlice (expression, left, rights);
	    
	    BracketVisitor::error (expression, left, rights);
	    return Generator::empty ();
	}

	Generator BracketVisitor::validateArray (const syntax::MultOperator & expression, const Generator & left, const std::vector<Generator> & right) {
	    if (right.size () == 1 && right [0].to <Value> ().getType ().is <Integer> ()) {
		auto innerType = left.to <Value> ().getType ().to <Array> ().getInners () [0];
		
		if (
		    left.to <Value> ().isLvalue () &&
		    left.to <Value> ().getType ().to <Type> ().isMutable () &&
		    left.to <Value> ().getType ().to <Array> ().getInners () [0].to <Type> ().isMutable () 		    
		)
		    innerType.to <Type> ().isMutable (true);
		else
		    innerType.to <Type> ().isMutable (false);
		
		return ArrayAccess::init (expression.getLocation (), innerType, left, right [0]);
	    }

	    BracketVisitor::error (expression, left, right);
	    return Generator::empty ();
	}	

	Generator BracketVisitor::validateSlice (const syntax::MultOperator & expression, const Generator & left, const std::vector<Generator> & right) {
	    if (right.size () == 1 && right [0].to <Value> ().getType ().is <Integer> ()) {
		auto innerType = left.to <Value> ().getType ().to <Slice> ().getInners () [0];

		if (
		    left.to <Value> ().isLvalue () &&
		    left.to <Value> ().getType ().to <Type> ().isMutable () &&
		    left.to <Value> ().getType ().to <Slice> ().getInners () [0].to <Type> ().isMutable ()
		)
		    innerType.to <Type> ().isMutable (true);
		else
		    innerType.to <Type> ().isMutable (false);

		return SliceAccess::init (expression.getLocation (), innerType, left, right [0]);
	    }

	    BracketVisitor::error (expression, left, right);
	    return Generator::empty ();
	}
	
	void BracketVisitor::error (const syntax::MultOperator & expression, const Generator & left, const std::vector <Generator> & rights) {	    
	    std::vector <std::string> names;
	    for (auto & it : rights)
		names.push_back (it.to <Value> ().getType ().to <Type> ().getTypeName ());
	    
	    Ymir::Error::occur (
		expression.getLocation (),
		expression.getEnd (),
		ExternalError::get (UNDEFINED_BRACKETS_OP),
		left.to <Value> ().getType ().to <Type> ().getTypeName (),
		names
	    );	    
	}
		
    }
    
}
