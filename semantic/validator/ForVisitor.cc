#include <ymir/semantic/validator/ForVisitor.hh>
#include <ymir/semantic/generator/_.hh>
#include <ymir/syntax/visitor/Keys.hh>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;


	ForVisitor::ForVisitor (Visitor & context) :
	    _context (context)
	{}

	ForVisitor ForVisitor::init (Visitor & context) {
	    return ForVisitor (context);
	}

	Generator ForVisitor::validate (const syntax::For & expression) {
	    auto value = this-> _context.validateValue (expression.getIter ());

	    match (value.to <Value> ().getType ()) {
		of (Array, a,
		    auto type = Slice::init (expression.getLocation (), a.getInners ()[0]);
		    type.to<Type> ().isMutable (a.isMutable ());
		    
		    return validateSlice (expression, Aliaser::init (expression.getLocation (), type, value));
		);

		of (Slice, s ATTRIBUTE_UNUSED,
		    return validateSlice (expression, value);
		);

		of (Range, r ATTRIBUTE_UNUSED,
		    return validateRange (expression, value);
		);
	    }
	    
	    error (expression, value);
	    return Generator::empty ();
	}


	Generator ForVisitor::validateArrayByValueIterator (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & val, const generator::Generator & value_, int level) {
	    auto var = val.to<syntax::VarDecl> ();
	    
	    if (var.getName () != Keys::UNDER)
		this-> _context.verifyShadow (var.getName ());
	    
	    Generator type (Generator::empty ());
	    auto innerType = array.to <Value> ().getType ().to <Type> ().getInners () [0];
	    if (!var.getType ().isEmpty ()) {
		type = this-> _context.validateType (var.getType ());
		this-> _context.verifyCompatibleType (type, innerType);
	    } else type = innerType;

	    type.to<Type> ().isMutable (false);
	    bool isMutable = false, isRef = false;
	    this-> _context.applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable);
	    this-> _context.verifyMutabilityRefParam (var.getLocation (), type, MUTABLE_CONST_ITER);
	    
	    auto value = Generator::empty ();
	    auto loc = var.getLocation ();
	    if (!isRef)
		value = value_;
	    else {
		if (level < 2)
		    Ymir::Error::occur (expression.getIter ().getLocation (),
					ExternalError::get (DISCARD_CONST_LEVEL),
					2, level
		    );
		
		value = Referencer::init (loc, type, value_);
	    }
	    
	    this-> _context.verifyMemoryOwner (loc, type, value, true);	    
	    
	    auto ret = generator::VarDecl::init (loc, var.getName ().str, type, value, isMutable);
	    
	    if (var.getName () != Keys::UNDER)
		this-> _context.insertLocal (var.getName ().str, ret);
	    return ret;
	}
	
	std::vector <Generator> ForVisitor::createIndexVar (const syntax::For &, const Generator & value, const syntax::VarDecl & decl) {
	    if (decl.getName () != Keys::UNDER)
		this-> _context.verifyShadow (decl.getName ());
	    
	    if (decl.getDecorators ().size () != 0) {
		auto deco = decl.getDecorators ()[0];
		Ymir::Error::occur (deco.getLocation (),
				    ExternalError::get (DECO_OUT_OF_CONTEXT),
				    deco.getLocation ().str
		);
	    }
	    
	    auto loc = decl.getLocation ();
	    auto zero = ufixed (0);	    
	    auto type = zero.to<Value> ().getType ();
	    auto rVar = generator::VarDecl::init (loc, "__",
						  value.to <Value> ().getType (),
						  value,
						  false);

	    auto rRef = VarRef::init (loc, "__", value.to <Value> ().getType (),
				      rVar.getUniqId (),
				      false, Generator::empty ());
	    
	    if (!decl.getType ().isEmpty ()) {
		type = this-> _context.validateType (decl.getType ());
		this-> _context.verifyCompatibleType (type, Integer::init (loc, -1, false));
	    } 

	    auto var = generator::VarDecl::init (loc,
						 decl.getName ().str,
						 type,
						 zero,
						 false
	    );
	    
	    auto ref = VarRef::init (loc,
				     decl.getName ().str,
				     type, 
				     var.getUniqId (),
				     false,
				     Generator::empty ()
	    );

	    if (decl.getName () != Keys::UNDER)
		this-> _context.insertLocal (decl.getName ().str, var);
	    
	    return {rVar, var, rRef, ref};
	}       

	std::vector <Generator> ForVisitor::createIndexVar (const syntax::For & expression, const Generator & value, const std::string & name) {	    
	    auto zero = ufixed (0);
	    auto loc = expression.getLocation ();
	    auto rVar = generator::VarDecl::init (loc, "__",
						  value.to <Value> ().getType (),
						  value,
						  false);

	    auto rRef = VarRef::init (loc, "__", value.to <Value> ().getType (),
				      rVar.getUniqId (),
				      false, Generator::empty ());
	    
	    auto var = generator::VarDecl::init (loc,
						 name,
						 zero.to <Value> ().getType (),
						 zero,
						 false
	    );

	    auto ref = VarRef::init (loc,
				     name,
				     zero.to<Value> ().getType (),
				     var.getUniqId (),
				     false,
				     Generator::empty ()
	    );
	    
	    return {rVar, var, rRef, ref};
	}                    
	
	Generator ForVisitor::validateSlice (const syntax::For & expression, const generator::Generator & value) {
	    auto vars = expression.getVars ();
	    if (vars.size () > 2) {
		Ymir::Error::occur (
		    value.getLocation (),
		    ExternalError::get (NOT_ITERABLE_WITH),
		    value.to <Value> ().getType ().to <Type> ().getTypeName (),
		    vars.size ()
		);
		return Generator::empty ();
	    } else if (vars.size () == 1) {
		return iterateSlice (expression, value, syntax::Expression::empty (), vars [0]);
	    } else { 
		return iterateSlice (expression, value, vars [0], vars [1]);
	    }
	}

	Generator ForVisitor::iterateSlice (const syntax::For & expression, const Generator & value, const syntax::Expression & index, const syntax::Expression & val) {	
	    std::vector <std::string> errors;
	    std::vector<Generator> values = {};
	    {
		TRY (
		    auto loc = val.getLocation ();
		    this-> _context.enterBlock ();
		    std::vector <Generator> vars;
		    if (index.isEmpty ()) 
			vars = createIndexVar (expression, value, "_iter");
		    else vars = createIndexVar (expression, value, index.to<syntax::VarDecl> ());		    
		    
		    values.push_back (vars [0]);
		    values.push_back (vars [1]);


		    auto array = vars [2];
		    auto iter = vars [3];
		    
		    auto one = ufixed (1);		    
		    auto len = StructAccess::init (loc, one.to <Value> ().getType (), array, "len");
		    
		    auto test = BinaryInt::init (loc, Binary::Operator::INF, Bool::init (loc), iter, len);

		    std::vector <Generator> innerValues;

		    auto innerType = array.to <Value> ().getType ().to <Slice> ().getInners () [0];
		    auto indexVal = SliceAccess::init (loc, innerType, array, iter);
		    
		    // Can be passed by ref, iif it is a lvalue, and mutability level is > 2 (mut [mut T])
		    bool canBeRef = array.to <Value> ().isLvalue ();
		    auto level = array.to <Value> ().getType ().to<Type> ().mutabilityLevel ();
		    
		    innerValues.push_back (validateArrayByValueIterator (expression, array, val, indexVal, canBeRef ? level : 0));		    
		    innerValues.push_back (this-> _context.validateValue (expression.getBlock ()));
		    
		    innerValues.push_back (Affect::init (
			loc, iter.to<Value> ().getType (), iter, BinaryInt::init (loc, Binary::Operator::ADD, one.to<Value> ().getType (), iter, one)
		    ));
		    values.push_back (Loop::init (expression.getLocation (), Void::init (expression.getLocation ()), test, Block::init (expression.getLocation (), Void::init (expression.getLocation ()), innerValues), false));
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;
	    }
	    
	    {
		TRY (
		    this-> _context.quitBlock ();
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;
	    }

	    if (errors.size () != 0) {
		THROW (ErrorCode::EXTERNAL, errors);
	    }
	    
	    return Block::init (expression.getLocation (), Void::init (expression.getLocation ()), values);
	}
	
	Generator ForVisitor::validateRange (const syntax::For & expression, const generator::Generator & value) {
	    auto vars = expression.getVars ();
	    if (vars.size () != 1) {
		Ymir::Error::occur (
		    value.getLocation (),
		    ExternalError::get (NOT_ITERABLE_WITH),
		    value.to <Value> ().getType ().to <Type> ().getTypeName (),
		    vars.size ()
		);
		return Generator::empty ();
	    }

	    return iterateRange (expression, value, vars [0]);
	}


	std::vector <Generator> ForVisitor::createIndexVarRange (const syntax::For &, const Generator & range, const syntax::VarDecl & decl) {
	    if (decl.getName ().str != Keys::UNDER)
		this-> _context.verifyShadow (decl.getName ());
	    
	    if (decl.getDecorators ().size () != 0) {
		auto deco = decl.getDecorators ()[0];
		Ymir::Error::occur (deco.getLocation (),
				    ExternalError::get (DECO_OUT_OF_CONTEXT),
				    deco.getLocation ().str
		);
	    }

	    auto loc = decl.getLocation ();
	    auto rVar = generator::VarDecl::init (loc, "__",
						  range.to <Value> ().getType (),
						  range,
						  false);

	    auto rRef = VarRef::init (loc, "__", range.to <Value> ().getType (),
				      rVar.getUniqId (),
				      false, Generator::empty ());
	    
	    auto innerType = range.to <Value> ().getType ().to <Type> ().getInners ()[0];
	    if (!decl.getType ().isEmpty ()) {
		auto type = this-> _context.validateType (decl.getType ());
		this-> _context.verifyCompatibleType (type, innerType);
	    } 
	    
	    auto zero = StructAccess::init (loc, innerType, rRef, Range::FST_NAME);
	    auto step = StructAccess::init (loc, innerType, rRef, Range::STEP_NAME);
	    
	    auto var = generator::VarDecl::init (loc,
						 decl.getName ().str,
						 zero.to <Value> ().getType (),
						 zero,
						 false
	    );

	    auto ref = VarRef::init (loc,
				     decl.getName ().str,
				     zero.to<Value> ().getType (),
				     var.getUniqId (),
				     false,
				     Generator::empty ()
	    );


	    if (decl.getName () != Keys::UNDER)
		this-> _context.insertLocal (decl.getName ().str, var);
	    
	    return {rVar, var, rRef, ref};
	}       

	
	Generator ForVisitor::iterateRange (const syntax::For & expression, const generator::Generator & range, const syntax::Expression & index) {
	    std::vector <std::string> errors;
	    std::vector <Generator> value = {};
	    {
		TRY (
		    auto loc = range.getLocation ();
		    this-> _context.enterBlock ();
		    auto innerType = range.to <Value> ().getType ().to <Type> ().getInners ()[0];
		    auto vars = createIndexVarRange (expression, range, index.to <syntax::VarDecl> ());
		    value.push_back (vars [0]);
		    value.push_back (vars [1]);
		    
		    auto rangeVar = vars [2];
		    auto left = StructAccess::init (expression.getLocation (), innerType, rangeVar, Range::FST_NAME);
		    auto right = StructAccess::init (expression.getLocation (), innerType, rangeVar, Range::SCD_NAME);
		    auto step = StructAccess::init (expression.getLocation (), innerType, rangeVar, Range::STEP_NAME);
		    auto isFull = StructAccess::init (expression.getLocation (), innerType, rangeVar, Range::FULL_NAME);
		    
		    auto iter = vars [3];
		    auto lTest = Generator::empty ();
		    auto rTest = Generator::empty ();
		    if (innerType.is <Float> ()) {
			auto iterTest = BinaryFloat::init (loc, Binary::Operator::INF, Bool::init (loc), step, FloatValue::init (loc, innerType, 0.0f));
			lTest = Conditional::init (loc, Bool::init (loc), iterTest, 			    
						   BinaryFloat::init (expression.getLocation (),
								    Binary::Operator::SUP,
								    Bool::init (expression.getLocation ()),
								    iter, right),
						   BinaryFloat::init (expression.getLocation (),
								    Binary::Operator::INF,
								    Bool::init (expression.getLocation ()),
								    iter, right)
			);

			rTest = Conditional::init (loc, Bool::init (loc), iterTest, 			    
						   BinaryFloat::init (expression.getLocation (),
								    Binary::Operator::SUP_EQUAL,
								    Bool::init (expression.getLocation ()),
								    iter, right),
						   BinaryFloat::init (expression.getLocation (),
								    Binary::Operator::INF_EQUAL,
								    Bool::init (expression.getLocation ()),
								    iter, right)
			);
		    } else {
			auto iterTest = BinaryInt::init (loc, Binary::Operator::INF, Bool::init (loc), step, ifixed (0));
			lTest = Conditional::init (loc, Bool::init (loc), iterTest, 			    
						   BinaryInt::init (expression.getLocation (),
								    Binary::Operator::SUP,
								    Bool::init (expression.getLocation ()),
								    iter, right),
						   BinaryInt::init (expression.getLocation (),
								    Binary::Operator::INF,
								    Bool::init (expression.getLocation ()),
								    iter, right)
			);

			rTest = Conditional::init (loc, Bool::init (loc), iterTest, 			    
						   BinaryInt::init (expression.getLocation (),
								    Binary::Operator::SUP_EQUAL,
								    Bool::init (expression.getLocation ()),
								    iter, right),
						   BinaryInt::init (expression.getLocation (),
								    Binary::Operator::INF_EQUAL,
								    Bool::init (expression.getLocation ()),
								    iter, right)
			);
			
		    }

		    auto test = Conditional::init (expression.getLocation (),
						   Bool::init (expression.getLocation ()),
						   isFull, rTest, lTest);

		    std::vector <Generator> innerValues;
		    innerValues.push_back (this->_context.validateValue (expression.getBlock ()));
		    innerValues.push_back (Affect::init (
			loc, iter.to <Value> ().getType (), iter, BinaryInt::init (loc, Binary::Operator::ADD, step.to<Value> ().getType (), iter, step)
		    ));
		    value.push_back (Loop::init (expression.getLocation (), Void::init (expression.getLocation ()), test, Block::init (expression.getLocation (), Void::init (loc), innerValues), false));		    
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;
	    }

	    {
		TRY (
		    this-> _context.quitBlock ();
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;
	    }

	    if (errors.size () != 0) {
		THROW (ErrorCode::EXTERNAL, errors);
	    }
	    
	    return Block::init (expression.getLocation (), Void::init (expression.getLocation ()), value);
	}
	
	void ForVisitor::error (const syntax::For &, const generator::Generator & value) {
	    Ymir::Error::occur (
		value.getLocation (),
		ExternalError::get (NOT_ITERABLE),
		value.to <Value> ().getType ().to <Type> ().getTypeName ()
	    );
	}
		

    }    
    
}
