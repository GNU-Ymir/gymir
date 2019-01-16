#include <ymir/semantic/visitor/Visitor.hh>
#include <ymir/semantic/symbol/_.hh>

namespace semantic {

    Visitor::Visitor () {}
    
    Visitor Visitor::init () {
	return Visitor ();
    }

    semantic::Symbol Visitor::visit (const syntax::Declaration & ast) {
	match (ast) {
	    of (syntax::Module, mod, {
		    return Module::init (mod.getIdent ());
		}
	    )	       
	}
	return Symbol::empty ();
    }    

}
