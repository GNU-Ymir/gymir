#include <ymir/syntax/expression/Try.hh>

namespace syntax {

    Try::Try () :
	IExpression (lexing::Word::eof ()),
	_inner (Expression::empty ())
    {}

    Try::Try (const lexing::Word & loc, const Expression & inner) :
	IExpression (loc),
	_inner (inner)
    {}

    Expression Try::init (const lexing::Word & loc, const Expression & inner) {
	return Expression {new (NO_GC) Try (loc, inner)};
    }

    void Try::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef("%*<Try> ", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _inner.treePrint (stream, i + 1);
    }

    std::string Try::prettyString () const {
	return Ymir::format ("(%)?", this-> _inner.prettyString ());
    }
    
    const syntax::Expression & Try::getInner () const {
	return this-> _inner;
    }
}
