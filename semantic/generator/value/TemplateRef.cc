#include <ymir/semantic/generator/value/TemplateRef.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/syntax/expression/_.hh>

namespace semantic {

    namespace generator {

	TemplateRef::TemplateRef () :
	    Value ()
	{}

	TemplateRef::TemplateRef (const lexing::Word & loc, const Symbol & ref) :
	    Value (loc, NoneType::init (loc))
	{
	    this-> _ref = ref.getPtr ();
	}
	
	Generator TemplateRef::init (const lexing::Word & loc, const Symbol & ref) {
	    return Generator {new (NO_GC) TemplateRef (loc, ref)};
	}
    
	Generator TemplateRef::clone () const {
	    return Generator {new (NO_GC) TemplateRef (*this)};
	}

	bool TemplateRef::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    TemplateRef thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool TemplateRef::equals (const Generator & gen) const {
	    if (!gen.is <TemplateRef> ()) return false;
	    auto fr = gen.to<TemplateRef> ();
	    return (Symbol {this-> _ref}).equals (Symbol {fr._ref});
	}

	std::vector <Symbol> TemplateRef::getLocal (const std::string & name) const {
	    return (Symbol {this-> _ref}).getLocal (name);
	}

	Symbol TemplateRef::getTemplateRef () const {
	    return Symbol {this-> _ref};
	}

	std::string TemplateRef::prettyString () const {	    
	    return Ymir::format ("%", (Symbol {this-> _ref}).to <semantic::Template> ().prettyString ());
	}
	
    }
    
}
