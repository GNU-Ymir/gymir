#include <ymir/syntax/declaration/Mixin.hh>

namespace syntax {

    Mixin::Mixin () :
	IDeclaration (lexing::Word::eof (), ""),
	_mixin (Expression::empty ())	
    {}

    Mixin::Mixin (const lexing::Word & loc, const std::string & comment, const Expression & mixin, const std::vector<Declaration> & decls) :
	IDeclaration (loc, comment),
	_mixin (mixin),
	_declarations (decls)
    {}
    

    Declaration Mixin::init (const lexing::Word & loc, const std::string & comment, const Expression & mixin, const std::vector<Declaration> & decls) {
	return Declaration {new (NO_GC) Mixin (loc, comment, mixin, decls)};
    }

    const Expression & Mixin::getMixin () const {
	return this-> _mixin;
    }
    
    const std::vector <Declaration> & Mixin::getDeclarations () const {
	return this-> _declarations;
    }
    
    void Mixin::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Mixin> ", i, '\t');
	this-> _mixin.treePrint (stream, i + 1);
    }
    
    const std::set <std::string> & Mixin::computeSubVarNames () {
	auto mSet = this-> _mixin.getSubVarNames ();
	for (auto & it : this-> _declarations) {
	    auto & iSet = it.getSubVarNames ();
	    mSet.insert (iSet.begin (), iSet.end ());
	}

	this-> setSubVarNames (mSet);
	return this-> getSubVarNames ();
    }
    
}
