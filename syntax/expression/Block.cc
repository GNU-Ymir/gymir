#include <ymir/syntax/expression/Block.hh>

namespace syntax {

    Block::Block () :
	IExpression (lexing::Word::eof ())
    {}
    
    Block::Block (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Block::init (const lexing::Word & location, const lexing::Word & end, const std::vector <Declaration> & decls, const std::vector <Expression> & content) {
	auto ret = new (Z0) Block (location);
	ret-> _end = end;
	ret-> _decls = decls;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression Block::clone () const {
	return Expression {new (Z0) Block (*this)};
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
    
    const std::vector <Expression> & Block::getContent () const {
	return this-> _content;
    }

    const lexing::Word & Block::getEnd () const {
	return this-> _end;
    }
    
}
