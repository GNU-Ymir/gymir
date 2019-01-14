#include <ymir/syntax/declaration/Block.hh>

namespace syntax {

    DeclBlock::DeclBlock () {}

    Declaration DeclBlock::init (const DeclBlock & decl) {
	auto ret = new (Z0) DeclBlock ();
	ret-> _token = decl._token;
	ret-> _inner = decl._inner;
	ret-> _isPrivate = decl._isPrivate;
	return Declaration {ret};
    }

    Declaration DeclBlock::init (const lexing::Word & token, const std::vector <Declaration> & content, bool isPrivate) {
	auto ret = new (Z0) DeclBlock ();
	ret-> _token = token;
	ret-> _inner = content;
	ret-> _isPrivate = isPrivate;
	return Declaration {ret};
    }

    Declaration DeclBlock::clone () const {
	return DeclBlock::init (*this);
    }
   
    bool DeclBlock::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	DeclBlock thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    
    
    void DeclBlock::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Block> : ", this-> _token, " ", this-> _isPrivate ? "private" : "public");
	
	for (auto & it : this-> _inner) {
	    it.treePrint (stream, i + 1);
	}
    }

      
}
