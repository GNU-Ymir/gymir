#include <ymir/syntax/expression/ClassVar.hh>

namespace syntax {

    ClassVar::ClassVar () :
	IExpression (lexing::Word::eof ())
    {}
    
    ClassVar::ClassVar (const lexing::Word & loc) :
	IExpression (loc)
    {}
        
    Expression ClassVar::init (const lexing::Word & location) {
	auto ret = new (Z0) ClassVar (location);
	return Expression {ret};
    }

    Expression ClassVar::clone () const {
	return Expression {new ClassVar (*this)};
    }

    bool ClassVar::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	ClassVar thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void ClassVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<ClassVar> ", i, '\t');
	stream.writeln (this-> getLocation ());
    }

    std::string ClassVar::prettyString () const {
	return "class " + this-> getLocation ().str;
    }
}
