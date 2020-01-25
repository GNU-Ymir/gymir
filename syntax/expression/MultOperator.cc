#include <ymir/syntax/expression/MultOperator.hh>

namespace syntax {

    MultOperator::MultOperator () :
	IExpression (lexing::Word::eof ()),
	_element (Expression::empty ())
    {}
    
    MultOperator::MultOperator (const lexing::Word & loc) :
	IExpression (loc),
	_element (Expression::empty ())
    {}

    Expression MultOperator::init (const lexing::Word & location, const lexing::Word & end, const Expression & element, const std::vector <Expression> & params, bool canbedotcall) {
	auto ret = new (Z0) MultOperator (location);
	ret-> _end = end;
	ret-> _element = element;
	ret-> _params = params;
	ret-> _canbedotCall = canbedotcall;
	return Expression {ret};
    }

    Expression MultOperator::clone () const {
	return Expression {new MultOperator (*this)};
    }

    bool MultOperator::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MultOperator thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void MultOperator::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<MultOperator> ", i, '\t');
	stream.writeln (this-> getLocation (), " ", this-> _end);
	this-> _element.treePrint (stream, i + 1);
	stream.writefln ("%*<Params> ", i + 1, '\t');
	for (auto & it : this-> _params)
	    it.treePrint (stream, i + 2);
    }

    const lexing::Word & MultOperator::getEnd () const {
	return this-> _end;
    }

    const Expression & MultOperator::getLeft () const {
	return this-> _element;
    }

    const std::vector <Expression> & MultOperator::getRights () const {
	return this-> _params;
    }

    bool MultOperator::canBeDotCall () const {
	return this-> _canbedotCall;
    }
    
}
