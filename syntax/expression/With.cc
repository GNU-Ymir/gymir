#include <ymir/syntax/expression/With.hh>

namespace syntax {

    With::With () :
	IExpression (lexing::Word::eof ()),
	_decls ({}),
	_content (Expression::empty ())
    {}    
    
    With::With (const lexing::Word & loc,  const std::vector <Expression> & decls, const Expression & content) :
	IExpression (loc),
	_decls (decls),
	_content (content)
    {}

    Expression With::init (const lexing::Word & location, const std::vector <Expression> & decls, const Expression & content) {
	return Expression {new (NO_GC) With (location, decls, content)};
    }

    void With::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<With>", i, '\t');	
	for (auto & it : this-> _decls)
	    it.treePrint (stream, i + 1);
	this-> _content.treePrint (stream, i + 2);
    }

    const std::vector <Expression> & With::getDecls () const {
	return this-> _decls;
    }

    const Expression & With::getContent () const {
	return this-> _content;
    }	
    
    std::string With::prettyString () const {
	Ymir::OutBuffer buf;
	int i = 0;
	buf.write ("with ");
	for (auto & it : this-> _decls) {
	    if (i != 0) buf.write (", ");
	    buf.write (it.prettyString ());
	    i += 1;
	}
	buf.write (this-> _content.prettyString ());
	return buf.str ();
    }

}
