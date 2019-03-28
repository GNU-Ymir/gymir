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

	Generator ForVisitor::validateArrayByValueIterator (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & val, const generator::Generator & value) {
	    auto var = val.to<syntax::VarDecl> ();
	    
	    Generator type (Generator::empty ());
	    auto innerType = array.to <Value> ().getType ().to <Type> ().getInners () [0];
	    if (!var.getType ().isEmpty ()) {
		type = this-> _context.validateType (var.getType ());
		if (!type.to <Type> ().isCompatible (innerType))
		    Ymir::Error::occur (expression.getLocation (),
					ExternalError::get (INCOMPATIBLE_TYPES),
					type.to <Type> ().getTypeName (),
					innerType.to <Type> ().getTypeName ()
		    );	    
	    } else type = innerType;

	    type.to<Type> ().isMutable (false);
	    bool isMutable = false, isRef = false;
	    for (auto & deco : var.getDecorators ()) {
		switch (deco.getValue ()) {
		case syntax::Decorator::REF : { type.to <Type> ().isRef (true); isRef = true; } break;
		case syntax::Decorator::MUT : { type.to <Type> ().isMutable (true); isMutable = true; } break;
		default :
		    Ymir::Error::occur (deco.getLocation (),
					ExternalError::get (DECO_OUT_OF_CONTEXT),
					deco.getLocation ().str
		    );
		}
	    }

	    if (type.to<Type> ().isMutable () && !type.to<Type> ().isRef () && !type.is <Slice> ()) {
		Ymir::Error::occur (var.getDecorator (syntax::Decorator::MUT).getLocation (),
				    ExternalError::get (MUTABLE_CONST_ITER)
		);
	    }

	    // index can be mutable iif array is mutable, and inner is mutable
	    if (type.to<Type> ().isMutable ()) {
		if (!array.to <Value> ().isLvalue ()) {
		    Ymir::Error::occur (array.getLocation (), ExternalError::get (NOT_A_LVALUE));
		} else if (!array.to <Value> ().getType ().to<Type> ().isMutable () || !innerType.to<Type> ().isMutable ()) {
		    Ymir::Error::occur (array.getLocation (), ExternalError::get (IMMUTABLE_LVALUE));
		}
	    }

	    auto ret = Generator::empty ();
	    auto loc = var.getLocation ();
	    if (!isRef)
		ret = generator::VarDecl::init (loc, var.getName ().str, type, value, isMutable);
	    else 
		ret = generator::VarDecl::init (loc, var.getName ().str, type, Referencer::init (loc, type, value), isMutable);
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
		    innerValues.push_back (validateArrayByValueIterator (expression, array, val, indexVal));		    
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
