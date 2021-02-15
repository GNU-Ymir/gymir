#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	Value::Value () :
	    IGenerator (lexing::Word::eof (), ""),
	    _type (Generator::empty ())
	{}

	Value::Value (const lexing::Word & loc, const Generator & type) :
	    IGenerator (loc, loc.getStr ()),
	    _type (type)
	{}

	Value::Value (const lexing::Word & loc, const std::string & name, const Generator & type) :
	    IGenerator (loc, name),
	    _type (type)
	{}

	Generator Value::initBrRet (const Value & other, bool isBreaker, bool isReturner, const lexing::Word & brLoc, const lexing::Word & rtLoc) {
	    auto ret = other.clone ();
	    ret.to<Value> ()._breaker = isBreaker;
	    ret.to<Value> ()._breakerLoc = brLoc;
	    ret.to<Value> ()._returner = isReturner;
	    ret.to<Value> ()._returnerLoc = rtLoc;
	    return ret;
	}
	
	Generator Value::init (const Value & other, const Generator & type) {
	    auto ret = other.clone ();
	    ret.to <Value> ()._type = type;
	    return ret;
	}

	Generator Value::init (const lexing::Word & loc, const Value & other, const Generator & type) {
	    auto ret = other.clone ();
	    ret.to <Value> ()._type = type;
	    ret.to <Value> ().changeLocation (loc);
	    return ret;
	}

	
	Generator Value::clone () const {
	    return Generator{new (NO_GC) Value (*this)};
	}
	
	// void Value::setType (const Generator & type) {
	//     this-> _type = type;
	//     this-> _type.changeLocation (this-> getLocation ());
	// }

	const Generator & Value::getType () const {
	    return this-> _type;
	}

	bool Value::equals (const Generator &) const {
	    return false;
	}

	bool Value::isBreaker () const {
	    return this-> _breaker;
	}

	const lexing::Word & Value::getBreakerLocation () const {
	    return this-> _breakerLoc;
	}

	bool Value::isReturner () const {
	    return this-> _returner;
	}

	const lexing::Word & Value::getReturnerLocation () const {
	    return this-> _returnerLoc;
	}

	void Value::setReturner (bool is) {
	    this-> _returner = is;
	    this-> _returnerLoc = this-> getLocation ();
	}

	void Value::setReturnerLocation (const lexing::Word & w) {
	    this-> _returnerLoc = w;
	}
	
	void Value::setBreaker (bool is) {
	    this-> _breaker = is;
	    this-> _breakerLoc = this-> getLocation ();
	}

	void Value::setBreakerLocation (const lexing::Word & loc) {
	    this-> _breakerLoc = loc;
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
