#include <ymir/syntax/declaration/CondBlock.hh>

namespace syntax {

    CondBlock::CondBlock () :	
	IDeclaration (lexing::Word::eof (), ""),
	_inner ({}),
	_else (Declaration::empty ()),
	_test (Expression::empty ())
    {}

    CondBlock::CondBlock (const lexing::Word & token, const std::string & comment, const Expression & test, const std::vector <Declaration> & content, const Declaration & else_) :
	IDeclaration (token, comment),
	_inner (content),
	_else (else_),
	_test (test)
    {}    
	
    
    Declaration CondBlock::init (const CondBlock & decl) {
	return Declaration {new (NO_GC) CondBlock (decl)};
    }

    Declaration CondBlock::init (const lexing::Word & token, const std::string & comment, const Expression & test, const std::vector <Declaration> & content, const Declaration & else_) {	
	return Declaration {new (NO_GC) CondBlock (token, comment, test, content, else_)};
    }
    
    void CondBlock::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<CondBlock> : ", this-> getLocation (), " ");
	
	for (auto & it : this-> _inner) {
	    it.treePrint (stream, i + 1);
	}
    }
    

    const std::vector <Declaration> & CondBlock::getDeclarations () const {
	return this-> _inner;
    }

    const Expression & CondBlock::getTest () const {
	return this-> _test;
    }

    const Declaration & CondBlock::getElse () const {
	return this-> _else;
    }


    const std::set <std::string> & CondBlock::computeSubVarNames () {
	auto eSet = this-> _else.getSubVarNames ();
	auto & tSet = this-> _test.getSubVarNames ();
	eSet.insert (tSet.begin (), tSet.end ());
	for (auto & it : this-> _inner) {
	    auto & iSet = it.getSubVarNames ();
	    eSet.insert (iSet.begin (), iSet.end ());
	}
	
	this-> setSubVarNames (eSet);
	return this-> getSubVarNames ();
    }
    
}
