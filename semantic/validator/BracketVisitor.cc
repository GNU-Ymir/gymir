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
	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector <Generator> rights;
	    
	    left = UniqValue::init (left.getLocation (), left.to<Value> ().getType (), left);	    
	    this-> _context.enterDollar (left);
	    
	    try {
		for (auto & it : expression.getRights ()) {
		    rights.push_back (this-> _context.validateValue (it));
		}
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    }
	    
	    this-> _context.quitDollar ();
	    
	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};
	    
	    if (left.to <Value> ().getType ().is <Array> ())
	    return validateArray (expression, left, rights);

	    if (left.to <Value> ().getType ().is <Slice> ()) {
		return validateSlice (expression, left, rights);
	    }
	    
	    if (left.to <Value> ().getType ().is<ClassPtr> ())
	    return validateClass (expression, left, rights);
	    
	    BracketVisitor::error (expression, left, rights);
	    return Generator::empty ();
	}


	/**
	 * ======================================================================================================
	 * ======================================================================================================
	 * ==============================================     ARRAY     =========================================
	 * ======================================================================================================
	 * ======================================================================================================
	 */

	
	Generator BracketVisitor::validateArray (const syntax::MultOperator & expression, const Generator & left, const std::vector<Generator> & right) {
	    auto loc = expression.getLocation ();
	    
	    if (right.size () == 1 && right [0].to <Value> ().getType ().is <Integer> ()) {
		return validateArrayInteger (expression, left, right [0]);
	    } else if (right.size () == 1 && right [0].to <Value> ().getType ().is <Range> () && right [0].to <Value> ().getType ().to <Range> ().getInners () [0].is <Integer> ()) {
		return validateArrayRange (expression, left, right [0]);
	    } else if (right.size () == 0) {
		return validateArrayNone (expression, left);
	    }

	    BracketVisitor::error (expression, left, right);
	    return Generator::empty ();
	}	

	Generator BracketVisitor::validateArrayInteger (const syntax::MultOperator & expression, const Generator & left, const generator::Generator & right) {
	    auto loc = expression.getLocation ();
	    auto len = ufixed (left.to <Value> ().getType ().to <Array> ().getSize ());
	    auto innerType = left.to <Value> ().getType ().to <Array> ().getInners () [0];
		
	    bool realFailure = false;
	    try { // If we can know at compile time, there is no reason to add a OUT_OF_ARRAY_EXCEPTION
		auto x = this-> _context.retreiveValue (right);
		if (x.to <Fixed> ().getUI ().u < left.to <Value> ().getType ().to <Array> ().getSize ()) {
		    return ArrayAccess::init (loc, innerType, left, right);
		} else {
		    realFailure = true;
		    Ymir::Error::occur (right.getLocation (), ExternalError::OVERFLOW_ARRAY, x.to<Fixed> ().getUI ().u, left.to <Value> ().getType ().to <Array> ().getSize ());
		}
	    } catch (Error::ErrorList list) {
		if (realFailure) throw list;
	    }
		
	    auto test = this-> _context.validateValue (
		syntax::Binary::init (
		    lexing::Word::init (loc, Token::INF),
		    TemplateSyntaxWrapper::init (loc, len), 
		    TemplateSyntaxWrapper::init (loc,
						 Cast::init (loc, len.to <Value> ().getType (), right)
			),
		    syntax::Expression::empty ()
		    )
		);
		

	    auto call = Panic::init (loc);
	    auto conditional = Conditional::init (loc, Void::init (loc), test, call, Generator::empty ());	   
		
	    if (
		left.to <Value> ().isLvalue () &&
		left.to <Value> ().getType ().to <Type> ().isMutable () &&
		left.to <Value> ().getType ().to <Array> ().getInners () [0].to <Type> ().isMutable () 
		) {
		innerType = Type::init (innerType.to <Type> (), true);
	    } else {
		innerType = Type::init (innerType.to <Type> (), false);
	    }

	    if (global::State::instance ().isDebugActive ()) {
		return LBlock::init (
		    loc,
		    innerType,
		    { conditional, ArrayAccess::init (loc, innerType, left, right) }
		    );
	    } else {
		return ArrayAccess::init (loc, innerType, left, right);
	    }
	}


	Generator BracketVisitor::validateArrayNone (const syntax::MultOperator & expression, const Generator & left) {
	    auto innerType = left.to <Value> ().getType ().to <Array> ().getInners () [0];
	    auto sliceType = Slice::init (expression.getLocation (), innerType);
		
	    if (
		left.to <Value> ().isLvalue () &&
		left.to <Value> ().getType ().to <Type> ().isMutable () &&
		left.to <Value> ().getType ().to <Array> ().getInners () [0].to <Type> ().isMutable () 		    
		) {
		sliceType = Type::init (sliceType.to <Type> (), true);
	    } else {
		sliceType = Type::init (sliceType.to <Type> (), false);
	    }

	    return Aliaser::init (
		expression.getLocation (),
		sliceType,
		left
		);		
	}

	Generator BracketVisitor::validateArrayRange (const syntax::MultOperator & expression, const Generator & left, const Generator & right) {
	    uint64_t arrayLen = left.to <Value> ().getType ().to <Array> ().getSize ();
	    auto innerType = left.to <Value> ().getType ().to <Array> ().getInners () [0];
	    
	    bool realFailure = false, checkRange = false;
	    try { // If we can know at compile time, there is no reason to add a OUT_OF_ARRAY_EXCEPTION
		auto x = this-> _context.retreiveValue (right);
		if (x.is <RangeValue> () && x.to<RangeValue> ().getLeft ().is <Fixed> ()) {
		    auto & rng = x.to <RangeValue> ();
		    if (rng.getLeft ().to <Fixed> ().getUI ().u >= rng.getRight ().to <Fixed> ().getUI ().u || rng.getRight ().to <Fixed> ().getUI ().u > arrayLen) {
			realFailure = true;
			Ymir::Error::occur (right.getLocation (), ExternalError::OVERFLOW_ARRAY, rng.prettyString (), arrayLen);
		    } else checkRange = true;
		}
	    } catch (Error::ErrorList list) {
		if (realFailure) throw list;
	    }

	    auto alias = validateArrayNone (expression, left);
	    return validateSliceRange (expression, alias, right, checkRange, left.to <Value> ().isLvalue ());	    	    
	}
	

	/**
	 * ======================================================================================================
	 * ======================================================================================================
	 * ==============================================     STRING     ========================================
	 * ======================================================================================================
	 * ======================================================================================================
	 */
	
	Generator BracketVisitor::validateStringLiteral (const syntax::MultOperator & expression, const Generator & left, const std::vector <Generator> & right) {
	    auto loc = expression.getLocation ();

	    if (right.size () == 1 && right [0].to <Value> ().getType ().is <Integer> ()) {
		return validateStringInteger (expression, left, right [0]);
	    } else if (right.size () == 1 && right [0].to <Value> ().getType ().is <Range> () && right [0].to <Value> ().getType ().to <Range> ().getInners () [0].is <Integer> ()) {		
		return validateStringRange (expression, left, right [0]);
	    }
	    
	    return Generator::empty ();
	}

	Generator BracketVisitor::validateStringInteger (const syntax::MultOperator & expression, const Generator & left, const Generator & right) {
	    auto loc = expression.getLocation ();
	    uint64_t len = left.to <StringValue> ().getLen ();
	    auto innerType = left.to <Value> ().getType ().to <Type> ().getInners () [0];	    
	    bool realFailure = false;
	    
	    try { // If we can know at compile time, there is no reason to add a OUT_OF_ARRAY_EXCEPTION
		auto x = this-> _context.retreiveValue (right);
		if (x.is <Fixed> () && x.to <Fixed> ().getUI ().u < len) {
		    uint32_t size = innerType.to <Char> ().getSize ();
		    if (size == 8) {
			char value = *((char*) (left.to<StringValue> ().getValue ().data () + (x.to <Fixed> ().getUI ().u * (size / 8))));			
			return CharValue::init (loc, innerType, (uint32_t) value);
		    } else {			
			uint32_t value = *((uint32_t*) (left.to<StringValue> ().getValue ().data () + (x.to <Fixed> ().getUI ().u * (size / 8))));			
			return CharValue::init (loc, innerType, value);
		    }
		} else {
		    realFailure = true;
		    Ymir::Error::occur (right.getLocation (), ExternalError::OVERFLOW_ARRAY, x.to<Fixed> ().getUI ().u, len);
		}
	    } catch (Error::ErrorList list) {
		if (realFailure) throw list;
	    }

	    auto alias = validateStringNone (expression, left);
	    return validateSliceInteger (expression, alias, right);	    
	}

	Generator BracketVisitor::validateStringRange (const syntax::MultOperator & expression, const Generator & left, const Generator & right) {
	    auto loc = expression.getLocation ();
	    uint64_t len = left.to <StringValue> ().getLen ();
	    auto innerType = left.to <Value> ().getType ().to <Type> ().getInners () [0];
	    
	    bool realFailure = false;
	    try { // If we can know at compile time, there is no reason to add a OUT_OF_ARRAY_EXCEPTION
		auto x = this-> _context.retreiveValue (right);
		if (x.is <RangeValue> () && x.to<RangeValue> ().getLeft ().is <Fixed> ()) {
		    auto & rng = x.to <RangeValue> ();
		    if (rng.getLeft ().to <Fixed> ().getUI ().u < rng.getRight ().to <Fixed> ().getUI ().u && rng.getRight ().to <Fixed> ().getUI ().u <= len) {
			uint32_t size = innerType.to <Char> ().getSize ();
			std::vector <char> value (left.to<StringValue> ().getValue ().begin () + (rng.getLeft ().to <Fixed> ().getUI ().u * (size / 8)),
						  left.to<StringValue> ().getValue ().begin () + (rng.getRight ().to <Fixed> ().getUI ().u * (size / 8))
			    );
			for (uint32_t i = 0 ; i < size / 8; i++) {
			    value.push_back ('\0');
			}
			    
			auto nlen = (value.size () - (size / 8)) / (size / 8);
			auto type = Array::init (loc, innerType, nlen);
			type = Type::init (type.to <Type> (), true);
			    
			auto slcType = Slice::init (loc, innerType);
			slcType = Type::init (slcType.to <Type> (), true);
			    
			return Aliaser::init (
			    loc, slcType,
			    StringValue::init (loc, type, value, nlen)
			    );
		    } else { 
			realFailure = true;
			Ymir::Error::occur (right.getLocation (), ExternalError::OVERFLOW_ARRAY, rng.prettyString (), len);
		    }
		}
	    } catch (Error::ErrorList list) {
		if (realFailure) throw list;
	    }
	       
	    auto alias = validateStringNone (expression, left);
	    return validateSliceRange (expression, alias, right, false, left.to <Value> ().isLvalue ());	    
	}

	Generator BracketVisitor::validateStringNone (const syntax::MultOperator & expression, const Generator & left) {
	    auto innerType = left.to <Value> ().getType ().to <Type> ().getInners () [0];
	    auto sliceType = Slice::init (expression.getLocation (), innerType);
	    
	    if (
		left.to <Value> ().isLvalue () &&
		left.to <Value> ().getType ().to <Type> ().isMutable () &&
		left.to <Value> ().getType ().to <Type> ().getInners () [0].to <Type> ().isMutable () 		    
		) {
		sliceType = Type::init (sliceType.to <Type> (), true);
	    } else {
		sliceType = Type::init (sliceType.to <Type> (), false);
	    }

	    return Aliaser::init (
		expression.getLocation (),
		sliceType,
		left
		);		
	}
	    		
	/**
	 * ======================================================================================================
	 * ======================================================================================================
	 * ==============================================     SLICE     =========================================
	 * ======================================================================================================
	 * ======================================================================================================
	 */
	
	Generator BracketVisitor::validateSlice (const syntax::MultOperator & expression, const Generator & left, const std::vector<Generator> & right) {
	    auto strLit = this-> _context.isStringLiteral (left);
	    if (!strLit.isEmpty ()) {
		auto ret = validateStringLiteral (expression, strLit, right);
		if (!ret.isEmpty ()) return ret;
	    } 	    
	    
	    if (right.size () == 1 && right [0].to <Value> ().getType ().is <Integer> ()) {
		return validateSliceInteger (expression, left, right [0]);
	    } else if (right.size () == 1 && right [0].to <Value> ().getType ().is <Range> () && right [0].to <Value> ().getType ().to <Range> ().getInners () [0].is <Integer> ()) {
		return validateSliceRange (expression, left, right [0]);
	    }

	    BracketVisitor::error (expression, left, right);
	    return Generator::empty ();
	}


	Generator BracketVisitor::validateSliceInteger (const syntax::MultOperator & expression, const generator::Generator & left, const generator::Generator & right) {
	    auto loc = expression.getLocation ();
	    auto innerType = left.to <Value> ().getType ().to <Slice> ().getInners () [0];
				    
	    auto leftType = left.to <Value> ().getType ();
	    leftType = Type::init (leftType.to<Type> (), leftType.to <Type> ().isMutable (), true);
	    auto lRef = UniqValue::init (loc, leftType, // Referencer::init (loc, leftType, 
					 left//)
		);
		
	    auto rightType = right.to <Value> ().getType ();
	    rightType = Type::init (rightType.to<Type> (), rightType.to <Type> ().isMutable (), true);
	    auto rRef = UniqValue::init (loc, rightType, // Referencer::init (loc, rightType,
					 right//)
		);

	    auto len = StructAccess::init (expression.getLocation (),
					   Integer::init (expression.getLocation (), 64, false),
					   lRef, Slice::LEN_NAME);
		
	    auto test = this-> _context.validateValue (
		syntax::Binary::init (
		    lexing::Word::init (loc, Token::INF_EQUAL),
		    TemplateSyntaxWrapper::init (loc, len), 
		    TemplateSyntaxWrapper::init (loc,
						 Cast::init (loc, len.to <Value> ().getType (), rRef)
			),
		    syntax::Expression::empty ()
		    ))
		;
		    

	    auto call = Panic::init (loc);
	    auto throwType = Ymir::format ("%::%::%", CoreNames::get (CORE_MODULE), CoreNames::get (ARRAY_MODULE), CoreNames::get (OUT_OF_ARRAY));
	    auto throwBl = ThrowBlock::init (loc, call, throwType);
		
	    auto conditional = Conditional::init (loc, Void::init (loc), test, throwBl, Generator::empty ());						

	    if (
		left.to <Value> ().isLvalue () &&
		left.to <Value> ().getType ().to <Type> ().isMutable () &&
		left.to <Value> ().getType ().to <Slice> ().getInners () [0].to <Type> ().isMutable ()
		)
	    innerType = Type::init (innerType.to <Type> (), true);
	    else
	    innerType = Type::init (innerType.to <Type> (), false);

	    if (global::State::instance ().isDebugActive ()) {
		return LBlock::init (
		    loc,
		    innerType,
		    { conditional, SliceAccess::init (expression.getLocation (), innerType, lRef, rRef) } 
		    );
	    } else {
		return SliceAccess::init (expression.getLocation (), innerType, lRef, rRef);
	    }
	}


	Generator BracketVisitor::validateSliceRange (const syntax::MultOperator & expression, const generator::Generator & left, const generator::Generator & right, bool checkedRange, bool forceLvalue) {
	    auto loc = expression.getLocation ();
		
	    auto leftType = left.to <Value> ().getType ();
	    leftType = Type::init (leftType.to<Type> (), leftType.to <Type> ().isMutable (), true);
	    auto lRef = UniqValue::init (loc, leftType, // Referencer::init (loc, leftType, 
					 left//)
		);
		
	    auto rightType = right.to <Value> ().getType ();
	    rightType = Type::init (rightType.to<Type> (), rightType.to <Type> ().isMutable (), true);
	    auto rRef = UniqValue::init (loc, rightType, // Referencer::init (loc, rightType, 
					 right//)
		);
		
	    auto len = StructAccess::init (expression.getLocation (),
					   Integer::init (expression.getLocation (), 64, false),
					   lRef, Slice::LEN_NAME);
		
	    auto rngInner = right.to <Value> ().getType ().to <Range> ().getInners () [0];
	    auto fst = StructAccess::init (expression.getLocation (),
					   rngInner,
					   rRef, Range::FST_NAME);

	    auto scd = StructAccess::init (expression.getLocation (),
					   rngInner,
					   rRef, Range::SCD_NAME);
	    
	    auto conditional = validateRangeCheck (expression, fst, scd, len);
	    
	    auto innerType = left.to <Value> ().getType ().to <Slice> ().getInners () [0];
	    if (
		(left.to <Value> ().isLvalue () || forceLvalue) &&
		left.to <Value> ().getType ().to <Type> ().isMutable () &&
		left.to <Value> ().getType ().to <Slice> ().getInners () [0].to <Type> ().isMutable ()
		) {
		innerType = Type::init (innerType.to <Type> (), true);
	    } else {
		innerType = Type::init (innerType.to <Type> (), false);
	    }

	    auto ptrType = Type::init (Pointer::init (loc, innerType).to <Type> (), left.to <Value> ().getType ().to <Type> ().isMutable ());
	    auto ptr = StructAccess::init (expression.getLocation (),
					   ptrType,
					   lRef, Slice::PTR_NAME);
		
		
	    auto ptrFinal = this-> _context.validateValue (
		syntax::Binary::init (
		    lexing::Word::init (loc, Token::PLUS),
		    TemplateSyntaxWrapper::init (loc, ptr),
		    syntax::Binary::init (
			lexing::Word::init (loc, Token::STAR),
			TemplateSyntaxWrapper::init (loc,
						     SizeOf::init (loc, Integer::init (loc, 0, false), innerType)),
			TemplateSyntaxWrapper::init (loc,
						     Cast::init (loc, len.to <Value> ().getType (), fst)),
			syntax::Expression::empty ()
			), syntax::Expression::empty ())
		);

		
	    auto lenFinal = this-> _context.validateValue (
		syntax::Binary::init (
		    lexing::Word::init (loc, Token::MINUS),
		    TemplateSyntaxWrapper::init (loc,
						 Cast::init (loc, len.to <Value> ().getType (), scd)),
		    TemplateSyntaxWrapper::init (loc,
						 Cast::init (loc, len.to <Value> ().getType (), fst)),
		    syntax::Expression::empty ()
		    )
		);

	    auto slcType = Slice::init (loc, innerType);
	    if ((left.to <Value> ().isLvalue () || forceLvalue) &&
		left.to <Value> ().getType ().to <Type> ().isMutable ()) {
		slcType = Type::init (slcType.to <Type> (), true);
	    } else {
		slcType = Type::init (slcType.to <Type> (), false);
	    }
;
	    auto value = SliceValue::init (loc, slcType, ptrFinal, lenFinal);
	    if (global::State::instance ().isDebugActive () && !checkedRange) {
		return Block::init (
		    loc,
		    slcType,
		    {rRef, lRef, conditional, value}
		    );
	    } else {
		return Block::init (
		    loc,
		    slcType,
		    {rRef, lRef, value}
		    );
	    }	    
	}

	Generator BracketVisitor::validateRangeCheck (const syntax::MultOperator & expression, const Generator & fst, const Generator & scd, const Generator & len) {
	    auto loc = expression.getLocation ();
	    auto testScd = syntax::Binary::init (
		lexing::Word::init (loc, Token::INF),
		TemplateSyntaxWrapper::init (loc, len), 
		TemplateSyntaxWrapper::init (loc,
					     Cast::init (loc, len.to <Value> ().getType (), scd)
		    ),
		syntax::Expression::empty ()
		);

	    auto testOrder = syntax::Binary::init (
		lexing::Word::init (loc, Token::INF),
		TemplateSyntaxWrapper::init (loc,
					     Cast::init (loc, len.to <Value> ().getType (), scd)), 
		TemplateSyntaxWrapper::init (loc,
					     Cast::init (loc, len.to <Value> ().getType (), fst)),		    
		syntax::Expression::empty ()
		);

	    auto test = this-> _context.validateValue (
		syntax::Binary::init (
		    lexing::Word::init (loc, Token::DPIPE),
		    testOrder,
		    testScd,
		    syntax::Expression::empty ()
		    )
		);

	    auto call = Panic::init (loc);
	    return Conditional::init (loc, Void::init (loc), test, call, Generator::empty ());
	}

	
	/**
	 * ======================================================================================================
	 * ======================================================================================================
	 * ==============================================     CLASS     =========================================
	 * ======================================================================================================
	 * ======================================================================================================
	 */

	
	Generator BracketVisitor::validateClass (const syntax::MultOperator & expression, const Generator & left, const std::vector<Generator> & right) {
	    auto loc = expression.getLocation ();

	    auto leftSynt = TemplateSyntaxWrapper::init (loc, left);
	    std::vector <syntax::Expression> rightSynt;
	    for (auto & it : right)
	    rightSynt.push_back (TemplateSyntaxWrapper::init (it.getLocation (), it));


	    auto bin = syntax::Binary::init (
		lexing::Word::init (loc, Token::DOT),
		leftSynt,
		syntax::Var::init (lexing::Word::init (loc, CoreNames::get (INDEX_OP_OVERRIDE))),
		syntax::Expression::empty ()
		);
	    
	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		bin,
		rightSynt, false
		);

	    return this-> _context.validateValue (call);
	}


	/**
	 * ======================================================================================================
	 * ======================================================================================================
	 * ==============================================     ERROR     =========================================
	 * ======================================================================================================
	 * ======================================================================================================
	 */

	
	
	void BracketVisitor::error (const syntax::MultOperator & expression, const Generator & left, const std::vector <Generator> & rights) {	    
	    std::vector <std::string> names;
	    for (auto & it : rights)
	    names.push_back (it.to <Value> ().getType ().to <Type> ().getTypeName ());
	    
	    Ymir::Error::occur (
		expression.getLocation (),
		expression.getEnd (),
		ExternalError::UNDEFINED_BRACKETS_OP,
		left.to <Value> ().getType ().to <Type> ().getTypeName (),
		names
		);	    
	}
		
    }
    
}
