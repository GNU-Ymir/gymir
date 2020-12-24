#include <ymir/syntax/declaration/Block.hh>

namespace syntax {

    DeclBlock::DeclBlock ()
	: IDeclaration (lexing::Word::eof ()),
	  _inner ({}),
	  _isPrivate (false),
	  _isProt (false)
    {}

    DeclBlock::DeclBlock (const lexing::Word & loc, const std::vector <Declaration> & content, bool isPrivate, bool isProt) : 
	IDeclaration (loc),
	_inner (content),
	_isPrivate (isPrivate),
	_isProt (isProt)
    {}
    
    Declaration DeclBlock::init (const DeclBlock & decl) {
	return Declaration {new (NO_GC) DeclBlock (decl)};
    }

    Declaration DeclBlock::init (const lexing::Word & token, const std::vector <Declaration> & content, bool isPrivate, bool isProt) {
	return Declaration {new (NO_GC) DeclBlock (token, content, isPrivate, isProt)};
    }
   
    bool DeclBlock::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	DeclBlock thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    
    
    void DeclBlock::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Block> : ", this-> getLocation (), " ", this-> _isPrivate ? "private" : "public");
	
	for (auto & it : this-> _inner) {
	    it.treePrint (stream, i + 1);
	}
    }
    
    bool DeclBlock::isPrivate () const {
	return this-> _isPrivate && !this-> _isProt;
    }

    bool DeclBlock::isProt () const {
	return this-> _isProt;
    }

    bool DeclBlock::isPublic () const {
	return !this-> _isPrivate && !this-> _isProt;
    }
    
    const std::vector <Declaration> & DeclBlock::getDeclarations () const {
	return this-> _inner;
    }
    
}
