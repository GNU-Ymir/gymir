#include <ymir/semantic/generator/value/TemplateSyntaxList.hh>

namespace semantic {

    namespace generator {

	TemplateSyntaxList::TemplateSyntaxList () :
	    syntax::IExpression (lexing::Word::eof ())
	{}

	TemplateSyntaxList::TemplateSyntaxList (const lexing::Word & lex, const std::vector <Generator> & contents) :
	    syntax::IExpression (lex),
	    _contents (contents)
	{}

	syntax::Expression TemplateSyntaxList::init (const lexing::Word & lex, const std::vector <Generator> & contents) {
	    return syntax::Expression {new (NO_GC) TemplateSyntaxList (lex, contents)};
	}

	void TemplateSyntaxList::treePrint (Ymir::OutBuffer & stream, int i) const {
	    stream.writefln ("%<TemplateSyntaxList> ", i, '\t');
	    for (auto & it : this-> _contents)
		stream.writefln ("%%", i + 1, '\t', it.prettyString ());
	}

	const std::vector<Generator> & TemplateSyntaxList::getContents () const {
	    return this-> _contents;
	}

	std::string TemplateSyntaxList::prettyString () const {
	    Ymir::OutBuffer buf;
	    buf.write ("{");
	    int i = 0;
	    for (auto & it : this-> _contents) {
		if (i != 0) buf.write (", ");
		buf.write (it.prettyString ());
		i += 1;
	    }
	    buf.write ("}");
	    return buf.str ();
	}
	
    }

}
