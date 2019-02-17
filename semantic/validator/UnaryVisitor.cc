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
	    auto operand = this-> _context.validateValue (expression.getContent ());	    
	    match (operand.to <Value> ().getType ()) {
		of (Bool, b ATTRIBUTE_UNUSED,
		    return validateBool (expression, operand);
		);

		// of (Char, c ATTRIBUTE_UNUSED,
		//     return validateChar (expression, operand);
		// );

		of (Float, f ATTRIBUTE_UNUSED,
		    return validateFloat (expression, operand);
		);

		of (Integer, i ATTRIBUTE_UNUSED,
		    return validateInt (expression, operand);
		);		
	    }

	    // auto op = toOperator (expression.getOperator ());
	    // if (op == Unary::Operand::ADDR) {
	    // 	// TODO pointers
	    // }
	    
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
	
	void UnaryVisitor::error (const syntax::Unary & un, const generator::Generator & operand) {
	    Ymir::Error::occur (un.getLocation (),
				ExternalError::get (UNDEFINED_UN_OP),
				operand.to <Value> ().getType ().to <Type> ().getTypeName ()
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
