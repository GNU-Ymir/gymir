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
	    return syntax::Expression {new (NO_GC) TemplateSyntaxWrapper (lex, gen)};
	}

	void TemplateSyntaxWrapper::treePrint (Ymir::OutBuffer & stream, int i) const {
	    stream.writefln ("%*<TemplateSyntaxWrapper> ", i, '\t');
	    stream.writefln ("%*%", i, '\t', this-> _content.prettyString ());
	}

	const Generator & TemplateSyntaxWrapper::getContent() const {
	    return this-> _content;
	}

	std::string TemplateSyntaxWrapper::prettyString () const {
	    return this-> _content.prettyString ();
	}

	const std::set <std::string> & TemplateSyntaxWrapper::computeSubVarNames () {
	    this-> setSubVarNames ({});
	    return this-> getSubVarNames ();
	}
	
    }

}
