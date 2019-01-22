#include <ymir/semantic/Generator.hh>

namespace semantic {
    namespace generator {

	Generator Generator::__empty__ (Generator::empty ());
	
	IGenerator::IGenerator () :
	    _location (lexing::Word::eof ()),
	    _name ("")
	{}
	
	IGenerator::IGenerator (const lexing::Word & location, const std::string & name) :
	    _location (location),
	    _name (name)
	{}

	bool IGenerator::isOf (const IGenerator *) const {
	    return false;
	}

	const std::string & IGenerator::getName () const {
	    return this-> _name;
	}

	const lexing::Word & IGenerator::getLocation () const {
	    return this-> _location;
	}
	
	IGenerator::~IGenerator () {}

	Generator::Generator (IGenerator * gen) : Proxy<IGenerator, Generator> (gen)
	{}

	Generator Generator::empty () {
	    return Generator {nullptr};
	}

	bool Generator::isEmpty () const {
	    return this-> _value == nullptr;
	}

	const std::string & Generator::getName () const {
	    if (this-> _value == nullptr)
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	    return this-> _value-> getName ();
	}

	const lexing::Word & Generator::getLocation () const {
	    if (this-> _value == nullptr)
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	    return this-> _value-> getLocation ();
	}	
	
	bool Generator::equals (const Generator & other) const {
	    if (this-> _value == nullptr) return other._value == nullptr;
	    else return this-> _value-> equals (other);
	}	
	
    }
}
