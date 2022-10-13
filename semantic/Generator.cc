#include <ymir/semantic/Generator.hh>

namespace semantic {
    namespace generator {
	
	Generator Generator::__empty__ (Generator::empty ());
	uint32_t IGenerator::__lastId__ = 0;
	std::vector <Generator> Generator::__nothrowers__;
	
	IGenerator::IGenerator () :
	    _location (lexing::Word::eof ()),
	    _name ("")
	{
	    this-> _uniqId = __lastId__;
	    __lastId__ ++ ;
	}
	
	IGenerator::IGenerator (const lexing::Word & location, const std::string & name) :
	    _location (location),
	    _name (name)
	{
	    this-> _uniqId = __lastId__;
	    __lastId__ ++ ;
	}

	IGenerator::IGenerator (const IGenerator & other) :
	    _location (other._location),
	    _name (other._name),
	    _throwers (other._throwers),
	    _uniqId (other._uniqId)
	{}

	const std::string & IGenerator::getName () const {
	    return this-> _name;
	}

	uint32_t IGenerator::getUniqId () const {
	    Ymir::Error::halt ("%(r) Asking uniq id of none Var", "Critical");
	    return this-> _uniqId;
	}

	void IGenerator::setUniqId (uint32_t)  {
	    Ymir::Error::halt ("%(r) Setting uniq id of none Var", "Critical");
	}
	
	void IGenerator::resetIdCount () {
	    __lastId__ = 0;
	}

	uint32_t IGenerator::getLastId () {
	    auto ret = __lastId__;
	    __lastId__ += 1;
	    return ret;
	}

	std::string IGenerator::prettyString () const {
	    return "";
	}
	
	const lexing::Word & IGenerator::getLocation () const {
	    return this-> _location;
	}

	void IGenerator::changeLocation (const lexing::Word & loc) {
	    this-> _location = loc;
	}

	void IGenerator::setThrowers (const std::vector <Generator> & locs) {
	    this-> _throwers = locs;	  // We want all locations 
	}
	    
	const std::vector <Generator> & IGenerator::getThrowers () const {
	    return this-> _throwers;
	}
	
	IGenerator::~IGenerator () {}

	Generator::Generator (IGenerator * gen) : RefProxy<IGenerator, Generator> (gen)
	{}

	Generator Generator::init (const lexing::Word & loc, const Generator & other) {
	    if (other._value != nullptr) {
		auto ret = other._value-> clone ();
		ret._value-> changeLocation (loc);
		return ret;
	    } else return Generator::empty ();
	}
	
	Generator Generator::empty () {
	    return Generator {nullptr};
	}

	bool Generator::isEmpty () const {
	    return this-> _value == nullptr;
	}

	const std::string & Generator::getName () const {
	    if (this-> _value == nullptr)
		Ymir::Error::halt (Ymir::ExternalError::NULL_PTR);
	    return this-> _value-> getName ();
	}

	const lexing::Word & Generator::getLocation () const {
	    if (this-> _value == nullptr)
		Ymir::Error::halt (Ymir::ExternalError::NULL_PTR);
	    return this-> _value-> getLocation ();
	}	
	
	bool Generator::equals (const Generator & other) const {
	    if (this-> _value == nullptr) return other._value == nullptr;
	    else return this-> _value-> equals (other);
	}	

	void Generator::resetIdCount () {
	    IGenerator::resetIdCount ();
	}

	uint32_t Generator::getLastId () {
	    return IGenerator::getLastId ();
	}
	
	std::string Generator::prettyString () const {
	    if (this-> _value == nullptr) return "";
	    return this-> _value-> prettyString ();
	}

	uint32_t Generator::getUniqId () const {
	    if (this-> _value == nullptr)
		Ymir::Error::halt (Ymir::ExternalError::NULL_PTR);
	    return this-> _value-> getUniqId ();	    
	}
       
	void Generator::setUniqId (uint32_t id) {
	    if (this-> _value == nullptr)
		Ymir::Error::halt (Ymir::ExternalError::NULL_PTR);
	    return this-> _value-> setUniqId (id);	    
	}

	void Generator::setThrowers (const std::vector <Generator> & locs) {
	    if (this-> _value == nullptr)
		Ymir::Error::halt (Ymir::ExternalError::NULL_PTR);
	    return this-> _value-> setThrowers (locs);
	}

	const std::vector <Generator> & Generator::getThrowers () const {
	    if (this-> _value != nullptr)
		return this-> _value-> getThrowers ();
	    else return __nothrowers__;
	}

	std::string Generator::getTypeId () const {
	    if (this-> _value != nullptr) {
		return typeid (*this-> _value).name ();
	    } else return "";
	}
	
    }
}
