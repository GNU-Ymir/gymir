#include <ymir/semantic/validator/BinaryVisitor.hh>
#include <ymir/semantic/generator/value/_.hh>
#include <algorithm>

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
	    std::vector <std::string> errors;
	    TRY (

		match (left.to<Value> ().getType ()) {
		    of (Integer, integer ATTRIBUTE_UNUSED,
			return validateMathIntLeft (op, expression, left, right);
		    );

		    of (Float, f ATTRIBUTE_UNUSED,
			return validateMathFloatLeft (op, expression, left, right);
		    );
		}
		
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors = msgs;

		TRY (

		    // match (right.to <Value>.getType ()) {
			
		    // }		    

		    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
					expression.getLocation ().str,
					left.to <Value> ().getType ().to <Type> ().getTypeName (),
					right.to <Value> ().getType ().to <Type> ().getTypeName ()
		    );
		    
		) CATCH (ErrorCode::EXTERNAL) {		    
		} FINALLY;
	    } FINALLY;
	    
	    if (errors.size () != 0) {
		THROW (ErrorCode::EXTERNAL, errors);
	    } 

	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().str,
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
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
				expression.getLocation ().str,
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}

	Generator BinaryVisitor::validateMathFloatLeft (Binary::Operator op, const syntax::Binary & expression, const Generator & left, const Generator & right) {
	    std::vector <Binary::Operator> possible = {
		Binary::Operator::ADD, Binary::Operator::SUB, Binary::Operator::MUL, Binary::Operator::DIV,
	    };
	    
	    if (right.to <Value> ().getType ().is <Float> () && op != Binary::Operator::CONCAT) {
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
				expression.getLocation ().str,
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
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

	    
	    std::vector <std::string> errors;
	    TRY (

		match (left.to<Value> ().getType ()) {
		    of (Integer, integer ATTRIBUTE_UNUSED,
			return validateLogicalIntLeft (op, expression, left, right);
		    );

		    of (Bool, b ATTRIBUTE_UNUSED,
			return validateLogicalBoolLeft (op, expression, left, right);
		    );

		    of (Float, f ATTRIBUTE_UNUSED,
			return validateLogicalFloatLeft (op, expression, left, right);
		    );
		}		
		
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors = msgs;

		TRY (

		    // match (right.to <Value>.getType ()) {
		    
		    // }		    

		    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
					expression.getLocation ().str,
					left.to <Value> ().getType ().to <Type> ().getTypeName (),
					right.to <Value> ().getType ().to <Type> ().getTypeName ()
		    );
		    
		) CATCH (ErrorCode::EXTERNAL) {		    
		} FINALLY;
	    } FINALLY;
	    
	    if (errors.size () != 0) {
		THROW (ErrorCode::EXTERNAL, errors);
	    } 

	    Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				expression.getLocation ().str,
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();	    
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
				expression.getLocation ().str,
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
				expression.getLocation ().str,
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
				expression.getLocation ().str,
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}	

	Generator BinaryVisitor::validateAffectation (Binary::Operator op, const syntax::Binary & expression) {	    
	    auto left = this-> _context.validateValue (expression.getLeft ());
	    auto right = this-> _context.validateValue (expression.getRight ());

	    if (op != Binary::Operator::LAST_OP) {
		right = validateMathOperation (op, expression, left, right);
	    }
	    
	    if (!left.to <Value> ().isLvalue ()) 
		Ymir::Error::occur (left.getLocation (), ExternalError::get (NOT_A_LVALUE));

	    if (!left.to <Value> ().getType ().to <Type> ().isMutable ()) 
		Ymir::Error::occur (left.getLocation (), ExternalError::get (IMMUTABLE_LVALUE), left.to <Value> ().getType ().to <Type> ().getTypeName ());	    

	    if (left.to <Value> ().getType ().to <Type> ().isComplex ()) {
		this-> _context.verifyMemoryOwner (expression.getLocation (), left.to <Value> ().getType (), right, false);
	    }
	    
	    if (left.to<Value> ().getType ().to <Type> ().isCompatible (right.to<Value> ().getType ()))
		// TODO verification of the constancy or imutability of the left operand
		
		return Affect::init (expression.getLocation (), left.to <Value> ().getType (), left, right);
	    else
		Ymir::Error::occur (expression.getLocation (), ExternalError::get (UNDEFINED_BIN_OP),
				    expression.getLocation ().str,
				    left.to <Value> ().getType ().to <Type> ().getTypeName (),
				    right.to <Value> ().getType ().to <Type> ().getTypeName ()
		);

	    return Generator::empty ();	    
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

			auto lVar = generator::VarDecl::init ({expression.getLocation (), "#1"}, "#1", c, left, false);
			auto rVar = generator::VarDecl::init ({expression.getLocation (), "#2"}, "#2", c, right, false);
			auto lVref = VarRef::init ({expression.getLocation (), "#1"}, "#1", c, lVar.getUniqId (), false, left);
			auto rVref = VarRef::init ({expression.getLocation (), "#2"}, "#2", c, rVar.getUniqId (), false, right);

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

			       auto lVar = generator::VarDecl::init ({expression.getLocation (), "#1"}, "#1", f, left, false);
			       auto rVar = generator::VarDecl::init ({expression.getLocation (), "#2"}, "#2", f, right, false);
			       auto lVref = VarRef::init ({expression.getLocation (), "#1"}, "#1", f, lVar.getUniqId (), false, left);
			       auto rVref = VarRef::init ({expression.getLocation (), "#2"}, "#2", f, rVar.getUniqId (), false, right);

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

			       auto lVar = generator::VarDecl::init ({expression.getLocation (),"#1"}, "#1", i, left, false);
			       auto rVar = generator::VarDecl::init ({expression.getLocation (), "#2"}, "#2", i, right, false);
			       auto lVref = VarRef::init ({expression.getLocation (), "#1"}, "#1", i, lVar.getUniqId (), false, left);
			       auto rVref = VarRef::init ({expression.getLocation (), "#2"}, "#2", i, rVar.getUniqId (), false, right);

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
				expression.getLocation ().str,
				left.to <Value> ().getType ().to <Type> ().getTypeName (),
				right.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	    
	    return Generator::empty ();
	}
	
	Binary::Operator BinaryVisitor::toOperator (const lexing::Word & loc, bool & isAff) {
	    string_match (loc.str) {
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
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
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
	    return !isMath (op) && op != Binary::Operator::RANGE && op != Binary::Operator::TRANGE;
	}

	bool BinaryVisitor::isRange (Binary::Operator op) {
	    return op == Binary::Operator::RANGE || op == Binary::Operator::TRANGE;
	}
	
    }

}
