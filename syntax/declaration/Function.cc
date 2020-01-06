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
	ret._isVariadic = false;
	return ret;
    }

    Function::Prototype Function::Prototype::init (const std::vector <Expression> & parameters, const Expression& retType, bool isVariadic) {
	Prototype ret;
	ret._parameters = parameters;
	ret._retType = retType;
	ret._isVariadic = isVariadic;
	return ret;
    }

    Function::Prototype Function::Prototype::clone () const {
	return Function::Prototype::init (*this);
    }
    
    void Function::Prototype::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Parameters> : ");
	for (auto & it : this->_parameters)
	    it.treePrint (stream, i + 1);

	stream.writef ("%*", i, '\t');
	stream.writeln ("<Return> : ");
	this-> _retType.treePrint (stream, i + 1);
    }

    void Function::Prototype::addParameter (const Expression & param) {
	this-> _parameters.push_back (param);
    }

    const std::vector <Expression> & Function::Prototype::getParameters () const {
	return this-> _parameters;
    }

    const Expression & Function::Prototype::getType () const {
	return this-> _retType;
    }
    
    void Function::Prototype::setType (const Expression & type) {
	this-> _retType = type;
    }

    void Function::Prototype::isVariadic (bool is) {
	this-> _isVariadic = is;
    }

    bool Function::Prototype::isVariadic () const {
	return this-> _isVariadic;
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

    void Function::Body::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<In>", i, '\t');
	this-> _inner.treePrint (stream, i + 1);
	stream.writefln ("%*<Out>", i, '\t');
	this-> _outer.treePrint (stream, i + 1);
	stream.writefln ("%*<Body>", i, '\t');
	this-> _body.treePrint (stream, i + 1);
    }
    
    void Function::Body::setBody (const Expression & body) {
	this-> _body = body;
    }

    const Expression & Function::Body::getBody () const {
	return this-> _body;
    }
    
    void Function::Body::setInner (const Expression & inner) {
	this-> _inner = inner;
    }

    const Expression & Function::Body::getInner () const {
	return this-> _inner;
    }
    
    void Function::Body::setOuter (const Expression & outer, const lexing::Word & name) {
	this-> _outer = outer;
	this-> _outerName = name;
    }

    const Expression & Function::Body::getOuter () const {
	return this-> _outer;
    }

    const lexing::Word & Function::Body::getOuterName () const {
	return this-> _outerName;
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
	auto ret = new (Z0) Function (func);
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

    void Function::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Function>", i, '\t');
	stream.writeln (this-> _name, " @{", this-> _cas, "}");

	stream.writefln ("%*<Test>", i + 1, '\t');	
	this-> _test.treePrint (stream, i + 2);

	stream.writefln ("%*<Proto>", i + 1, '\t');	
	this-> _proto.treePrint (stream, i + 2);

	stream.writefln ("%*<Body>", i + 1, '\t');	
	this-> _body.treePrint (stream, i + 2);
    }
    
    bool Function::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast<const void* const*> (type) [0];
	Function thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast<const void* const*> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    const lexing::Word & Function::getName () const {
	return this-> _name;
    }
    
    void Function::addCustomAttribute (const lexing::Word & ca) {
	this-> _cas.push_back (ca);
    }

    void Function::setCustomAttributes (const std::vector <lexing::Word> & cas) {
	this-> _cas = cas;
    }
    
    const std::vector <lexing::Word> & Function::getCustomAttributes () const {
	return this-> _cas;
    }

    void Function::setName (const lexing::Word & name) {
	this-> _name = name;
    }

    void Function::setPrototype (const Prototype & proto) {
	this-> _proto = proto;
    }

    const Function::Prototype & Function::getPrototype () const {
	return this-> _proto;
    }
    
    void Function::setBody (const Body & body) {
	this-> _body = body;
    }

    const Function::Body & Function::getBody () const {
	return this-> _body;
    }

    void Function::setWeak () {
	this-> _isWeak = true;
    }

    bool Function::isWeak () const {
	return this-> _isWeak;
    }

    void Function::setOver () {
	this-> _isOver = true;
    }

    bool Function::isOver () const {
	return this-> _isOver;
    }
    
}
