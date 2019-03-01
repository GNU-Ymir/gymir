#include <ymir/semantic/validator/SubVisitor.hh>
#include <ymir/syntax/expression/Var.hh>
#include <ymir/semantic/generator/value/ModuleAccess.hh>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	
	SubVisitor::SubVisitor (Visitor & context) :
	    _context (context) 
	{}

	SubVisitor SubVisitor::init (Visitor & context) {
	    return SubVisitor {context};
	}

	generator::Generator SubVisitor::validate (const syntax::Binary & expression) {
	    auto left = this-> _context.validateValue (expression.getLeft ());

	    match (left) {
		of (MultSym, mult, return validateMultSym (expression, mult));
		of (ModuleAccess, acc, return validateModuleAccess (expression, acc));
		of (FrameProto, proto,
		    {
			auto right = expression.getRight ().to <syntax::Var> ().getName ().str;
			Ymir::Error::occur (expression.getLocation (),
					    ExternalError::get (UNDEFINED_SUB_PART_FOR),
					    right, proto.getLocation ().str);
		    }
		);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator SubVisitor::validateMultSym (const syntax::Binary &expression, const MultSym & mult) {
	    auto right = expression.getRight ().to <syntax::Var> ().getName ().str;
	    std::vector <Symbol> syms;
	    for (auto & gen : mult.getGenerators ()) {
		if (gen.is<ModuleAccess> ()) {
		    auto elems = gen.to <ModuleAccess> ().getLocal (right);
		    syms.insert (syms.end (), elems.begin (), elems.end ());
		} 
	    }

	    if (syms.size () == 0) {
		Ymir::Error::occur (expression.getLocation (),
				    ExternalError::get (UNDEFINED_SUB_PART),
				    right);
	    }

	    return this-> _context.validateMultSym (expression.getLocation (), syms);
	}

	Generator SubVisitor::validateModuleAccess (const syntax::Binary &expression, const ModuleAccess & acc) {
	    auto right = expression.getRight ().to <syntax::Var> ().getName ().str;
	    std::vector <Symbol> syms = acc.getLocal (right);
	    if (syms.size () == 0) {
		Ymir::Error::occur (expression.getLocation (),
				    ExternalError::get (UNDEFINED_SUB_PART_FOR),
				    right, acc.getModRef ().getRealName ()
		);
	    }
	    
	    return this-> _context.validateMultSym (expression.getLocation (), syms);
	}

	
    }    

}
