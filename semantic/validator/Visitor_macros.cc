#include <ymir/semantic/validator/_.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/utils/map.hh>
#include <ymir/global/Core.hh>
#include <ymir/utils/Path.hh>
#include <ymir/global/State.hh>
#include <string>
#include <algorithm>

using namespace global;

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;       


	Generator Visitor::validateMacroExpression (const semantic::Symbol & sol, const syntax::Expression & content) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    // pushReferent (sol, "validateMacroExpression"); // the context of the macro expression is the context that created the expression (caller)
	    // enterForeign (); // We also don't enter a foreign because local variable may have been used inside the expression content
	    
	    Generator gen (Generator::empty ());
	    try {
		Visitor::__TEMPLATE_NB_RECURS__ += 1;
		if (Visitor::__TEMPLATE_NB_RECURS__ >= VisitConstante::LIMIT_TEMPLATE_RECUR) {
		    Ymir::Error::occur (sol.getName (), ExternalError::TEMPLATE_RECURSION, Visitor::__TEMPLATE_NB_RECURS__);
		}

		gen = this-> validateValue (content);
	    } catch (Error::ErrorList list) {
		static std::list <Error::ErrorMsg> __last_error__;			
		if (Visitor::__TEMPLATE_NB_RECURS__ == 2 && !global::State::instance ().isVerboseActive ()) {
		    // list.errors.push_back (format ("     : %(B)", "..."));
		    // list.errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::OTHER_CALL));
		} else if (Visitor::__TEMPLATE_NB_RECURS__ <  2 || global::State::instance ().isVerboseActive () || Visitor::__LAST_TEMPLATE__) {
		    list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", content.getLocation (), content.prettyString ()));
		    list.errors.insert (list.errors.begin (), Ymir::Error::createNote (content.getLocation (), ExternalError::IN_TEMPLATE_DEF));
		    Visitor::__LAST_TEMPLATE__ = true;
		    __last_error__ = {};
		} else if (Visitor::__LAST_TEMPLATE__) {			    
		    Visitor::__LAST_TEMPLATE__ = false;
		    __last_error__ = list.errors;
		} else {
		    if (__last_error__.size () != 0)
		    list.errors = __last_error__;
		}
		
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }

	    Visitor::__TEMPLATE_NB_RECURS__ -= 1;
	    // exitForeign ();
	    // popReferent ("validateMacroExpression");
	    
	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};

	    return gen;
	}	
	

	Generator Visitor::validateMacroCall (const syntax::MacroCall & call) {
	    auto visitor = MacroVisitor::init (*this);
	    return visitor.validate (call);
	}	
    }

}
