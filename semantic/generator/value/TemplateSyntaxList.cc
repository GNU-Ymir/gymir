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
	    return syntax::Expression {new TemplateSyntaxList (lex, contents)};
	}

	syntax::Expression TemplateSyntaxList::clone () const {
	    return syntax::Expression {new TemplateSyntaxList (*this)};
	}

	bool TemplateSyntaxList::isOf (const syntax::IExpression * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    TemplateSyntaxList thisType; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	    return IExpression::isOf (type);
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
