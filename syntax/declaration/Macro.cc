#include <ymir/syntax/declaration/Macro.hh>

namespace syntax {

    Macro::Macro () :
	IDeclaration (lexing::Word::eof ())
    {}

    Macro::Macro (const lexing::Word & loc, const std::vector<std::string> & skips, const std::vector <Declaration> & content) :
	IDeclaration (loc),
	_skips (skips),
	_content (content)
    {}

    Declaration Macro::init (const lexing::Word & loc, const std::vector<std::string> & skips, const std::vector <Declaration> & content) {
	return Declaration {new (Z0) Macro (loc, skips, content)};
    }

    bool Macro::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Macro thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    void Macro::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Macro> : ", this-> getLocation ());
	for (auto & it : this-> _content) {
	    it.treePrint (stream, i + 1);
	}
    }

    const std::vector <std::string> & Macro::getSkips () const {
	return this-> _skips;
    }

    const std::vector <Declaration> & Macro::getContent () const {
	return this-> _content;
    }
    
}
