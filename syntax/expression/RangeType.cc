#include <ymir/syntax/expression/RangeType.hh>
#include <ymir/lexing/Token.hh>

namespace syntax {

    RangeType::RangeType (const lexing::Word & loc, const Expression & type) :
	IExpression (loc),
	_type (type)
    {}

    Expression RangeType::init (const lexing::Word & loc, const Expression & type) {
	return Expression {new (NO_GC) RangeType (loc, type)};
    }

    Expression RangeType::init (const RangeType & other) {
	return Expression {new (NO_GC) RangeType (other)};
    }

    void RangeType::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<RangeType> ", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _type.treePrint (stream, i + 1);
    }

    const Expression & RangeType::getType () const {
	return this-> _type;
    }

    std::string RangeType::prettyString () const {
	Ymir::OutBuffer buf;
	buf.write (Token::LPAR, Token::DDOT);
	buf.write (this-> _type.prettyString ());
	buf.write (Token::RPAR);

	return buf.str ();
    }
    
    
}
