#include <ymir/syntax/expression/Set.hh>

namespace syntax {

    Set::Set () :
	IExpression (lexing::Word::eof ())
    {}    
    
    Set::Set (const lexing::Word & loc,  const std::vector <Expression> & params) :
	IExpression (loc),
	_params (params)
    {}

    Expression Set::init (const lexing::Word & location, const std::vector <Expression> & params) {
	return Expression {new (Z0) Set (location, params)};
    }

    bool Set::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Set thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Set::treePrint (Ymir::OutBuffer & stream, int i) const {
	for (auto & it : this-> _params)
	    it.treePrint (stream, i);
    }

    const std::vector <Expression> Set::getContent () const {
	return this-> _params;
    }
    
}
