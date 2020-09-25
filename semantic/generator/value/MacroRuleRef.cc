#include <ymir/semantic/generator/value/MacroRuleRef.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/syntax/expression/_.hh>

namespace semantic {

    namespace generator {

	MacroRuleRef::MacroRuleRef () :
	    Value ()
	{}

	MacroRuleRef::MacroRuleRef (const lexing::Word & loc, const Symbol & ref) :
	    Value (loc, NoneType::init (loc))
	{
	    this-> _ref = ref.getPtr ();
	}
	
	Generator MacroRuleRef::init (const lexing::Word & loc, const Symbol & ref) {
	    return Generator {new (Z0) MacroRuleRef (loc, ref)};
	}
    
	Generator MacroRuleRef::clone () const {
	    return Generator {new (Z0) MacroRuleRef (*this)};
	}

	bool MacroRuleRef::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    MacroRuleRef thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool MacroRuleRef::equals (const Generator & gen) const {
	    if (!gen.is <MacroRuleRef> ()) return false;
	    auto fr = gen.to<MacroRuleRef> ();
	    return (Symbol {this-> _ref}).equals (Symbol {fr._ref});
	}

	std::vector <Symbol> MacroRuleRef::getLocal (const std::string & name) const {
	    return (Symbol {this-> _ref}).getLocal (name);
	}

	Symbol MacroRuleRef::getMacroRuleRef () const {
	    return Symbol {this-> _ref};
	}

	std::string MacroRuleRef::prettyString () const {	    
	    return Ymir::format ("__macro %", (Symbol {this-> _ref}).to <semantic::MacroRule> ().getName ().str);
	}
	
    }
    
}
