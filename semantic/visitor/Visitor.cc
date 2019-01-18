#include <ymir/semantic/visitor/Visitor.hh>
#include <ymir/semantic/symbol/_.hh>
#include <ymir/utils/Path.hh>
#include <ymir/syntax/visitor/Keys.hh>

using namespace Ymir;

namespace semantic {

    Visitor::Visitor () {}
    
    Visitor Visitor::init () {
	return Visitor ();
    }

    semantic::Symbol Visitor::visit (const syntax::Declaration & ast) {
	match (ast) {
	    of (syntax::Module, mod, 
		return visitModule (mod);		
	    );
	    
	    of (syntax::Function, func,
		return visitFunction (func);
	    );

	    of (syntax::Struct, str,
		return visitStruct (str);
	    );
	}

	Error::halt ("%(r) - reaching impossible point", "Critical");
	return Symbol::empty ();
    }    

    semantic::Symbol Visitor::visitModule (const syntax::Module & mod) {
	if (mod.isGlobal () && !mod.getIdent ().isEof ()) {
	    auto file_location = Path {mod.getIdent ().locFile}.stripExtension ();
	    if (!Path {mod.getIdent ().str, "::"}.isRelativeFrom (file_location)) {
		Ymir::Error::occur (mod.getIdent (), ExternalError::get (WRONG_MODULE_NAME), mod.getIdent ().str, Path {mod.getIdent ().str, "::"}.toString () + ".yr");
	    }
	}
	
	pushReferent (Module::init (mod.getIdent ()));	
	for (auto & it : mod.getDeclarations ()) {
	    visit (it);
	}

	auto ret = popReferent ();
	getReferent ().insert (ret);
	
	return ret;
    }

    semantic::Symbol Visitor::visitFunction (const syntax::Function & func) {
	auto function = Function::init (func.getName (), func);
	
	auto symbol = getReferent ().getLocal (func.getName ().str);
	if (symbol.is <Function> ()) {
	    auto vec = symbol.to <Function> ().getOverloading ();
	    vec.push_back (symbol);
	    function.to<Function> ().setOverloading (vec);
	} else if (!symbol.isEmpty ()) {	    
	    auto note = Ymir::Error::createNote (symbol.getName ());
	    Ymir::Error::occurAndNote (func.getName (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), func.getName ().str);
	}
	
	for (auto & ca : func.getCustomAttributes ()) {
	    if (ca == Keys::INLINE) function.to <Function> ().isInline (true);
	    else if (ca == Keys::SAFE) function.to <Function> ().isSafe (true);
	    else if (ca == Keys::PURE) function.to <Function> ().isPure (true);
	    else {
		Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.str);
	    }
	}	

	getReferent ().insert (function);
	return function;
    }        

    semantic::Symbol Visitor::visitStruct (const syntax::Struct & str) {
	auto structure = Struct::init (str.getName (), str);
	
	auto symbol = getReferent ().getLocal (str.getName ().str);	
	if (symbol.is <Struct> ()) {
	    auto vec = symbol.to <Struct> ().getOverloading ();
	    vec.push_back (symbol);
	    structure.to <Struct> ().setOverloading (vec);
	} else if (!symbol.isEmpty ()) {
	    auto note = Ymir::Error::createNote (symbol.getName ());
	    Ymir::Error::occurAndNote (str.getName (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), str.getName ().str);
	}
	
	for (auto & ca : str.getCustomAttributes ()) {
	    if (ca == Keys::PACKED) {
		structure.to<Struct> ().isPacked (true);
	    } else if (ca == Keys::UNION) {
		structure.to<Struct> ().isUnion (true);
	    } else Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.str);

	    if (structure.to <Struct> ().isUnion () && structure.to <Struct> ().isPacked ()) {
		Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::PACKED_AND_UNION));
	    }
	}	

	getReferent ().insert (structure);
	return structure;
    }
    
    void Visitor::pushReferent (const Symbol & sym) {
	this-> _referent.push_front (sym);
    }

    Symbol Visitor::popReferent () {
	if (this-> _referent.size () == 0)
	    Ymir::Error::halt ("%(r) - poping a symbol referent, while there is none", "Critical");

	// We can't get a reference since the pop_front will erase the data
	auto /** & */ ret = this-> _referent.front ();
	
	this-> _referent.pop_front ();
	return ret;
    }

    Symbol & Visitor::getReferent () {
	if (this-> _referent.size () != 0)
	    return this-> _referent.front ();
	else return Symbol::__empty__;
    }
    
}
