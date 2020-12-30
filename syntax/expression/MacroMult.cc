#include <ymir/syntax/expression/MacroMult.hh>

namespace syntax {

    MacroMult::MacroMult () :
	IExpression (lexing::Word::eof ()),
	_end (lexing::Word::eof ()),
	_mult (lexing::Word::eof ())
    {}
    
    MacroMult::MacroMult (const lexing::Word & loc, const lexing::Word & end, const std::vector<Expression> & content, const lexing::Word & mult) :
	IExpression (loc),
	_end (end),
	_mult (mult),
	_content (content)
    {}
        
    Expression MacroMult::init (const lexing::Word & location, const lexing::Word & end, const std::vector<Expression> & content, const lexing::Word & mult) {
	return Expression {new (NO_GC) MacroMult (location, end, content, mult)};
    }

    bool MacroMult::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MacroMult thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void MacroMult::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<MacroMult> ", i, '\t');
	for (auto & it : this-> _content)
	    it.treePrint (stream, i + 1);
    }

    std::string MacroMult::prettyString () const {
	Ymir::OutBuffer buf;
	buf.write ("(");
	for (auto & it : this-> _content) {
	    buf.write (it.prettyString ());
	}
	buf.write (")");
	if (!this-> _mult.isEof ()) buf.write (this-> _mult.getStr ());
	return buf.str ();
    }

    const std::vector<Expression> & MacroMult::getContent () const {
	return this-> _content;
    }

    const lexing::Word & MacroMult::getMult () const {
	return this-> _mult;
    }

    const lexing::Word & MacroMult::getEnd () const {
	return this-> _end;
    }
}
