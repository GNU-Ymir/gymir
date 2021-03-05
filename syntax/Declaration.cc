#include <ymir/syntax/Declaration.hh>

namespace syntax {

    std::set <std::string> Declaration::__empty_sub_names__;
    
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

    const std::set <std::string> & IDeclaration::getSubVarNames () const {
	return this-> _sub_var_names;
    }

    void IDeclaration::setSubVarNames (const std::set <std::string> & s) {
	this-> _set_sub_var_names = true;
	this-> _sub_var_names = s;
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

    const std::set <std::string> & Declaration::getSubVarNames () const {
	if (this-> _value == nullptr) {
	    return __empty_sub_names__;
	}
	if (this-> _value-> _set_sub_var_names) 
	    return this-> _value-> getSubVarNames ();
	else {
	    auto aux = *this;
	    std::set <std::string> names;
	    return aux._value-> computeSubVarNames ();
	}
    }
    
    const std::set <std::string> & Declaration::computeSubVarNames () {
	if (this-> _value != nullptr)
	    return this-> _value-> computeSubVarNames ();
	else
	    return __empty_sub_names__;
    }

    IDeclaration::~IDeclaration () {}
    
}
