#include <ymir/syntax/expression/Block.hh>
#include <ymir/errors/_.hh>

namespace syntax {
    
    Block::Block (const lexing::Word & loc, const lexing::Word & end, const Declaration & declModule, const std::vector <Expression> & content, const Expression & catcher, const std::vector <Expression> & scopes) :
    	IExpression (loc),
	_end (end),
	_declModule (declModule),
	_catcher (catcher),
	_content (content),
	_scopes (scopes)
    {}

    Expression Block::init (const lexing::Word & location, const lexing::Word & end, const Declaration & declModule, const std::vector <Expression> & content, const Expression & catcher, const std::vector <Expression> & scopes) {
	return Expression {new (NO_GC) Block (location, end, declModule, content, catcher, scopes)};
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

    std::string Block::prettyString () const {
	if (!this-> _declModule.isEmpty ()) Ymir::Error::halt ("", ""); // TODO
	Ymir::OutBuffer buf;
	buf.write ("{\n");
	int i = 0;
	for (auto & it : this-> _content) {
	    if (i != 0) buf.write (";\n");
	    auto in = it.prettyString ();
	    buf.write ("\t");
	    for (auto & j : in) {
		buf.write (j);
		if (j == '\n') buf.write ("\t");
	    }
	    i += 1;
	}
	buf.write ("\n}");
	if (!this-> _catcher.isEmpty ()) {
	    buf.write (this-> _catcher.prettyString ());
	}

	for (auto & it : this-> _scopes) {
	    buf.write (it.prettyString ());
	}
	return buf.str ();
    }
    
}
