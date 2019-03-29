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
	    } else if (vars.size () == 1) {
		return iterateArrayByValue (expression, value, vars [0]);
	    } else { 
		return iterateArrayByIndexAndValue (expression, value, vars [0], vars [1]);
	    }
	}

	Generator ForVisitor::validateArrayByValueIterator (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & val, const generator::Generator & value_, int level) {
	    auto var = val.to<syntax::VarDecl> ();
	    
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
	
	Generator ForVisitor::iterateArrayByValue (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & val) {
	    std::vector <std::string> errors;
	    std::vector<Generator> value = {};
	    auto test = Generator::empty ();
	    auto loc = val.getLocation ();
	    {
		TRY (
		    this-> _context.enterBlock ();
		    Fixed::UI ui; ui.u = 0;
		    auto iterType = Integer::init (loc, 64, false);
		    value.push_back (
			generator::VarDecl::init (loc, "_iter",
						  iterType,
						  Fixed::init (loc, iterType, ui),
						  false
			)
		    );

		    ui.u = array.to <Value> ().getType ().to <Array> ().getSize ();
		    auto len = Fixed::init (loc, iterType, ui);
		    ui.u = 1;
		    auto one = Fixed::init (loc, iterType, ui);

		    auto iter = VarRef::init (loc, "_iter", iterType, value.back ().getUniqId (), false, Generator::empty ());
		    test = BinaryInt::init (loc, Binary::Operator::INF, Bool::init (loc), iter, len);

		    std::vector <Generator> innerValues;

		    auto innerType = array.to <Value> ().getType ().to <Array> ().getInners () [0];
		    auto indexVal = ArrayAccess::init (loc, innerType, array, iter);
		    // Can be passed by ref, iif it is a lvalue, and mutability level is > 2 (mut [mut T])
		    bool canBeRef = array.to <Value> ().isLvalue ();
		    auto level = array.to <Value> ().getType ().to<Type> ().mutabilityLevel ();
		    
		    innerValues.push_back (validateArrayByValueIterator (expression, array, val, indexVal, canBeRef ? level : 0));		    
		    innerValues.push_back (this-> _context.validateValue (expression.getBlock ()));
		    
		    innerValues.push_back (Affect::init (
			loc, iterType, iter, BinaryInt::init (loc, Binary::Operator::ADD, iterType, iter, one)
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
	
	Generator ForVisitor::iterateArrayByIndexAndValue (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & index, const syntax::Expression & val) {
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();	    
	}
	
	Generator ForVisitor::validateSlice (const syntax::For & expression, const generator::Generator & value) {
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
