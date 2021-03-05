#include <ymir/syntax/declaration/Trait.hh>

namespace syntax {

    Trait::Trait () :
	IDeclaration (lexing::Word::eof (), "")
    {}

    Trait::Trait (const lexing::Word & name, const std::string & comment, const std::vector <Declaration> & decls) :
	IDeclaration (name, comment),
	_inner (decls)
    {}
    
    Declaration Trait::init (const lexing::Word & name, const std::string & comment, const std::vector <Declaration> & decls) {
	return Declaration {new (NO_GC) Trait (name, comment, decls)};
    }
        
    void Trait::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Trait> ", i, '\t');
	stream.writeln (this-> getLocation ());

	for (auto & it : this-> _inner)
	    it.treePrint (stream, i + 1);
    }

    const std::vector<Declaration> & Trait::getDeclarations () const {
	return this-> _inner;
    }


    const std::set <std::string> & Trait::computeSubVarNames () {
	std::set <std::string> s;
	for (auto & it : this-> _inner) {
	    auto & iSet = it.getSubVarNames ();
	    s.insert (iSet.begin (), iSet.end ());
	}
	this-> setSubVarNames (s);
	return this-> getSubVarNames ();
    }
    
}
