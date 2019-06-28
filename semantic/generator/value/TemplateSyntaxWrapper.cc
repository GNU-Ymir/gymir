#include <ymir/semantic/generator/value/TemplateSyntaxWrapper.hh>

namespace semantic {

    namespace generator {

	TemplateSyntaxWrapper::TemplateSyntaxWrapper () :
	    syntax::IExpression (lexing::Word::eof ()),
	    _content (Generator::empty ())
	{}

	TemplateSyntaxWrapper::TemplateSyntaxWrapper (const lexing::Word & lex, const Generator & gen) :
	    syntax::IExpression (lex),
	    _content (gen)
	{}

	syntax::Expression TemplateSyntaxWrapper::init (const lexing::Word & lex, const Generator & gen) {
	    return syntax::Expression {new TemplateSyntaxWrapper (lex, gen)};
	}

	syntax::Expression TemplateSyntaxWrapper::clone () const {
	    return syntax::Expression {new TemplateSyntaxWrapper (*this)};
	}

	bool TemplateSyntaxWrapper::isOf (const syntax::IExpression * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    TemplateSyntaxWrapper thisType; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	    return IExpression::isOf (type);
	}

	void TemplateSyntaxWrapper::treePrint (Ymir::OutBuffer & stream, int i) const {
	    stream.writefln ("%<TemplateSyntaxWrapper> ", i, '\t');
	    stream.writeln ("%%", i, '\t', this-> _content.prettyString ());
	}

	const Generator & TemplateSyntaxWrapper::getContent() const {
	    return this-> _content;
	}
	
    }

}
