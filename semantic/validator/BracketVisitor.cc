#include <ymir/semantic/validator/BracketVisitor.hh>
#include <ymir/global/Core.hh>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	using namespace global;
	
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

	    if (left.to <Value> ().getType ().is <ClassRef> ()) 
		return validateClass (expression, left, rights);
	    
	    BracketVisitor::error (expression, left, rights);
	    return Generator::empty ();
	}

	Generator BracketVisitor::validateArray (const syntax::MultOperator & expression, const Generator & left, const std::vector<Generator> & right) {
	    auto loc = expression.getLocation ();
	    
	    if (right.size () == 1 && right [0].to <Value> ().getType ().is <Integer> ()) {
		auto func = this-> _context.createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (ARRAY_MODULE), CoreNames::get (OUT_OF_ARRAY)});
		auto len = ufixed (left.to <Value> ().getType ().to <Array> ().getSize ());		
		auto test = this-> _context.validateValue (syntax::Binary::init (
		    {loc, Token::INF},
		    TemplateSyntaxWrapper::init (loc, len), 
		    TemplateSyntaxWrapper::init (loc,
						 Cast::init (loc, len.to <Value> ().getType (), right[0])
		    ),
		    syntax::Expression::empty ()
		));
		    
		auto call = this-> _context.validateValue (syntax::MultOperator::init (
		    {loc, Token::LPAR}, {loc, Token::RPAR},
		    func,
		    {}
		));
		
		auto conditional = Conditional::init (loc, Void::init (loc), test, call, Generator::empty ());				
		auto innerType = left.to <Value> ().getType ().to <Array> ().getInners () [0];
		
		if (
		    left.to <Value> ().isLvalue () &&
		    left.to <Value> ().getType ().to <Type> ().isMutable () &&
		    left.to <Value> ().getType ().to <Array> ().getInners () [0].to <Type> ().isMutable () 		    
		)
		    innerType = Type::init (innerType.to <Type> (), true);
		else
		    innerType = Type::init (innerType.to <Type> (), false);

		return LBlock::init (
		    loc,
		    innerType,
		    { conditional, ArrayAccess::init (expression.getLocation (), innerType, left, right [0]) }
		);		
	    }

	    BracketVisitor::error (expression, left, right);
	    return Generator::empty ();
	}	

	Generator BracketVisitor::validateSlice (const syntax::MultOperator & expression, const Generator & left, const std::vector<Generator> & right) {
	    if (right.size () == 1 && right [0].to <Value> ().getType ().is <Integer> ()) {
		auto loc = expression.getLocation ();
		
		auto func = this-> _context.createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (ARRAY_MODULE), CoreNames::get (OUT_OF_ARRAY)});
		auto len = StructAccess::init (expression.getLocation (),
					       Integer::init (expression.getLocation (), 64, false),
					       left, Slice::LEN_NAME);
		
		auto test = this-> _context.validateValue (syntax::Binary::init (
		    {loc, Token::INF},
		    TemplateSyntaxWrapper::init (loc, len), 
		    TemplateSyntaxWrapper::init (loc,
						 Cast::init (loc, len.to <Value> ().getType (), right[0])
		    ),
		    syntax::Expression::empty ()
		));
		    
		auto call = this-> _context.validateValue (syntax::MultOperator::init (
		    {loc, Token::LPAR}, {loc, Token::RPAR},
		    func,
		    {}
		));
		
		auto conditional = Conditional::init (loc, Void::init (loc), test, call, Generator::empty ());						
		auto innerType = left.to <Value> ().getType ().to <Slice> ().getInners () [0];

		if (
		    left.to <Value> ().isLvalue () &&
		    left.to <Value> ().getType ().to <Type> ().isMutable () &&
		    left.to <Value> ().getType ().to <Slice> ().getInners () [0].to <Type> ().isMutable ()
		)
		    innerType = Type::init (innerType.to <Type> (), true);
		else
		    innerType = Type::init (innerType.to <Type> (), false);

		return LBlock::init (
		    loc,
		    innerType,
		    { conditional, SliceAccess::init (expression.getLocation (), innerType, left, right [0]) }
		);
	    		
	    } else if (right.size () == 1 && right [0].to <Value> ().getType ().is <Range> ()) {
		// Call a core function is probably better 
	    }

	    BracketVisitor::error (expression, left, right);
	    return Generator::empty ();
	}

	Generator BracketVisitor::validateClass (const syntax::MultOperator & expression, const Generator & left, const std::vector<Generator> & right) {
	    auto loc = expression.getLocation ();

	    auto leftSynt = TemplateSyntaxWrapper::init (loc, left);
	    std::vector <syntax::Expression> rightSynt;
	    for (auto & it : right)
		rightSynt.push_back (TemplateSyntaxWrapper::init (it.getLocation (), it));


	    auto bin = syntax::Binary::init (
		{loc, Token::DOT},
		leftSynt,
		syntax::Var::init ({loc, CoreNames::get (INDEX_OP_OVERRIDE)}),
		syntax::Expression::empty ()
	    );
	    
	    auto call = syntax::MultOperator::init (
		{loc, Token::LPAR}, {loc, Token::RPAR},
		bin,
		rightSynt, false
	    );

	    return this-> _context.validateValue (call);
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
