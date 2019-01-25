#include <ymir/syntax/expression/Cast.hh>

namespace syntax {

    Cast::Cast () :
	IExpression (lexing::Word::eof ()),
	_type (Expression::empty ()),
	_content (Expression::empty ())
    {}
    
    Cast::Cast (const lexing::Word & loc) :
	IExpression (loc),
	_type (Expression::empty ()),
	_content (Expression::empty ())
    {}

    Expression Cast::init (const lexing::Word & location, const Expression & type, const Expression & content) {
	auto ret = new (Z0) Cast (location);
	ret-> _type = type;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression Cast::clone () const {
	return Expression {new (Z0) Cast (*this)};
    }

    bool Cast::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Cast thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
    void Cast::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Cast>", i, '\t');
	this-> _type.treePrint (stream, i + 1);
	this-> _content.treePrint (stream, i + 1);
    }
}
