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

    const std::set <std::string> & Function::Prototype::getSubVarNames () {
	if (!this-> _set_sub_var_names)
	    return this-> computeSubVarNames ();
	return this-> _sub_var_names;
    }
   
    const std::set <std::string> & Function::Prototype::computeSubVarNames () {
	this-> _set_sub_var_names = true;
	auto rSet = this-> _retType.getSubVarNames ();
	for (auto & it : this-> _parameters) {
	    auto & iSet = it.getSubVarNames ();
	    rSet.insert (iSet.begin (), iSet.end ());
	}

	this-> _sub_var_names = rSet;
	return this-> _sub_var_names;
    }        
    
    Function::Function () :
	IDeclaration (lexing::Word::eof (), ""),
	_proto (Function::Prototype::empty ()),
	_body (Expression::empty ())
    {}

    Function::Function (const lexing::Word & name, const std::string & comment, const Prototype & proto, const Expression & body, const std::vector <lexing::Word> & cas, const std::vector <Expression> & throwers, bool isOver) :
	IDeclaration (name, comment),
	_proto (proto),
	_body (body),
	_cas (cas),
	_throwers (throwers),
	_isOver (isOver)
    {}
    
    Declaration Function::init (const lexing::Word & name, const std::string & comment, const Prototype & proto, const Expression & body, const std::vector <lexing::Word> & cas, const std::vector <Expression> & throwers, bool isOver) {
	return Declaration {new (NO_GC) Function (name, comment, proto, body, cas, throwers, isOver)};
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

    const std::vector <syntax::Expression> & Function::getThrowers () const {
	return this-> _throwers;
    }

    const std::set <std::string> & Function::computeSubVarNames () {
	auto pSet = this-> _proto.getSubVarNames ();
	auto & bSet = this-> _body.getSubVarNames ();
	pSet.insert (bSet.begin (), bSet.end ());
	for (auto & it : this-> _throwers) {
	    auto & iSet = it.getSubVarNames ();
	    pSet.insert (iSet.begin (), iSet.end ());
	}
	this-> setSubVarNames (pSet);
	return this-> getSubVarNames ();
    }
    
}
