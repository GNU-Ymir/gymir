#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {

	Binary::Binary () :
	    Value (),
	    _left (Generator::empty ()),
	    _right (Generator::empty ())
	{}

	Binary::Binary (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) :
	    Value (loc, type),
	    _left (left),
	    _right (right),
	    _operator (op)	    
	{}
	
	Generator Binary::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) {
	    return Generator {new Binary (loc, op, type, left, right)};
	}
    
	Generator Binary::clone () const {
	    return Generator {new (Z0) Binary (*this)};
	}

	bool Binary::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Binary thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Binary::equals (const Generator & gen) const {
	    if (!gen.is <Binary> ()) return false;
	    auto bin = gen.to<Binary> ();	    
	    return this-> _left.equals (bin._left) &&
		this-> _right.equals (bin._right) &&
		this-> _operator == bin._operator;		
	}

	const Generator & Binary::getLeft () const {
	    return this-> _left;
	}

	const Generator & Binary::getRight () const {
	    return this-> _right;
	}

	Binary::Operator Binary::getOperator () const {
	    return this-> _operator;
	}

	std::string Binary::prettyString () const {
	    std::string op;
	    switch (this-> _operator) {
	    case Operator::OR : op = "||"; break;
	    case Operator::AND : op = "&&"; break;
	    case Operator::INF : op = "<"; break;
	    case Operator::SUP : op = ">"; break;
	    case Operator::INF_EQUAL : op = "<="; break;
	    case Operator::SUP_EQUAL : op = ">="; break;
	    case Operator::NOT_EQUAL : op = "!="; break;
	    case Operator::EQUAL : op = "=="; break;
	    case Operator::LEFT_SHIFT : op = "<<"; break;
	    case Operator::RIGHT_SHIFT : op = ">>"; break;
	    case Operator::BIT_OR : op = "|"; break;
	    case Operator::BIT_AND : op = "&"; break;
	    case Operator::BIT_XOR : op = "^"; break;
	    case Operator::ADD : op = "+"; break;
	    case Operator::CONCAT : op = "~"; break;
	    case Operator::SUB : op = "-"; break;
	    case Operator::MUL : op = "*"; break;
	    case Operator::DIV : op = "/"; break;
	    case Operator::MODULO : op = "%"; break;
	    case Operator::EXP : op = "^^"; break;
	    case Operator::RANGE : op = ".."; break;
	    case Operator::TRANGE : op = "..."; break;
	    case Operator::IS : op = "is"; break;
	    case Operator::NOT_IS : op = "!is"; break;
	    default : op = "??"; break;
	    }
	    
	    return Ymir::format ("(% % %)", this-> _left.prettyString (), op, this-> _right.prettyString ());
	}
	
    }
    
}
