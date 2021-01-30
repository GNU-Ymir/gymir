#include <ymir/semantic/validator/BinaryVisitor.hh>
#include <ymir/semantic/generator/value/_.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/global/Core.hh>
#include <algorithm>

using namespace global;

namespace semantic {

    namespace validator {

	using namespace Ymir;
	using namespace generator;
	
	
	BinaryVisitor::BinaryVisitor (Visitor & context) :
	    _context (context) 
	{}

	BinaryVisitor BinaryVisitor::init (Visitor & context) {
	    return BinaryVisitor (context);
	}

	Generator BinaryVisitor::validate (const syntax::Binary & expression) {
	    bool isAff = false;
	    auto op = toOperator (expression.getLocation (), isAff);
	    if (isMath (op) && !isAff) {
		return validateMathOperation (op, expression);
	    } else if (isLogical (op) && !isAff) {
		return validateLogicalOperation (op, expression);
	    } else if (isRange (op)) {
		return validateRangeOperation (op, expression);
	    } else if (isPointer (op)) {
		return validatePointerOperation (op, expression);
	    } else if (isContain (op)) {
		return validateContain (op, expression);
	    } else if (isAff) {
		return validateAffectation (op, expression);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator BinaryVisitor::validateMathOperation (Binary::Operator op, const syntax::Binary & expression) {
	    auto leftExp = expression.getLeft ();
	    auto rightExp = expression.getRight ();
	    if (!expression.getType ().isEmpty ()) {
		leftExp = syntax::Cast::init (expression.getLocation (), expression.getType (), leftExp);
		rightExp = syntax::Cast::init (expression.getLocation (), expression.getType (), rightExp);		
	    }
	    
	    auto left = this-> _context.validateValue (leftExp);
	    auto right = this-> _context.validateValue (rightExp);

	    return validateMathOperation (op, expression, left, right);
	}
	
	Generator BinaryVisitor::validateMathOperation (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {	
	    std::list <Ymir::Error::ErrorMsg> errors;	    
	    Generator ret (Generator::empty ());
	    try {

		match (left.to<Value> ().getType ()) {
		    of (Integer, integer ATTRIBUTE_UNUSED,
			ret = validateMathIntLeft (op, expression, left, right);
		    );

		    of (Char, chr ATTRIBUTE_UNUSED,
			ret = validateMathCharLeft (op, expression, left, right);
		    );
		    
		    of (Float, f ATTRIBUTE_UNUSED,
			ret = validateMathFloatLeft (op, expression, left, right);
		    );

		    of (Pointer, p ATTRIBUTE_UNUSED, 
			ret = validateMathPtrLeft (op, expression, left, right);
		    );

		    of (Array, a ATTRIBUTE_UNUSED,
			ret = validateMathArray (op, expression, left, right);
		    );

		    of (Slice, s ATTRIBUTE_UNUSED,
			ret = validateMathSlice (op, expression, left, right);
		    );

		    of (ClassPtr, p ATTRIBUTE_UNUSED,
			ret = validateMathClass (op, expression, left, right);		    
		    );
		}
		
	    } catch (Error::ErrorList list) {		
		errors = list.errors;
	    }  
	    	    	    
	    if (!ret.isEmpty ()) return ret;	    
	    else {
		try {
		    if (right.to <Value> ().getType ().is<ClassPtr> ())
			ret = validateMathClassRight (op, expression, left, right);		    		    
		} catch (Error::ErrorList list) {		    
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
		
		if (ret.isEmpty ()) {
		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }
		    
		    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
					expression.getLocation ().getStr (),
					left.to <Value> ().getType ().to <Type> ().getTypeName (),
					right.to <Value> ().getType ().to <Type> ().getTypeName ()
		    );
		    
		    return Generator::empty ();
		} else return ret;
	    }
	}

	Generator BinaryVisitor::validateMathIntLeft (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    if (right.to <Value> ().getType ().is <Integer> () && op != Binary::Operator::CONCAT) {
		const Integer & leftType = left.to <Value> ().getType ().to <Integer> ();
		const Integer & rightType = right.to <Value> ().getType ().to <Integer> ();
		
		if (leftType.isSigned () == rightType.isSigned ()) {
		    auto max = leftType.getSize () > rightType.getSize () ? leftType.getSize () : rightType.getSize ();
		    return BinaryInt::init (expression.getLocation (),
					    op,
					    Integer::init (expression.getLocation (), max, leftType.isSigned ()),
					    left, right
		    );
		    
		}		
	    }
	    
	    
	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().getStr (),
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}
	
	Generator BinaryVisitor::validateMathCharLeft (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    std::vector <Binary::Operator> possible = {
		Binary::Operator::ADD, Binary::Operator::SUB
	    };
	    
	    if (right.to <Value> ().getType ().is <Char> () && std::find (possible.begin (), possible.end (), op) != possible.end ()) {
		const Char & leftType = left.to <Value> ().getType ().to <Char> ();
		const Char & rightType = right.to <Value> ().getType ().to <Char> ();
		
		auto max = leftType.getSize () > rightType.getSize () ? leftType.getSize () : rightType.getSize ();
		return BinaryChar::init (expression.getLocation (),
					op,
					Char::init (expression.getLocation (), max),
					left, right
		);			    		
	    }	    
	    
	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().getStr (),
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}

	Generator BinaryVisitor::validateMathPtrLeft (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    std::vector <Binary::Operator> possible = {
		Binary::Operator::ADD, Binary::Operator::SUB
	    };
	    	    
	    if (right.to <Value> ().getType ().is <Integer> () &&
		std::find (possible.begin (), possible.end (), op) != possible.end ()) {
		const Integer & rightType = right.to <Value> ().getType ().to <Integer> ();
		
		if (!rightType.isSigned ()) {
		    return BinaryPtr::init (
			expression.getLocation (),
			op,
			left.to <Value> ().getType (),
			left, right
		    );
		    
		}		
	    }
	    	    
	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().getStr (),
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}	

	Generator BinaryVisitor::validateMathFloatLeft (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    std::vector <Binary::Operator> possible = {
		Binary::Operator::ADD, Binary::Operator::SUB, Binary::Operator::MUL, Binary::Operator::DIV,
	    };
	    
	    if (right.to <Value> ().getType ().is <Float> () &&
		std::find (possible.begin (), possible.end (), op) != possible.end ()) {
		const Float & leftType = left.to <Value> ().getType ().to <Float> ();
		const Float & rightType = right.to <Value> ().getType ().to <Float> ();
		
		auto max = leftType.getSize () > rightType.getSize () ? leftType.getSize () : rightType.getSize ();		
		return BinaryFloat::init (expression.getLocation (),
					  op,
					  Float::init (expression.getLocation (), max),
					  left, right
		);
		
	    }
	    
	    
	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().getStr (),
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}

	Generator BinaryVisitor::validateMathArray (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    if (op == Binary::Operator::CONCAT) {
		auto loc = expression.getLocation ();
		auto leftSynt = syntax::Intrinsics::init (lexing::Word::init (loc, Keys::ALIAS), TemplateSyntaxWrapper::init (loc, left));
		syntax::Expression rightSynt (syntax::Expression::empty ()); 
		if (right.to <Value> ().getType ().is <Array> ()) {
		    rightSynt = syntax::Intrinsics::init (lexing::Word::init (loc, Keys::ALIAS), TemplateSyntaxWrapper::init (loc, right));
		} else if (right.to <Value> ().getType ().is <Slice> ()) {
		    rightSynt = TemplateSyntaxWrapper::init (loc, right);
		} else return Generator::empty ();

		auto lgen = this-> _context.validateValue (leftSynt);
		auto rgen = this-> _context.validateValue (rightSynt);
		
		auto llevel = lgen.to <Value> ().getType ().to <Type> ().mutabilityLevel ();
		auto rlevel = rgen.to <Value> ().getType ().to <Type> ().mutabilityLevel ();

		this-> _context.verifySameType (lgen.to <Value> ().getType (), rgen.to <Value> ().getType ());
		
		Generator retType (Generator::empty ());
		if (llevel < rlevel) retType = lgen.to <Value> ().getType ();
		else retType = rgen.to <Value> ().getType ();
		
		return SliceConcat::init (expression.getLocation (), retType, lgen, rgen);
	    }
	    
	    return Generator::empty ();
	}
	
	Generator BinaryVisitor::validateMathSlice (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    if (op == Binary::Operator::CONCAT) {
		auto loc = expression.getLocation ();
		
		auto leftSynt = TemplateSyntaxWrapper::init (loc, left);
		syntax::Expression rightSynt (syntax::Expression::empty ()); 
		if (right.to <Value> ().getType ().is <Array> ()) {
		    rightSynt = syntax::Intrinsics::init (lexing::Word::init (loc, Keys::ALIAS), TemplateSyntaxWrapper::init (loc, right));
		} else if (right.to <Value> ().getType ().is <Slice> ()) {
		    rightSynt = TemplateSyntaxWrapper::init (loc, right);
		} else return Generator::empty ();

		auto lgen = this-> _context.validateValue (leftSynt);
		auto rgen = this-> _context.validateValue (rightSynt);
		
		auto llevel = lgen.to <Value> ().getType ().to <Type> ().mutabilityLevel ();
		auto rlevel = rgen.to <Value> ().getType ().to <Type> ().mutabilityLevel ();

		this-> _context.verifySameType (lgen.to <Value> ().getType (), rgen.to <Value> ().getType ());
		
		Generator retType (Generator::empty ());
		if (llevel < rlevel) retType = lgen.to <Value> ().getType ();
		else retType = rgen.to <Value> ().getType ();
		
		return SliceConcat::init (expression.getLocation (), retType, lgen, rgen);

	    }
	    
	    return Generator::empty ();
	}

	Generator BinaryVisitor::validateMathClass (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    lexing::Word loc = lexing::Word::init (expression.getLocation (), toString (op));
	    lexing::Word type_of = lexing::Word::init (expression.getLocation (), Keys::TYPEOF);
	    auto leftSynt = TemplateSyntaxWrapper::init (loc, left);
	    auto rightSynt = TemplateSyntaxWrapper::init (loc, right);
	    auto templ = syntax::TemplateCall::init (
		loc,
		{
		    syntax::String::init (loc, loc, loc, lexing::Word::eof ())
		},
		syntax::Binary::init (
		    lexing::Word::init (loc, Token::DOT),
		    leftSynt,		    
		    syntax::Var::init (lexing::Word::init (loc, CoreNames::get (BINARY_OP_OVERRIDE))),
		    syntax::Expression::empty ()
		)
	    );

	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		templ,
		{rightSynt}, false
		);
	    
	    try {
		return this-> _context.validateValue (call);
	    }  catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), call.prettyString ());
		list.errors.back ().addNote (note);
		throw list;
	    }
	}

	Generator BinaryVisitor::validateMathClassRight (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    lexing::Word loc = lexing::Word::init (expression.getLocation (), toString (op));
	    auto leftSynt = TemplateSyntaxWrapper::init (loc, right);
	    auto rightSynt = TemplateSyntaxWrapper::init (loc, left);
	    auto templ = syntax::TemplateCall::init (
		loc,
		{syntax::String::init (loc, loc, loc, lexing::Word::eof ())},
		syntax::Binary::init (
		    lexing::Word::init (loc, Token::DOT),
		    leftSynt,		    
		    syntax::Var::init (lexing::Word::init (loc, CoreNames::get (BINARY_OP_OVERRIDE_RIGHT))),
		    syntax::Expression::empty ()
		)
	    );

	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		templ,
		{rightSynt}, false
	    );

	    try {
		return this-> _context.validateValue (call);
	    } catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), call.prettyString ());
		list.errors.back ().addNote (note);
		throw list;
	    }
	}	
	
	Generator BinaryVisitor::validateLogicalOperation (Binary::Operator op, const syntax::Binary & expression) {
	    auto leftExp = expression.getLeft ();
	    auto rightExp = expression.getRight ();
	    if (!expression.getType ().isEmpty ()) {
		leftExp = syntax::Cast::init (expression.getLocation (), expression.getType (), leftExp);
		rightExp = syntax::Cast::init (expression.getLocation (), expression.getType (), rightExp);		
	    }
	    
	    auto left = this-> _context.validateValue (leftExp);
	    auto right = this-> _context.validateValue (rightExp);

	    Generator ret (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    try {

		match (left.to<Value> ().getType ()) {
		    of (Integer, integer ATTRIBUTE_UNUSED,
			ret = validateLogicalIntLeft (op, expression, left, right);
		    );

		    of (Bool, b ATTRIBUTE_UNUSED,
			ret = validateLogicalBoolLeft (op, expression, left, right);
		    );

		    of (Float, f ATTRIBUTE_UNUSED,
			ret = validateLogicalFloatLeft (op, expression, left, right);
		    );

		    of (Char, c ATTRIBUTE_UNUSED,
			ret = validateLogicalCharLeft (op, expression, left, right);
		    );

		    of (Array, a ATTRIBUTE_UNUSED,
			ret = validateLogicalArrayLeft (op, expression, left, right);
		    );

		    of (Slice, s ATTRIBUTE_UNUSED,
			ret = validateLogicalSliceLeft (op, expression, left, right);
		    );

		    of (ClassPtr, p ATTRIBUTE_UNUSED, 
			ret = validateLogicalClass (op, expression, left, right);
		    );
		}		
		
	    } catch (Error::ErrorList list) {		
		errors = list.errors;
	    } 
	    
	    if (!ret.isEmpty  ()) return ret;
	    else {
		try {
		    if (right.to <Value> ().getType ().is<ClassPtr> ())
			ret = validateLogicalClassRight (op, expression, left, right);
		} catch (Error::ErrorList list) {		    
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
		
		if (ret.isEmpty ()) {
		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }
		    
		    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
					expression.getLocation ().getStr (),
					left.to <Value> ().getType ().to <Type> ().getTypeName (),
					right.to <Value> ().getType ().to <Type> ().getTypeName ()
		    );
		    
		    return Generator::empty ();
		} else return ret;
	    }
	}

	Generator BinaryVisitor::validateLogicalIntLeft (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    auto possible = {Binary::Operator::SUP, Binary::Operator::INF,
			     Binary::Operator::INF_EQUAL, Binary::Operator::SUP_EQUAL,
			     Binary::Operator::EQUAL, Binary::Operator::NOT_EQUAL};
	    
	    if (right.to<Value> ().getType ().is <Integer> () &&		
		std::find (possible.begin (), possible.end (), op) != possible.end ()) {
		const Integer & leftType = left.to <Value> ().getType ().to <Integer> ();
		const Integer & rightType = right.to <Value> ().getType ().to <Integer> ();
		
		if (leftType.isSigned () == rightType.isSigned ()) {
		    return BinaryInt::init (expression.getLocation (),
					    op,
					    Bool::init (expression.getLocation ()),
					    left, right
		    );
		}
	    }

	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().getStr (),
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}	

	Generator BinaryVisitor::validateLogicalCharLeft (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    auto possible = {Binary::Operator::SUP, Binary::Operator::INF,
			     Binary::Operator::INF_EQUAL, Binary::Operator::SUP_EQUAL,
			     Binary::Operator::EQUAL, Binary::Operator::NOT_EQUAL};

	    if (right.to<Value> ().getType ().to <Type> ().isCompatible (left.to<Value> ().getType ()) &&		
		std::find (possible.begin (), possible.end (), op) != possible.end ()) {
		
		return BinaryChar::init (expression.getLocation (),
					 op,
					 Bool::init (expression.getLocation ()),
					 left, right
		);	    
	    }

	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().getStr (),
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}	
	
	Generator BinaryVisitor::validateLogicalFloatLeft (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    auto possible = {Binary::Operator::SUP, Binary::Operator::INF,
			     Binary::Operator::INF_EQUAL, Binary::Operator::SUP_EQUAL,
			     Binary::Operator::EQUAL, Binary::Operator::NOT_EQUAL};
	    
	    if (right.to<Value> ().getType ().is <Float> () &&		
		std::find (possible.begin (), possible.end (), op) != possible.end ()) {

		return BinaryFloat::init (expression.getLocation (),
					op,
					Bool::init (expression.getLocation ()),
					left, right
		);	    
	    }

	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().getStr (),
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}	
	
	Generator BinaryVisitor::validateLogicalBoolLeft (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    auto possible = {Binary::Operator::EQUAL, Binary::Operator::NOT_EQUAL,
			     Binary::Operator::AND, Binary::Operator::OR};
	    
	    if (right.to<Value> ().getType ().is <Bool> () &&		
		std::find (possible.begin (), possible.end (), op) != possible.end ()) {
		
		return BinaryBool::init (expression.getLocation (),
					op,
					Bool::init (expression.getLocation ()),
					left, right
		);		
	    }

	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().getStr (),
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}

	Generator BinaryVisitor::validateLogicalArrayLeft (Binary::Operator, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    auto loc = expression.getLocation ();
	    auto leftSynt = syntax::Intrinsics::init (lexing::Word::init (loc, Keys::ALIAS), TemplateSyntaxWrapper::init (loc, left));
	    syntax::Expression rightSynt (syntax::Expression::empty ()); 
	    if (right.to <Value> ().getType ().is <Array> ()) {
		rightSynt = syntax::Intrinsics::init (lexing::Word::init (loc, Keys::ALIAS), TemplateSyntaxWrapper::init (loc, right));
	    } else if (right.to <Value> ().getType ().is <Slice> ()) {
		rightSynt = TemplateSyntaxWrapper::init (loc, right);
	    } else return Generator::empty ();

	    auto var = this-> _context.createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (ARRAY_MODULE), CoreNames::get (LOGICAL_OP_OVERRIDE)});		
	    	    
	    auto call = this-> _context.validateValue (
		syntax::MultOperator::init (
		    lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		    var,
		    {leftSynt, rightSynt}
		    )
		);

	    auto test = syntax::Binary::init (
		expression.getLocation (),
		TemplateSyntaxWrapper::init (loc, call),
		syntax::Cast::init (loc,
				    TemplateSyntaxWrapper::init (loc, call.to<Value> ().getType ()),
				    TemplateSyntaxWrapper::init (loc, ufixed (0))
		), syntax::Expression::empty ()
	    );
	    
	    return this-> _context.validateValue (test);
	}

	Generator BinaryVisitor::validateLogicalSliceLeft (Binary::Operator, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    auto loc = expression.getLocation ();
	    auto leftSynt = TemplateSyntaxWrapper::init (loc, left);
	    syntax::Expression rightSynt (syntax::Expression::empty ()); 
	    if (right.to <Value> ().getType ().is <Array> ()) {
		rightSynt = syntax::Intrinsics::init (lexing::Word::init (loc, Keys::ALIAS), TemplateSyntaxWrapper::init (loc, right));
	    } else if (right.to <Value> ().getType ().is <Slice> ()) {
		rightSynt = TemplateSyntaxWrapper::init (loc, right);
	    } else return Generator::empty ();

	    auto var = this-> _context.createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (ARRAY_MODULE), CoreNames::get (LOGICAL_OP_OVERRIDE)});			    	    
	    auto call = this-> _context.validateValue (
		syntax::MultOperator::init (
		    lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		    var,
		    {leftSynt, rightSynt}
		    )
		);

	    auto test = syntax::Binary::init (
		expression.getLocation (),
		TemplateSyntaxWrapper::init (loc, call),
		syntax::Cast::init (loc,
				    TemplateSyntaxWrapper::init (loc, call.to<Value> ().getType ()),
				    TemplateSyntaxWrapper::init (loc, ufixed (0))
		), syntax::Expression::empty ()
	    );
	    
	    return this-> _context.validateValue (test);
	}

	Generator BinaryVisitor::validateLogicalClass (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    auto possible = {Binary::Operator::SUP, Binary::Operator::INF,
			     Binary::Operator::INF_EQUAL, Binary::Operator::SUP_EQUAL,
			     Binary::Operator::EQUAL, Binary::Operator::NOT_EQUAL};
	    
	    if (std::find (possible.begin (), possible.end (), op) == possible.end ()) return Generator::empty ();

	    std::list <Ymir::Error::ErrorMsg> errors;
	    if (op == Binary::Operator::EQUAL || op == Binary::Operator::NOT_EQUAL) {
		Generator result (Generator::empty ());
		{
		    try {
			result = validateEqualClass (op, expression, left, right);
		    } catch (Error::ErrorList list) {
			errors = list.errors;		    
		    } 
		}
			       		
		if (!result.isEmpty ()) return result;
	    }
	    
	    lexing::Word loc = lexing::Word::init (expression.getLocation (), toString (op));
	    auto leftSynt = TemplateSyntaxWrapper::init (loc, left);
	    auto rightSynt = TemplateSyntaxWrapper::init (loc, right);
	    auto templ = syntax::Binary::init (
		lexing::Word::init (loc, Token::DOT),
		leftSynt,		    
		syntax::Var::init (lexing::Word::init (loc, CoreNames::get (LOGICAL_OP_OVERRIDE))),
		syntax::Expression::empty ()
	    );	    

	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		templ,
		{rightSynt}, false
		);

	    Generator cl (Generator::empty ());
	    try {
		cl = this-> _context.validateValue (call);
	    } catch (Error::ErrorList list) {		
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), call.prettyString ());
		list.errors.back ().addNote (note);
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());		    
	    }
	    
	    if (cl.isEmpty ())
		throw Error::ErrorList {errors};
	    
	    auto final_test = syntax::Binary::init (
		loc,
		call,
		syntax::Cast::init (loc,
				    TemplateSyntaxWrapper::init (loc, cl.to <Value> ().getType ()),
				    TemplateSyntaxWrapper::init (loc, ifixed (0))
		),
		syntax::Expression::empty ()
	    );		    

	    return this-> _context.validateValue (final_test);
	}

	Generator BinaryVisitor::validateLogicalClassRight (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    auto possible = {Binary::Operator::SUP, Binary::Operator::INF,
			     Binary::Operator::INF_EQUAL, Binary::Operator::SUP_EQUAL,
			     Binary::Operator::EQUAL, Binary::Operator::NOT_EQUAL};
	    
	    if (std::find (possible.begin (), possible.end (), op) == possible.end ()) return Generator::empty ();

	    std::list <Ymir::Error::ErrorMsg> errors;	    
	    if (op == Binary::Operator::EQUAL || op == Binary::Operator::NOT_EQUAL) {
		Generator result (Generator::empty ());
		{
		    try {
			result = validateEqualClassRight (op, expression, left, right);
		    } catch (Error::ErrorList list) {			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());		    
		    } 
		
		}
		
		if (!result.isEmpty ()) return result;
	    }
	    
	    lexing::Word loc = lexing::Word::init (expression.getLocation (), toString (op));
	    auto leftSynt = TemplateSyntaxWrapper::init (loc, right);
	    auto rightSynt = TemplateSyntaxWrapper::init (loc, left);
	    auto templ = syntax::Binary::init (
		lexing::Word::init (loc, Token::DOT),
		leftSynt,		    
		syntax::Var::init (lexing::Word::init (loc, CoreNames::get (LOGICAL_OP_OVERRIDE))),
		syntax::Expression::empty ()
	    );	    

	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		templ,
		{rightSynt}, false
	    );

	    Generator cl (Generator::empty ());
	    
	    try {
		cl = this-> _context.validateValue (call);
	    } catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), call.prettyString ());
		list.errors.back ().addNote (note);
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());		    
	    }

	    if (cl.isEmpty ())
		throw Error::ErrorList {errors};
	    
	    auto final_test = syntax::Binary::init (
		lexing::Word::init (loc, inverseOperator (op)),
		call,
		syntax::Cast::init (loc,
				    TemplateSyntaxWrapper::init (loc, cl.to <Value> ().getType ()),
				    TemplateSyntaxWrapper::init (loc, ifixed (0))
		),
		syntax::Expression::empty ()
	    );		    

	    return this-> _context.validateValue (final_test);
	}

	Generator BinaryVisitor::validateEqualClass (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    lexing::Word loc = lexing::Word::init (expression.getLocation (), toString (op));
	    auto leftSynt = TemplateSyntaxWrapper::init (loc, left);
	    auto rightSynt = TemplateSyntaxWrapper::init (loc, right);
	    auto templ = syntax::Binary::init (
		lexing::Word::init (loc, Token::DOT),
		leftSynt,		    
		syntax::Var::init (lexing::Word::init (loc, CoreNames::get (EQUALS_OP_OVERRIDE))),
		syntax::Expression::empty ()
		);	    

	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		templ,
		{rightSynt}, false
	    );

	    if (op == Binary::Operator::NOT_EQUAL) {
		call = syntax::Unary::init (lexing::Word::init (loc, Token::NOT), call);
	    }

	    try {
		return this-> _context.validateValue (call);
	    } catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), call.prettyString ());
		list.errors.back ().addNote (note);
		throw list;
	    }
	}

	Generator BinaryVisitor::validateEqualClassRight (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    lexing::Word loc = lexing::Word::init (expression.getLocation (), toString (op));
	    auto leftSynt = TemplateSyntaxWrapper::init (loc, left);
	    auto rightSynt = TemplateSyntaxWrapper::init (loc, right);
	    auto templ = syntax::Binary::init (
		lexing::Word::init (loc, Token::DOT),
		rightSynt,		    
		syntax::Var::init (lexing::Word::init (loc, CoreNames::get (EQUALS_OP_OVERRIDE))),
		syntax::Expression::empty ()
	    );	    

	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		templ,
		{leftSynt}, false
	    );

	    if (op == Binary::Operator::NOT_EQUAL) {
		call = syntax::Unary::init (lexing::Word::init (loc, Token::NOT), call);
	    }


	    try {
		return this-> _context.validateValue (call);
	    } catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), call.prettyString ());
		list.errors.back ().addNote (note);
		throw list;
	    }
	}	
	
	Generator BinaryVisitor::validateAffectation (Binary::Operator op, const syntax::Binary & expression) {
	    if (expression.getLeft ().is <syntax::MultOperator> () &&
		expression.getLeft ().to <syntax::MultOperator> ().getEnd () == Token::RCRO) {
		//try {
		auto gen = validateIndexAssign (op, expression);
		if (!gen.isEmpty ()) return gen;
		// } catch (Error::ErrorList list) {}
	    } else if (expression.getLeft ().is <syntax::Unary> () &&
		       expression.getLeft ().to <syntax::Unary> ().getLocation () == Token::STAR) {
		auto gen = validateUnaryAssign (op, expression);
		if (!gen.isEmpty ()) return gen;
	    }
	    
	    auto left = this-> _context.validateValue (expression.getLeft ());
	    auto right = this-> _context.validateValue (expression.getRight ());

	    if (op != Binary::Operator::LAST_OP) {
		right = validateMathOperation (op, expression, left, right);
	    }

	    if (!left.to <Value> ().isLvalue () || (left.is <VarRef> () && left.to <VarRef> ().isSelf ())) // We cannot change the reference of the self paramvar even if it is mutable
		Ymir::Error::occur (left.getLocation (), ExternalError::get (NOT_A_LVALUE));

	    if (!left.to <Value> ().getType ().to <Type> ().isMutable ()) 
		Ymir::Error::occur (left.getLocation (), ExternalError::get (IMMUTABLE_LVALUE), left.to <Value> ().getType ().to <Type> ().getTypeName ());	    

	    this-> _context.verifyMemoryOwner (expression.getLocation (), left.to <Value> ().getType (), right, false);	    
	    return Affect::init (expression.getLocation (), left.to <Value> ().getType (), left, right);	    
	}

	Generator BinaryVisitor::validateIndexAssign (Binary::Operator op, const syntax::Binary & expression) {
	    auto loc = expression.getLocation ();
	    auto left = expression.getLeft ().to <syntax::MultOperator> ();

	    auto leftIndex = this-> _context.validateValue (left.getLeft ()); // a in a[i] = b
	    auto right = this-> _context.validateValue (expression.getRight ()); // b in a [i] = b
	    
	    if (!leftIndex.to <Value> ().getType ().is <ClassPtr> ())
		return Generator::empty ();
		
	    if (op != Binary::Operator::LAST_OP) {
		auto leftTotal = this-> _context.validateValue (expression.getLeft ()); // a [i] in a [i] += b
		right = validateMathOperation (op, expression, leftTotal, right); // a [i] + b in a [i] += b
	    }
	    
	    auto leftSynt = TemplateSyntaxWrapper::init (leftIndex.getLocation (), leftIndex);	    
	    auto rightSynts = left.getRights ();
	    rightSynts.push_back (TemplateSyntaxWrapper::init (right.getLocation (), right));
				  
	    auto bin = syntax::Binary::init (
		lexing::Word::init (loc, Token::DOT),
		leftSynt,
		syntax::Var::init (lexing::Word::init (loc, CoreNames::get (INDEX_ASSIGN_OP_OVERRIDE))),
		syntax::Expression::empty ()
	    );

	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		bin,
		rightSynts, false
	    );

	    try {
		return this-> _context.validateValue (call);
	    } catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), call.prettyString ());
		list.errors.back ().addNote (note);
		throw list;
	    }
	}
	
	Generator BinaryVisitor::validateUnaryAssign (Binary::Operator op, const syntax::Binary & expression) {
	    auto loc = expression.getLocation ();
	    auto left = expression.getLeft ().to <syntax::Unary> ();
	    
	    auto leftIndex = this-> _context.validateValue (left.getContent ()); // a in *(a) = b
	    auto right = this-> _context.validateValue (expression.getRight ()); // b in *(a) = b
	    if (!leftIndex.to <Value> ().getType ().is <ClassPtr> ()) {
		return Generator::empty ();
	    }

	    if (op != Binary::Operator::LAST_OP) {
		auto leftTotal = this-> _context.validateValue (expression.getLeft ()); // *a in *a += b
		right = validateMathOperation (op, expression, leftTotal, right); // *a + b in *a += b
	    }

	    auto leftSynt = TemplateSyntaxWrapper::init (leftIndex.getLocation (), leftIndex);
	    std::vector <syntax::Expression> rightSynts = {TemplateSyntaxWrapper::init (right.getLocation (), right)};

	    auto bin = syntax::Binary::init (
		lexing::Word::init (loc, Token::DOT),
		leftSynt,
		syntax::Var::init (lexing::Word::init (loc, CoreNames::get (UNARY_ASSIGN_OP_OVERRIDE))),
		syntax::Expression::empty ()
		);

	    auto templ = syntax::TemplateCall::init (
		loc,
		{syntax::String::init (left.getLocation (), left.getLocation (), left.getLocation (), lexing::Word::eof ())},
		bin
		);

	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		templ,
		rightSynts, false
		);

	    try {
		return this-> _context.validateValue (call);
	    } catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), call.prettyString ());
		list.errors.back ().addNote (note);
		throw list;
	    }
	}
	
	Generator BinaryVisitor::validateRangeOperation (Binary::Operator op, const syntax::Binary & expression) {
	    auto leftExp = expression.getLeft ();
	    auto rightExp = expression.getRight ();
	    if (!expression.getType ().isEmpty ()) {
		leftExp = syntax::Cast::init (expression.getLocation (), expression.getType (), leftExp);
		rightExp = syntax::Cast::init (expression.getLocation (), expression.getType (), rightExp);		
	    }
	    
	    auto left = this-> _context.validateValue (leftExp);
	    auto right = this-> _context.validateValue (rightExp);

	    if (left.to <Value> ().getType ().to <Type> ().isCompatible (right.to <Value> ().getType ())) {
		match (left.to <Value> ().getType ()) {
		    of (Char, c_,
			auto c = left.to <Value> ().getType ();
			auto rangeType = Range::init (expression.getLocation (), c);
			auto lName = lexing::Word::init (expression.getLocation (), "#1");
			auto rName = lexing::Word::init (expression.getLocation (), "#2");

			auto lVar = generator::VarDecl::init (lName, "#1", c, left, false);
			auto rVar = generator::VarDecl::init (rName, "#2", c, right, false);
			
			auto lVref = VarRef::init (lName, "#1", c, lVar.getUniqId (), false, left);
			auto rVref = VarRef::init (rName, "#2", c, rVar.getUniqId (), false, right);

			Fixed::UI hui;
			Fixed::UI lui;
			hui.i = 1;
			lui.i = -1;
			auto itype = Integer::init (expression.getLocation (), c_.getSize (), true);
			auto hVal = Fixed::init (expression.getLocation (), itype, hui);
			auto lVal = Fixed::init (expression.getLocation (), itype, lui);

			auto bin = BinaryInt::init (expression.getLocation (),
						    Binary::Operator::INF,
						    Bool::init (expression.getLocation ()),
						    lVref, rVref);
			
			auto step = Conditional::init (expression.getLocation (), itype, bin, hVal, lVal); 

			auto isFull = BoolValue::init (expression.getLocation (), Bool::init (expression.getLocation ()), op == Binary::Operator::TRANGE);
			auto rangeValue = RangeValue::init (expression.getLocation (), rangeType, lVref, rVref, step, isFull);

			std::vector <Generator> actions;
			actions.push_back (lVar);
			actions.push_back (rVar);
			actions.push_back (rangeValue);
			
			return Block::init (expression.getLocation (), rangeType, actions);
		    ) else of (Float, f_ ATTRIBUTE_UNUSED,
			       auto f = left.to <Value> ().getType ();
			       auto rangeType = Range::init (expression.getLocation (), f);
			       auto lName = lexing::Word::init (expression.getLocation (), "#1");
			       auto rName = lexing::Word::init (expression.getLocation (), "#2");

			       auto lVar = generator::VarDecl::init (lName, "#1", f, left, false);
			       auto rVar = generator::VarDecl::init (rName, "#2", f, right, false);
			
			       auto lVref = VarRef::init (lName, "#1", f, lVar.getUniqId (), false, left);
			       auto rVref = VarRef::init (rName, "#2", f, rVar.getUniqId (), false, right);

			       auto hVal = FloatValue::init (expression.getLocation (), f, 1.0f);
			       auto lVal = FloatValue::init (expression.getLocation (), f, -1.0f);

			       auto bin = BinaryFloat::init (expression.getLocation (),
							     Binary::Operator::INF,
							     Bool::init (expression.getLocation ()),
							     lVref, rVref);
			       
			       auto step = Conditional::init (expression.getLocation (), f, bin, hVal, lVal); 

			       auto isFull = BoolValue::init (expression.getLocation (), Bool::init (expression.getLocation ()), op == Binary::Operator::TRANGE);
			       auto rangeValue = RangeValue::init (expression.getLocation (), rangeType, lVref, rVref, step, isFull);

			       std::vector <Generator> actions;
			       actions.push_back (lVar);
			       actions.push_back (rVar);
			       actions.push_back (rangeValue);
			       return Block::init (expression.getLocation (), rangeType, actions);
		    ) else of (Integer, i_,
			       auto i = left.to <Value> ().getType ();
			       auto rangeType = Range::init (expression.getLocation (), i);
			       auto lName = lexing::Word::init (expression.getLocation (), "#1");
			       auto rName = lexing::Word::init (expression.getLocation (), "#2");

			       auto lVar = generator::VarDecl::init (lName, "#1", i, left, false);
			       auto rVar = generator::VarDecl::init (rName, "#2", i, right, false);
			
			       auto lVref = VarRef::init (lName, "#1", i, lVar.getUniqId (), false, left);
			       auto rVref = VarRef::init (rName, "#2", i, rVar.getUniqId (), false, right);


			       Fixed::UI hui;
			       Fixed::UI lui;
			       hui.i = 1; lui.i = -1;
			       auto itype = Integer::init (expression.getLocation (), i_.getSize (), true);
			       auto hVal = Fixed::init (expression.getLocation (), itype, hui);
			       auto lVal = Fixed::init (expression.getLocation (), itype, lui);

			       auto bin = BinaryInt::init (expression.getLocation (),
							   Binary::Operator::INF,
							   Bool::init (expression.getLocation ()),
							   lVref, rVref);
			       
			       auto step = Conditional::init (expression.getLocation (), itype, bin, hVal, lVal); 

			       auto isFull = BoolValue::init (expression.getLocation (), Bool::init (expression.getLocation ()), op == Binary::Operator::TRANGE);
			       auto rangeValue = RangeValue::init (expression.getLocation (), rangeType, lVref, rVref, step, isFull);

			       std::vector <Generator> actions;
			       actions.push_back (lVar);
			       actions.push_back (rVar);
			       actions.push_back (rangeValue);
			       return Block::init (expression.getLocation (), rangeType, actions);
		    );
		}
	    }

	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().getStr (),
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}

	Generator BinaryVisitor::validatePointerOperation (Binary::Operator op, const syntax::Binary & expression) {
	    auto leftExp = expression.getLeft ();
	    auto rightExp = expression.getRight ();
	    if (!expression.getType ().isEmpty ()) {
		leftExp = syntax::Cast::init (expression.getLocation (), expression.getType (), leftExp);
		rightExp = syntax::Cast::init (expression.getLocation (), expression.getType (), rightExp);		
	    }
	    
	    auto left = this-> _context.validateValue (leftExp);
	    auto right = this-> _context.validateValue (rightExp);

	    if (left.to <Value> ().getType ().is <Pointer> ()) {		
		auto lptr = left.to <Value> ().getType ();
		auto rptr = right.to <Value> ().getType ();
		if (lptr.to <Type> ().isCompatible (rptr)) 
		    return BinaryPtr::init (expression.getLocation (),
					    op,
					    Bool::init (expression.getLocation ()),
					    left, right);
		else if (right.is<NullValue> ()) 
		    return BinaryPtr::init (expression.getLocation (),
					    op,
					    Bool::init (expression.getLocation ()),
					    left, right);
	    }

	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().getStr (),
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
		);
		    
	    return Generator::empty ();
	}

	Generator BinaryVisitor::validateContain (Binary::Operator op, const syntax::Binary & expression) {
	    auto loc = expression.getLocation ();
	    auto leftExp = expression.getLeft ();
	    if (!expression.getType ().isEmpty ()) {
		leftExp = syntax::Cast::init (expression.getLocation (), expression.getType (), leftExp);
	    }

	    auto right = this-> _context.validateValue (expression.getRight ());
	    if (!right.to <Value> ().getType ().is<ClassPtr> ())
		return Generator::empty ();

	    auto templ = syntax::Binary::init (
		lexing::Word::init (loc, Token::DOT),
		TemplateSyntaxWrapper::init (loc, right),		    
		syntax::Var::init (lexing::Word::init (loc, CoreNames::get (CONTAIN_OP_OVERRIDE))),
		syntax::Expression::empty ()	    
	    );	    

	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		templ,
		{leftExp}, false
	    );

	    if (op == Binary::Operator::NOT_IN) {
		call = syntax::Unary::init (lexing::Word::init (expression.getLocation (), Token::NOT), call);
	    }

	    try {
		return this-> _context.validateValue (call);
	    } catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), call.prettyString ());
		list.errors.back ().addNote (note);
		throw list;
	    }
	}
	
	Binary::Operator BinaryVisitor::toOperator (const lexing::Word & loc, bool & isAff) {
	    isAff = false;
	    string_match (loc.getStr ()) {
		eq (Token::EQUAL, { isAff = true; return Binary::Operator::LAST_OP; });
		eq (Token::DIV_AFF, { isAff = true; return Binary::Operator::DIV; });
		eq (Token::MINUS_AFF, { isAff = true; return Binary::Operator::SUB; });
		eq (Token::PLUS_AFF, { isAff = true; return Binary::Operator::ADD; });
		eq (Token::STAR_AFF, { isAff = true; return Binary::Operator::MUL; });
		eq (Token::TILDE_AFF, { isAff = true; return Binary::Operator::CONCAT; });		
		eq (Token::LEFTD_AFF, { isAff = true; return Binary::Operator::LEFT_SHIFT; });
		eq (Token::RIGHTD_AFF, { isAff = true; return Binary::Operator::RIGHT_SHIFT; });
		
		eq (Token::DPIPE, return Binary::Operator::OR;);
		eq (Token::DAND, return Binary::Operator::AND;);
		eq (Token::INF, return Binary::Operator::INF;);
		eq (Token::SUP, return Binary::Operator::SUP;);
		eq (Token::INF_EQUAL, return Binary::Operator::INF_EQUAL;);
		eq (Token::SUP_EQUAL, return Binary::Operator::SUP_EQUAL;);
		eq (Token::DEQUAL, return Binary::Operator::EQUAL;);
		eq (Token::NOT_EQUAL, return Binary::Operator::NOT_EQUAL;);
		eq (Token::LEFTD, return Binary::Operator::LEFT_SHIFT;);
		eq (Token::RIGHTD, return Binary::Operator::RIGHT_SHIFT;);
		eq (Token::PIPE, return Binary::Operator::BIT_OR;);
		eq (Token::AND, return Binary::Operator::BIT_AND;);
		eq (Token::XOR, return Binary::Operator::BIT_XOR;);
		eq (Token::TILDE, return Binary::Operator::CONCAT;);
		eq (Token::PLUS, return Binary::Operator::ADD;);
		eq (Token::MINUS, return Binary::Operator::SUB;);
		eq (Token::STAR, return Binary::Operator::MUL;);
		eq (Token::DIV, return Binary::Operator::DIV;);
		eq (Token::PERCENT, return Binary::Operator::MODULO;);
		eq (Token::DXOR, return Binary::Operator::EXP;);
		eq (Token::DDOT, return Binary::Operator::RANGE;);
		eq (Token::TDOT, return Binary::Operator::TRANGE;);
		eq (Keys::IS, return Binary::Operator::IS;);
		eq (Keys::NOT_IS, return Binary::Operator::NOT_IS;);
		eq (Keys::IN, return Binary::Operator::IN);
		eq (Keys::NOT_IN, return Binary::Operator::NOT_IN);		
	    }

	    return Binary::Operator::LAST_OP;
	}

	bool BinaryVisitor::isMath (Binary::Operator op) {
	    auto maths = {
		Binary::Operator::ADD, Binary::Operator::SUB,
		Binary::Operator::MUL, Binary::Operator::DIV,
		Binary::Operator::MODULO, Binary::Operator::EXP,
		Binary::Operator::CONCAT, Binary::Operator::BIT_OR,
		Binary::Operator::BIT_AND, Binary::Operator::BIT_XOR,
		Binary::Operator::LEFT_SHIFT, Binary::Operator::RIGHT_SHIFT
	    };
	    
	    return (std::find (maths.begin (), maths.end (), op) != maths.end ());
	}
	
	bool BinaryVisitor::isLogical (Binary::Operator op) {
	    auto impossible = {
		Binary::Operator::TRANGE, Binary::Operator::RANGE,
		Binary::Operator::IS, Binary::Operator::NOT_IS,
		Binary::Operator::IN, Binary::Operator::NOT_IN
	    };
	    return !isMath (op) &&
		(std::find (impossible.begin (), impossible.end (), op) == impossible.end ());
	}

	bool BinaryVisitor::isRange (Binary::Operator op) {
	    return op == Binary::Operator::RANGE || op == Binary::Operator::TRANGE;
	}

	bool BinaryVisitor::isPointer (Binary::Operator op) {
	    return op == Binary::Operator::IS || op == Binary::Operator::NOT_IS;
	}

	bool BinaryVisitor::isContain (Binary::Operator op) {
	    return op == Binary::Operator::IN || op == Binary::Operator::NOT_IN;
	}

	std::string BinaryVisitor::toString (Binary::Operator op) {
	    switch (op) {
	    case Binary::Operator::OR : return Token::DPIPE;
	    case Binary::Operator::AND : return Token::DAND;		
	    case Binary::Operator::INF : return Token::INF;
	    case Binary::Operator::SUP : return Token::SUP;
	    case Binary::Operator::INF_EQUAL : return Token::INF_EQUAL;
	    case Binary::Operator::SUP_EQUAL : return Token::SUP_EQUAL;
	    case Binary::Operator::NOT_EQUAL : return Token::NOT_EQUAL;
	    case Binary::Operator::EQUAL : return Token::DEQUAL;
	    case Binary::Operator::LEFT_SHIFT : return Token::LEFTD;
	    case Binary::Operator::RIGHT_SHIFT : return Token::RIGHTD;
	    case Binary::Operator::BIT_OR : return Token::PIPE;
	    case Binary::Operator::BIT_AND : return Token::AND; 
	    case Binary::Operator::BIT_XOR : return Token::XOR;
	    case Binary::Operator::ADD : return Token::PLUS;
	    case Binary::Operator::CONCAT : return Token::TILDE;
	    case Binary::Operator::SUB : return Token::MINUS;
	    case Binary::Operator::MUL : return Token::STAR;
	    case Binary::Operator::DIV : return Token::DIV;
	    case Binary::Operator::MODULO : return Token::PERCENT;
	    case Binary::Operator::EXP : return Token::DXOR;
	    case Binary::Operator::RANGE : return Token::DDOT;
	    case Binary::Operator::TRANGE : return Token::TDOT;
	    case Binary::Operator::IS : return Keys::IS;
	    case Binary::Operator::NOT_IS : return Keys::NOT_IS;
	    case Binary::Operator::IN : return Keys::IN;
	    case Binary::Operator::NOT_IN : return Keys::NOT_IN;
	    default :
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    }
	    return "";
	}
	

	std::string BinaryVisitor::inverseOperator (Binary::Operator op) {
	    switch (op) {
	    case Binary::Operator::INF : return Token::SUP;
	    case Binary::Operator::INF_EQUAL : return Token::SUP_EQUAL;
	    case Binary::Operator::SUP : return Token::INF;
	    case Binary::Operator::SUP_EQUAL : return Token::INF_EQUAL;
	    case Binary::Operator::EQUAL : return Token::DEQUAL;
	    case Binary::Operator::NOT_EQUAL : return Token::NOT_EQUAL;
	    default :
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    }
	    
	    return "";
	}
	
    }

}
