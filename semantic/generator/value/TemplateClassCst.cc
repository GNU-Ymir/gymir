#include <ymir/semantic/generator/value/TemplateClassCst.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/syntax/expression/_.hh>

namespace semantic {

    namespace generator {

	TemplateClassCst::TemplateClassCst () :
	    TemplateRef ()
	{}

	TemplateClassCst::TemplateClassCst (const lexing::Word & loc, const Symbol & ref, const std::vector <syntax::Function::Prototype> & cst) :
	    TemplateRef (loc, ref),
	    _cst (cst)
	{}
	
	Generator TemplateClassCst::init (const lexing::Word & loc, const Symbol & ref, const std::vector <syntax::Function::Prototype> & cst) {
	    return Generator {new (NO_GC) TemplateClassCst (loc, ref, cst)};
	}
    
	Generator TemplateClassCst::clone () const {
	    return Generator {new (NO_GC) TemplateClassCst (*this)};
	}

	bool TemplateClassCst::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    TemplateClassCst thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return TemplateRef::isOf (type);	
	}

	bool TemplateClassCst::equals (const Generator & gen) const {
	    if (!gen.is <TemplateClassCst> ()) return false;
	    auto fr = gen.to<TemplateClassCst> ();
	    return TemplateRef::equals (gen);
	}


	const std::vector <syntax::Function::Prototype> & TemplateClassCst::getPrototypes () const {
	    return this-> _cst;
	}

	std::string TemplateClassCst::prettyString () const {
	    return TemplateRef::prettyString () + "::new";
	}
	
    }
    
}
