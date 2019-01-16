#include <ymir/syntax/expression/Pragma.hh>

namespace syntax {

    Pragma::Pragma () {}

    Expression Pragma::init (const Pragma & alloc) {
	auto ret = new (Z0) Pragma ();
	ret-> _token = alloc._token;
	ret-> _params = alloc._params;
	return Expression {ret};
    }

    Expression Pragma::init (const lexing::Word & location, const std::vector <Expression> & params) {
	auto ret = new (Z0) Pragma ();
	ret-> _token = location;
	ret-> _params = params;
	return Expression {ret};
    }

    Expression Pragma::clone () const {
	return Pragma::init (*this);
    }

    bool Pragma::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Pragma thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Pragma::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Pragma> ", i, '\t');
	stream.writeln (this-> _token);
	for (auto & it : this-> _params)
	    it.treePrint (stream, i + 1);
    }
    
}
