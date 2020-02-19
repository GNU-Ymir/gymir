#include <ymir/semantic/generator/value/ClassCst.hh>

namespace semantic {

    namespace generator {

	ClassCst::ClassCst () :
	    Value (),
	    _frame (Generator::empty ()),
	    _self (Generator::empty ()),
	    _types ({}),
	    _params ({})
	{
	    this-> isLvalue (true);
	}

	ClassCst::ClassCst (const lexing::Word & loc, const Generator & type, const Generator & frame, const std::vector<Generator> & types, const std::vector <Generator> & params) :
	    Value (loc, type),
	    _frame (frame),
	    _self (Generator::empty ()),
	    _types (types),
	    _params (params)
	{
	    this-> isLvalue (true);
	    auto lth = this-> _frame.getThrowers ();
	    if (lth.size () != 0) {
		for (auto &it : lth) it.changeLocation (loc);
	    }
	    
	    for (auto & it : this-> _params) {
		auto &ith = it.getThrowers ();
		lth.insert (lth.end (), ith.begin (), ith.end ());
	    }
	    
	    this-> setThrowers (lth);
	}
	
	Generator ClassCst::init (const lexing::Word & loc, const Generator & type, const Generator & frame, const std::vector<Generator> & types, const std::vector <Generator> & params) {
	    return Generator {new ClassCst (loc, type, frame, types, params)};
	}
    
	Generator ClassCst::clone () const {
	    return Generator {new (Z0) ClassCst (*this)};
	}

	bool ClassCst::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    ClassCst thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool ClassCst::equals (const Generator & gen) const {
	    if (!gen.is <ClassCst> ()) return false;
	    auto call = gen.to <ClassCst> ();
	    if (call.getParameters ().size () != this-> _params.size ()) return false;
	    if (!call.getFrame ().equals (this-> _frame)) return false;
	    for (auto it : Ymir::r (0, this-> _params.size ())) {
		if (!this-> _params [it].equals (call.getParameters () [it])) return false;
		if (!this-> _types [it].equals (call.getTypes () [it])) return false;
	    }

	    return true;
	}

	const Generator & ClassCst::getFrame () const {
	    return this-> _frame;
	}

	const std::vector <Generator> & ClassCst::getTypes () const {
	    return this-> _types;
	}
	
	const std::vector <Generator> & ClassCst::getParameters () const {
	    return this-> _params;
	}

	void ClassCst::setSelf (const Generator & self) {
	    this-> _self = self;
	}
	
	const Generator & ClassCst::getSelf () const {
	    return this-> _self;
	}
	
	std::string ClassCst::prettyString () const {
	    std::vector <std::string> params;
	    for (auto & it : this-> _params)
		params.push_back (it.prettyString ());
	    return Ymir::format ("% (%)", this-> _frame.prettyString (), params);
	}
	
    }
    
}
