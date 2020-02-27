#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	Value::Value () :
	    IGenerator (lexing::Word::eof (), ""),
	    _type (Generator::empty ())
	{}

	Value::Value (const lexing::Word & loc, const Generator & type) :
	    IGenerator (loc, loc.str),
	    _type (type)
	{}

	Value::Value (const lexing::Word & loc, const std::string & name, const Generator & type) :
	    IGenerator (loc, name),
	    _type (type)
	{}

	Generator Value::initBrRet (const Value & other, bool isBreaker, bool isReturner) {
	    auto ret = other.clone ();
	    ret.to<Value> ()._breaker = isBreaker;
	    ret.to<Value> ()._returner = isReturner;
	    return ret;
	}
	
	Generator Value::init (const Value & other, const Generator & type) {
	    auto ret = other.clone ();
	    ret.to <Value> ()._type = type;
	    return ret;
	}
	
	Generator Value::clone () const {
	    return Generator{new (Z0) Value (*this)};
	}
	
	// void Value::setType (const Generator & type) {
	//     this-> _type = type;
	//     this-> _type.changeLocation (this-> getLocation ());
	// }

	const Generator & Value::getType () const {
	    return this-> _type;
	}

	bool Value::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Value thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return IGenerator::isOf (type);	
	}

	bool Value::equals (const Generator &) const {
	    return false;
	}

	bool Value::isBreaker () const {
	    return this-> _breaker;
	}

	bool Value::isReturner () const {
	    return this-> _returner;
	}

	void Value::setReturner (bool is) {
	    this-> _returner = is;
	}

	void Value::setBreaker (bool is) {
	    this-> _breaker = is;
	}
	
	bool Value::isLvalue () const {
	    return this-> _isLvalue;
	}

	void Value::isLvalue (bool is) {
	    this-> _isLvalue = is;
	}

	bool Value::isLocal () const {
	    return this-> _isLocal;
	}

	void Value::isLocal (bool is) {
	    this-> _isLocal = is;
	}

    }
    
}
