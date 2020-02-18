#include <ymir/syntax/declaration/CondBlock.hh>

namespace syntax {

    CondBlock::CondBlock () :
	_token (lexing::Word::eof ()),
	_inner ({}),
	_else (Declaration::empty ()),
	_test (Expression::empty ())
    {}

    Declaration CondBlock::init (const CondBlock & decl) {
	auto ret = new (Z0) CondBlock (decl);	
	return Declaration {ret};
    }

    Declaration CondBlock::init (const lexing::Word & token, const Expression & test, const std::vector <Declaration> & content, const Declaration & else_) {
	auto ret = new (Z0) CondBlock ();
	ret-> _token = token;
	ret-> _inner = content;
	ret-> _else = else_;
	ret-> _test = test;
	
	return Declaration {ret};
    }

    Declaration CondBlock::clone () const {
	return CondBlock::init (*this);
    }
   
    bool CondBlock::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	CondBlock thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    
    
    void CondBlock::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<CondBlock> : ", this-> _token, " ");
	
	for (auto & it : this-> _inner) {
	    it.treePrint (stream, i + 1);
	}
    }
    
    const lexing::Word & CondBlock::getLocation () const {
	return this-> _token;
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
