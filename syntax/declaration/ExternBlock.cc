#include <ymir/syntax/declaration/ExternBlock.hh>

namespace syntax {

    ExternBlock::ExternBlock () :
	IDeclaration (lexing::Word::eof (), ""), 
	_content (Declaration::empty ())
    {}

    ExternBlock::ExternBlock (const lexing::Word & location, const std::string & comment, const lexing::Word & from, const lexing::Word & space, const Declaration & content) :
	IDeclaration (location, comment),
	_from (from),
	_space (space),
	_content (content)
    {}

    Declaration ExternBlock::init (const lexing::Word & location, const std::string & comment, const lexing::Word & from, const lexing::Word & space, const Declaration & content) {
	return Declaration {new (NO_GC) ExternBlock (location, comment, from, space, content)};
    }

    bool ExternBlock::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	ExternBlock thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    void ExternBlock::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Extern> : ", this-> getLocation (), " ", this-> _from, " ", this-> _space);
	
	this-> _content.treePrint (stream, i + 1);
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
