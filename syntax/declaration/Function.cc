#include <ymir/syntax/declaration/Function.hh>

namespace syntax {

    Function::Prototype::Prototype () :
	_retType (Expression::empty ())
    {}

    Function::Prototype::Prototype (const std::vector <Expression> & parameters, const Expression& retType, bool isVariadic) :
	_parameters (parameters),
	_retType (retType),
	_isVariadic (isVariadic)
    {}	

    Function::Prototype Function::Prototype::empty () {
	return Prototype ();
    }
    
    Function::Prototype Function::Prototype::init (const std::vector <Expression> & parameters, const Expression& retType, bool isVariadic) {
	return Prototype (parameters, retType, isVariadic);
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

    const std::vector <Expression> & Function::Prototype::getParameters () const {
	return this-> _parameters;
    }

    const Expression & Function::Prototype::getType () const {
	return this-> _retType;
    }

    bool Function::Prototype::isVariadic () const {
	return this-> _isVariadic;
    }
    
    
    Function::Function () :
	IDeclaration (lexing::Word::eof (), ""),
	_proto (Function::Prototype::empty ()),
	_body (Expression::empty ())
    {}

    Function::Function (const lexing::Word & name, const std::string & comment, const Prototype & proto, const Expression & body, const std::vector <lexing::Word> & cas, const std::vector <Expression> & throwers, bool isOver, bool isTest) :
	IDeclaration (name, comment),
	_proto (proto),
	_body (body),
	_cas (cas),
	_throwers (throwers),
	_isOver (isOver),
	_isTest (isTest)
    {}
    
    Declaration Function::init (const lexing::Word & name, const std::string & comment, const Prototype & proto, const Expression & body, const std::vector <lexing::Word> & cas, const std::vector <Expression> & throwers, bool isOver, bool isTest) {
	return Declaration {new (NO_GC) Function (name, comment, proto, body, cas, throwers, isOver, isTest)};
    }

    Declaration Function::init (const Function & other) {
	return Declaration {new (NO_GC) Function (other)};
    }

    void Function::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Function>", i, '\t');
	stream.writeln (this-> getLocation (), " @{", this-> _cas, "}");

	stream.writefln ("%*<Proto>", i + 1, '\t');	
	this-> _proto.treePrint (stream, i + 2);

	stream.writefln ("%*<Body>", i + 1, '\t');	
	this-> _body.treePrint (stream, i + 2);
    }
            
    const std::vector <lexing::Word> & Function::getCustomAttributes () const {
	return this-> _cas;
    }
    
    const Function::Prototype & Function::getPrototype () const {
	return this-> _proto;
    }
    
    const Expression & Function::getBody () const {
	return this-> _body;
    }

    bool Function::isOver () const {
	return this-> _isOver;
    }

    bool Function::isTest () const {
	return this-> _isTest;
    }

    const std::vector <syntax::Expression> & Function::getThrowers () const {
	return this-> _throwers;
    }
    
}
