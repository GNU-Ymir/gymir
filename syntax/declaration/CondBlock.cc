#include <ymir/syntax/declaration/CondBlock.hh>

namespace syntax {

    CondBlock::CondBlock () :	
	IDeclaration (lexing::Word::eof ()),
	_inner ({}),
	_else (Declaration::empty ()),
	_test (Expression::empty ())
    {}

    CondBlock::CondBlock (const lexing::Word & token, const Expression & test, const std::vector <Declaration> & content, const Declaration & else_) :
	IDeclaration (token),
	_inner (content),
	_else (else_),
	_test (test)
    {}    
	
    
    Declaration CondBlock::init (const CondBlock & decl) {
	return Declaration {new (Z0) CondBlock (decl)};
    }

    Declaration CondBlock::init (const lexing::Word & token, const Expression & test, const std::vector <Declaration> & content, const Declaration & else_) {	
	return Declaration {new (Z0) CondBlock (token, test, content, else_)};
    }

    bool CondBlock::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	CondBlock thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
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
    
}
