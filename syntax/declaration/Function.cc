#include <ymir/syntax/declaration/Function.hh>

namespace syntax {

    Function::Prototype::Prototype () : _retType (Expression::empty ())
    {}

    Function::Prototype Function::Prototype::init () {
	return Prototype {};
    }
    
    Function::Prototype Function::Prototype::init (const Function::Prototype& proto) {
	Prototype ret;
	ret._parameters = proto._parameters;
	ret._retType = proto._retType;
	return ret;
    }

    Function::Prototype Function::Prototype::init (const std::vector <Expression> & parameters, const Expression& retType) {
	Prototype ret;
	ret._parameters = parameters;
	ret._retType = retType;
	return ret;
    }

    Function::Prototype Function::Prototype::clone () const {
	return Function::Prototype::init (*this);
    }
    
    void Function::Prototype::addParameter (const Expression & param) {
	this-> _parameters.push_back (param);
    }

    void Function::Prototype::setType (const Expression & type) {
	this-> _retType = type;
    }

    Function::Body::Body () :
	_body (Expression::empty ()),
	_inner (Expression::empty ()),
	_outer (Expression::empty ())
    {}

    Function::Body Function::Body::init () {
	return Function::Body {};
    }

    Function::Body Function::Body::init (const Expression & body) {
	Function::Body ret;
	ret._body = body;
	return ret;
    }

    Function::Body Function::Body::init (const Function::Body& body) {
	Function::Body ret;
	ret._body = body._body;
	ret._inner = body._inner;
	ret._outer = body._outer;
	ret._outerName = body._outerName;
	return ret;
    }

    Function::Body Function::Body::init (const Expression & inner, const Expression & body, const Expression & outer, const lexing::Word & name) {
	Function::Body ret;
	ret._inner = inner;
	ret._body = body;
	ret._outer = outer;
	ret._outerName = name;
	return ret;	
    }

    Function::Body Function::Body::clone () const {
	return Function::Body::init (*this);
    }

    void Function::Body::setBody (const Expression & body) {
	this-> _body = body;
    }

    void Function::Body::setInner (const Expression & inner) {
	this-> _inner = inner;
    }

    void Function::Body::setOuter (const Expression & outer, const lexing::Word & name) {
	this-> _outer = outer;
	this-> _outerName = name;
    }
    
    Function::Function () :
	_test (Expression::empty ()),
	_proto (Function::Prototype::init ()),
	_body (Function::Body::init ())
    {}

    Declaration Function::init () {
	return Declaration {new (Z0) Function ()};
    }

    Declaration Function::init (const Function & func) {
	auto ret = new (Z0) Function ();
	ret-> _proto = func._proto;
	ret-> _body = func._body;
	ret-> _cas = func._cas;
	ret-> _name = func._name;
	return Declaration {ret};
    }

    Declaration Function::init (const lexing::Word & name, const Prototype & proto, const Body & body) {
	auto ret = new (Z0) Function ();
	ret-> _proto = proto;
	ret-> _body = body;
	ret-> _name = name;
	return Declaration {ret};
    }

    Declaration Function::clone () const {
	return Function::init (*this);
    }
    
    bool Function::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast<const void* const*> (type) [0];
	Function thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast<const void* const*> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    void Function::addCustomAttribute (const lexing::Word & ca) {
	this-> _cas.push_back (ca);
    }

    void Function::setCustomAttributes (const std::vector <lexing::Word> & cas) {
	this-> _cas = cas;
    }
    
    void Function::setName (const lexing::Word & name) {
	this-> _name = name;
    }

    void Function::setPrototype (const Prototype & proto) {
	this-> _proto = proto;
    }

    void Function::setBody (const Body & body) {
	this-> _body = body;
    }

}
