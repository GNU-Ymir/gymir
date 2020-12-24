#include <ymir/syntax/declaration/Class.hh>

namespace syntax {

    Class::Class () :
	IDeclaration (lexing::Word::eof ()), 
	_over (Expression::empty ())
    {}
    
    Class::Class (const lexing::Word & name, const Expression & over, const std::vector <Declaration> & decls, const std::vector <lexing::Word> & attribts) :
	IDeclaration (name),
	_over (over),
	_innerDeclaration (decls),
	_attributes (attribts)
    {}
    
    Declaration Class::init (const Class & cl) {
	return Declaration {new (NO_GC) Class (cl)};
    }

    Declaration Class::init (const lexing::Word & name, const Expression & over, const std::vector <Declaration> & decls, const std::vector <lexing::Word> & attribs) {
	return Declaration {new (NO_GC) Class (name, over, decls, attribs)};
    }
   
    bool Class::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Class thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    const Expression & Class::getAncestor () const {
	return this-> _over;
    }

    const std::vector <Declaration> & Class::getDeclarations () const {
	return this-> _innerDeclaration;
    }

    const std::vector <lexing::Word> & Class::getAttributes () const {
	return this-> _attributes;
    }

    bool Class::isAbstract () const {
	for (auto & it : this-> _attributes) {
	    if (it == Keys::ABSTRACT) return true;
	}
	return false;
    }
    
}
