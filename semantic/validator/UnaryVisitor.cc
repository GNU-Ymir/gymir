#include <ymir/semantic/validator/UnaryVisitor.hh>
#include <ymir/semantic/generator/value/_.hh>

namespace semantic {

    namespace validator {

	using namespace Ymir;
	using namespace generator;

	UnaryVisitor::UnaryVisitor (Visitor & context) :
	    _context (context)
	{}

	UnaryVisitor UnaryVisitor::init (Visitor & context) {
	    return UnaryVisitor (context);
	}

	Generator UnaryVisitor::validate (const syntax::Unary & expression) {
	    auto op = toOperator (expression.getOperator ());
	    // If the operator is &, we want to have access to the adress of the function,
	    // so it is forbidden to fast call any function (even if it is a dot templateCall)
	    auto operand = this-> _context.validateValue (expression.getContent (), false, op == Unary::Operator::ADDR);

	    if (op == Unary::Operator::ADDR) { // Pointer
		match (operand) {
		    of (FrameProto, proto ATTRIBUTE_UNUSED,
			return validateFunctionPointer (expression, operand);
		    ) else {
			if (operand.to <Value> ().isLvalue ()) {
			    auto type = Pointer::init (operand.getLocation (), operand.to<Value> ().getType ());
			    type.to<Type> ().isMutable(true);
			    return Addresser::init (expression.getLocation (), type, operand);
			} else {
			    Ymir::Error::occur (operand.getLocation (),
						ExternalError::get (NOT_A_LVALUE)
			    );
			}
		    }
		}
	    } else {	    	
		match (operand.to <Value> ().getType ()) {
		    of (Bool, b ATTRIBUTE_UNUSED,
			return validateBool (expression, operand);
		    );

		    of (Float, f ATTRIBUTE_UNUSED,
			return validateFloat (expression, operand);
		    );

		    of (Integer, i ATTRIBUTE_UNUSED,
			return validateInt (expression, operand);
		    );


		    of (Pointer, p ATTRIBUTE_UNUSED,
			return validatePointer (expression, operand);
		    );
		}
	    }
	    
	    UnaryVisitor::error (expression, operand);	
	    return Generator::empty ();	    
	}

	Generator UnaryVisitor::validateBool (const syntax::Unary & un, const Generator & operand) {
	    auto op = toOperator (un.getOperator ());
	    if (op == Unary::Operator::NOT)
		return UnaryBool::init (un.getLocation (), op, operand.to <Value> ().getType (), operand);
	    
	    error (un, operand);
	    return Generator::empty ();
	}       

	Generator UnaryVisitor::validateFloat (const syntax::Unary & un, const Generator & operand) {
	    auto op = toOperator (un.getOperator ());
	    if (op == Unary::Operator::MINUS)
		return UnaryFloat::init (un.getLocation (), op, operand.to <Value> ().getType (), operand);
	    
	    error (un, operand);
	    return Generator::empty ();
	}

       	Generator UnaryVisitor::validateInt (const syntax::Unary & un, const Generator & operand) {
	    auto op = toOperator (un.getOperator ());
	    if (op == Unary::Operator::MINUS)
		return UnaryInt::init (un.getLocation (), op, operand.to <Value> ().getType (), operand);
	    
	    error (un, operand);
	    return Generator::empty ();
	}

	Generator UnaryVisitor::validatePointer (const syntax::Unary & un, const Generator & operand) {
	    auto op = toOperator (un.getOperator ());
	    if (op == Unary::Operator::UNREF) {
		this-> _context.verifySafety (un.getLocation ());
		auto type = operand.to <Value> ().getType ().to <Type> ().getInners ()[0];
		if (!operand.to <Value> ().getType ().to <Type> ().isMutable ())
		    type.to<Type> ().isMutable (false);
		return UnaryPointer::init (un.getLocation (),
					   op,
					   type,
					   operand
		);
	    }

	    error (un, operand);
	    return Generator::empty ();
	}

	Generator UnaryVisitor::validateFunctionPointer (const syntax::Unary & un, const Generator & proto) {
	    auto params = proto.to <FrameProto> ().getParameters ();
	    auto ret = proto.to <FrameProto> ().getReturnType ();
	    std::vector <Generator> paramTypes;
	    for (auto & it : params) {
		paramTypes.push_back (it.to <generator::ProtoVar> ().getType ());
	    }
	    
	    auto funcType = FuncPtr::init (un.getLocation (), ret, paramTypes);
	    return Addresser::init (un.getLocation (), funcType, proto);
	}
	
	void UnaryVisitor::error (const syntax::Unary & un, const generator::Generator & left) {
	    std::string leftName;
	    match (left) {
		of (FrameProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())
		else of (MultSym,    sym,   leftName = sym.getLocation ().str)
		else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ());
	    }
	    
	    Ymir::Error::occur (un.getLocation (),
				ExternalError::get (UNDEFINED_UN_OP),
				un.getLocation ().str, 
				leftName
	    );
	}	

	Unary::Operator UnaryVisitor::toOperator (const lexing::Word & loc) {
	    string_match (loc.str) {
		eq (Token::NOT, return Unary::Operator::NOT;);
		eq (Token::MINUS, return Unary::Operator::MINUS;);
		eq (Token::STAR, return Unary::Operator::UNREF;);
		eq (Token::AND, return Unary::Operator::ADDR;);
	    }	    

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Unary::Operator::LAST_OP;
	}
	
    }
    
}
