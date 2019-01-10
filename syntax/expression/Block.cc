#include <ymir/syntax/expression/Block.hh>

namespace syntax {

    Block::Block () {}

    Expression Block::init (const Block & alloc) {
	auto ret = new (Z0) Block ();
	ret-> _begin = alloc._begin;
	ret-> _end = alloc._end;
	ret-> _content = alloc._content;
	return Expression {ret};
    }

    Expression Block::init (const lexing::Word & location, const lexing::Word & end, const std::vector <Expression> & content) {
	auto ret = new (Z0) Block ();
	ret-> _begin = location;
	ret-> _end = end;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression Block::clone () const {
	return Block::init (*this);
    }

    bool Block::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Block thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
