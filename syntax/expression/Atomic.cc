#include <ymir/syntax/expression/Atomic.hh>

namespace syntax {

    Atomic::Atomic () :
	IExpression (lexing::Word::eof ()),
	_who (Expression::empty ()),
	_content (Expression::empty ())
    {}
    
    Atomic::Atomic (const lexing::Word & loc, const Expression & who, const Expression & content) :
	IExpression (loc),
	_who (who),
	_content (content)
    {}

    Expression Atomic::init (const lexing::Word & location, const Expression & who, const Expression & content) {
	return Expression {new (NO_GC) Atomic (location, who, content)};
    }
    
    void Atomic::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Atomic>", i, '\t');
	this-> _who.treePrint (stream, i + 1);
	this-> _content.treePrint (stream, i + 1);
    }

    const Expression & Atomic::getWho () const {
	return this-> _who;
    }

    const Expression & Atomic::getContent () const {
	return this-> _content;
    }

    std::string Atomic::prettyString () const {
	return Ymir::format ("atomic (%) %", this-> _who.prettyString(), this-> _content.prettyString ());
    }
}
