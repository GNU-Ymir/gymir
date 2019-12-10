#include <ymir/syntax/expression/Scope.hh>
#include <ymir/syntax/visitor/Keys.hh>

namespace syntax {

    Scope::Scope (const lexing::Word& loc) :
	IExpression (loc),
	_content (Expression::empty ())
    {}

    Scope::Scope () :
	IExpression (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}
    
    Expression Scope::init (const lexing::Word & location, const Expression & content) {
	auto ret = new (Z0) Scope (location);
	ret-> _content = content;
	return Expression {ret};
    }

    Expression Scope::clone () const {
	return Expression {new Scope (*this)};
    }

    bool Scope::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Scope thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    bool Scope::isExit () const {
	return this-> getLocation ().str == Keys::EXIT;
    }

    bool Scope::isSuccess () const {
	return this-> getLocation ().str == Keys::SUCCESS;
    }

    bool Scope::isFailure () const {
	return this-> getLocation ().str == Keys::FAILURE;
    }

    void Scope::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Scope> ", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _content.treePrint (stream, i + 1);
    }    

    const Expression& Scope::getContent () const {
	return this-> _content;
    }
    

}
