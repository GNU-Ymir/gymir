#include <ymir/syntax/Declaration.hh>

namespace syntax {

    bool IDeclaration::isOf (const IDeclaration*) const {
	return false; // IDeclaration is abstract, type cannot be of type IDeclaration
    }
    
    Declaration::Declaration (IDeclaration * decl) : Proxy<IDeclaration, Declaration> (decl) 
    {}

    Declaration Declaration::empty () {
	return Declaration {nullptr};
    }

    IDeclaration::~IDeclaration () {}
    
}
