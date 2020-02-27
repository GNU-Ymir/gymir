#include <ymir/syntax/Declaration.hh>

namespace syntax {
    
    IDeclaration::IDeclaration (const lexing::Word & location) :
	_location (location)
    {}

    const lexing::Word & IDeclaration::getLocation () const {
	return this-> _location;
    }
    
    bool IDeclaration::isOf (const IDeclaration*) const {
	return false; // IDeclaration is abstract, type cannot be of type IDeclaration
    }
    
    void IDeclaration::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<TODO>", i, '\t');
    }

    Declaration::Declaration (IDeclaration * decl) : RefProxy<IDeclaration, Declaration> (decl) 
    {}

    Declaration Declaration::empty () {
	return Declaration {nullptr};
    }

    bool Declaration::isEmpty () const {
	return this-> _value == nullptr;
    }

    const lexing::Word & Declaration::getLocation () const {
	if (this-> _value == nullptr) {
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));	    
	}
	return this-> _value-> getLocation ();
    }
    
    void Declaration::treePrint (Ymir::OutBuffer & stream, int i) const {	
	if (this-> _value == nullptr) {
	    stream.writef ("%*", i, '\t');
	    stream.writeln ("<null>");
	} else this-> _value-> treePrint (stream, i);	
    }
    
    IDeclaration::~IDeclaration () {}
    
}
