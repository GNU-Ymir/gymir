#include <ymir/syntax/declaration/Class.hh>

namespace syntax {

    Class::Class () : _over (Expression::empty ()) {}
    
    Declaration Class::init () {
	return Declaration {new (Z0) Class ()};
    }

    Declaration Class::init (const Class & cl) {
	auto ret = new (Z0) Class ();
	ret-> _name = cl._name;
	ret-> _over = cl._over;
	ret-> _innerDeclaration = cl._innerDeclaration;
	return Declaration {ret};
    }

    Declaration Class::init (const lexing::Word & name, const Expression & over, const std::vector <Declaration> & decls) {
	auto ret = new (Z0) Class ();
	ret-> _name = name;
	ret-> _over = over;
	ret-> _innerDeclaration = decls;
	return Declaration {ret};
    }

    Declaration Class::clone () const {
	return Class::init (*this);
    }

    bool Class::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Class thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    const lexing::Word & Class::getName () const {
	return this-> _name;
    }

    const Expression & Class::getAncestor () const {
	return this-> _over;
    }

    const std::vector <Declaration> & Class::getDeclarations () const {
	return this-> _innerDeclaration;
    }
    
}
