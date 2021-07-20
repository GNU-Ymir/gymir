#include <ymir/semantic/generator/value/SliceCompare.hh>

namespace semantic {

    namespace generator {

	SliceCompare::SliceCompare () :
	    Value (),
	    _value (Generator::empty ()),
	    _left (Generator::empty ()),
	    _right (Generator::empty ()),
	    _operator (Binary::Operator::EQUAL)
	{
	}

	SliceCompare::SliceCompare (const lexing::Word & loc, Binary::Operator op, const Generator & type, const Generator & value, const Generator & left, const Generator & right) :
	    Value (loc, type),
	    _value (value),
	    _left (left),
	    _right (right),
	    _operator (op)
	{
	    auto vth = this-> _left.getThrowers ();
	    auto & ith = this-> _right.getThrowers ();
	    vth.insert (vth.end (), ith.begin (), ith.end ());
	    
	    this-> setThrowers (vth);
	}
	
	Generator SliceCompare::init (const lexing::Word & loc, Binary::Operator op, const Generator & type, const Generator & value, const Generator & left, const Generator & right) {
	    return Generator {new (NO_GC) SliceCompare (loc, op, type, value, left, right)};
	}
    
	Generator SliceCompare::clone () const {
	    return Generator {new (NO_GC) SliceCompare (*this)};
	}

	bool SliceCompare::equals (const Generator & gen) const {
	    if (!gen.is <SliceCompare> ()) return false;
	    auto bin = gen.to<SliceCompare> ();	    
	    return bin._left.equals (this-> _left) && bin._right.equals (this-> _right) && bin._value.equals (this-> _value);
	}

	Binary::Operator SliceCompare::getOperator () const {
	    return this-> _operator;
	}

	const Generator & SliceCompare::getValue () const {
	    return this-> _value;
	}
	
	const Generator & SliceCompare::getLeft () const {
	    return this-> _left;
	}

	const Generator & SliceCompare::getRight () const {
	    return this-> _right;
	}

	std::string SliceCompare::prettyString () const {
	    	    std::string op;
	    switch (this-> _operator) {
	    case Binary::Operator::OR : op = "||"; break;
	    case Binary::Operator::AND : op = "&&"; break;
	    case Binary::Operator::INF : op = "<"; break;
	    case Binary::Operator::SUP : op = ">"; break;
	    case Binary::Operator::INF_EQUAL : op = "<="; break;
	    case Binary::Operator::SUP_EQUAL : op = ">="; break;
	    case Binary::Operator::NOT_EQUAL : op = "!="; break;
	    case Binary::Operator::EQUAL : op = "=="; break;
	    case Binary::Operator::LEFT_SHIFT : op = "<<"; break;
	    case Binary::Operator::RIGHT_SHIFT : op = ">>"; break;
	    case Binary::Operator::BIT_OR : op = "|"; break;
	    case Binary::Operator::BIT_AND : op = "&"; break;
	    case Binary::Operator::BIT_XOR : op = "^"; break;
	    case Binary::Operator::ADD : op = "+"; break;
	    case Binary::Operator::CONCAT : op = "~"; break;
	    case Binary::Operator::SUB : op = "-"; break;
	    case Binary::Operator::MUL : op = "*"; break;
	    case Binary::Operator::DIV : op = "/"; break;
	    case Binary::Operator::MODULO : op = "%"; break;
	    case Binary::Operator::EXP : op = "^^"; break;
	    case Binary::Operator::RANGE : op = ".."; break;
	    case Binary::Operator::TRANGE : op = "..."; break;
	    case Binary::Operator::IS : op = "is"; break;
	    case Binary::Operator::NOT_IS : op = "!is"; break;
	    case Binary::Operator::IN : op = "in"; break;
	    case Binary::Operator::NOT_IN : op = "!in"; break;
	    default : op = "??"; break;
	    }

	    return Ymir::format ("(% % %)", this-> _left.prettyString (), op, this-> _right.prettyString ());
	}
    }
    
}
