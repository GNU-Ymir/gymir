#include <ymir/syntax/Declaration.hh>

namespace syntax {

    bool IDeclaration::isOf (const IDeclaration*) const {
	return false; // IDeclaration is abstract, type cannot be of type IDeclaration
    }
    
    void IDeclaration::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<TODO>", i, '\t');
    }

    Declaration::Declaration (IDeclaration * decl) : Proxy<IDeclaration, Declaration> (decl) 
    {}

    Declaration Declaration::empty () {
	return Declaration {nullptr};
    }

    void Declaration::treePrint (Ymir::OutBuffer & stream, int i) const {	
	if (this-> _value == NULL) {
	    stream.writef ("%*", i, '\t');
	    stream.writeln ("<null>");
	} else this-> _value-> treePrint (stream, i);
    }
    
    IDeclaration::~IDeclaration () {}
    
}
