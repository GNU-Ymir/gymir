#include <ymir/syntax/declaration/Template.hh>

namespace syntax {

    Template::Template () : _content (Declaration::empty ())
    {}
    
    Declaration Template::init () {
	return Declaration {new (Z0) Template ()};
    }

    Declaration Template::init (const Template & tmpl) {
	auto ret = new (Z0) Template ();
	ret-> _parameters = tmpl._parameters;
	ret-> _content = tmpl._content;
	return Declaration {ret};
    }

    Declaration Template::init (const std::vector <Expression> & params, const Declaration & content) {
	auto ret = new (Z0) Template ();
	ret-> _parameters = params;
	ret-> _content = content;
	return Declaration {ret};
    }

    Declaration Template::clone () const {
	return Template::init (*this);
    }
    
    bool Template::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Template thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    void Template::addParameter (const Expression & param) {
	this-> _parameters.push_back (param);
    }

    void Template::setContent (const Declaration& content) {
	this-> _content = content;
    }
    
    
}
