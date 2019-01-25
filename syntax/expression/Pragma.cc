#include <ymir/syntax/expression/Pragma.hh>

namespace syntax {

    Pragma::Pragma () :
	IExpression (lexing::Word::eof ())
    {}
    
    Pragma::Pragma (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Pragma::init (const lexing::Word & location, const std::vector <Expression> & params) {
	auto ret = new (Z0) Pragma (location);
	ret-> _params = params;
	return Expression {ret};
    }

    Expression Pragma::clone () const {
	return Expression {new Pragma (*this)};
    }

    bool Pragma::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Pragma thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Pragma::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Pragma> ", i, '\t');
	stream.writeln (this-> getLocation ());
	for (auto & it : this-> _params)
	    it.treePrint (stream, i + 1);
    }
    
}
