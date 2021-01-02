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
