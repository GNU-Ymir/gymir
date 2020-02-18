#include <ymir/semantic/generator/value/TemplateClassCst.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/syntax/expression/_.hh>

namespace semantic {

    namespace generator {

	TemplateClassCst::TemplateClassCst () :
	    TemplateRef (),
	    _cst (syntax::Declaration::empty ())
	{}

	TemplateClassCst::TemplateClassCst (const lexing::Word & loc, const Symbol & ref, const syntax::Declaration & cst) :
	    TemplateRef (loc, ref),
	    _cst (cst)
	{}
	
	Generator TemplateClassCst::init (const lexing::Word & loc, const Symbol & ref, const syntax::Declaration & cst) {
	    return Generator {new TemplateClassCst (loc, ref, cst)};
	}
    
	Generator TemplateClassCst::clone () const {
	    return Generator {new (Z0) TemplateClassCst (*this)};
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


	const syntax::Declaration & TemplateClassCst::getConstructor () const {
	    return this-> _cst;
	}

	std::string TemplateClassCst::prettyString () const {
	    return TemplateRef::prettyString () + "::new";
	}
	
    }
    
}
