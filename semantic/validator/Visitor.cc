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

	int Visitor::__CALL_NB_RECURS__ = 0;
	int Visitor::__TEMPLATE_NB_RECURS__ = 0;
	bool Visitor::__LAST__ = true;
	bool Visitor::__LAST_TEMPLATE__ = true;

	VisitorCache::VisitorCache (Visitor & context) :
	    typeInfo (&context, &Visitor::getTypeInfoType),
	    typeIds (&context, &Visitor::getTypeInfoIds),
	    outOfArray (&context, &Visitor::getOutOfArrayCall),
	    disposeTrait (&context, &Visitor::getDisposeTrait),
	    exceptionType (&context, &Visitor::getExceptionType),
	    objectType (&context, &Visitor::getObjectType),
	    segFault (&context, &Visitor::getSegFault)
	{}
	
	Visitor::Visitor () :
	    _cache (*this)
	{
	    enterForeign ();
	}

	Visitor Visitor::init () {
	    return Visitor ();
	}

	void Visitor::validate (const semantic::Symbol & sym, bool inModule) {
	    match (sym) {
		s_of (semantic::Module, mod) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    pushReferent (sym, "validate::module");
		    try {
			validateModule (mod);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
		       			
		    popReferent ("validate::module");
			
		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }
		    return;
		}

		s_of (semantic::Function, func) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    pushReferent (sym, "validate::function");
		    try {
			validateFunction (func);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
			
		    popReferent ("validate::function");
			
		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }
		    return;
		}
		

		s_of_u (semantic::VarDecl) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    pushReferent (sym, "validate::vdecl");
		    try {
			validateVarDecl (sym);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
			
		    popReferent ("validate::vdecl");
			
		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }
		    return;
		}


		s_of_u (semantic::Aka) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    pushReferent (sym, "validate::alias");
		    try {
			validateAka (sym);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }

		    popReferent ("validate::alias");
		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }
		    return;
		}
		
		
		s_of_u (semantic::Struct) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    pushReferent (sym, "validate::struct");
		    try {
			validateStruct (sym);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
			
		    popReferent ("validate::struct");
			
		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }

		    return;
		}
		
		
		s_of_u (semantic::TemplateSolution) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    if (insertTemplateSolution (sym, errors) || inModule) {
			if (errors.size () != 0) {			    
			    throw Error::ErrorList {errors};
			}
			
			pushReferent (sym, "validate::tmpl");
			try {
			    validateTemplateSolution (sym, inModule);
			} catch (Error::ErrorList list) {
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			    removeTemplateSolution (sym); // If there is an error, we don't wan't to store the template solution anymore
			    insertErrorTemplateSolution (sym, errors);
			}
			
			popReferent ("validate::tmpl");
			if (errors.size () != 0) {
			    throw Error::ErrorList {errors};
			}
		    }		    
		    return;
		}
		
		s_of_u (semantic::Enum) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    pushReferent (sym, "validate::enum");
		    try {
			validateEnum (sym);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
			
		    popReferent ("validate::enum");
		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }

		    return;
		}

		s_of_u (semantic::Class) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    pushReferent (sym, "validate::class");
		    try {
			validateClass (sym, inModule);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
			
		    popReferent ("validate::class");
			
		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }
		    
		    return;
		}
	    
		s_of_u (semantic::Trait) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    pushReferent (sym, "validate::trait");
		    try {
			validateTrait (sym);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }

		    popReferent ("validate::trait");

		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }

		    return;
		}

		s_of_u (semantic::Template) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    pushReferent (sym, "validate::template");
		    try {
			validateTemplateNoSpecialization (sym);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }

		    popReferent ("validate::template");

		    if (errors.size () != 0) {
			throw Error::ErrorList {errors};
		    }

		    return;
		}
		
		/** Nothing to do for those kind of symbols */
		s_of_u (semantic::ModRef) return;
		s_of_u (semantic::Macro) return;
	    }
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	}

	void Visitor::validateAllClasses () {
	    auto classes = std::move (this-> _classToValidate);
	    std::list <Error::ErrorMsg> errors;
	    for (auto & it : classes) {
		try {
		    auto clVisitor = ClassVisitor::init (*this);
		    clVisitor.validateClassFull (it);
		} catch (Error::ErrorList & list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    }
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	    
	    // we might add new class ref while validating the others
	    if (this-> _classToValidate.size () != 0) {
	    	this-> validateAllClasses (); 
	    }
	}	

	const std::vector <generator::Generator> & Visitor::getGenerators () const {
	    return this-> _list;
	}
	
	void Visitor::insertNewGenerator (const Generator & generator) {
	    this-> _list.push_back (generator);
	}

	const Generator & Visitor::retreiveFrameFromProto (const FrameProto & proto) {
	    auto name = Mangler::init ().mangleFrameProto (proto);
	    for (auto & it : this-> _list) {
		if (it.is<Frame> ()) { // The frames are retreived thanks to their name
		    auto sec_name = Mangler::init ().mangleFrame (it.to <Frame> ());
		    if (sec_name == name) return it;
		}
	    }
	    return Generator::__empty__;
	}
	
    }
    
}
