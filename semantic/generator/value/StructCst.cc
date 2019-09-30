#include <ymir/semantic/generator/value/StructCst.hh>

namespace semantic {

    namespace generator {

	StructCst::StructCst () :
	    Value (),
	    _str (Generator::empty ()),
	    _types ({}),
	    _params ({})
	{
	    this-> isLvalue (true);
	}

	StructCst::StructCst (const lexing::Word & loc, const Generator & type, const Generator & str, const std::vector<Generator> & types, const std::vector <Generator> & params) :
	    Value (loc, type),
	    _str (str),
	    _types (types),
	    _params (params)
	{
	    this-> isLvalue (true);
	}
	
	Generator StructCst::init (const lexing::Word & loc, const Generator & type, const Generator & str, const std::vector<Generator> & types, const std::vector <Generator> & params) {
	    return Generator {new StructCst (loc, type, str, types, params)};
	}
    
	Generator StructCst::clone () const {
	    return Generator {new (Z0) StructCst (*this)};
	}

	bool StructCst::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    StructCst thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool StructCst::equals (const Generator & gen) const {
	    if (!gen.is <StructCst> ()) return false;
	    auto call = gen.to <StructCst> ();
	    if (call.getParameters ().size () != this-> _params.size ()) return false;
	    if (!call.getStr ().equals (this-> _str)) return false;
	    for (auto it : Ymir::r (0, this-> _params.size ())) {
		if (!this-> _params [it].equals (call.getParameters () [it])) return false;
		if (!this-> _types [it].equals (call.getTypes () [it])) return false;
	    }

	    return true;
	}

	const Generator & StructCst::getStr () const {
	    return this-> _str;
	}

	const std::vector <Generator> & StructCst::getTypes () const {
	    return this-> _types;
	}
	
	const std::vector <Generator> & StructCst::getParameters () const {
	    return this-> _params;
	}

	std::string StructCst::prettyString () const {
	    std::vector <std::string> params;
	    for (auto & it : this-> _params)
		params.push_back (it.prettyString ());
	    return Ymir::format ("% (%)", this-> _str.prettyString (), params);
	}
	
    }
    
}