#include <ymir/syntax/declaration/Constructor.hh>


namespace syntax {
    

    Constructor::Constructor () :
	_name (lexing::Word::eof ()),
	_proto (Function::Prototype::init ()),
	_superParams ({}),
	_construction ({}),
	_body (Expression::empty ()),
	_explicitSuperCall (lexing::Word::eof ()),
	_explicitSelfCall (lexing::Word::eof ())
    {}

    Constructor::Constructor (const lexing::Word & name, const Function::Prototype & proto, const std::vector <Expression> & super, const std::vector <std::pair <lexing::Word, Expression> > & constructions, const Expression & body, const lexing::Word & explicitSuperCall, const lexing::Word & explicitSelfCall, const std::vector <lexing::Word> & cas, const std::vector <Expression> & thrower) :
	_name (name),
	_proto (proto),
	_superParams (super),
	_construction (constructions),
	_body (body),
	_explicitSuperCall (explicitSuperCall),
	_explicitSelfCall (explicitSelfCall),
	_cas (cas),
	_throwers (thrower)
    {}
    
    Declaration Constructor::init (const lexing::Word & name, const Function::Prototype & proto, const std::vector <Expression> & super, const std::vector <std::pair <lexing::Word, Expression> > & constructions, const Expression & body, const lexing::Word & explicitSuperCall, const lexing::Word & explicitSelfCall, const std::vector <lexing::Word> & cas, const std::vector <Expression> & thrower) {
	return Declaration {new (Z0) Constructor (name, proto, super, constructions, body, explicitSuperCall, explicitSelfCall, cas, thrower)};
    }
    
    Declaration Constructor::clone () const {
	return Declaration { new (Z0) Constructor (*this) };
    }
    
    bool Constructor::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast<const void* const*> (type) [0];
	Constructor thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast<const void* const*> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    const lexing::Word & Constructor::getName () const {
	return this-> _name;
    }

    const std::vector <Expression> & Constructor::getSuperParams () const {
	return this-> _superParams;
    }

    const std::vector <std::pair <lexing::Word, Expression> > & Constructor::getFieldConstruction () const {
	return this-> _construction;
    }
    
    const Function::Prototype & Constructor::getPrototype () const {
	return this-> _proto;
    }

    const Expression & Constructor::getBody () const {
	return this-> _body;
    }

    const lexing::Word & Constructor::getExplicitSuperCall () const {
	return this-> _explicitSuperCall;
    }

    const lexing::Word & Constructor::getExplicitSelfCall () const {
	return this-> _explicitSelfCall;
    }
    
    const std::vector <lexing::Word> & Constructor::getCustomAttributes () const {
	return this-> _cas;
    }

    const std::vector <Expression> & Constructor::getThrowers () const {
	return this-> _throwers;
    }
    
}
