#include <ymir/syntax/expression/For.hh>

namespace syntax {

    For::For () :
	IExpression (lexing::Word::eof ()),
	_iter (Expression::empty ()),
	_block (Expression::empty ())
    {}
    
    For::For (const lexing::Word & loc) :
	IExpression (loc),
	_iter (Expression::empty ()),
	_block (Expression::empty ())
    {}


    Expression For::init (const lexing::Word & location, const std::vector <Expression> & vars, const Expression & iter, const Expression & block) {
	auto ret = new (Z0) For (location);
	ret-> _vars = vars;
	ret-> _iter = iter;
	ret-> _block = block;
	return Expression {ret};
    }

    Expression For::clone () const {
	return Expression {new (Z0) For (*this)};
    }

    bool For::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	For thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
    void For::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<For>", i, '\t');
	stream.writefln ("%*<Vars>", i + 1, '\t');
	for (auto & it : this-> _vars)
	    it.treePrint (stream, i + 2);

	stream.writefln ("%*<Iter>", i + 1, '\t');
	this-> _iter.treePrint (stream, i + 2);
	this-> _block.treePrint (stream, i + 1);
    }

    const Expression & For::getIter () const {
	return this-> _iter;
    }

    const Expression & For::getBlock () const {
	return this-> _block;
    }

    const std::vector <Expression> & For::getVars () const {
	return this-> _vars;
    }
    
}
