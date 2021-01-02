#include <ymir/semantic/generator/value/MethodTemplateRef.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/syntax/expression/_.hh>

namespace semantic {

    namespace generator {

	MethodTemplateRef::MethodTemplateRef () :
	    TemplateRef (),
	    _self (Generator::empty ())
	{}

	MethodTemplateRef::MethodTemplateRef (const lexing::Word & loc, const Symbol & ref, const Generator & self) :
	    TemplateRef (loc, ref),
	    _self (self)
	{}
	
	Generator MethodTemplateRef::init (const lexing::Word & loc, const Symbol & ref, const Generator & self) {
	    return Generator {new (NO_GC) MethodTemplateRef (loc, ref, self)};
	}
    
	Generator MethodTemplateRef::clone () const {
	    return Generator {new (NO_GC) MethodTemplateRef (*this)};
	}

	bool MethodTemplateRef::equals (const Generator & gen) const {
	    if (!gen.is <MethodTemplateRef> ()) return false;
	    auto fr = gen.to<MethodTemplateRef> ();
	    return this-> _self.equals (fr._self) && TemplateRef::equals (gen);
	}

	const Generator & MethodTemplateRef::getSelf () const {
	    return this-> _self;
	}
	
	std::string MethodTemplateRef::prettyString () const {
	    if (this-> _self.is<Value> ()) {
		return Ymir::format ("(%) %", this-> _self.to <Value> ().getType ().prettyString (), TemplateRef::prettyString ());
	    }
	    return Ymir::format ("(%) %", this-> _self.prettyString (), TemplateRef::prettyString ());
	}
	
    }
    
}
