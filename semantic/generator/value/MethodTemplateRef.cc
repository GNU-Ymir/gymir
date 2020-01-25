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
	    return Generator {new MethodTemplateRef (loc, ref, self)};
	}
    
	Generator MethodTemplateRef::clone () const {
	    return Generator {new (Z0) MethodTemplateRef (*this)};
	}

	bool MethodTemplateRef::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    MethodTemplateRef thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return TemplateRef::isOf (type);	
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
	    return Ymir::format ("(%) %", this-> _self.prettyString (), TemplateRef::prettyString ());
	}
	
    }
    
}
