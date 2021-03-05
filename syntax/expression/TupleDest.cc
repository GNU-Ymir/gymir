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
	return Expression {new (NO_GC) TupleDest (location, vars, value, isVariadic)};
    }

    void TupleDest::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<TupleDest>", i, '\t');
	stream.writeln (this-> _isVariadic ? "..." : "");
	stream.writefln ("%*<Params>", i + 1, '\t');
	for (auto & it : this-> _vars)
	    it.treePrint (stream, i + 2);
	this-> _value.treePrint (stream, i + 1);
    }

    const std::set <std::string> & TupleDest::computeSubVarNames () {
	auto vSet = this-> _value.getSubVarNames ();
	for (auto & it : this-> _vars) {
	    auto & iSet = it.getSubVarNames ();
	    vSet.insert (iSet.begin (), iSet.end ());
	}
	this-> setSubVarNames (vSet);
	return this-> getSubVarNames ();
    }
    
}
