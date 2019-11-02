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
		of (generator::Enum, en, return validateEnum (expression, en));
	    }

	    this-> error (expression, left, expression.getRight ().to <syntax::Var> ().getName ().str);
	    
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
		this-> error (expression, mult.clone (), right);
	    }

	    return this-> _context.validateMultSym (expression.getLocation (), syms);
	}

	Generator SubVisitor::validateModuleAccess (const syntax::Binary &expression, const ModuleAccess & acc) {
	    auto right = expression.getRight ().to <syntax::Var> ().getName ().str;
	    std::vector <Symbol> syms = acc.getLocal (right);
	    if (syms.size () == 0) {
		this-> error (expression, acc.clone (), right);
	    }
	    
	    return this-> _context.validateMultSym (expression.getLocation (), syms);
	}

	Generator SubVisitor::validateEnum (const syntax::Binary & expression, const generator::Enum & en) {
	    auto right = expression.getRight ().to <syntax::Var> ().getName ().str;
	    auto val = en.getFieldValue (right);
	    if (val.isEmpty ()) {
		this-> error (expression, en.clone (), right);
	    }

	    auto prox = EnumRef::init (en.getLocation (), en.getRef ());
	    auto type = val.to <Value> ().getType ();
	    type.to <Type> ().setProxy (prox);
	    val.to <Value> ().setType (type);
	    
	    return val;
	}

	void SubVisitor::error (const syntax::Binary & expression, const generator::Generator & left, const std::string & right) {
	    std::string leftName;
	    match (left) {
		of (FrameProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())
		else of  (generator::Enum, en, leftName = en.getName ())
		else of (MultSym,    sym,   leftName = sym.getLocation ().str)
		else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ());
	    }

	    Ymir::Error::occur (
		expression.getLocation (),
		ExternalError::get (UNDEFINED_SUB_PART_FOR),
		right,
		leftName
	    );
	}
	
	
    }    

}
