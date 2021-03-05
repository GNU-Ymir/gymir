#include <ymir/syntax/declaration/Macro.hh>

namespace syntax {

    Macro::Macro () :
	IDeclaration (lexing::Word::eof (), "")
    {}

    Macro::Macro (const lexing::Word & loc, const std::string & comment, const std::vector <Declaration> & content) :
	IDeclaration (loc, comment),
	_content (content)
    {}

    Declaration Macro::init (const lexing::Word & loc, const std::string & comment, const std::vector <Declaration> & content) {
	return Declaration {new (NO_GC) Macro (loc, comment,  content)};
    }

    void Macro::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Macro> : ", this-> getLocation ());
	for (auto & it : this-> _content) {
	    it.treePrint (stream, i + 1);
	}
    }

    const std::vector <Declaration> & Macro::getContent () const {
	return this-> _content;
    }
    
}
