#include <ymir/semantic/generator/value/MacroRef.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/syntax/expression/_.hh>

namespace semantic {

    namespace generator {

	MacroRef::MacroRef () :
	    Value ()
	{}

	MacroRef::MacroRef (const lexing::Word & loc, const Symbol & ref) :
	    Value (loc, NoneType::init (loc))
	{
	    this-> _ref = ref.getPtr ();
	}
	
	Generator MacroRef::init (const lexing::Word & loc, const Symbol & ref) {
	    return Generator {new (Z0) MacroRef (loc, ref)};
	}
    
	Generator MacroRef::clone () const {
	    return Generator {new (Z0) MacroRef (*this)};
	}

	bool MacroRef::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    MacroRef thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool MacroRef::equals (const Generator & gen) const {
	    if (!gen.is <MacroRef> ()) return false;
	    auto fr = gen.to<MacroRef> ();
	    return (Symbol {this-> _ref}).equals (Symbol {fr._ref});
	}

	std::vector <Symbol> MacroRef::getLocal (const std::string & name) const {
	    return (Symbol {this-> _ref}).getLocal (name);
	}

	Symbol MacroRef::getMacroRef () const {
	    return Symbol {this-> _ref};
	}

	std::string MacroRef::prettyString () const {	    
	    return Ymir::format ("__macro %", (Symbol {this-> _ref}).to <semantic::Macro> ().getName ().str);
	}
	
    }
    
}
