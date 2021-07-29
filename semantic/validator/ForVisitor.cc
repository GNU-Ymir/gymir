#include <ymir/semantic/validator/ForVisitor.hh>
#include <ymir/semantic/generator/_.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/global/Core.hh>
#include <ymir/semantic/generator/value/_.hh>

using namespace global;

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
		of (Array, a) {
		    auto type = Slice::init (expression.getLocation (), a.getInners ()[0]);
		    type = Type::init (type.to<Type> (), a.isMutable ());
		    
		    return validateSlice (expression, Aliaser::init (expression.getLocation (), type, value));
		}
		
		elof_u (Slice) 
		    return validateSlice (expression, value);		    
		
		elof_u (Range)
		    return validateRange (expression, value);		    

		elof_u (Tuple)
		    return validateTuple (expression, value);		    
		
		elof_u (ClassPtr)
		    return validateClass (expression, value);
		
		fo;
		
	    }	    
	    
	    error (expression, value);
	    return Generator::empty ();
	}

	Generator ForVisitor::validateCte (const syntax::For & expression) {
	    auto value = this-> _context.validateValue (expression.getIter ());
	    match (value.to <Value> ().getType ()) {		
		of_u (Range)
		    return validateRange (expression, value, true);
		fo;
	    }
	    
	    error (expression, value, true);
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
		this-> _context.verifyCompatibleType (var.getLocation (), innerType.getLocation (), type, innerType);
	    } else type = innerType;

	    type = Type::init (type.to<Type> (), false);
	    
	    bool isMutable = false, isRef = false, dmut = false;	    
	    type = this-> _context.applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut);
	    this-> _context.verifyMutabilityRefParam (var.getLocation (), type, ExternalError::MUTABLE_CONST_ITER);
	    
	    auto value = Generator::empty ();
	    auto loc = var.getLocation ();
	    if (!isRef) {
		value = value_;
	    } else {
	    	if (level < 2)
	    	    Ymir::Error::occur (expression.getIter ().getLocation (),
	    				ExternalError::DISCARD_CONST_LEVEL,
	    				2, level
	    	    );
		
	    	value = Aliaser::init (loc, type, value_);
	    }
	    
	    this-> _context.verifyMemoryOwner (loc, type, value, true);	    
	    
	    auto ret = generator::VarDecl::init (loc, var.getName ().getStr (), type, value, isMutable);
	    
	    if (var.getName () != Keys::UNDER) {
		this-> _context.insertLocal (var.getName ().getStr (), ret);
	    }
	    
	    return ret;
	}
	
	std::vector <Generator> ForVisitor::createIndexVar (const syntax::For &, const Generator & value, const syntax::VarDecl & decl) {
	    if (decl.getName () != Keys::UNDER)
		this-> _context.verifyShadow (decl.getName ());
	    
	    if (decl.getDecorators ().size () != 0) {
		auto deco = decl.getDecorators ()[0];
		Ymir::Error::occur (deco.getLocation (),
				    ExternalError::DECO_OUT_OF_CONTEXT,
				    deco.getLocation ().getStr ()
		);
	    }
	    
	    auto loc = decl.getLocation ();
	    auto zero = ufixed (0);	    
	    auto type = zero.to<Value> ().getType ();

	    auto valType = value.to <Value> ().getType ();
	    valType = Type::init (valType.to<Type> (), valType.to <Type> ().isMutable (), true);
	    
	    auto rRef = UniqValue::init (loc, valType, value); //Referencer::init (loc, valType, value));
	    
	    if (!decl.getType ().isEmpty ()) {
		type = this-> _context.validateType (decl.getType ());
		this-> _context.verifyCompatibleType (decl.getLocation (), loc, type, Integer::init (loc, 0, false));
	    } 

	    auto var = generator::VarDecl::init (lexing::Word::init (loc, "iter"),
						 decl.getName ().getStr (),
						 type,
						 zero,
						 false
	    );
	    
	    auto ref = VarRef::init (lexing::Word::init (loc, "iter"),
				     decl.getName ().getStr (),
				     type, 
				     var.getUniqId (),
				     false,
				     Generator::empty ()
	    );

	    if (decl.getName () != Keys::UNDER) {
		this-> _context.insertLocal (decl.getName ().getStr (), var);
	    }
	    
	    return {var, rRef, ref};
	}       

	std::vector <Generator> ForVisitor::createIndexVar (const syntax::For & expression, const Generator & value, const std::string & name) {	    
	    auto zero = ufixed (0);
	    auto loc = expression.getLocation ();
	    auto type = value.to <Value> ().getType ();
	    type = Type::init (type.to<Type> (), type.to <Type> ().isMutable (), true);

	    auto rRef = UniqValue::init (loc, type, value);//Referencer::init (loc, type, value));
	    
	    auto var = generator::VarDecl::init (lexing::Word::init (loc, name),
						 name,
						 zero.to <Value> ().getType (),
						 zero,
						 false
	    );

	    auto ref = VarRef::init (lexing::Word::init (loc, name),
				     name,
				     zero.to<Value> ().getType (),
				     var.getUniqId (),
				     false,
				     Generator::empty ()
	    );
	    
	    return {var, rRef, ref};
	}                    
	
	Generator ForVisitor::validateSlice (const syntax::For & expression, const generator::Generator & value) {
	    auto vars = expression.getVars ();
	    if (vars.size () > 2) {
		Ymir::Error::occur (
		    value.getLocation (),
		    ExternalError::NOT_ITERABLE_WITH,
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
	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector<Generator> values = {};
	    auto loc = val.getLocation ();
	    Generator loop_type (Void::init (loc));
	    
	    try {
		this-> _context.enterBlock ();
		std::vector <Generator> vars;
		if (index.isEmpty ()) 
		    vars = createIndexVar (expression, value, "_iter");
		else vars = createIndexVar (expression, value, index.to<syntax::VarDecl> ());
		values.push_back (vars [0]);
		values.push_back (vars [1]); // We push it here to add the validation before the loop

		auto array = vars [1];
		auto iter = vars [2];
		    
		auto one = ufixed (1);		    
		auto len = StructAccess::init (loc, one.to <Value> ().getType (), array, "len");
		    
		auto test = BinaryInt::init (loc, Binary::Operator::INF, Bool::init (loc), iter, len);

		std::vector <Generator> innerValues;

		auto innerType = array.to <Value> ().getType ().to <Slice> ().getInners () [0];
		auto indexVal = SliceAccess::init (loc, innerType, array, iter);
		    
		// Can be passed by ref, iif it is a lvalue, and mutability level is > 2 (mut [mut T])
		bool canBeRef = value.is <Aliaser> () || value.is<Referencer> () || value.to <Value> ().isLvalue ();
		auto level = value.to <Value> ().getType ().to<Type> ().mutabilityLevel ();
		    
		innerValues.push_back (validateArrayByValueIterator (expression, array, val, indexVal, canBeRef ? level : 0));

		innerValues.push_back (this-> validateLoopContent (expression));		    
		innerValues.push_back (Affect::init (
		    loc, iter.to<Value> ().getType (), iter, BinaryInt::init (loc, Binary::Operator::ADD, one.to<Value> ().getType (), iter, one)
		));
		    		    
		values.push_back (Loop::init (lexing::Word::init (expression.getLocation (), "#_for"), loop_type, test, Block::init (expression.getLocation (), loop_type, innerValues), false));
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
	    
	    
	    
	    try {
		this-> _context.quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {
		    
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	    

	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }

	    return Block::init (expression.getLocation (), loop_type, values);
	}

	Generator ForVisitor::validateRange (const syntax::For & expression, const generator::Generator & value, bool isCte) {
	    auto vars = expression.getVars ();
	    if (vars.size () != 1) {
		Ymir::Error::occur (
		    value.getLocation (),
		    ExternalError::NOT_ITERABLE_WITH,
		    value.to <Value> ().getType ().to <Type> ().getTypeName (),
		    vars.size ()
		);
		return Generator::empty ();
	    }

	    if (isCte)
		return iterateRangeCte (expression, value, vars [0]);
	    else
		return iterateRange (expression, value, vars [0]);
	}


	std::vector <Generator> ForVisitor::createIndexVarRange (const syntax::For &, const Generator & range, const syntax::VarDecl & decl) {
	    if (decl.getName ().getStr () != Keys::UNDER)
		this-> _context.verifyShadow (decl.getName ());
	    
	    if (decl.getDecorators ().size () != 0) {
		auto deco = decl.getDecorators ()[0];
		Ymir::Error::occur (deco.getLocation (),
				    ExternalError::DECO_OUT_OF_CONTEXT,
				    deco.getLocation ().getStr ()
		);
	    }

	    auto loc = decl.getLocation ();
	    auto type = range.to <Value> ().getType ();
	    type = Type::init (type.to<Type> (), type.to <Type> ().isMutable (), true);
	    auto rRef = UniqValue::init (loc, type, range); //Referencer::init (loc, type, range));
	    
	    
	    auto innerType = range.to <Value> ().getType ().to <Type> ().getInners ()[0];
	    if (!decl.getType ().isEmpty ()) {
		auto type = this-> _context.validateType (decl.getType ());
		this-> _context.verifyCompatibleType (decl.getLocation (), innerType.getLocation (), type, innerType);
	    } 
	    
	    auto zero = StructAccess::init (loc, innerType, rRef, Range::FST_NAME);
	    auto step = StructAccess::init (loc, innerType, rRef, Range::STEP_NAME);
	    
	    auto var = generator::VarDecl::init (loc,
						 decl.getName ().getStr (),
						 zero.to <Value> ().getType (),
						 zero,
						 false
	    );

	    auto ref = VarRef::init (loc,
				     decl.getName ().getStr (),
				     zero.to<Value> ().getType (),
				     var.getUniqId (),
				     false,
				     Generator::empty ()
	    );


	    if (decl.getName () != Keys::UNDER)
		this-> _context.insertLocal (decl.getName ().getStr (), var);
	    
	    return {var, rRef, ref};
	}       

	
	Generator ForVisitor::iterateRange (const syntax::For & expression, const generator::Generator & range, const syntax::Expression & index) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector <Generator> value = {};
	    auto loc = range.getLocation ();
	    Generator loop_type (Void::init (loc));
	    
	    try {
		this-> _context.enterBlock ();
		auto innerType = range.to <Value> ().getType ().to <Type> ().getInners ()[0];
		auto vars = createIndexVarRange (expression, range, index.to <syntax::VarDecl> ());
		value.push_back (vars [0]);
		value.push_back (vars [1]); // We want the uniq value to be defined outside the loop
		    
		auto rangeVar = vars [1];
		auto left = StructAccess::init (expression.getLocation (), innerType, rangeVar, Range::FST_NAME);
		auto right = StructAccess::init (expression.getLocation (), innerType, rangeVar, Range::SCD_NAME);
		auto step = StructAccess::init (expression.getLocation (), innerType, rangeVar, Range::STEP_NAME);
		auto isFull = StructAccess::init (expression.getLocation (), innerType, rangeVar, Range::FULL_NAME);
		    
		auto iter = vars [2];
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
		innerValues.push_back (this-> validateLoopContent (expression));		

		if (innerType.is<Float> ()) {
		    innerValues.push_back (Affect::init (
			loc, iter.to <Value> ().getType (), iter, BinaryFloat::init (loc, Binary::Operator::ADD, step.to<Value> ().getType (), iter, step)
		    ));
		} else {
		    innerValues.push_back (Affect::init (
			loc, iter.to <Value> ().getType (), iter, BinaryInt::init (loc, Binary::Operator::ADD, step.to<Value> ().getType (), iter, step)
		    ));			
		}
		    
		value.push_back (Loop::init (expression.getLocation (), loop_type, test, Block::init (expression.getLocation (), loop_type, innerValues), false));		    
	    } catch (Error::ErrorList list) {
		    
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
	    

	    
	    try {
		this-> _context.quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {
		    
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
	    

	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	    
	    return Block::init (lexing::Word::init (expression.getLocation (), "#_for_block"), loop_type, value);
	}

	Generator ForVisitor::iterateRangeCte (const syntax::For & expression, const generator::Generator & range, const syntax::Expression & index) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto loc = expression.getLocation ();
	    
	    this-> _context.enterBlock ();
	    auto rng = this-> _context.retreiveValue (range);
	    if (!rng.is <RangeValue> ()) {
		this-> error (expression, range, true);
	    }
	    
	    auto l = rng.to <RangeValue> ().getLeft ();
	    auto r = rng.to <RangeValue> ().getRight ();
	    auto s = rng.to <RangeValue> ().getStep ();
	    bool isFull = rng.to <RangeValue> ().getIsFull ().to <BoolValue> ().getValue ();
	    
	    if (!l.is <Fixed> ())
		this-> error (expression, range, true);
	    
	    auto decl = index.to <syntax::VarDecl> ();	    
	    if (decl.getName ().getStr () != Keys::UNDER)
		this-> _context.verifyShadow (decl.getName ());

	    if (!decl.getType ().isEmpty ()) {
		auto iterType = this-> _context.validateType (decl.getType ());
		this-> _context.verifyMemoryOwner (decl.getType ().getLocation (), iterType, l, true);
	    }
	    
	    if (decl.getDecorators ().size () != 0) {
		auto deco = decl.getDecorators ()[0];
		Ymir::Error::occur (deco.getLocation (),
				    ExternalError::DECO_OUT_OF_CONTEXT,
				    deco.getLocation ().getStr ()
		    );
	    }
	    
	    std::vector <Generator> innerValues;
	    if (l.to <Value> ().getType ().to <Integer> ().isSigned ()) {
		long l_i = l.to<Fixed> ().getUI ().u;
		long r_i = r.to<Fixed> ().getUI ().u;
		long s_i = s.to <Fixed> ().getUI ().i;
		for (long i = l_i;; i += s_i) {
		    if (s_i < 0) {
			if (isFull) {
			    if (i < r_i) break;
			} else {
			    if (i <= r_i) break;
			}
		    } else {
			if (isFull) {
			    if (i > r_i) break;
			} else {
			    if (i >= r_i) break;
			}
		    }
		    try {
			this-> _context.enterBlock ();		    
			if (decl.getName () != Keys::UNDER) {
			    auto var = generator::VarDecl::init (loc,
								 decl.getName ().getStr (),
								 l.to <Value> ().getType (), 
								 ifixed (i),
								 false
				);

			    auto ref = VarRef::init (loc,
						     decl.getName ().getStr (),
						     l.to<Value> ().getType (),
						     var.getUniqId (),
						     false,
						     ifixed (i)
				);
			    this-> _context.insertLocal (decl.getName ().getStr (), var);
			    innerValues.push_back (var);			
			}

			innerValues.push_back (this-> validateLoopContent (expression));
		    } catch (Error::ErrorList list) {			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
		
		    try {
			this-> _context.quitBlock (errors.size () == 0);
		    } catch (Error::ErrorList list) {
			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		

		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }
		}
	    } else {
		ulong l_i = l.to<Fixed> ().getUI ().u;
		ulong r_i = r.to<Fixed> ().getUI ().u;
		long s_i = s.to <Fixed> ().getUI ().i;
		for (ulong i = l_i;; i += s_i) {
		    
		    if (s_i < 0) {
			if (isFull) {
			    if (i < r_i) break;
			} else {
			    if (i <= r_i) break;
			}
		    } else {
			if (isFull) {
			    if (i > r_i) break;
			} else {
			    if (i >= r_i) break;
			}
		    }
		    
		    try {
			this-> _context.enterBlock ();		    
			if (decl.getName () != Keys::UNDER) {
			    auto var = generator::VarDecl::init (loc,
								 decl.getName ().getStr (),
								 l.to <Value> ().getType (), 
								 ufixed (i),
								 false
				);

			    auto ref = VarRef::init (loc,
						     decl.getName ().getStr (),
						     l.to<Value> ().getType (),
						     var.getUniqId (),
						     false,
						     ufixed (i)
				);
			    this-> _context.insertLocal (decl.getName ().getStr (), var);
			    innerValues.push_back (var);			
			}

			innerValues.push_back (this-> validateLoopContent (expression));
		    } catch (Error::ErrorList list) {			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
		
		    try {
			this-> _context.quitBlock (errors.size () == 0);
		    } catch (Error::ErrorList list) {
			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		

		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }
		}
	    }
	    
	    try {
		this-> _context.quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {		    
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
		
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }

	    auto loopType = Void::init (expression.getLocation ());
	    
	    auto test = BoolValue::init (loc, Bool::init (loc), false);
	    auto bl = Block::init (expression.getLocation (), loopType, innerValues);
	    return Loop::init (lexing::Word::init (loc, "#_for"), loopType, test, bl, true); 	    
	}
	
	Generator ForVisitor::validateTuple (const syntax::For & expression, const generator::Generator & tuple) {
	    auto vars = expression.getVars ();
	    if (vars.size () > 1) {
		Ymir::Error::occur (
		    tuple.getLocation (),
		    ExternalError::NOT_ITERABLE_WITH,
		    tuple.to <Value> ().getType ().to <Type> ().getTypeName (),
		    vars.size ()
		);
		return Generator::empty ();
	    } else {
		return iterateTuple (expression, tuple, vars [0]);
	    }
	}

	std::vector <Generator> ForVisitor::createIndexVarTuple (const syntax::For & expression, const Generator & innerTuple, const syntax::VarDecl & decl, int level) {
	    if (decl.getName ().getStr () != Keys::UNDER)
		this-> _context.verifyShadow (decl.getName ());

	    if (decl.getDecorators ().size () != 0) {
		auto deco = decl.getDecorators ()[0];
		Ymir::Error::occur (deco.getLocation (),
				    ExternalError::DECO_OUT_OF_CONTEXT,
				    deco.getLocation ().getStr ()
		);
	    }

	    auto type = innerTuple.to <Value> ().getType ();
	    if (!decl.getType ().isEmpty ())
		type = this-> _context.validateType (decl.getType ());
	    	    
	    auto loc = decl.getLocation ();

	    auto value = innerTuple;	    
	    auto var = generator::VarDecl::init (loc,
						 decl.getName ().getStr (),
						 type, 
						 value,
						 false
	    );

	    auto ref = VarRef::init (loc,
				     decl.getName ().getStr (),
				     type,
				     var.getUniqId (),
				     false,
				     Generator::empty ()
	    );


	    if (decl.getName () != Keys::UNDER)
		this-> _context.insertLocal (decl.getName ().getStr (), var);
	    
	    return {var, ref};

	}
	
	Generator ForVisitor::iterateTuple (const syntax::For & expression, const Generator & value, const syntax::Expression & index) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    // The iteration on tuple is done at compile time, we just replicate the block multiple times
	    this-> _context.enterBlock ();
	    auto type = value.to <Value> ().getType ();
	    type = Type::init (type.to<Type> (), type.to <Type> ().isMutable (), true);
	    auto loc = expression.getLocation ();
	    
	    auto rRef = UniqValue::init (loc, type, value); //Referencer::init (loc, type, value));

	    std::vector <Generator> innerValues;
	    for (auto  it : Ymir::r (0, value.to <Value> ().getType ().to <Type> ().getInners ().size ())) {
		auto currentType =  value.to <Value> ().getType ().to <Type> ().getInners () [it];
		
		try {
		    this-> _context.enterBlock ();
		    auto decl = index.to <syntax::VarDecl> ();
		    auto innerTuple = TupleAccess::init (index.getLocation (), currentType, rRef, it);
		    // Can be passed by ref, iif it is a lvalue, and mutability level is > 2 (mut [mut T])
		    bool canBeRef = value.to <Value> ().isLvalue ();
		    auto level = type.to<Type> ().mutabilityLevel ();
			
		    auto vars = createIndexVarTuple (expression, innerTuple, decl, canBeRef ? level : 0);
		    innerValues.push_back (vars [0]);
		    innerValues.push_back (this-> validateLoopContent (expression));			
		} catch (Error::ErrorList list) {
			
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 

		try {
		    this-> _context.quitBlock (errors.size () == 0);
		} catch (Error::ErrorList list) {
			
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
		

		if (errors.size () != 0) {
		    throw Error::ErrorList {errors};
		}
	    }

	    
	    try {
		this-> _context.quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {		    
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
		
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }

	    auto loopType = Void::init (expression.getLocation ());
	    auto test = BoolValue::init (loc, Bool::init (loc), false);
	    auto bl = Block::init (expression.getLocation (), loopType, innerValues);
	    
	    return Loop::init (lexing::Word::init (loc, "#_for"), loopType, test, bl, true); 	    
	}


	Generator ForVisitor::validateIndexVarClass (const syntax::For & expr, const Generator & iterator, const syntax::VarDecl & decl, int i) {
	    auto loc = decl.getLocation ();
	    auto syntIterVal = TemplateSyntaxWrapper::init (expr.getLocation (), iterator);
	    auto templ = syntax::TemplateCall::init (
		loc,
		{syntax::Fixed::init (lexing::Word::init (loc, Ymir::format ("%", i)), lexing::Word::eof ())},
		syntax::Binary::init (
		    lexing::Word::init (loc, Token::DOT),
		    syntIterVal,
		    syntax::Var::init (lexing::Word::init (loc, CoreNames::get (GET_OP_OVERRIDE))),
		    syntax::Expression::empty ()
		    )
		);
	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		templ,
		{}, false
		);
		    
	    auto value = this-> _context.validateValue (call);
	    auto type = value.to <Value> ().getType ();
	    if (decl.getDecorators ().size () != 0) {
		auto deco = decl.getDecorators ()[0];
		Ymir::Error::occur (deco.getLocation (),
				    ExternalError::DECO_OUT_OF_CONTEXT,
				    deco.getLocation ().getStr ()
		    );
	    }
	    
	    type = Type::init (type.to<Type> (), type.to <Type> ().isMutable (), false); // The iteration is necessarily immutable and by value

	    if (!decl.getType ().isEmpty ()) {
		auto d_type = this-> _context.validateType (decl.getType ());
		this-> _context.verifyCompatibleType (decl.getLocation (), type.getLocation (), d_type, type);
		type = d_type;
	    }
	    
	    auto var = generator::VarDecl::init (loc,
						 decl.getName ().getStr (),
						 type,
						 value,
						 false
	    );

	    if (decl.getName () != Keys::UNDER)
		this-> _context.insertLocal (decl.getName ().getStr (), var);
	    
	    return var;
	}

	/***
	 * let i = begin ();
	 * { 
	 *    let e = end ();
	 *    {
	 *        while (i != e) {
	 *            let i_0, i_1, ... i_n = i.get!{0}, i.get!{1} ... i.get!{n}
	 *            {
	 *                 inner_values
	 *            }
	 *            i:.next ();
	 *        }
	 *   } exit { // cte if e impl Disposable
	 *      e.dispose ();
	 *   }
	 * } exit { // cte if i impl Disposable
	 *    i.dispose ();
	 * }
	 *
	 */
	generator::Generator ForVisitor::validateClass (const syntax::For & expr, const generator::Generator & value) {
	    std::list <Error::ErrorMsg> errors;
	    Generator loop = Generator::empty ();
	    std::vector <Generator> preLoopVarDecls;
	    
	    auto loc = expr.getLocation ();
	    
	    Generator loop_type (Void::init (loc));
	    syntax::Expression syntIterVal (syntax::Expression::empty ());
	    
	    this-> _context.enterBlock ();

	    try {
		Generator iterRef (Generator::empty ()), endRef (Generator::empty ());
		preLoopVarDecls = this-> validateClassPreLoop (expr, value, iterRef, endRef);
		syntIterVal = TemplateSyntaxWrapper::init (expr.getLocation (), iterRef);
		auto syntEndVal  = TemplateSyntaxWrapper::init (expr.getLocation (), endRef);
		
		auto test = this-> _context.validateValue (
		    syntax::Binary::init (lexing::Word::init (loc, Token::NOT_EQUAL), syntIterVal, syntEndVal, syntax::Expression::empty ())
		    );

		std::vector <Generator> innerValues;
		
		int i = 0;
		for (auto & it : expr.getVars ()) {
		    innerValues.push_back (
			validateIndexVarClass (expr, iterRef, it.to <syntax::VarDecl> (), i)
			);
		    i += 1;
		}
		
		auto nextBin = syntax::Binary::init (
		    lexing::Word::init (loc, Token::DOT),
		    syntIterVal,
		    syntax::Var::init (lexing::Word::init (loc, CoreNames::get (NEXT_OP_OVERRIDE))),
		    syntax::Expression::empty ()
		    );
		    
		auto call = syntax::MultOperator::init (
		    lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		    nextBin,
		    {}, false
		    );

		auto right = this-> _context.validateValue (call);
		auto content = this-> validateLoopContent (expr);
		
		innerValues.push_back (content);		
		innerValues.push_back (right);
		
		loop = Loop::init (lexing::Word::init (loc, "#_for"), loop_type, test, Block::init (loc, loop_type, innerValues), false);
	    } catch (Error::ErrorList list) {
		errors = list.errors;
		// We discard local to avoid useless error message when quitting the block
		this-> _context.discardAllLocals ();
	    }

	    try {
		this-> _context.quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }
	    
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};

	    auto impl = this-> _context.getCache ().disposeTrait.getValue ();
	    int i = 0;
	    for (auto & val : preLoopVarDecls) {
		auto doesImpl = this-> _context.verifyClassImpl (loc, val.to <generator::VarDecl> ().getVarType (), impl, false);
		if (doesImpl) {
		    try { // this can fail for mutability reasons
			auto valType = val.to<generator::VarDecl> ().getVarType ();
			auto valRef = Aliaser::init (
			    loc,
			    valType,
			    VarRef::init (loc, i == 0 ? "#_end" : "#_iter", valType, val.getUniqId (), false, Generator::empty ()));
			
			auto syntVal  = TemplateSyntaxWrapper::init (expr.getLocation (), valRef);
			auto call = syntax::MultOperator::init (
			    lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
			    syntax::Binary::init (lexing::Word::init (loc, Token::DOT),
						  syntVal,
						  syntax::Var::init (lexing::Word::init (loc, global::CoreNames::get (DISPOSE_OP_OVERRIDE))),
						  syntax::Expression::empty ()),
			    {}, false
			    );
			auto exit = this-> _context.validateValue (call);
			auto jmp_buf_type = this-> _context.validateType (syntax::Var::init (lexing::Word::init (loc, global::CoreNames::get (JMP_BUF_TYPE))));
		    
			loop = ExitScope::init (loc, loop.to <Value> ().getType (), jmp_buf_type, loop, {exit}, {}, Generator::empty (), Generator::empty (), Generator::empty ());
			loop = Block::init (loc, loop.to <Value> ().getType (), {val, loop});
		    } catch (Error::ErrorList&){
			loop = Block::init (loc, loop_type, {val, loop});;
		    }
		} else loop = Block::init (loc, loop_type, {val, loop});
		i += 1;
	    }
	    return loop;
	}



	std::vector <Generator> ForVisitor::validateClassPreLoop (const syntax::For & expr, const generator::Generator & value, generator::Generator & iterRef, generator::Generator & endRef) {
	    auto loc = expr.getLocation ();
	    std::vector <Generator> preLoopVarDecls;
	    auto cRef = UniqValue::init (expr.getLocation (), value.to <Value> ().getType (), value);

	    auto syntCRef = TemplateSyntaxWrapper::init (loc, cRef);
	    auto syntBegin = syntax::Intrinsics::init (lexing::Word::init (loc, Keys::ALIAS),
						       syntax::MultOperator::init (lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
										   syntax::Binary::init (lexing::Word::init (loc, Token::DOT),
													 syntCRef,
													 syntax::Var::init (lexing::Word::init (loc, CoreNames::get (BEGIN_OP_OVERRIDE))),
													 syntax::Expression::empty ()
										       ),
										   {}, false));

	    auto syntEnd = syntax::MultOperator::init (lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
						       syntax::Binary::init (lexing::Word::init (loc, Token::DOT),
									     syntCRef,
									     syntax::Var::init (lexing::Word::init (loc, CoreNames::get (END_OP_OVERRIDE))),
									     syntax::Expression::empty ()
							   ),
						       {}, false);
	    Generator begin (Generator::empty ());
	    Generator end (Generator::empty ());
		
	    try {
		begin = this-> _context.validateValue (syntBegin);
	    } catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::VALIDATING, syntBegin.prettyString ());
		list.errors.back ().addNote (note);
		throw list;
	    }

	    try {
		end = this-> _context.validateValue (syntEnd);
	    } catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::VALIDATING, syntEnd.prettyString ());
		list.errors.back ().addNote (note);
		throw list;
	    }

	    auto iterType = begin.to <Value> ().getType ().to <Type> ().toDeeplyMutable ();		
	    this-> _context.verifyMemoryOwner (loc, iterType, begin, false);
				
	    syntBegin = TemplateSyntaxWrapper::init (loc, begin);
	    syntEnd = TemplateSyntaxWrapper::init (loc, end);

	    auto iterVal = generator::VarDecl::init (loc, "#_iter", iterType, begin, true);
	    iterRef = Aliaser::init (
		loc,
		iterType,
		VarRef::init (loc, "#_iter", iterType, iterVal.getUniqId (), true, Generator::empty ()));
	    
	    auto endVal = generator::VarDecl::init (loc, "#_end", end.to <Value> ().getType (), end, false);
	    endRef = VarRef::init (loc, "#_end", end.to<Value> ().getType (), endVal.getUniqId (), false, Generator::empty ());

	    preLoopVarDecls.push_back (endVal);
	    preLoopVarDecls.push_back (iterVal);		
	
	    return preLoopVarDecls;	    
	}

	Generator ForVisitor::validateLoopContent (const syntax::For & loop) {
	    Generator content (Generator::empty ());
	    this-> _context.enterLoop ();
	    auto voidType = Void::init (loop.getLocation ());
	    this-> _context.setCurrentLoopType (voidType);
	    try {
		content = this->_context.validateValue (loop.getBlock (), false, false, false);
	    } catch (Error::ErrorList list) {
		this-> _context.quitLoop ();
		throw list;
	    }
		
	    this-> _context.quitLoop ();
	    this-> _context.verifyCompatibleType (content.getLocation (), loop.getLocation (), content.to <Value> ().getType (), voidType);
	    return content;
	}

	
	void ForVisitor::error (const syntax::For &, const generator::Generator & value, bool isCte) {
	    if (isCte) {
		Ymir::Error::occur (
		    value.getLocation (),
		    ExternalError::NOT_CTE_ITERABLE,
		    value.to <Value> ().getType ().to <Type> ().getTypeName ()
		    );
	    } else {
		Ymir::Error::occur (
		    value.getLocation (),
		    ExternalError::NOT_ITERABLE,
		    value.to <Value> ().getType ().to <Type> ().getTypeName ()
		    );
	    }
	}
		

    }    
    
}
