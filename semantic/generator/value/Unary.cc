#include <ymir/semantic/generator/value/Unary.hh>

namespace semantic {

    namespace generator {

	Unary::Unary () :
	    Value (),
	    _operand (Generator::empty ())
	{}

	Unary::Unary (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand) :
	    Value (loc, type),
	    _operand (operand),
	    _operator (op)	    
	{
	    this-> setThrowers (this-> _operand.getThrowers ());
	}
	
	Generator Unary::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand) {
	    return Generator {new (NO_GC) Unary (loc, op, type, operand)};
	}
    
	Generator Unary::clone () const {
	    return Generator {new (NO_GC) Unary (*this)};
	}

	bool Unary::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Unary thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Unary::equals (const Generator & gen) const {
	    if (!gen.is <Unary> ()) return false;
	    auto bin = gen.to<Unary> ();	    
	    return this-> _operand.equals (bin._operand) &&
		this-> _operator == bin._operator;		
	}

	const Generator & Unary::getOperand () const {
	    return this-> _operand;
	}

	Unary::Operator Unary::getOperator () const {
	    return this-> _operator;
	}

	std::string Unary::prettyString () const {
	    std::string op;
	    switch (this-> _operator) {
	    case Operator::NOT : op = "!"; break;
	    case Operator::MINUS : op = "-"; break;
	    case Operator::UNREF : op = "*"; break;
	    case Operator::ADDR : op = "&"; break;
	    default : op = "??"; break;
	    }

	    return Ymir::format ("% (%)", op, this-> _operand.prettyString ());
	}
	
    }
    
}
