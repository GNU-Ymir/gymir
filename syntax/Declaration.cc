#include <ymir/syntax/Declaration.hh>

namespace syntax {
    
    IDeclaration::IDeclaration (const lexing::Word & location, const std::string & comment) :
	_location (location),
	_comments (comment)
    {}

    const lexing::Word & IDeclaration::getLocation () const {
	return this-> _location;
    }
    
    const std::string & IDeclaration::getComments () const {
	return this-> _comments;
    }

    void IDeclaration::setComments (const std::string & comments) {
	this-> _comments = comments;
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

    const std::string & Declaration::getComments () const {
	if (this-> _value == nullptr) {
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	}
	return this-> _value-> getComments ();
    }

    void Declaration::setComments (const std::string & comments) {
	if (this-> _value == nullptr) {
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	}
	this-> _value-> setComments (comments);
    }

    
    void Declaration::treePrint (Ymir::OutBuffer & stream, int i) const {	
	if (this-> _value == nullptr) {
	    stream.writef ("%*", i, '\t');
	    stream.writeln ("<null>");
	} else this-> _value-> treePrint (stream, i);	
    }
    
    IDeclaration::~IDeclaration () {}
    
}
