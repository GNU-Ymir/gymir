#include <ymir/semantic/validator/ForVisitor.hh>
#include <ymir/semantic/generator/_.hh>

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
		of (Array, a ATTRIBUTE_UNUSED,
		    return validateArray (expression, value);
		);

		of (Slice, s ATTRIBUTE_UNUSED,
		    return validateSlice (expression, value);
		);
	    }
	    
	    error (expression, value);
	    return Generator::empty ();
	}

	Generator ForVisitor::validateArray (const syntax::For & expression, const generator::Generator & value) {
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
		return iterateArray (expression, value, syntax::Expression::empty (), vars [0]);
	    } else { 
		return iterateArray (expression, value, vars [0], vars [1]);
	    }
	}

	Generator ForVisitor::validateArrayByValueIterator (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & val, const generator::Generator & value_, int level) {
	    auto var = val.to<syntax::VarDecl> ();
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
	    this-> _context.insertLocal (var.getName ().str, ret);
	    return ret;
	}
	
	std::vector <Generator> ForVisitor::createIndexVar (const syntax::For &, const syntax::VarDecl & decl) {
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

	    this-> _context.insertLocal (decl.getName ().str, var);
	    
	    return {var, ref};
	}       

	std::vector <Generator> ForVisitor::createIndexVar (const syntax::For & expression, const std::string & name) {	    
	    auto zero = ufixed (0);
	    auto loc = expression.getLocation ();
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
	    
	    return {var, ref};
	}       

	
	Generator ForVisitor::iterateArray (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & index, const syntax::Expression & val) {
	    std::vector <std::string> errors;
	    std::vector<Generator> value = {};
	    {
		TRY (
		    auto loc = val.getLocation ();
		    this-> _context.enterBlock ();
		    std::vector <Generator> vars;
		    if (index.isEmpty ()) 
			vars = createIndexVar (expression, "_iter");
		    else vars = createIndexVar (expression, index.to<syntax::VarDecl> ());		    
		    
		    value.push_back (vars [0]);
		    
		    auto len = ufixed (array.to <Value> ().getType ().to <Array> ().getSize ()) ;
		    auto one = ufixed (1);
		    
		    auto iter = vars [1];
		    auto test = BinaryInt::init (loc, Binary::Operator::INF, Bool::init (loc), iter, len);

		    std::vector <Generator> innerValues;

		    auto innerType = array.to <Value> ().getType ().to <Array> ().getInners () [0];
		    auto indexVal = ArrayAccess::init (loc, innerType, array, iter);
		    
		    // Can be passed by ref, iif it is a lvalue, and mutability level is > 2 (mut [mut T])
		    bool canBeRef = array.to <Value> ().isLvalue ();
		    auto level = array.to <Value> ().getType ().to<Type> ().mutabilityLevel ();
		    
		    innerValues.push_back (validateArrayByValueIterator (expression, array, val, indexVal, canBeRef ? level : 0));		    
		    innerValues.push_back (this-> _context.validateValue (expression.getBlock ()));
		    
		    innerValues.push_back (Affect::init (
			loc, iter.to<Value> ().getType (), iter, BinaryInt::init (loc, Binary::Operator::ADD, one.to<Value> ().getType (), iter, one)
		    ));
		    value.push_back (Loop::init (expression.getLocation (), Void::init (expression.getLocation ()), test, Block::init (expression.getLocation (), Void::init (expression.getLocation ()), innerValues), false));
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
       
	
	Generator ForVisitor::validateSlice (const syntax::For & expression ATTRIBUTE_UNUSED, const generator::Generator & value ATTRIBUTE_UNUSED) {
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
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
