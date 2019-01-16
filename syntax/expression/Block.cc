#include <ymir/syntax/expression/Block.hh>

namespace syntax {

    Block::Block () {}

    Expression Block::init (const Block & alloc) {
	auto ret = new (Z0) Block ();
	ret-> _begin = alloc._begin;
	ret-> _end = alloc._end;
	ret-> _decls = alloc._decls;
	ret-> _content = alloc._content;
	return Expression {ret};
    }

    Expression Block::init (const lexing::Word & location, const lexing::Word & end, const std::vector <Declaration> & decls, const std::vector <Expression> & content) {
	auto ret = new (Z0) Block ();
	ret-> _begin = location;
	ret-> _end = end;
	ret-> _decls = decls;
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

    void Block::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Block>", i, '\t');
	for (auto & it : this-> _decls)
	    it.treePrint (stream, i + 1);
	
	for (auto & it : this-> _content)
	    it.treePrint (stream, i + 1);
    }
    
}
