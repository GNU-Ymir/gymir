#include <ymir/syntax/declaration/Constructor.hh>


namespace syntax {
    

    Constructor::Constructor () :
	_name (lexing::Word::eof ()),
	_proto (Function::Prototype::init ()),
	_superParams ({}),
	_construction ({}),
	_body (Expression::empty ())
    {}

    Constructor::Constructor (const lexing::Word & name, const Function::Prototype & proto, const std::vector <Expression> & super, const std::vector <std::pair <lexing::Word, Expression> > & constructions, const Expression & body) :
	_name (name),
	_proto (proto),
	_superParams (super),
	_construction (constructions),
	_body (body)
    {}
    
    Declaration Constructor::init (const lexing::Word & name, const Function::Prototype & proto, const std::vector <Expression> & super, const std::vector <std::pair <lexing::Word, Expression> > & constructions, const Expression & body) {
	return Declaration {new (Z0) Constructor (name, proto, super, constructions, body)};
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

    const Function::Prototype & Constructor::getPrototype () const {
	return this-> _proto;
    }

    const Expression & Constructor::getBody () const {
	return this-> _body;
    }
    
}
