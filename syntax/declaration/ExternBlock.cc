#include <ymir/syntax/declaration/ExternBlock.hh>

namespace syntax {

    ExternBlock::ExternBlock () : _content (Declaration::empty ()) {}

    Declaration ExternBlock::init (const ExternBlock & decl) {
	auto ret = new (Z0) ExternBlock ();
	ret-> _location = decl._location;
	ret-> _from = decl._from;
	ret-> _space = decl._space;
	ret-> _content = decl._content;
	return Declaration {ret};
    }

    Declaration ExternBlock::init (const lexing::Word & location, const lexing::Word & from, const lexing::Word & space, const Declaration & content) {
	auto ret = new (Z0) ExternBlock ();
	ret-> _location = location;
	ret-> _from = from;
	ret-> _space = space;
	ret-> _content = content;
	return Declaration {ret};
    }

    Declaration ExternBlock::clone () const {
	return ExternBlock::init (*this);
    }
   
    bool ExternBlock::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	ExternBlock thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    void ExternBlock::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Extern> : ", this-> _location, " ", this-> _from, " ", this-> _space);
	
	this-> _content.treePrint (stream, i + 1);
    }

    const lexing::Word & ExternBlock::getLocation () const {
	return this-> _location;
    }
    
    const Declaration & ExternBlock::getDeclaration () const {
	return this-> _content;
    }

    const lexing::Word & ExternBlock::getSpace () const {
	return this-> _space;	
    }

    const lexing::Word & ExternBlock::getFrom () const {
	return this-> _from;
    }
    
}
