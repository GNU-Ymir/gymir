#include <ymir/syntax/expression/TupleDest.hh>

namespace syntax {

    TupleDest::TupleDest () :
	IExpression (lexing::Word::eof ()),
	_value (Expression::empty ())
    {}
    
    TupleDest::TupleDest (const lexing::Word & loc, const std::vector <Expression> & vars, const Expression & value, bool isVariadic) :
	IExpression (loc),
	_vars (vars),
	_value (value),
	_isVariadic (isVariadic)
    {}
    
    Expression TupleDest::init (const lexing::Word & location, const std::vector <Expression> & vars, const Expression & value, bool isVariadic) {
	return Expression {new (Z0) TupleDest (location, vars, value, isVariadic)};
    }

    Expression TupleDest::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	TupleDest thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }    

    void TupleDest::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<TupleDest>", i, '\t');
	stream.writeln (this-> _isVariadic ? "..." : "");
	stream.writefln ("%*<Params>", i + 1, '\t');
	for (auto & it : this-> _vars)
	    it.treePrint (stream, i + 2);
	this-> _value.treePrint (stream, i + 1);
    }
    
}
