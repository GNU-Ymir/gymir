#include <ymir/syntax/expression/Block.hh>

namespace syntax {

    Block::Block () :
	IExpression (lexing::Word::eof ()),
	_declModule (Declaration::empty ()),
	_catcher (Expression::empty ())
    {}
    
    Block::Block (const lexing::Word & loc) :
	IExpression (loc),
	_declModule (Declaration::empty ()),
	_catcher (Expression::empty ())
    {}

    Expression Block::init (const lexing::Word & location, const lexing::Word & end, const Declaration & declModule, const std::vector <Expression> & content, const Expression & catcher, const std::vector <Expression> & scopes) {
	auto ret = new (Z0) Block (location);
	ret-> _end = end;
	ret-> _content = content;
	ret-> _declModule = declModule;
	ret-> _catcher = catcher;
	ret-> _scopes = scopes;
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
	this-> _declModule.treePrint (stream, i + 1);
	
	for (auto & it : this-> _content)
	    it.treePrint (stream, i + 1);
    }
    
    const std::vector <Expression> & Block::getContent () const {
	return this-> _content;
    }

    const Declaration & Block::getDeclModule () const {
	return this-> _declModule;
    }
    
    const lexing::Word & Block::getEnd () const {
	return this-> _end;
    }

    const Expression & Block::getCatcher () const {
	return this-> _catcher;
    }

    const std::vector <Expression> & Block::getScopes () const {
	return this-> _scopes;
    }
    
}
