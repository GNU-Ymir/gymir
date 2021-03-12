#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/validator/BinaryVisitor.hh>
#include <ymir/semantic/validator/BracketVisitor.hh>
#include <ymir/semantic/validator/CallVisitor.hh>
#include <ymir/semantic/validator/ForVisitor.hh>
#include <ymir/semantic/validator/CompileTime.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/semantic/validator/UnaryVisitor.hh>
#include <ymir/semantic/validator/CastVisitor.hh>
#include <ymir/semantic/validator/SubVisitor.hh>
#include <ymir/semantic/validator/DotVisitor.hh>
#include <ymir/semantic/validator/TemplateVisitor.hh>
#include <ymir/semantic/validator/PragmaVisitor.hh>
#include <ymir/semantic/validator/UtfVisitor.hh>
#include <ymir/semantic/validator/MatchVisitor.hh>
#include <ymir/semantic/validator/MacroVisitor.hh>
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
	
	Visitor::Visitor ()
	{
	    enterForeign ();
	}

	Visitor Visitor::init () {
	    return Visitor ();
	}

	void Visitor::validate (const semantic::Symbol & sym) {	    
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


		s_of_u (semantic::Alias) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    pushReferent (sym, "validate::alias");
		    try {
			validateAlias (sym);
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
		
		
		s_of (semantic::TemplateSolution, sol) {
		    if (insertTemplateSolution (sym)) {
			//println ("No template solution : ", sol.getSolutionName ());
			std::list <Ymir::Error::ErrorMsg> errors;			
			pushReferent (sym, "validate::tmpl");
			try {
			    validateTemplateSolution (sol);
			} catch (Error::ErrorList list) {
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			    removeTemplateSolution (sym); // If there is an error, we don't wan't to store the template solution anymore
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
			validateClass (sym, true);
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

		
		/** Nothing to do for those kind of symbols */
		s_of_u (semantic::ModRef) return;		
		s_of_u (semantic::Template) return;
		s_of_u (semantic::Macro) return;
	    }
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	}      
	
	void Visitor::validateModule (const semantic::Module & mod) {
	    if (!mod.isExtern ()) {
		const std::vector <Symbol> & syms = mod.getAllLocal ();
		for (auto & it : syms) {
		    validate (it);
		}
	    }
	}

	Generator Visitor::validateTemplateTest (const Symbol & context, const syntax::Expression & expr) {
	    Generator value (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    pushReferent (context, "validateTemplateTest");
	    enterForeign ();
	    try {
		value = this-> validateValue (expr);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 

	    exitForeign ();
	    popReferent ("validateTemplateTest");
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	    return value;
	}
	
	void Visitor::validateTemplateSolution (const semantic::TemplateSolution & sol) {
	    Visitor::__TEMPLATE_NB_RECURS__ += 1;
	    std::list <Error::ErrorMsg> errors;
	    try {
		if (Visitor::__TEMPLATE_NB_RECURS__ >= VisitConstante::LIMIT_TEMPLATE_RECUR) {
		    Ymir::Error::occur (sol.getName (), ExternalError::get (TEMPLATE_RECURSION), Visitor::__TEMPLATE_NB_RECURS__);
		}
		const std::vector <Symbol> & syms = sol.getAllLocal ();
		for (auto & it : syms) {
		    validate (it);
		}
	    } catch (Error::ErrorList lst) {
		errors = lst.errors;
	    }
	    
	    Visitor::__TEMPLATE_NB_RECURS__ -= 1;
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }	    
	}

	Generator Visitor::validateTemplateSolutionMethod (const semantic::Symbol & sol, const Generator & self) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    const std::vector <Symbol> & syms = sol.to <TemplateSolution> ().getAllLocal ();	    
	    if (syms.size () != 1) Ymir::Error::halt ("", "");
	    
	    auto classType = sol.getReferent ().to <semantic::Class> ().getGenerator ().to <generator::Class> ().getClassRef ();
	    
	    if (insertTemplateSolution (sol)) { // If it is the first time the solution is presented
		pushReferent (sol, "validateTemplateSolutionMethod");
		enterForeign ();
		try {
		    Visitor::__TEMPLATE_NB_RECURS__ += 1;
		    if (Visitor::__TEMPLATE_NB_RECURS__ >= VisitConstante::LIMIT_TEMPLATE_RECUR) {
			Ymir::Error::occur (sol.getName (), ExternalError::get (TEMPLATE_RECURSION), Visitor::__TEMPLATE_NB_RECURS__);
		    }

		    this-> validateMethod (syms [0].to <semantic::Function> (), classType);
		} catch (Error::ErrorList list) {
		    static std::list <Error::ErrorMsg> __last_error__;
		    if (Visitor::__TEMPLATE_NB_RECURS__ == 2 && !global::State::instance ().isVerboseActive ()) {
			// list.errors.push_back (format ("     : %(B)", "..."));
			// list.errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (OTHER_CALL)));
		    } else if (Visitor::__TEMPLATE_NB_RECURS__ < 2 || global::State::instance ().isVerboseActive () || Visitor::__LAST_TEMPLATE__) {
			list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", syms [0].getName (), syms[0].getRealName ()));
			list.errors.insert (list.errors.begin (), Ymir::Error::createNote (syms [0].getName (), ExternalError::get (IN_TEMPLATE_DEF)));
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
		    removeTemplateSolution (sol); // If there is an error, we don't want to store the solution anymore
		}
		
		Visitor::__TEMPLATE_NB_RECURS__ -= 1;
		exitForeign ();
		popReferent ("validateTemplateSolutionMethod");
		if (errors.size () != 0) {
		    throw Error::ErrorList {errors};
		}
	    }
	    
	    if (syms [0].to <semantic::Function> ().isOver ()) {
		Ymir::Error::occur (syms [0].getName (), ExternalError::get (NOT_OVERRIDE), syms [0].getName ().getStr ());
	    }

	    pushReferent (syms [0], "validateTemplateSolutionMethod");
	    Generator proto (Generator::empty ());
	    try {
		proto = this-> validateMethodProto (syms [0].to <semantic::Function> (), classType, Generator::empty ());
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    }
	    
	    popReferent ("validateTemplateSolutionMethodProto");
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	    
	    return proto;
	}

	Generator Visitor::validateMacroExpression (const semantic::Symbol & sol, const syntax::Expression & content) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    // pushReferent (sol, "validateMacroExpression"); // the context of the macro expression is the context that created the expression (caller)
	    // enterForeign (); // We also don't enter a foreign because local variable may have been used inside the expression content
	    
	    Generator gen (Generator::empty ());
	    try {
		Visitor::__TEMPLATE_NB_RECURS__ += 1;
		if (Visitor::__TEMPLATE_NB_RECURS__ >= VisitConstante::LIMIT_TEMPLATE_RECUR) {
		    Ymir::Error::occur (sol.getName (), ExternalError::get (TEMPLATE_RECURSION), Visitor::__TEMPLATE_NB_RECURS__);
		}

		gen = this-> validateValue (content);
	    } catch (Error::ErrorList list) {
		static std::list <Error::ErrorMsg> __last_error__;			
		if (Visitor::__TEMPLATE_NB_RECURS__ == 2 && !global::State::instance ().isVerboseActive ()) {
		    // list.errors.push_back (format ("     : %(B)", "..."));
		    // list.errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (OTHER_CALL)));
		} else if (Visitor::__TEMPLATE_NB_RECURS__ <  2 || global::State::instance ().isVerboseActive () || Visitor::__LAST_TEMPLATE__) {
		    list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", content.getLocation (), content.prettyString ()));
		    list.errors.insert (list.errors.begin (), Ymir::Error::createNote (content.getLocation (), ExternalError::get (IN_TEMPLATE_DEF)));
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
	
	void Visitor::validateFunction (const semantic::Function & func) {
	    auto & function = func.getContent ();
	    std::vector <Generator> params;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    Generator retType (Generator::empty ());
	    
	    enterContext (function.getCustomAttributes ());
	    std::vector <Generator> throwers;
	    for (auto &it : func.getThrowers ()) {
		try {
		    throwers.push_back (Generator::init (it.getLocation (), validateType (it)));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    enterBlock ();
	    {
		try {
		    validatePrototypeForFrame (func.getName (), function.getPrototype (), params, retType);	       	
		    if (function.getLocation () == Keys::MAIN) {		
			if (params.size () > 1) {
			    Ymir::Error::occur (params [1].getLocation (),
						ExternalError::get (MAIN_FUNCTION_ONE_ARG));		    
			} else if (params.size () == 1) {
			    auto argtype =  Slice::init (func.getName (),
							 Slice::init (
							     func.getName (),
							     Char::init (func.getName (), 8) // The input given by the task launcher is in utf8
							 )
			    );
			    
			    verifyCompatibleTypeWithValue (params [0].getLocation (), argtype, params [0]);
			}
		    }
	    
		    if (!retType.isEmpty ()) {
			if (function.getLocation () == Keys::MAIN) {
			    auto itype       = Integer::init (func.getName (), 32, true);
			    verifyCompatibleType (itype.getLocation (), retType.getLocation (), itype, retType);
			}
		    } else retType = Void::init (func.getName ());
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    this-> setCurrentFuncType (retType);
	    
	    if (!function.getBody ().isEmpty () && errors.size () == 0) {
		Generator body (Generator::empty ());
		bool needFinalReturn = false;				
		{
		    try {
			body = validateValue (function.getBody ());
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		}
		if (!body.isEmpty ()) {
		    if (!body.to<Value> ().isReturner ()) {
			verifyMemoryOwner (function.getBody ().getLocation (), retType, body, true);		    
			needFinalReturn = !retType.is<Void> ();
		    }
		    		    
		    std::vector <Generator> unused, notfound;		    
		    verifyThrows (body.getThrowers (), throwers, unused, notfound);

		    std::vector <Generator> types;
		    std::vector <Error::ErrorMsg> msg_types;
		    for (auto & it : notfound) {
			bool found = false;
			for (auto jt : Ymir::r (0, types.size ())) {
			    if (it.to <Type> ().isCompatible (types [jt])) {
				found = true;				
				msg_types[jt].addNote (Ymir::Error::createNote (it.getLocation ()));
			    }
			}
			if (!found) {
			    auto note = Ymir::Error::createNote (it.getLocation ());
			    msg_types.push_back (Error::makeOccurAndNote (func.getName (), note, ExternalError::get (THROWS_NOT_DECLARED), func.getRealName (), it.prettyString ()));
			    types.push_back (it);
			}
		    }
		    errors.insert (errors.end (), msg_types.begin (), msg_types.end ());			
		    
		    for (auto & it : unused) {
			auto note = Ymir::Error::createNote (it.getLocation ());
			errors.push_back (Error::makeOccurAndNote (func.getName (), note, ExternalError::get (THROWS_NOT_USED), func.getRealName (), it.prettyString ()));
		    }
		    		    
		    {
			try {
			    quitBlock ();
			} catch (Error::ErrorList list) {
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			} 
		    }
		}
		
		exitContext ();
		if (errors.size () != 0)
		    throw Error::ErrorList {errors};
		
		auto frame = Frame::init (function.getLocation (), func.getRealName (), params, retType, body, needFinalReturn);
		auto ln = func.getExternalLanguage ();
		if (ln == Keys::CLANG) 
		    frame.to <Frame> ().setManglingStyle (Frame::ManglingStyle::C);
		else if (ln == Keys::CPPLANG)
		    frame.to <Frame> ().setManglingStyle (Frame::ManglingStyle::CXX);

		frame.to <Frame> ().isWeak (func.isWeak ());
		frame.to <Frame> ().setMangledName (func.getMangledName ());

		insertNewGenerator (frame);		
	    } else {
		// If the function has no body, it is normal that none of the parameters are used
		this-> discardAllLocals ();
		try {
		    quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
		exitContext ();
		if (errors.size () != 0)
		    throw Error::ErrorList {errors};
	    }
	}

	void Visitor::validateVarDecl (const semantic::Symbol & sym) {
	    if (sym.to <semantic::VarDecl> ().getGenerator ().isEmpty ()) {
		auto var = sym.to <semantic::VarDecl> ();
		auto elemSym = sym;
		
		Generator type (Generator::empty ());
		Generator value (Generator::empty ());
	    
		if (!var.getType ().isEmpty ()) {
		    type = validateType (var.getType ());
		}

		if (!var.getValue ().isEmpty ()) {
		    value = validateValueNonVoid (var.getValue ());		
		}

		if (var.getValue ().isEmpty () && !var.isExtern ()) {
		    Error::occur (var.getName (), ExternalError::get (VAR_DECL_WITHOUT_VALUE));
		} 
		
		if (type.isEmpty ()) {
		    type = Type::init (value.to <Value> ().getType ().to<Type> (), false, false);
		}

		bool isMutable = false;		
		for (auto & deco : var.getDecorators ()) {
		    switch (deco.getValue ()) {
		    case syntax::Decorator::MUT : { type = Type::init (type.to <Type> (), true); isMutable = true; } break;
		    case syntax::Decorator::DMUT : { type = type.to <Type> ().toDeeplyMutable (); isMutable = true; } break;
		    default :
			Ymir::Error::occur (deco.getLocation (),
					    ExternalError::get (DECO_OUT_OF_CONTEXT),
					    deco.getLocation ().getStr ()
			);
		    }
		}
		if (!isMutable) type = Type::init (type.to <Type> (), false);

		if (!value.isEmpty ()) {
		    if (var.isExtern ())
			Ymir::Error::occur (value.getLocation (), ExternalError::get (EXTERNAL_VAR_WITH_VALUE), var.getRealName ());
		    
		    if (isMutable || !type.is <LambdaType> ())
			verifyMemoryOwner (var.getName (), type, value, true);
		    // verifyCompatibleType (value.getLocation (), type, value.to<Value> ().getType ());
		}
		
		auto glbVar = GlobalVar::init (var.getName (), var.getRealName (), var.getExternalLanguage (), isMutable, type, value);		
		elemSym.to<semantic::VarDecl> ().setGenerator (glbVar);		
		
		insertNewGenerator (glbVar);		
	    }
	}
	
	generator::Generator Visitor::validateAlias (const semantic::Symbol & sym) {
	    if (sym.to <semantic::Alias> ().getGenerator ().isEmpty ()) {
		auto alias = sym.to <semantic::Alias> ();
		auto elemSym = sym;

		Generator elem (Generator::empty ());
		try {
		    elem = validateValue (alias.getValue (), true);
		} catch (Error::ErrorList list) {
		    elem = Generator::empty ();
		} 
		
		if (elem.isEmpty ())
		    elem = validateType (alias.getValue (), true);
		
		if (elem.is <Value> ()) {
		    auto type = Type::init (elem.to <Value> ().getType ().to <Type> (), false, false);
		    elem = Value::init (elem.to <Value> (), type);
		}
		
		elemSym.to <semantic::Alias> ().setGenerator (elem);
	    }

	    return sym.to <semantic::Alias> ().getGenerator ();
	}

	generator::Generator Visitor::validateStruct (const semantic::Symbol & str) {	    
	    if (str.to <semantic::Struct> ().getGenerator ().isEmpty ()) {
		auto sym = str;
		auto gen = generator::Struct::init (sym.getName (), sym);
		sym.to <semantic::Struct> ().setGenerator (gen);
		std::list <Ymir::Error::ErrorMsg> errors;
		std::map <std::string, generator::Generator> syms;
		enterForeign ();
		pushReferent (sym, "validateStruct");
		try {
		    this-> enterBlock ();
		    std::vector <std::string> fields;
		    std::vector <generator::Generator> types;
		    for (auto & it : sym.to<semantic::Struct> ().getFields ()) {
			this-> validateVarDeclValue (it.to <syntax::VarDecl> (), false);
			if (str.to <semantic::Struct> ().isUnion () && !it.to <syntax::VarDecl> ().getValue ().isEmpty ())
			    errors.push_back (Ymir::Error::makeOccur (it.to <syntax::VarDecl> ().getValue ().getLocation (), ExternalError::get (UNION_INIT_FIELD)));
		    }
		    		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 		

		{
		    try {
			syms = this-> discardAllLocals ();
			this-> quitBlock ();
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		}
		
		exitForeign ();		
		popReferent ("validateStruct");
		
		if (errors.size () != 0) {
		    sym.to <semantic::Struct> ().setGenerator (NoneType::init (sym.getName ()));
		    throw Error::ErrorList {errors};
		}
		
		std::vector <Generator> fieldsDecl;
		for (auto & it : sym.to <semantic::Struct> ().getFields ()) {
		    auto gen = syms.find (it.to <syntax::VarDecl> ().getName ().getStr ());		    
		    fieldsDecl.push_back (gen-> second);
		}
		
		gen = generator::Struct::init (gen.to <generator::Struct> (), fieldsDecl);
		for (auto & it : gen.to <generator::Struct> ().getFields ()) {
		    verifyRecursivity (it.getLocation (), it.to <generator::VarDecl> ().getVarType (), sym);
		}
		
 		sym.to <semantic::Struct> ().setGenerator (gen);
		return StructRef::init (str.getName (), sym);
	    }

	    if (str.to <semantic::Struct> ().getGenerator ().is <generator::Struct> ())
		return StructRef::init (str.getName (), str);
	    else {
		Ymir::Error::occur (str.getName (), ExternalError::get (INCOMPLETE_TYPE_CLASS), str.getRealName ());
		return Generator::empty ();
	    }
	}

	void Visitor::validateTrait (const semantic::Symbol & tr) {
	    std::list <Error::ErrorMsg> errors;
	    for (auto & it : tr.to <semantic::Trait> ().getAllInner ()) {
		if (it.is <semantic::Template> ()) {
		    errors.push_back (Ymir::Error::makeOccur (it.getName (), ExternalError::get (TEMPLATE_IN_TRAIT)));
		} else if (it.is <semantic::VarDecl> ()) {
		    errors.push_back (Ymir::Error::makeOccur (it.getName (), ExternalError::get (VAR_DECL_IN_TRAIT)));
		} else if (it.to <semantic::Function> ().isOver ())
		    errors.push_back (Ymir::Error::makeOccur (it.getName (), ExternalError::get (NOT_OVERRIDE), it.getName ().getStr ()));
	    }

	    if (tr.to <semantic::Trait> ().getAssertions ().size () != 0) {
		pushReferent (tr, "validateClassAssertions");
		enterForeign ();
		    
		for (auto & it : tr.to <semantic::Trait> ().getAssertions ()) {
		    try {
			this-> validateCteValue (it);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
		}
		    
		exitForeign ();
		popReferent ("validateClassAssertions");
	    }

	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	}

	void Visitor::validateInnerClass (const semantic::Symbol & cls) {
	    auto & clRef = cls.to <semantic::Class> ().getGenerator ().to <generator::Class> ().getClassRef ();
	    auto & ancestor = clRef.to <ClassRef> ().getAncestor ();
	    auto & addMethods = cls.to <semantic::Class> ().getAddMethods ();
	    
	    std::vector <Generator> ancestorFields;
	    if (!ancestor.isEmpty ())
		ancestorFields = ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <generator::Class> ().getFields ();
	    
	    auto allInners = cls.to <semantic::Class> ().getAllInner ();
	    allInners.insert (allInners.end (), addMethods.begin (), addMethods.end ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    for (auto & it : allInners) {
		pushReferent (it, "validate::innerClass");
		try {
		    match (it) {
			of (semantic::Function, func) {
			    if (!func.getContent ().getBody ().isEmpty ()) {
				validateMethod (func, clRef, cls.isWeak ()); // We need to pass weak here
				// The method could have been imported from a trait that is not weak
			    }
			}
			elof_u (semantic::Constructor) {
			    validateConstructor (it, clRef, ancestor, ancestorFields);
			} fo;			
		    }
		} catch (Error::ErrorList list) {
		    errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), it.getRealName ()));
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 

		popReferent ("validate::innerClass");			
	    }
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	    
	}
	
	generator::Generator Visitor::validateClass (const semantic::Symbol & cls, bool inModule) {	    
	    Generator ancestor (Generator::empty ());
	    if (!cls.to <semantic::Class> ().getAncestor ().isEmpty ()) {
		ancestor = this-> validateValue (cls.to <semantic::Class> ().getAncestor (), true, false);
		if (ancestor.is <generator::Class> ()) ancestor = ancestor.to <generator::Class> ().getClassRef ();
		if (!ancestor.is <ClassRef> ()) {
		    Ymir::Error::occur (cls.to <semantic::Class> ().getAncestor ().getLocation (),
					ExternalError::get (INHERIT_NO_CLASS),
					ancestor.prettyString ()
		    );
		} else if (ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().isFinal ()) {
		    Ymir::Error::occur (cls.to <semantic::Class> ().getAncestor ().getLocation (),
					ExternalError::get (INHERIT_FINAL_CLASS),
					ancestor.prettyString ()
		    );
		}
	    }
	    
	    if (cls.to <semantic::Class> ().getGenerator ().isEmpty () || inModule) {
		auto sym = cls;
		auto gen = generator::Class::init (cls.getName (), sym, ClassRef::init (cls.getName (), ancestor, sym));
		// To avoid recursive validation 
		sym.to <semantic::Class> ().setGenerator (gen);
		
		std::list <Ymir::Error::ErrorMsg> errors;
		
		if (cls.to <semantic::Class> ().getAssertions ().size () != 0) {
		    pushReferent (sym, "validateClassAssertions");
		    enterForeign ();
		    
		    for (auto & it : cls.to <semantic::Class> ().getAssertions ()) {
			try {
			    this-> validateCteValue (it);
			} catch (Error::ErrorList list) {
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			}
		    }
		    
		    exitForeign ();
		    popReferent ("validateClassAssertions");
		}
		
	      		
		{
		    try {
			std::vector <Symbol> addMethods;
			std::vector <generator::Class::MethodProtection> protections;
			auto vtable = validateClassDeclarations (cls, ClassRef::init (cls.getName (), ancestor, sym), ancestor, protections, addMethods);
			
			gen = generator::Class::initVtable (gen.to <generator::Class> (), vtable, protections);
			
			sym.to <semantic::Class> ().setGenerator (gen);
			sym.to <semantic::Class> ().setTypeInfo (validateTypeInfo (gen.getLocation (), ClassRef::init (cls.getName (), ancestor, sym)));
			
			// Add methods is the list of methods that have been added by trait implementation
			sym.to <semantic::Class> ().setAddMethods (addMethods); // We don't put them in the table of the symbol, because they are not declared in it

		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		}
		
		if (errors.size () != 0) {
		    sym.to <semantic::Class> ().setGenerator (NoneType::init (cls.getName ()));
		    Ymir::Error::occurAndNote (cls.getName (), errors, ExternalError::get (VALIDATING), cls.getRealName ());
		}

		{
		    std::map <std::string, generator::Generator> syms;
		    {
			enterForeign ();
			pushReferent (sym, "validateClass");
			try {
			    this-> enterBlock ();			
			    std::vector <std::string> fields;
			    std::vector <generator::Generator> types;
			    for (auto & it : sym.to<semantic::Class> ().getFields ()) {
				this-> validateVarDeclValue (it.to <syntax::VarDecl> (), false);
			    }
		 
			    syms = this-> discardAllLocals ();
		    
			    this-> quitBlock ();
			} catch (Error::ErrorList list) {
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			} 
		    
			popReferent ("validateClass");
			exitForeign ();
			
			if (errors.size () != 0) {
			    sym.to <semantic::Class> ().setGenerator (NoneType::init (cls.getName ()));
			    throw Error::ErrorList {errors};
			}
		    }
		    		
		    try {
			std::vector <Generator> fieldsDecl;
			std::vector <Generator> ancestorFields;
			
			if (!ancestor.isEmpty ()) {
			    fieldsDecl = ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <generator::Class> ().getFields ();
			    ancestorFields = fieldsDecl;
			    for (auto & it : fieldsDecl) { // Verify shadowing 
				auto gen = syms.find (it.to <generator::VarDecl> ().getName ());
				if (gen != syms.end ()) {
				    auto note = Ymir::Error::createNote (it.getLocation ());
				    auto note2 = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), cls.getRealName ());
				    Error::occurAndNote (gen-> second.getLocation (), {note, note2}, ExternalError::get (SHADOWING_DECL), it.to <generator::VarDecl> ().getName ());
				}
			    }
			}

			std::vector <generator::Generator> localFields;
			for (auto & it : sym.to <semantic::Class> ().getFields ()) {
			    auto gen = syms.find (it.to <syntax::VarDecl> ().getName ().getStr ());
			    if (gen != syms.end ()) {
				fieldsDecl.push_back (gen-> second);
				localFields.push_back (gen-> second);
			    } else {
				Ymir::Error::halt ("", "");
			    }
			}

			gen = generator::Class::initFields (gen.to <generator::Class> (), fieldsDecl, localFields);
			sym.to <semantic::Class> ().setGenerator (gen);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		}

		if (errors.size () != 0) {
		    sym.to <semantic::Class> ().setGenerator (NoneType::init (cls.getName ()));
		    throw Error::ErrorList {errors};
		}

		if (inModule) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    try {
			validateInnerClass (cls);		    
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		
		    if (errors.size () != 0) {
			auto sym = cls;
			sym.to <semantic::Class> ().setGenerator (NoneType::init (cls.getName ()));
			Ymir::Error::occurAndNote (cls.getName (), errors, ExternalError::get (VALIDATING), cls.getRealName ());
		    }
		
		    insertNewGenerator (cls.to <semantic::Class> ().getGenerator ());
		}
	    }
	    
	    
	    if (cls.to <semantic::Class> ().getGenerator ().is <generator::Class> ()) {		
		return ClassRef::init (cls.getName (), ancestor, cls);
	    } else {
		Ymir::Error::occur (cls.getName (), ExternalError::get (INCOMPLETE_TYPE_CLASS), cls.getRealName ());
		return Generator::empty ();
	    }
	}

	std::vector<generator::Generator> Visitor::validateClassDeclarations (const semantic::Symbol & cls, const Generator & classType, const Generator & ancestor, std::vector <generator::Class::MethodProtection>  & protection, std::vector <Symbol> & addMethods)  {
	    std::vector <Generator> vtable;
	    std::vector <Generator> ancVtable;
	    std::vector <generator::Class::MethodProtection> ancProtection;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    addMethods = {};
	    if (!ancestor.isEmpty ()) {
		auto & ancClas = ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ();
		vtable = ancClas.to <generator::Class> ().getVtable ();
		protection = ancClas.to <generator::Class> ().getProtectionVtable ();
		ancVtable = vtable;
		ancProtection = protection;
		for (auto & it : protection) {
		    if (it == generator::Class::MethodProtection::PRV)
			it = generator::Class::MethodProtection::PRV_PARENT;
		    
		    addMethods.push_back (Symbol::empty ());
		}
	    }

	    std::vector <semantic::Symbol> implemented;
	    // We verify the vtable for implementation that have already been made in ancestor class
	    for (auto it : cls.to <semantic::Class> ().getAllInner ()) {
		try {
		    match (it) {
			of (semantic::Impl, im) {
			    validateVtableImplement (im, classType, ancestor, vtable, protection, ancVtable, addMethods);
			    implemented.push_back (it);
			} fo;
		    }		    		
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    auto implVtable = vtable; 
	    for (auto it : cls.to <semantic::Class> ().getAllInner ()) {
		try {
		    match (it) {
			of (semantic::Function, func) {				
			    auto index = validateVtableMethod (func, classType, ancestor, vtable, protection, implVtable, Generator::empty ());
			    if (func.getContent ().getBody ().isEmpty () && !cls.to <semantic::Class> ().isAbs ()) {
				Ymir::Error::occur (vtable [index].getLocation (), ExternalError::get (NO_BODY_METHOD), vtable [index].prettyString ());
			    }
			} fo;
		    }		    		
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    for (auto it : implemented) {
		try {		    
		    auto trait = this-> validateType (it.to <semantic::Impl> ().getTrait ());
		    for (auto jt : it.to <semantic::Impl> ().getAllInner ()) {
			if (jt.is <semantic::Function> ()) {
			    auto & func = jt.to <semantic::Function> ();
			    auto index = validateVtableMethod (func, classType, ancestor, vtable, protection, implVtable, trait);
			    if (func.getContent ().getBody ().isEmpty ()) {
				Ymir::Error::occur (vtable [index].getLocation (), ExternalError::get (NO_BODY_METHOD), vtable [index].prettyString ());
			    }
					
			    // Definition of a new method in implement is forbidden
			    if (index >= (int) addMethods.size ()) {
				std::list <Ymir::Error::ErrorMsg> names;
				for (auto & ft : trait.to <TraitRef> ().getRef ().to <semantic::Trait> ().getAllInner ()) { // It is necessarily a trait, we verified that earlier
				    if (ft.getName ().getStr () == func.getName ().getStr () && ft.is <semantic::Function> ()) {
					auto proto = validateMethodProto (ft.to <semantic::Function> (), classType, trait);
					names.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), proto.getLocation (), proto.prettyString ()));
				    }
				}
				Ymir::Error::occurAndNote (func.getName (), names, ExternalError::get (TRAIT_NO_METHOD), trait.prettyString (), vtable [index].prettyString ());
			    }
			    addMethods [index] = jt;
			} else {
			    Ymir::Error::halt ("", ""); 
			}
		    }			    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 	
	    }
	    
	    for (auto i : Ymir::r (0, vtable.size ())) {
		for (auto j : Ymir::r (0, vtable.size ())) {
		    // Verification of the collision (since all reimplemented function are marked override)
		    if (i != j && Ymir::Path (vtable[i].to <FrameProto> ().getName (), "::").fileName ().toString () ==
			Ymir::Path (vtable [j].to <FrameProto> ().getName (), "::").fileName ().toString ()) {
			auto fptr = validateFunctionType (vtable [i]);
			auto protoFptr = validateFunctionType (vtable [j]);
			if (protoFptr.equals (fptr) && vtable [i].to<MethodProto> ().isMutable () == vtable [j].to<MethodProto> ().isMutable ()) {
			    auto note = Ymir::Error::createNote (vtable [i].getLocation ());
			    auto note2 = Ymir::Error::createNote (cls.getName (), ExternalError::get (VALIDATING), cls.getRealName ());
			    Ymir::Error::occurAndNote (vtable [j].getLocation (), {note, note2}, ExternalError::get (CONFLICTING_DECLARATIONS), vtable [i].prettyString ());
			}
		    }
		}
	    }
	    
	    if (errors.size () != 0) 
		throw Error::ErrorList {errors};		

	    if (!cls.to <semantic::Class> ().isAbs ()) {
		for (auto & it : vtable) {
		    if (it.to <MethodProto> ().isEmptyFrame ()) {
			auto note = Ymir::Error::createNote (it.getLocation ());
			auto loc = classType.getLocation ();
			Ymir::Error::occurAndNote (loc, note, ExternalError::get (NOT_ABSTRACT_NO_OVER), classType.prettyString (), it.prettyString ());
		    }
		}
	    }

	    /** Save only the symbol that are not empty */
	    auto aux = addMethods;
	    addMethods = {};
	    for (auto & it : aux)
		if (!it.isEmpty ()) addMethods.push_back (it);
		
	    return vtable;
	}

	Generator Visitor::validateFunctionType (const Generator & proto) {
	    auto params = proto.to <FrameProto> ().getParameters ();
	    auto ret = proto.to <FrameProto> ().getReturnType ();
	    std::vector <Generator> paramTypes;
	    for (auto & it : params) {
		paramTypes.push_back (it.to <generator::ProtoVar> ().getType ());
		bool isMut = false;
		bool isRef = paramTypes.back ().to <Type> ().isRef ();
		if (it.to <generator::ProtoVar> ().isMutable ()) isMut = true;
		if (it.to <generator::ProtoVar> ().getType ().to <Type> ().isRef ()) isRef = true;
		
		paramTypes.back () = Type::init (paramTypes.back ().to <Type> (), isMut, isRef);
	    }
	    
	    return FuncPtr::init (proto.getLocation (), ret, paramTypes);
	}
	
	void Visitor::validateVtableImplement (const semantic::Impl & impl, const Generator & classType, const Generator & ancestor, std::vector <Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <Generator> & ancVtable, std::vector<Symbol> & addMethods) {
	    auto trait = this-> validateType (impl.getTrait ());
	    if (!trait.is <TraitRef> ()) {
		Ymir::Error::occur (impl.getTrait ().getLocation (), ExternalError::get (IMPL_NO_TRAIT), trait.prettyString ());
	    }

	    std::list <Ymir::Error::ErrorMsg> errors;
	    for (auto it : trait.to <TraitRef> ().getRef ().to <semantic::Trait> ().getAllInner ()) {
		pushReferent (it, "vtableImplement");
		try {
		    if (it.is <semantic::Template> ()) {
			Ymir::Error::occur (it.getName (), ExternalError::get (TEMPLATE_IN_TRAIT));
		    } else if (it.is <semantic::VarDecl> ()) {
			Ymir::Error::occur (it.getName (), ExternalError::get (VAR_DECL_IN_TRAIT));
		    } else if (it.is <semantic::Function> ()) {
			auto & func = it.to <semantic::Function> ();
			int index = validateVtableMethodImplement (func, classType, ancestor, vtable, protection, ancVtable, trait);
			if (index < (int) ancVtable.size ())
			    addMethods [index] = it;
			else 
			    addMethods.push_back (it);
		    } else {
			Ymir::Error::halt ("", "");		    			
		    }
		} catch (Error::ErrorList list) {		    
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    errors.back ().addNote (Ymir::Error::createNote (impl.getTrait ().getLocation (), ExternalError::get (IN_TRAIT_VALIDATION)));
		}
		popReferent ("vtableImplement");
	    }
	    
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	}

	int Visitor::validateVtableMethodImplement (const semantic::Function & func, const Generator & classType, const Generator &, std::vector <Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <Generator> & ancVtable, const generator::Generator & trait) {
	    auto proto = validateMethodProto (func, classType, trait);
	    auto protoFptr = validateFunctionType (proto);
	    for (auto i : Ymir::r (0, ancVtable.size ())) {
		if (Ymir::Path (ancVtable[i].to <FrameProto> ().getName (), "::").fileName ().toString () == func.getName ().getStr ()) {		    
		    auto fptr = validateFunctionType (ancVtable [i]);
		    if (protoFptr.equals (fptr) && ancVtable [i].to<MethodProto> ().isMutable () == proto.to<MethodProto> ().isMutable ()) {
			if (ancVtable [i].to <MethodProto> ().getTrait ().isEmpty () || ancVtable [i].getLocation () != func.getName ()) {
			    auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
			    Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (IMPLICIT_OVERRIDE_BY_TRAIT), ancVtable[i].prettyString ());
			}

			vtable [i] = proto;
			return i;
		    }
		}
	    }

	    vtable.push_back (proto);
		
	    if (func.isPublic ())
		protection.push_back (generator::Class::MethodProtection::PUB);
	    else if (func.isProtected ())
		protection.push_back (generator::Class::MethodProtection::PROT);
	    else
		protection.push_back (generator::Class::MethodProtection::PRV);
	    return vtable.size () - 1;
	}	
	
	int Visitor::validateVtableMethod (const semantic::Function & func, const Generator & classType, const Generator &, std::vector <Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <Generator> & ancVtable, const Generator & trait) {	    
	    auto proto = validateMethodProto (func, classType, trait);	    
	    auto protoFptr = validateFunctionType (proto);
	    for (auto i : Ymir::r (0, ancVtable.size ())) {
		if (Ymir::Path (ancVtable[i].to <FrameProto> ().getName (), "::").fileName ().toString () == func.getName ().getStr ()) {		    
		    auto fptr = validateFunctionType (ancVtable [i]);
		    if (protoFptr.equals (fptr) && ancVtable [i].to<MethodProto> ().isMutable () == proto.to<MethodProto> ().isMutable ()) {
			// If we are inside an impl Trait, and the method is not overriden but rewritten by reimplementation this is ok
			
			if (!func.isOver ()) {
			    auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
			    Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (IMPLICIT_OVERRIDE), ancVtable[i].prettyString ());
			} else if (protection [i] == generator::Class::MethodProtection::PRV_PARENT) {
			    auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
			    Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (OVERRIDE_PRIVATE), ancVtable[i].prettyString ());
			} else if (((func.isPublic () && protection [i] != generator::Class::MethodProtection::PUB) ||
					       (func.isProtected () && protection [i] != generator::Class::MethodProtection::PROT))) {
			    std::string prot, prot_over;
			    if (protection [i] == generator::Class::MethodProtection::PUB) prot = Keys::PUBLIC;
			    else prot = Keys::PROTECTED;
			    if (func.isPublic ()) prot_over = Keys::PUBLIC;
			    else if (func.isProtected ()) prot_over = Keys::PRIVATE;
			    else prot_over = Keys::PRIVATE;
			    
			    auto note = Ymir::Error::createNote (ancVtable [i].getLocation (), ExternalError::get (DECLARED_PROTECTION), prot);			    
			    Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (OVERRIDE_MISMATCH_PROTECTION), prot_over, proto.prettyString ());
			} else if (!func.isPublic () && !func.isProtected ()) {
			    auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
			    Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (CANNOT_OVERRIDE_AS_PRIVATE), proto.prettyString ());
			} else if (ancVtable [i].to <MethodProto> ().isFinal ()) {
			    auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
			    Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (CANNOT_OVERRIDE_FINAL), ancVtable [i].prettyString ()); 
			} else if (trait.isEmpty () && !ancVtable [i].to <MethodProto> ().getTrait ().isEmpty ()) {
			    auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
			    Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (CANNOT_OVERRIDE_TRAIT_OUTSIDE_IMPL), ancVtable [i].prettyString ()); 
			} else if (!trait.isEmpty () && ancVtable [i].to <MethodProto> ().getTrait ().isEmpty ()) {
			    auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
			    Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (CANNOT_OVERRIDE_NON_TRAIT_IN_IMPL), ancVtable [i].prettyString ()); 
			} else if (!trait.isEmpty () && !ancVtable[i].to <MethodProto> ().getTrait ().isEmpty ()) {
			    try {
				this-> verifyCompatibleType (func.getName (), ancVtable [i].to <MethodProto> ().getTrait ().getLocation (), trait, ancVtable [i].to <MethodProto> ().getTrait ());
			    } catch (Error::ErrorList list) {
				auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
				Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (WRONG_IMPLEMENT), ancVtable [i].prettyString (), ancVtable [i].to<MethodProto> ().getTrait ().prettyString (), trait.prettyString ());
			    }
			}				   

			std::vector <Generator> unused, notfound;
			verifyThrows (proto.getThrowers (), ancVtable[i].getThrowers (), unused, notfound);
			// There can be unused throwers that is not important
			// A method override does not need to throw the same things as a parent method
			// But it must not rethrow elements that are not rethrowed by the overloaded method

			std::list <Ymir::Error::ErrorMsg> errors;
			for (auto & it : notfound) {
			    auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
			    errors.push_back (Ymir::Error::makeOccurAndNote (it.getLocation (), note, ExternalError::get (RETHROW_NOT_MATCHING_PARENT), it.prettyString ()));
			}

			if (errors.size () != 0)
			    throw Error::ErrorList {errors};
			
			// Verify the attributes
			// They must be the same			
			
			vtable [i] = proto; // We do that afterward, when impl a trait the vtable might be empty and always different from ancVtable
			// But we can ensure that an error will be thrown before then, since the func is never marked override in a trait
			
			return i;
		    }
		}		
	    }
	    	
	    if (func.isOver ()) {
		std::list <Ymir::Error::ErrorMsg> names;
		for (auto & it : ancVtable) {
		    if (Ymir::Path (it.to <FrameProto> ().getName (), "::").fileName ().toString () == func.getName ().getStr ())
			names.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.getLocation (), it.prettyString ()));
		}
		Ymir::Error::occurAndNote (func.getName (), names, ExternalError::get (NOT_OVERRIDE), proto.prettyString ());
	    }

	    for (auto i : Ymir::r (ancVtable.size (), vtable.size ())) {
		if (Ymir::Path (vtable[i].to <FrameProto> ().getName (), "::").fileName ().toString () == func.getName ().getStr ()) {
		    auto fptr = validateFunctionType (vtable [i]);
		    if (protoFptr.equals (fptr) && vtable [i].to<MethodProto> ().isMutable () == proto.to<MethodProto> ().isMutable ()) {
			auto note = Ymir::Error::createNote (vtable [i].getLocation ());
			Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (CONFLICTING_DECLARATIONS), vtable [i].prettyString ());
		    }
		}
	    }

	    vtable.push_back (proto);
		
	    if (func.isPublic ())
		protection.push_back (generator::Class::MethodProtection::PUB);
	    else if (func.isProtected ())
		protection.push_back (generator::Class::MethodProtection::PROT);
	    else
		protection.push_back (generator::Class::MethodProtection::PRV);
	    
	    return vtable.size () - 1;
	}
	
	void Visitor::validateConstructor (const semantic::Symbol & sym, const Generator & classType_, const Generator & ancestor, const std::vector <Generator> & ancestorFields) {
	    auto & cs = sym.to <Constructor> ();
	    auto constr = cs.getContent ();
	    std::vector <Generator> params;
	    Generator retType (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto classType = classType_;
	    
	    auto proto = validateConstructorProto (sym);
	    verifyConstructionLoop (proto.getLocation (), proto);

	    auto currentClassDef = classType_.to <ClassRef> ().getRef ();
	    enterClassDef (currentClassDef);
	    enterContext (cs.getCustomAttributes ());
	    
	    classType = Type::init (proto.getLocation (), ClassPtr::init (proto.getLocation (), Type::init (proto.getLocation (), classType.to <Type> (), true, false)).to <Type> (), true, false);
	    enterForeign ();

	    std::vector <Generator> throwers;
	    for (auto &it : constr.getThrowers ()) {
		try {
		    throwers.push_back (Generator::init (it.getLocation (), validateType (it)));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    enterBlock ();
	    try {
		validatePrototypeForFrame (cs.getName (), constr.getPrototype (), params, retType);
		retType = classType.to <ClassPtr> ().getInners ()[0].to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <Value> ().getType ();
		params.insert (params.begin (), ParamVar::init (cs.getName (), classType, true, true));
		insertLocal (params [0].getName (), params [0]);
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    } 

	    Generator body (Generator::empty ());
	    {
		try {
		    auto preConstruct = validatePreConstructor (cs, classType_, ancestor, ancestorFields);
		    this-> setCurrentFuncType (retType);
		    body = validateValue (constr.getBody ());
		    auto loc = constr.getBody ().getLocation ();
		    auto ret = Return::init (loc,
					     Void::init (loc),
					     classType,
					     VarRef::init (loc, params [0].to <ParamVar> ().getName (), classType, params [0].getUniqId (), true, Generator::empty (), true)
		    );	    
		    body = Block::init (loc, Void::init (loc), {preConstruct, body, ret});
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }		    
	    
	    if (!body.isEmpty ()) {
		std::vector <Generator> unused, notfound;		    
		verifyThrows (body.getThrowers (), throwers, unused, notfound);		    		    
		for (auto & it : notfound) {
		    auto note = Ymir::Error::createNote (it.getLocation ());
		    errors.push_back (Error::makeOccurAndNote (sym.getName (), note, ExternalError::get (THROWS_NOT_DECLARED), sym.getRealName (), it.prettyString ()));
		}

		for (auto & it : unused) {
		    auto note = Ymir::Error::createNote (it.getLocation ());
		    errors.push_back (Error::makeOccurAndNote (sym.getName (), note, ExternalError::get (THROWS_NOT_USED), sym.getRealName (), it.prettyString ()));
		}
	    }
	    
	    {
		try {
		    quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    exitForeign ();
	    exitContext ();
	    exitClassDef (currentClassDef);
	    
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    auto frame = Frame::init (constr.getLocation (), cs.getRealName (), params, classType, body, false);
	    frame.to <Frame> ().setMangledName (cs.getMangledName ());
	    frame.to <Frame> ().isWeak (cs.isWeak ());
	    insertNewGenerator (frame);
	}

	void Visitor::validateMethod (const semantic::Function & func, const Generator & classType_, bool isWeak) {
	    auto function = func.getContent ();
	    std::vector <Generator> params;
	    Generator retType (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    auto classType = classType_;
	    auto & cs = classType.to <ClassRef> ().getRef ().to <semantic::Class> ();
	    auto currentClassDef = classType.to <ClassRef> ().getRef ();
	    enterClassDef (currentClassDef);
	    enterContext (function.getCustomAttributes ());
	    
	    classType = Type::init (function.getLocation (), ClassPtr::init (function.getLocation (), classType).to <Type> ().toDeeplyMutable ().to <Type> (), true, false);
	    enterForeign ();

	    std::vector <Generator> throwers;
	    for (auto &it : func.getThrowers ()) {
		try {
		    throwers.push_back (Generator::init (it.getLocation (), validateType (it)));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }	    
	    
	    enterBlock ();
	    
	    try {
		bool isMutable = false;
		for (auto & it : function.getPrototype ().getParameters ()[0].to <syntax::VarDecl> ().getDecorators ()) {
		    if (it.getValue () == syntax::Decorator::MUT) isMutable = true;
		    else {
			Ymir::Error::occur (it.getLocation (),
					    ExternalError::get (DECO_OUT_OF_CONTEXT),
					    it.getLocation ().getStr ()
			);				
		    }
		}
		auto & __params = function.getPrototype ().getParameters ();
		
		classType = Type::init (__params [0].getLocation (), classType.to <Type> (), isMutable, false);		
		params.insert (params.begin (), ParamVar::init (lexing::Word::init (__params [0].getLocation (), Keys::SELF), classType, isMutable, true));
		insertLocal (params [0].getName (), params [0]);		

		auto fakeParams = std::vector <syntax::Expression> (__params.begin () + 1, __params.end ());
		auto proto = syntax::Function::Prototype::init (fakeParams, function.getPrototype ().getType (), false);
		
		validatePrototypeForFrame (cs.getName (), proto, params, retType);
		if (retType.isEmpty ()) retType = Void::init (func.getName ());
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    } 
	    
	    bool needFinalReturn = false;
	    Generator body (Generator::empty ());
	    if (errors.size () == 0) 
		{
		    try {
			this-> setCurrentFuncType (retType);
			body = validateValue (function.getBody ());
		
			if (!body.to<Value> ().isReturner ()) {
			    verifyMemoryOwner (body.getLocation (), retType, body, true);		    
			    needFinalReturn = !retType.is<Void> ();
			}
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		}


	    if (!body.isEmpty ()) {
		std::vector <Generator> unused, notfound;		    
		verifyThrows (body.getThrowers (), throwers, unused, notfound);		    		    
		for (auto & it : notfound) {
		    auto note = Ymir::Error::createNote (it.getLocation ());
		    errors.push_back (Error::makeOccurAndNote (func.getName (), note, ExternalError::get (THROWS_NOT_DECLARED), func.getRealName (), it.prettyString ()));
		}

		for (auto & it : unused) {
		    auto note = Ymir::Error::createNote (it.getLocation ());
		    errors.push_back (Error::makeOccurAndNote (func.getName (), note, ExternalError::get (THROWS_NOT_USED), func.getRealName (), it.prettyString ()));
		}
	    }
	    
	    {
		try {
		    if (errors.size () != 0)
			this-> discardAllLocals ();
		    
		    quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    exitForeign ();
	    exitContext ();
	    exitClassDef (currentClassDef);

	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
		
	    auto frame = Frame::init (function.getLocation (), func.getRealName (), params, retType, body, needFinalReturn);
	    frame.to <Frame> ().isWeak (func.isWeak () || isWeak);
	    frame.to <Frame> ().setMangledName (func.getMangledName ());
	    
	    insertNewGenerator (frame);		
	}

	generator::Generator Visitor::getClassConstructors (const lexing::Word & loc, const generator::Generator & cl, const lexing::Word & name) {
	    bool prot = false, prv = false;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    getClassContext (cl.to <generator::Class> ().getRef (), prv, prot);
	    std::vector <Symbol> syms;
	    for (auto & gen : cl.to <generator::Class> ().getRef ().to <semantic::Class> ().getAllInner ()) {
		match (gen) {
		    of (semantic::Constructor, cst) {
			if (prv || (prot && gen.isProtected ()) || gen.isPublic ()) {
			    if (cst.getRename () == name || name.isEof ())
			    syms.push_back (gen);
			} else {
			    errors.push_back (
				Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), gen.getName (), validateConstructorProto (gen).prettyString ())					    
				);
			}
		    } fo;
		}
	    }
	    
	    if (syms.size () != 0) 
		return validateMultSym (loc, syms);
	    else if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    return Generator::empty ();
	}

	std::vector <syntax::Declaration> Visitor::getAllConstructors (const std::vector <syntax::Declaration> & decls, const lexing::Word & name) {
	    std::vector <syntax::Declaration> results;
	    for (auto & it : decls) {
		match (it) {
		    of (syntax::Constructor, cs) {
			if (cs.getRename () == name || name.isEof ()) results.push_back (it);
		    }
		    elof (syntax::DeclBlock, dc) {
			auto inner = getAllConstructors (dc.getDeclarations (), name);
			results.insert (results.end (), inner.begin (), inner.end ());
		    }
		    elof (syntax::CondBlock, cd) {
			auto inner = getAllConstructors (cd.getDeclarations (), name);
			results.insert (results.end (), inner.begin (), inner.end ());
			if (!cd.getElse ().isEmpty ()) {
			    auto inner = getAllConstructors ({cd.getElse ()}, name);
			    results.insert (results.end (), inner.begin (), inner.end ());
			}
		    } fo;		    
		}
	    }
	    return results;
	}       

	std::vector <Symbol> Visitor::getMacroConstructor (const lexing::Word & loc, const generator::MacroRef & mref) {
	    bool prot = false, prv = false;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    getClassContext (mref.getMacroRef (), prv, prot);
	    std::vector <Symbol> syms;

	    for (auto & gen : mref.getMacroRef ().to <semantic::Macro> ().getAllInner ()) {
		match (gen) {
		    of_u (semantic::MacroConstructor) {
			if (prv || (prot && gen.isProtected ()) || gen.isPublic ()) 
			syms.push_back (gen);
			else {
			    errors.push_back (
				Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), gen.getName (), gen.getName ().getStr ())					    
				);
			}
		    } fo;
		}
	    }
	    
	    if (syms.size () != 0) 
		return syms;
	    else if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    return {};
	}

	std::vector <Symbol> Visitor::getMacroRules (const lexing::Word & loc, const generator::MacroRef & mref, const std::string & name) {
	    bool prv = false;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    getMacroContext (mref.getMacroRef (), prv);
	    std::vector <Symbol> syms;

	    for (auto & gen : mref.getMacroRef ().to <semantic::Macro> ().getAllInner ()) {
		match (gen) {
		    of (semantic::MacroRule, rule) {
			if (rule.getName ().getStr () == name) {
			    if (prv || gen.isPublic ()) {
				syms.push_back (gen);
			    } else {
				errors.push_back (
				    Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), gen.getName (), gen.getName ().getStr ())
				    );
			    }
			}
		    } fo;		    
		}
	    }

	    if (syms.size () != 0) 
		return syms;
	    else if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    return {};
	}

	semantic::Symbol Visitor::getCurrentMacroRules (const lexing::Word & loc, const std::string & name) {
	    // if (this-> _classContext.size () == 0 || !this-> _classContext.back ().is <semantic::Macro> ())
	    // 	Ymir::Error::halt ("", "");
	    for (auto & gen : this-> _classContext.back ().to <semantic::Macro> ().getAllInner ()) {
		match (gen) {
		    of (semantic::MacroRule, rule) {
			if (rule.getName ().getStr () == name) {
			    return gen;
			}
		    } fo;		    
		}
	    }
	    return Symbol::empty ();
	}
	
	generator::Generator Visitor::validatePreConstructor (const semantic::Constructor & cs, const Generator & classType, const Generator & ancestor, const std::vector<Generator> & ancestorFields) {
	    auto & superParams = cs.getContent ().getSuperParams ();
	    auto classR = classType;
	    std::vector <Generator> instructions;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    if (!cs.getContent ().getExplicitSuperCall ().isEof () && ancestor.isEmpty ())		
		Ymir::Error::occur (cs.getContent ().getExplicitSuperCall (), ExternalError::get (NO_SUPER_FOR_CLASS), classR.prettyString ());
	    
	    if (!cs.getContent ().getExplicitSelfCall ().isEof ()) {
		if (cs.getContent ().getFieldConstruction ().size () != 0)
		    Ymir::Error::occur (cs.getContent ().getFieldConstruction ()[0].first, ExternalError::get (MULTIPLE_FIELD_INIT), cs.getContent ().getFieldConstruction ()[0].first.getStr ());
		
		auto loc = cs.getContent ().getExplicitSelfCall ();

		Generator cstrs (Generator::empty ());
		try {
		    cstrs = getClassConstructors (loc, classR.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator (), lexing::Word::eof ());
		} catch (Error::ErrorList list) {
		    Ymir::Error::occurAndNote (
			loc,
			list.errors, 
			ExternalError::get (UNDEFINED_SUB_PART_FOR),
			ClassRef::INIT_NAME,
			classR.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().prettyString ()
		    );
		}
		
		if (!cstrs.isEmpty ()) {
		    auto superBin = TemplateSyntaxWrapper::init (loc, cstrs);				      
		    auto call = syntax::MultOperator::init (lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR), superBin, superParams);
		    auto result = validateValue (call);
		    instructions.push_back (ClassCst::init (result.to <ClassCst> (), validateValue (syntax::Var::init (lexing::Word::init (loc, Keys::SELF)))));
		} else {
		    Ymir::Error::occur (
			loc,
			ExternalError::get (UNDEFINED_SUB_PART_FOR),
			ClassRef::INIT_NAME,
			classR.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().prettyString ()
		    );
		}
	    } else {
		if (!ancestor.isEmpty ()) {
		    auto loc = cs.getContent ().getExplicitSuperCall ();
		    if (loc.isEof ()) loc = cs.getName ();
		    Generator cstrs (Generator::empty ());
		    try {
			cstrs = getClassConstructors (loc, ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator (), lexing::Word::eof ());
		    } catch (Error::ErrorList list) {						
			Ymir::Error::occurAndNote (
			    loc,
			    list.errors,
			    ExternalError::get (UNDEFINED_SUB_PART_FOR),
			    ClassRef::INIT_NAME,
			    ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().prettyString ()
			);
		    }

		    if (!cstrs.isEmpty ()) {
			auto superBin = TemplateSyntaxWrapper::init (loc, cstrs);				      
			
			auto call = syntax::MultOperator::init (lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR), superBin, superParams);
			auto result = validateValue (call);
			instructions.push_back (ClassCst::init (result.to <ClassCst> (), validateValue (syntax::Var::init (lexing::Word::init (loc, Keys::SELF)))));
		    } else {
			Ymir::Error::occur (
			    loc,
			    ExternalError::get (UNDEFINED_SUB_PART_FOR),
			    ClassRef::INIT_NAME,
			    ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().prettyString ()
			);
		    }
		}

		std::set <std::string> validated;
		for (auto & it : ancestorFields) validated.emplace (it.to <generator::VarDecl> ().getName ());
		for (auto & it : cs.getContent ().getFieldConstruction ()) {
		    auto name = it.first;
		    auto access = syntax::Binary::init (lexing::Word::init (name, Token::DOT),
							syntax::Var::init (lexing::Word::init (name, Keys::SELF)),
							syntax::Var::init (name), syntax::Expression::empty ());
		    try {		
			if (validated.find (name.getStr ()) != validated.end ()) {
			    Ymir::Error::occur (name, ExternalError::get (MULTIPLE_FIELD_INIT), name.getStr ());
			}
		    
			auto left = this-> validateValue (access);
			auto right = this-> validateValue (it.second);
			verifyMemoryOwner (left.getLocation (), left.to <Value> ().getType (), right, true);
			instructions.push_back (Affect::init (left.getLocation (), left.to <Value> ().getType (), left, right, true));			
			validated.emplace (name.getStr ());
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 		    
		}

		if (errors.size () != 0)
		    throw Error::ErrorList {errors};
		
		for (auto & it : classR.to <ClassRef> ().getRef ().to <semantic::Class> ().getFields ()) {
		    if (validated.find (it.to <syntax::VarDecl> ().getName ().getStr ()) == validated.end ()) {
			if (it.to <syntax::VarDecl> ().getValue ().isEmpty ()) {
			    auto note = Ymir::Error::createNote (cs.getName ());
			    Error::occurAndNote (it.to <syntax::VarDecl> ().getLocation (), note, ExternalError::get (UNINIT_FIELD), it.to <syntax::VarDecl> ().getName ().getStr ());
			} else {
			    auto name = it.to <syntax::VarDecl> ().getName ();
			    auto access = syntax::Binary::init (lexing::Word::init (name, Token::DOT),
								syntax::Var::init (lexing::Word::init (name, Keys::SELF)),
								syntax::Var::init (name), syntax::Expression::empty ());
			    auto left = this-> validateValue (access);
			    auto right = this-> validateValue (it.to <syntax::VarDecl> ().getValue ());
			    instructions.push_back (Affect::init (left.getLocation (), left.to <Value> ().getType (), left, right));
			}
		    }
		}
	    }
	    
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    auto loc = cs.getName ();
	    return Block::init (loc, Void::init (loc), instructions);
	}

	void Visitor::verifyConstructionLoop (const lexing::Word & location, const Generator & call) {
	    static std::vector <Symbol> protos;
	    static std::vector <Generator> gen_protos;
	    static std::vector <lexing::Word> locs;
	    Symbol sym (Symbol::empty ());
	    Generator current_proto (Generator::empty ());
	    Generator clRef (Generator::empty ());
	    
	    if (call.is <Call> () && call.to <Call> ().getFrame ().is <ConstructorProto> ()) {
		sym = call.to <Call> ().getFrame ().to <ConstructorProto> ().getRef ();
		current_proto = call.to <Call> ().getFrame ();
		clRef = call.to <Call> ().getFrame ().to <ConstructorProto> ().getReturnType ().to <ClassPtr> ().getInners ()[0];	    
	    } else if (call.is <ConstructorProto> ()) {
		sym = call.to <ConstructorProto> ().getRef ();
		current_proto = call;
		clRef = call.to <ConstructorProto> ().getReturnType ().to <ClassPtr> ().getInners ()[0];	    
	    } else if (call.is <ClassCst> ()) {
		sym = call.to <ClassCst> ().getFrame ().to <ConstructorProto> ().getRef ();
		current_proto = call.to <ClassCst> ().getFrame ();
		clRef = call.to <ClassCst> ().getFrame ().to <ConstructorProto> ().getReturnType ().to <ClassPtr> ().getInners ()[0];	    
	    } else return; // This is not a class constructor, we can't check that
	    
	    auto & cs = sym.to <semantic::Constructor> ();	    
	    for (auto & it : protos) {
		if (it.equals (sym)) {		    
		    std::list <Ymir::Error::ErrorMsg> notes;
		    for (auto z : Ymir::r (0, locs.size ())) {
			notes.push_back (Ymir::Error::createNote (locs [z], gen_protos [z].prettyString ()));
		    }
		    Ymir::Error::occurAndNote (call.getLocation (), notes, ExternalError::get (INFINITE_CONSTRUCTION_LOOP));
		}
	    }

	    pushReferent (sym, "verifyConstructionLoop");
	    protos.push_back (sym);
	    gen_protos.push_back (current_proto);
	    locs.push_back (location);
	    
	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector <Generator> params;
	    Generator retType (Generator::empty ());

	    auto currentClassDef = clRef.to <ClassRef> ().getRef ();
	    enterClassDef (currentClassDef);
	    enterForeign ();
	    enterBlock ();

	    {
		try {
		    validatePrototypeForFrame (cs.getName (), cs.getContent ().getPrototype (), params, retType);
		    retType = clRef.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <Value> ().getType ();
		    params.insert (params.begin (), ParamVar::init (cs.getName (), clRef, true, true));
		    insertLocal (params [0].getName (), params [0]);
		} catch (Error::ErrorList list) {
		    errors = list.errors;
		} 
	    }
	    
	    try {
		// If this is just a construction redirection, there is no need to check 
		if (cs.getContent ().getExplicitSelfCall ().isEof ()) {
		    std::set <std::string> validated;
		    auto & superParams = cs.getContent ().getSuperParams ();
		    if (!clRef.to <ClassRef> ().getAncestor ().isEmpty ()) {
			auto ancestor = clRef.to <ClassRef> ().getAncestor ();
			auto loc = cs.getContent ().getExplicitSuperCall ();
			if (loc.isEof ()) loc = cs.getName ();
			Generator cstrs (Generator::empty ());
			
			try {
			    cstrs = getClassConstructors (loc, ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator (), lexing::Word::eof ());
			} catch (Error::ErrorList list) {
			    errors = list.errors;
			    return;
			}
			
			if (!cstrs.isEmpty ()) {			    
			    auto superBin = TemplateSyntaxWrapper::init (loc, cstrs);				      			    
			    auto call = syntax::MultOperator::init (lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR), superBin, superParams);			    
			    auto result = validateValue (call);
			    verifyConstructionLoop (loc, result);
			}
		    }

		    for (auto & it : cs.getContent ().getFieldConstruction ()) {
			auto right = this-> validateValue (it.second);
			if (right.to <Value> ().getType ().is <ClassPtr> ()) {
			    locs.back () = it.second.getLocation ();
			    verifyConstructionLoop (it.second.getLocation (), right);
			    validated.emplace (it.first.getStr ());
			}
		    }

		    for (auto & it : clRef.to<ClassRef> ().getRef ().to <semantic::Class> ().getFields ()) {
			if (validated.find (it.to<syntax::VarDecl> ().getName ().getStr ()) == validated.end ()) {
			    if (!it.to <syntax::VarDecl> ().getValue ().isEmpty ()) {
				auto right = this-> validateValue (it.to <syntax::VarDecl> ().getValue ());
				if (right.to <Value> ().getType ().is <ClassPtr> ()) {
				    auto loc = it.to <syntax::VarDecl> ().getValue ().getLocation ();
				    locs.back () = loc;
				    verifyConstructionLoop (loc, right);
				}
			    }
			}
		    }
		}		
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 

	    {
		try {
		    this-> discardAllLocals ();
		    quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    exitForeign ();
	    exitClassDef (currentClassDef);	    	    
	    protos.pop_back ();
	    gen_protos.pop_back ();
	    locs.pop_back ();	    
	    popReferent ("verifyConstructionLoop");

	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	}	

	generator::Generator Visitor::validateEnum (const semantic::Symbol & en) {
	    if (en.to<semantic::Enum> ().getGenerator ().isEmpty ()) {
		auto sym = en;
		auto gen = generator::Enum::init (sym.getName (), sym);
		sym.to<semantic::Enum> ().setGenerator (gen);

		Generator type (Generator::empty ());
		std::list <Ymir::Error::ErrorMsg> errors;
		std::map <std::string, generator::Generator> syms;		

		enterForeign ();
		pushReferent (en, "validateEnum");
		try {		
		    this-> enterBlock ();

		    if (!sym.to<semantic::Enum>().getType ().isEmpty ())
			type = validateType (sym.to<semantic::Enum> ().getType (), true);
		    
		    std::vector <std::string> fields;
		    if (sym.to<semantic::Enum> ().getFields ().size () == 0) {
			Ymir::Error::occur (sym.getName (), ExternalError::get (ENUM_EMPTY));
		    }
		    
		    for (auto & it : sym.to <semantic::Enum> ().getFields ()) {
			try {
			    match (it) {
				of (syntax::VarDecl, decl) {
				    if (decl.getValue ().isEmpty ()) {
					Ymir::Error::occur (decl.getName (), ExternalError::get (EN_NO_VALUE), decl.getName ().getStr ());
				    }
				} fo;
			    }
			    
			    auto val = this-> validateValue (it);			    
			    if (type.isEmpty ()) {
				type = Generator::init (gen.getLocation (), val.to <generator::VarDecl> ().getVarType ());
			    } else verifyCompatibleType (val.getLocation (), type.getLocation (), type, val.to<generator::VarDecl> ().getVarType ());
			} catch (Error::ErrorList list) {
			    auto note = Ymir::Error::createNote (it.getLocation ());
			    for (auto it : list.errors) 
				note.addNote (it);
			    errors.push_back (note);
			}
		    }
		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 

		{
		    try {
			syms = this-> discardAllLocals ();
			this-> quitBlock ();
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		}
		
		popReferent ("validateEnum");
		exitForeign ();

		if (errors.size () != 0) {
		    sym.to <semantic::Enum> ().setGenerator (NoneType::init (sym.getName ()));
		    throw Error::ErrorList {errors};
		}
		
		std::vector <Generator> fieldsDecl;
		for (auto & it : sym.to <semantic::Enum> ().getFields ()) {
		    auto gen = syms.find (it.to <syntax::VarDecl> ().getName ().getStr ());		    
		    fieldsDecl.push_back (gen-> second);
		}

		type = Type::init (type.to <Type> (), EnumRef::init (en.getName (), sym));		
		gen = generator::Enum::init (gen.to <generator::Enum> (), type, fieldsDecl);
		
		sym.to <semantic::Enum> ().setGenerator (gen);	       		
		return type;
	    }
	    
	    auto gen = en.to <semantic::Enum> ().getGenerator ();
	    if (gen.is <Value> () && !gen.to <semantic::generator::Enum> ().getType ().isEmpty ()) {
		auto type = gen.to <semantic::generator::Enum> ().getType ();
		
		return type;
	    } else {
		Ymir::Error::occur (en.getName (), ExternalError::get (INCOMPLETE_TYPE_CLASS), en.getRealName ());
		return Generator::empty ();
	    }
	}

	void Visitor::verifyRecursivity (const lexing::Word & loc, const generator::Generator & gen, const semantic::Symbol & sym) const {
	    match (gen) {
		of (StructRef, str_ref) {
		    if (str_ref.isRefOf (sym)) {
			auto note = Ymir::Error::createNote (sym.getName ());
			Ymir::Error::occurAndNote (loc, note, ExternalError::get (NO_SIZE_FORWARD_REF));
		    } else {
			auto & str = str_ref.getRef ().to <semantic::Struct> ().getGenerator ();
			for (auto & it : str.to<generator::Struct> ().getFields ()) {
			    verifyRecursivity (loc, it.to <generator::VarDecl> ().getVarType (), sym);
			}
		    }
		}
		elof_u (Pointer) {} // No forward problem on pointer types		    
		elof_u (Slice) {} // No problem for slice, their size can be 0			 
		elof (Type, t) {
		    if (t.isComplex ()) {
			for (auto & it : t.getInners ()) verifyRecursivity (loc, it, sym);
		    }
		} fo;
	    }
	}

	Generator Visitor::validateValueNonVoid (const syntax::Expression & expr) {
	    auto ret = this-> validateValue (expr, false, false);
	    if (ret.to <Value> ().getType ().is<NoneType> () || ret.to <Value> ().getType ().is <Void> ()) {
		Ymir::Error::occur (expr.getLocation (), ExternalError::get (VOID_VALUE));
	    }
	    return ret;
	}
	
	Generator Visitor::validateValue (const syntax::Expression & expr, bool canBeType, bool fromCall, bool checkReach, bool fromValidateType) {
	    Generator value (Generator::empty ());
	    try {
		if (canBeType) 
		    value = validateValueNoReachable (expr, true);
		else
		    value = validateValueNoReachable (expr, fromCall);
		// If it can be a type, that means we are looking for a type, and that implicit call is not an option
	    } catch (Error::ErrorList & list) {
		if (!canBeType) throw list;
	    }
	    
	    if (value.isEmpty () && !fromValidateType)
		value = validateType (expr);
	    
	    else if (value.isEmpty ()) {
		auto note = Ymir::Error::createNote (expr.getLocation ());
		Ymir::Error::occurAndNote (expr.getLocation (), note, ExternalError::get (USE_AS_VALUE));
	    }
		
	    if (!value.is <Value> () && !canBeType) {
		auto note = Ymir::Error::createNote (expr.getLocation ());
		Ymir::Error::occurAndNote (expr.getLocation (), note, ExternalError::get (USE_AS_VALUE));
	    }

	    if (checkReach) {
		if (value.is <Value> () && value.to <Value> ().isBreaker () && !value.to <Value> ().getType ().is <Void> ()) {
		    auto note = Ymir::Error::createNote (value.getLocation ());
		    Ymir::Error::occurAndNote (value.to<Value> ().getBreakerLocation (), note, ExternalError::get (BREAK_INSIDE_EXPR));
		}
	    }
	    
	    return value;
	}

	Generator Visitor::validateCteValue (const syntax::Expression & value) {
	    match (value) {
		of (syntax::If, fi) return validateCteIfExpression (fi);
		elof (syntax::Assert, as) return validateCteAssert (as);
		elof_u (syntax::Block) return validateValue (value);
		elof (syntax::For, fo_) return validateForExpression (fo_, true);
		fo;
	    }
	    
	    return retreiveValue (validateValue (value));
	}
	
	Generator Visitor::validateValueNoReachable (const syntax::Expression & value, bool fromCall) {
	    match (value) {
		s_of (syntax::Block, block)
		    return validateBlock (block);		
		
		s_of (syntax::Fixed, fixed)
		    return validateFixed (fixed);		

		s_of (syntax::Bool, b)
		    return validateBool (b);		
		
		s_of (syntax::Float, f)
		    return validateFloat (f);		

		s_of (syntax::Char, c)
		    return validateChar (c);		

		s_of (syntax::String, s)
		    return validateString (s);
		
		s_of (syntax::Binary, binary)
		    return validateBinary (binary, fromCall);		
		
		s_of (syntax::Var, var)
		    return validateVar (var);		

		s_of (syntax::VarDecl, var)
		    return validateVarDeclValue (var);		

		s_of (syntax::Set, set)
		    return validateSet (set);		
		
		s_of (syntax::DecoratedExpression, dec_expr)
		    return validateDecoratedExpression (dec_expr);		

		s_of (syntax::If, _if)
		    return validateIfExpression (_if);		

		s_of (syntax::While, _while)
		    return validateWhileExpression (_while);		

		s_of (syntax::For, _for)
		    return validateForExpression (_for);		
		
		s_of (syntax::Break, _break)
		    return validateBreak (_break);		

		// of (syntax::Return, _return,
		//     return validateReturn (_return);
		// );
		
		s_of (syntax::List, list)
		    return validateList (list);		

		s_of (syntax::Intrinsics, intr)
		    return validateIntrinsics (intr);		

		s_of (syntax::Unit, u)
		    return None::init (u.getLocation ());		

		s_of (syntax::MultOperator, mult)
		    return validateMultOperator (mult);		

		s_of (syntax::Unary, un)
		    return validateUnary (un);		

		s_of (syntax::NamedExpression, named) {
		    auto inner = validateValue (named.getContent ());
		    return NamedGenerator::init (named.getLocation (), inner);
		}

		s_of (syntax::TemplateCall, cl) {
		    auto ret = validateTemplateCall (cl);
		    if (!fromCall && ret.is <FrameProto> ()) {
			std::list <Ymir::Error::ErrorMsg> errors;
			int score;
			auto visit = CallVisitor::init (*this);			    
			auto sec_ret = visit.validateFrameProto (cl.getLocation (), ret.to <FrameProto> (), {}, score, errors);
			if (errors.size () != 0) {
			    throw Error::ErrorList {errors};
			}
			if (!sec_ret.isEmpty ()) ret = sec_ret;
		    } else if (!fromCall && ret.is <DelegateValue> () && ret.to <DelegateValue> ().getType ().to<Type> ().getInners ()[0].is <FrameProto> ()) { // Template method proto, and dot template calls
			std::list <Ymir::Error::ErrorMsg> errors;
			int score;
			auto visit = CallVisitor::init (*this);			    
			auto sec_ret = visit.validateDelegate (cl.getLocation (), ret, {}, score, errors);
			if (errors.size () != 0) {
			    throw Error::ErrorList {errors};
			}
			if (!sec_ret.isEmpty ()) ret = sec_ret;
		    } else if (!fromCall && ret.is <generator::Struct> ()) {
			std::list <Ymir::Error::ErrorMsg> errors;
			int score;
			auto visit = CallVisitor::init (*this);			    
			auto sec_ret = visit.validateStructCst (cl.getLocation (), ret.to <generator::Struct> (), {}, score, errors);
			if (errors.size () != 0) {
			    throw Error::ErrorList {errors};
			}
			if (!sec_ret.isEmpty ()) ret = sec_ret;
		    }
		    return ret;
		}		

		s_of (syntax::Return, rt)
		    return validateReturn (rt);		

		s_of (TemplateSyntaxList, lst)
		    return validateListTemplate (lst);		
		
		s_of (TemplateSyntaxWrapper, st)
		    return st.getContent ();		

		s_of (syntax::Cast, cst)
		    return validateCast (cst);		

		s_of (syntax::ArrayAlloc, alloc)
		    return validateArrayAlloc (alloc);		

		s_of (syntax::DestructDecl, destr)
		    return validateDestructDecl (destr);		

		s_of (syntax::Lambda, lmbd)
		    return validateLambda (lmbd);		

		s_of (syntax::FuncPtr, ptr)
		    return validateFuncPtr (ptr);		

		s_of (syntax::Null, nl)
		    return validateNullValue (nl);		

		s_of (syntax::TemplateChecker, ch)
		    return validateTemplateChecker (ch);		

		s_of (syntax::Throw, thr)
		    return validateThrow (thr);		
		
		s_of (syntax::Match, match)
		    return validateMatch (match);		

		s_of (syntax::Catch, cat)
		    return validateCatchOutOfScope (cat); // Out of scope is useless		

		s_of (syntax::Scope, scope)
		    return validateScopeOutOfScope (scope); // Out of scope is useless		

		s_of (syntax::Assert, assert)
		    return validateAssert (assert);		

		s_of (syntax::MacroCall, call)
		    return validateMacroCall (call);		

		s_of (syntax::Pragma, prg)
		    return validatePragma (prg);		

		s_of (syntax::Dollar, dl)
		    return validateDollar (dl);		

		s_of (syntax::Try, tr)
		    return validateTry (tr);		

		s_of (syntax::With, wh)
		    return validateWith (wh);		

		s_of (syntax::Atomic, at)
		    return validateAtomic (at);		
	    }	    

	    OutBuffer buf;
	    value.treePrint (buf, 0);
	    println (buf.str ());	    
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator Visitor::validateBlock (const syntax::Block & block, const std::vector <Generator> & init) {
	    std::vector <Generator> values = init;
	    
	    Generator type (Void::init (block.getLocation ()));
	    lexing::Word valueLoc (block.getLocation ());
	    
	    bool breaker = false, returner = false;
	    lexing::Word brLoc = lexing::Word::eof (), rtLoc = lexing::Word::eof ();
	    std::list <Ymir::Error::ErrorMsg> errors;
	    Symbol decl (Symbol::empty ());
	    try {
		enterBlock ();
		decl = validateInnerModule (block.getDeclModule ());
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		decl = Symbol::empty ();
	    } 	    

	    if (!decl.isEmpty ()) {
		pushReferent (decl, "validateBlock");
	    }
	    	    
	    for (int i = 0 ; i < (int) block.getContent ().size () ; i ++) {
		try {
		    if ((returner || breaker) && !block.getContent ()[i].is <syntax::Unit> ()) {			
			Error::occur (block.getContent () [i].getLocation (), ExternalError::get (UNREACHBLE_STATEMENT));
		    }

		    auto value = validateValue (block.getContent () [i], false, false, false);
		    bool isMutable = value.to <Value> ().getType ().to <Type> ().isMutable ();
		    if (value.to <Value> ().isReturner ()) { returner = true; rtLoc = value.to<Value> ().getReturnerLocation (); }
		    if (value.to <Value> ().isBreaker ()) { breaker = true; brLoc = value.to<Value> ().getBreakerLocation (); }
		    if (!canImplicitAlias (value)) isMutable = false;
		    
		    type = value.to <Value> ().getType ();
		    type = Type::init (block.getContent() [i].getLocation (), type.to <Type> (), isMutable, false);
		    valueLoc = value.getLocation ();
		    
		    values.push_back (value);		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    {
		try {
		    if (!type.is<Void> ()) {
			verifyMemoryOwner (block.getEnd (), type, values.back(), false);
		    } else if (type.is<Void> () && values.size () != 0 && !values.back ().is <None> () && isUseless (values.back ()))
			Ymir::Error::occur (block.getContent ().back ().getLocation (), ExternalError::get (USE_UNIT_FOR_VOID));
		} catch (Error::ErrorList list) {  
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }	    

	    if (!decl.isEmpty ()) {
		popReferent ("validateBlock");
	    }
	    
	    {
		try {
		    
		    // If there are some errors, no need to add the warning about the unused vars
		    // Moreover, they may be not pertinent 
		    if (errors.size () != 0)
			discardAllLocals ();
		    
		    quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    auto ret = Value::initBrRet (Block::init (valueLoc, type, values).to <Value> (), breaker, returner, brLoc, rtLoc);	    
	    Generator catchVar (Generator::empty ());
	    Generator catchInfo (Generator::empty ());
	    Generator catchAction (Generator::empty ());	    
	    if (!block.getCatcher ().isEmpty ()) {
		try {
		    if (errors.size () == 0 || ret.getThrowers ().size () != 0) 
			validateCatcher (block.getCatcher (), catchVar, catchInfo, catchAction, type, ret.getThrowers ());
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    std::vector <Generator> onExit;
	    std::vector <Generator> onSuccess;
	    std::vector <Generator> onFailure;

	    {
		try {
		    for (auto & scope_ : block.getScopes ()) {
			auto scope = scope_.to <syntax::Scope> ();
			if (scope.isExit ()) {
			    onExit.push_back (validateValue (scope.getContent()));
			    if (onExit.back ().to <Value> ().isReturner ()) returner = true;
			    if (onExit.back ().to <Value> ().isBreaker ()) breaker = true;
			} else if (scope.isSuccess ()) {
			    onSuccess.push_back (validateValue (scope.getContent ()));
			    if (onSuccess.back ().to <Value> ().isReturner ()) returner = true;
			    if (onSuccess.back ().to <Value> ().isBreaker ()) breaker = true;
			} else if (scope.isFailure ()) {
			    if (ret.getThrowers ().size () == 0) {
				Ymir::Error::occur (scope.getLocation (), ExternalError::get (FAILURE_NO_THROW));
			    }
			    onFailure.push_back (validateValue (scope.getContent ()));
			} else Ymir::Error::occur (scope.getLocation (), ExternalError::get (UNDEFINED_SCOPE_GUARD), scope.getLocation ().getStr ());			
		    } 
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	   
	    
	    if (onSuccess.size () != 0) ret = SuccessScope::init (valueLoc, type, ret, onSuccess);
	    if (onExit.size () != 0 || onFailure.size () != 0 || !catchVar.isEmpty ()) {
		auto jmp_buf_type = validateType (syntax::Var::init (lexing::Word::init (valueLoc, global::CoreNames::get (JMP_BUF_TYPE))));		
		auto ex = ExitScope::init (valueLoc, type, jmp_buf_type, ret, onExit, onFailure, catchVar, catchInfo, catchAction);
		return ex;
	    }
	    return ret;
	}

	void Visitor::validateCatcher (const syntax::Expression & catcher, Generator & varDecl, Generator & typeInfo, Generator & action, generator::Generator & typeBlock, const std::vector <Generator> & throwsTypes) {
	    if (throwsTypes.size () == 0) {
		Error::occur (catcher.getLocation (), ExternalError::get (NOTHING_TO_CATCH));
	    }
	    
	    std::list <Ymir::Error::ErrorMsg> errors;	    
	    enterBlock ();
	    {
		try {

		    auto loc = catcher.getLocation ();
		    auto syntaxType = createClassTypeFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (EXCEPTION_TYPE)});
		    auto type = Type::init (validateType (syntaxType).to <Type> (), false, false);

		    varDecl = generator::VarDecl::init (lexing::Word::init (loc, "#catch"), "#catch", type, Generator::empty (), false);
		    insertLocal ("#catch", varDecl);
		    typeInfo = validateTypeInfo (loc, type);
		    auto vref = VarRef::init (loc, "#catch", type, varDecl.getUniqId (),  false, Generator::empty ());

		    auto visitor = MatchVisitor::init (*this);

		    action = visitor.validateCatcher (vref, throwsTypes, catcher.to <syntax::Catch> ());
		    
		    if (!action.to <Value> ().isReturner () && !action.to <Value> ().isBreaker ()) {
			if (!action.to <Value> ().getType ().to <Type> ().isCompatible (typeBlock)) {
			    auto anc = getCommonAncestor (action.to <Value> ().getType (), typeBlock);
			    if (!anc.isEmpty ())
				typeBlock = anc;
			}
			this-> verifyMemoryOwner (loc, typeBlock, action, false, true, false);
		    }
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    {
		try {
		    this-> discardAllLocals ();
		    quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }	
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	}

	Symbol Visitor::validateInnerModule (const syntax::Declaration & decl) {
	    if (decl.isEmpty ()) return Symbol::empty ();
	    auto sym = declarator::Visitor::init ().visit (decl);
	    if (!sym.isEmpty ()) {
		std::list <Ymir::Error::ErrorMsg> errors;

		this-> _referent.back ().insert (sym);
		enterForeign ();
		try {
		    this-> validate (sym);
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 		
		exitForeign ();
		
		if (errors.size () != 0) {
		    throw Error::ErrorList {errors};
		}
		
	    }
	    return sym;
	}
	
	void Visitor::validateTemplateSymbol (const semantic::Symbol & sym, const Generator & gen) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    pushReferent (sym, "validateTemplateSymbol");
	    
	    enterForeign ();	    
	    try {
		if (gen.is <MethodTemplateRef> () && sym.is <TemplateSolution> ())
		    this-> validateTemplateSolutionMethod (sym, gen.to <MethodTemplateRef> ().getSelf ());
		else 
		    this-> validate (sym);		
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	    
	    exitForeign ();
	    
	    popReferent ("validateTemplateSymbol");
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	}	
	
	Generator Visitor::validateSet (const syntax::Set & set) {
	    std::vector <Generator> values;
	    Generator type (Void::init (set.getLocation ()));
	    bool breaker = false, returner = false;

	    std::list <Ymir::Error::ErrorMsg> errors;
	    for (int i = 0 ; i < (int) set.getContent ().size () ; i ++) {
		try {
		    if (returner || breaker) {			
			Error::occur (set.getContent () [i].getLocation (), ExternalError::get (UNREACHBLE_STATEMENT));
		    }
		    
		    auto value = validateValue (set.getContent () [i]);
		    
		    if (value.to <Value> ().isReturner ()) returner = true;
		    if (value.to <Value> ().isBreaker ()) breaker = true;
		    type = value.to <Value> ().getType ();
		    
		    values.push_back (value);		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());		    
		} 
	    }

	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }

	    return Set::init (set.getLocation (), type, values);
	}
	
	Generator Visitor::validateFixed (const syntax::Fixed & fixed, int base) {
	    struct Anonymous {

		static std::string removeUnder (const std::string & value) {
		    auto aux = value;
		    aux.erase (std::remove (aux.begin (), aux.end (), '_'), aux.end ());
		    return aux;
		}
		
		static ulong convertU (const lexing::Word & loc, const Integer & type, int base) { 
		    char * temp = nullptr; errno = 0; // errno !!
		    auto val = removeUnder (loc.getStr ());
		    if (val.length () > 2 && val [0] == '0' && val [1] == Keys::LX[0]) {
			val = val.substr (2, val.length () - 1);
			base = 16;
		    } else if (val.length () > 2 && val [0] == '0' && val [1] == 'o') {
			val = val.substr (2, val.length () - 1);
			base = 8;
		    }
		    
		    ulong value = std::strtoul (val.c_str (), &temp, base);
		    bool overflow = false;
		    if (temp == val.c_str () || *temp != '\0' ||
			((value == 0 || value == ULONG_MAX) && errno == ERANGE)) {
			overflow = true;
		    }
		    
		    if (overflow || (value > getMaxU (type) && getMaxU (type) != 0))
			Error::occur (loc, ExternalError::get (OVERFLOW), type.getTypeName (), val);
		    
		    return value;
		}
		
		static long convertS (const lexing::Word & loc, const Integer & type, int base) {
		    char * temp = nullptr; errno = 0; // errno !!
		    auto val = removeUnder (loc.getStr ());
		    if (val.length () > 2 && val [0] == '0' && val [1] == Keys::LX[0]) {			
			val = val.substr (2, val.length () - 1);
			base = 16;
		    } else if (val.length () > 2 && val [0] == '0' && val [1] == 'o') {
			val = val.substr (2, val.length () - 1);
			base = 8;
		    }

		    ulong value = std::strtol (val.c_str (), &temp, base);
		    bool overflow = false;
		    if (temp == val.c_str () || *temp != '\0' ||
			((value == 0 || value == ULONG_MAX) && errno == ERANGE)) {
			overflow = true;
		    }
		    
		    if (overflow || (value > getMaxS (type) && getMaxS (type) != 0))
			Error::occur (loc, ExternalError::get (OVERFLOW), type.getTypeName (), val);
		    
		    return value;
		}

		static ulong getMaxU (const Integer & type) {
		    switch (type.getSize ()) {
		    case 8 : return UCHAR_MAX;
		    case 16 : return USHRT_MAX;
		    case 32 : return UINT_MAX;
		    case 64 : return ULONG_MAX;
		    case 0 : return 0;
		    }
		    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		    return 0;
		}
		
		static ulong getMaxS (const Integer & type) {
		    switch (type.getSize ()) {
		    case 8 : return SCHAR_MAX;
		    case 16 : return SHRT_MAX;
		    case 32 : return INT_MAX;
		    case 64 : return LONG_MAX;
		    case 0 : return 0;
		    }
		    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		    return 0;
		}
		
	    };

	    Generator type (Generator::empty ());
	    if (fixed.getSuffix () == Keys::U8) type = Integer::init (fixed.getLocation (), 8, false);
	    if (fixed.getSuffix () == Keys::U16) type = Integer::init (fixed.getLocation (), 16, false);
	    if (fixed.getSuffix () == Keys::U32) type = Integer::init (fixed.getLocation (), 32, false);
	    if (fixed.getSuffix () == Keys::U64) type = Integer::init (fixed.getLocation (), 64, false);
	    if (fixed.getSuffix () == Keys::USIZE) type = Integer::init (fixed.getLocation (), 0, false);
	    
	    if (fixed.getSuffix () == Keys::I8) type = Integer::init (fixed.getLocation (), 8, true);
	    if (fixed.getSuffix () == Keys::I16) type = Integer::init (fixed.getLocation (), 16, true);
	    if (fixed.getSuffix () == Keys::I32 || fixed.getSuffix () == "") type = Integer::init (fixed.getLocation (), 32, true);
	    if (fixed.getSuffix () == Keys::I64) type = Integer::init (fixed.getLocation (), 64, true);
	    if (fixed.getSuffix () == Keys::ISIZE) type = Integer::init (fixed.getLocation (), 0, true);
		
	    auto integer = type.to<Integer> ();
	    //type.to <Type> ().isMutable (true);
	    Fixed::UI value;
	    
	    if (integer.isSigned ()) value.i = Anonymous::convertS (fixed.getLocation (), integer, base);
	    else value.u = Anonymous::convertU (fixed.getLocation (), integer, base);	    
	    
	    return Fixed::init (fixed.getLocation (), type, value);
	}       
	
	Generator Visitor::validateBool (const syntax::Bool & b) {
	    return BoolValue::init (b.getLocation (), Bool::init (b.getLocation ()), b.getLocation () == Keys::TRUE_);
	}

	Generator Visitor::validateFloat (const syntax::Float & f) {
	    Generator type (Generator::empty ());
	    struct Anonymous {
		
		static std::string removeUnder (const std::string & value) {
		    auto aux = value;
		    aux.erase (std::remove (aux.begin (), aux.end (), '_'), aux.end ());
		    return aux;
		}
		
	    };
	    
	    if (f.getSuffix () == Keys::FLOAT_S) type = Float::init (f.getLocation (), 32);
	    else {
		type = Float::init (f.getLocation (), 64);
	    }
	    
	    return FloatValue::init (f.getLocation (), type, Anonymous::removeUnder (f.getValue ()));
	}

	Generator Visitor::validateChar (const syntax::Char & c) {	  	    
	    Generator type (Generator::empty ());	    
	    if (c.getSuffix () == Keys::C8) type = Char::init (c.getLocation (), 8);
	    else type = Char::init (c.getLocation (), 32);

	    auto visitor = UtfVisitor::init (*this);
	    
	    uint value = visitor.convertChar (c.getLocation (), c.getSequence (), type.to<Char> ().getSize ());	   
	    return CharValue::init (c.getLocation (), type, value);
	}
	
	Generator Visitor::validateBinary (const syntax::Binary & bin, bool isFromCall) {
	    if (bin.getLocation () == Token::DCOLON) {
		auto subVisitor = SubVisitor::init (*this);
		return subVisitor.validate (bin);
	    } else if (bin.getLocation () == Token::DOT) {
		auto dotVisitor = DotVisitor::init (*this);
		return dotVisitor.validate (bin, isFromCall);
	    } else if (bin.getLocation () == Token::DOT_AND) {
		try {
		    auto intr = syntax::Intrinsics::init (lexing::Word::init (bin.getLocation (), Keys::ALIAS), bin.getLeft ());
		    auto n_bin = syntax::Binary::init (lexing::Word::init (bin.getLocation (), Token::DOT), intr, bin.getRight (), bin.getType ());
		    return this-> validateBinary (n_bin.to <syntax::Binary> (), isFromCall);		    
		} catch (Error::ErrorList list) {
		    auto note = Ymir::Error::createNote (bin.getLocation ());
		    list.errors.back ().addNote (note);
		    throw list;
		}
	    } else {
		auto binVisitor = BinaryVisitor::init (*this);
		return binVisitor.validate (bin);
	    }
	}

	Generator Visitor::validateUnary (const syntax::Unary & un) {
	    auto unVisitor = UnaryVisitor::init (*this);
	    return unVisitor.validate (un);
	}
	
	Generator Visitor::validateVar (const syntax::Var & var) {
	    auto gen = getLocal (var.getName ().getStr ());	    
	    
	    if (gen.isEmpty ()) {
		auto sym = getGlobal (var.getName ().getStr ());
		if (sym.empty ()) {
		    sym = getGlobalPrivate (var.getName ().getStr ());
		    std::list <Ymir::Error::ErrorMsg> notes;
		    for (auto it : Ymir::r (0, sym.size ())) {
			notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), sym[it].getName (), sym [it].getRealName ()));
		    }
		    Error::occurAndNote (var.getLocation (), notes, ExternalError::get (UNDEF_VAR), var.getName ().getStr ());
		}
		
		auto ret = validateMultSym (var.getLocation (), sym);
		if (ret.is <MultSym> () && ret.to <MultSym> ().getGenerators ().size () == 0) {
		    Error::occur (var.getLocation (), ExternalError::get (UNDEF_VAR), var.getName ().getStr ());
		} else return ret;		
	    }
	    	    
	    // The gen that we got can be either a param decl or a var decl, or inside a closure
	    if (gen.is <ParamVar> ()) {
		return VarRef::init (var.getLocation (), var.getName ().getStr (), gen.to<Value> ().getType (), gen.getUniqId (), gen.to<ParamVar> ().isMutable (), Generator::empty (), gen.to <ParamVar> ().isSelf ());
	    } else if (gen.is <generator::VarDecl> ()) {
		Generator value (Generator::empty ());
		if (!gen.to <generator::VarDecl> ().isMutable ())
		    value = gen.to <generator::VarDecl> ().getVarValue ();
		return VarRef::init (var.getLocation (), var.getName ().getStr (), gen.to<generator::VarDecl> ().getVarType (), gen.getUniqId (), gen.to<generator::VarDecl> ().isMutable (), value);		
	    } else if (gen.is <StructAccess> ()) {// Closure
		return Generator::init (var.getLocation (), gen);
	    } else if (gen.is <ProtoVar> ()) { // PrototypeForProto validation
		return VarRef::init (var.getLocation (), var.getName ().getStr (), gen.to <Value> ().getType (), gen.getUniqId (), gen.to <ProtoVar> ().isMutable (), Generator::empty ());
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}
	
	Generator Visitor::validateMultSym (const lexing::Word & loc, const std::vector <Symbol> & multSym) {	    
	    std::vector <Generator> gens;
	    for (auto & sym : multSym) {
		pushReferent (sym, "validateMultSym");
		bool succ = false;
		std::list <Ymir::Error::ErrorMsg> errors;
		
		try {
		    match (sym) {
			of (semantic::Function, func) {
			    if (!func.isMethod ()) {
				gens.push_back (validateFunctionProto (func));			    
			    }
			    succ = true;
			}
			elof_u (semantic::Constructor) {
			    gens.push_back (validateConstructorProto (sym));		    
			    succ = true;
			}
			elof_u (semantic::ModRef) {
			    gens.push_back (ModuleAccess::init (loc, sym));
			    succ = true;
			}		    		    
			elof_u (semantic::Module) {
			    gens.push_back (ModuleAccess::init (loc, sym));
			    succ = true;
			}
			elof_u (semantic::Struct) {
			    auto str_ref = validateStruct (sym);
			    gens.push_back (str_ref.to <StructRef> ().getRef ().to <semantic::Struct> ().getGenerator ());
			    succ = true;
			}
			elof_u (semantic::Class) {
			    auto cl_ref = validateClass (sym);
			    if (cl_ref.is<ClassRef> ())
			    gens.push_back (cl_ref.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ());
			    else gens.push_back (cl_ref);
			    succ = true;
			    }
			elof (semantic::Trait, tr) {
			    gens.push_back (TraitRef::init (lexing::Word::init (loc, tr.getName ().getStr ()), sym));
			    succ = true;
			}
			elof_u (semantic::Enum) {
			    auto en_ref = validateEnum (sym);
			    gens.push_back (en_ref.to <Type> ().getProxy ().to <EnumRef> ().getRef ().to <semantic::Enum> ().getGenerator ());
			    succ = true;
			}
			elof_u (semantic::Template) {
			    gens.push_back (TemplateRef::init (sym.getName (), sym));
			    succ = true;
			}
			elof (semantic::Macro, mc) {
			    gens.push_back (MacroRef::init (mc.getName (), sym));
			    succ = true;
			} 
			elof (semantic::TemplateSolution, sol) {			    
			    auto loc_gens = validateMultSym (loc, sol.getAllLocal ());
			    match (loc_gens) {
				of (MultSym, mlt_sym) {
				    gens.insert (gens.end (), mlt_sym.getGenerators ().begin (), mlt_sym.getGenerators ().end ());
				    succ = true;
				} elfo {
				    gens.push_back (loc_gens);
				    succ = true;
				}
			    }
			}
			elof (semantic::VarDecl, decl) {
			    validateVarDecl (sym);
			    auto gen = decl.getGenerator ().to <GlobalVar> ();
			    Generator value (Generator::empty ());
			    if (!gen.isMutable ())
			    value = gen.getValue ();
			    gens.push_back (VarRef::init (decl.getName (), decl.getName ().getStr (), gen.getType (), gen.getUniqId (), gen.isMutable (), value));
			    succ = true;
			}
			elof_u (semantic::Alias) {
			    auto al_ref = validateAlias (sym);
			    gens.push_back (al_ref);
			    succ = true;
			} fo;			
		    }
		} catch (Error::ErrorList list) {
		    errors = list.errors;
		} 		

		popReferent ("validateMultSym");
		if (errors.size () != 0)
		    throw Error::ErrorList {errors};
		
		if (!succ) {
		    println (sym.formatTree ());
		    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		}
	    }
	    
	    if (gens.size () == 1) return gens [0];
	    else return MultSym::init (loc, gens);
	}

	Generator Visitor::validateMultSymType (const lexing::Word & loc, const std::vector <Symbol> & multSym) {
	    Generator gen (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;

	    for (auto  it : Ymir::r (0, multSym.size ())) {
		pushReferent (multSym [it], "validateMultSymType");
		Generator locGen (Generator::empty ());
		try {
		    match (multSym [it]) {		    
			of_u (semantic::Struct) {
			    locGen = validateStruct (multSym [it]);
			}
			elof_u (semantic::Enum) {
			    locGen = validateEnum (multSym [it]);
			}
		        elof_u (semantic::Class) {
			    locGen = validateClass (multSym [it]);
			}		
			elof (semantic::Trait, tr) {
			    locGen = TraitRef::init (lexing::Word::init (loc, tr.getName ().getStr ()), multSym [it]);
			}			
			elof_u (semantic::Template) {
			    Ymir::Error::occur (loc, ExternalError::get (USE_AS_TYPE));
			}							     
			elof_u (semantic::Module) {
			    Ymir::Error::occur (loc, ExternalError::get (USE_AS_TYPE));
			}
			elof_u (semantic::ModRef) {
			    Ymir::Error::occur (loc, ExternalError::get (USE_AS_TYPE));
			}
			elof_u (semantic::Alias) {
			    locGen = validateAlias (multSym [0]);
			}
			elof (semantic::Function, func) {
			    if (!func.isMethod ())
			    locGen = validateFunctionProto (func);
			} elfo {			    
			    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");				
			}
		    }
		} catch (Error::ErrorList list) {
		    errors = list.errors;
		} 

		popReferent ("validateMultSymType");
		
		if (!gen.isEmpty () && !locGen.isEmpty ()) {
		    std::list <Ymir::Error::ErrorMsg> notes;
		    notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), gen.getLocation (), gen.prettyString ()));
		    notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), locGen.getLocation (), locGen.prettyString ()));
			
		    Ymir::Error::occurAndNote (loc,
					       notes,
					       ExternalError::get (SPECIALISATION_WORK_TYPE_BOTH));
		}
		
		if (!locGen.isEmpty ())
		    gen = locGen;
	    }
	    
	    if (errors.size () != 0 && gen.isEmpty ())
		throw Error::ErrorList {errors};
	    
	    return gen;
	}
	
	Generator Visitor::validateFunctionProto (const semantic::Function & func) {
	    enterForeign ();
	    std::vector <Generator> params;
	    static std::list <lexing::Word> __validating__; 
	    auto & function = func.getContent ();
	    std::list <Ymir::Error::ErrorMsg> errors;
	    bool no_value = false;
	    Generator retType (Generator::empty ());
	    for (auto func_loc : __validating__) {
		// If there is a foward reference, we can't validate the values
		if (func_loc.isSame (func.getName ())) no_value = true;		    
	    }

	    __validating__.push_back (func.getName ());
	    enterBlock ();
	    
	    try {		
		validatePrototypeForProto (func.getName (), function.getPrototype (), no_value, params, retType);
	    } catch (Error::ErrorList list) {		
		errors = list.errors;
	    } 

	    {
		try {
		    this-> discardAllLocals ();
		    this-> quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    std::vector <Generator> throwers;
	    for (auto &it : func.getThrowers ()) {
		try {
		    throwers.push_back (validateType (it));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    __validating__.pop_back ();
	    exitForeign ();

	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};		
	    }


	    auto frame = FrameProto::init (function.getLocation (), func.getRealName (), retType, params, func.isVariadic (), func.isSafe (), throwers);
	    auto ln = func.getExternalLanguage ();
	    auto style = Frame::ManglingStyle::Y;
	    if (ln == Keys::CLANG) style = Frame::ManglingStyle::C;
	    else if (ln == Keys::CPPLANG) style = Frame::ManglingStyle::CXX;
	    
	    frame = FrameProto::init (frame.to <FrameProto> (), func.getMangledName (), style);
	    return frame;
	}

	Generator Visitor::validateConstructorProto (const semantic::Symbol & sym) {
	    auto & func = sym.to <Constructor> ();	    
	    pushReferent (sym, "validateConstructorProto");
	    enterForeign ();
	    
	    std::vector <Generator> params;
	    static std::list <lexing::Word> __validating__;
	    auto & function = func.getContent ();
	    std::list <Ymir::Error::ErrorMsg> errors;
	    bool no_value = false;
	    Generator retType (Generator::empty ());
	    
	    for (auto func_loc : __validating__) {
		if (func_loc.isSame (func.getName ())) no_value = true;
	    }

	    Generator cl (Generator::empty ());
	    try {
		cl = Type::init (func.getClass ().getName (), ClassPtr::init (func.getClass ().getName (), Type::init (func.getClass ().getName (), validateClass (func.getClass ()).to <Type> (), true, false)).to <Type> (), true, false);
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    } 
	    
	    if (!cl.isEmpty ()) {		
		__validating__.push_back (func.getName ());
		enterBlock ();
		this-> insertLocal (Keys::SELF, ProtoVar::init (func.getName (), cl, Generator::empty (), true, 1, true));
		try {		
		    validatePrototypeForProto (func.getName (), function.getPrototype (), no_value, params, retType);
		} catch (Error::ErrorList list) {
		    errors = list.errors;
		} 

		{
		    try {
			this-> discardAllLocals ();
			this-> quitBlock ();
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		}

		__validating__.pop_back ();
	    }
	    
	    std::vector <Generator> throwers;
	    for (auto &it : func.getThrowers ()) {
		try {
		    throwers.push_back (validateType (it));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    exitForeign ();
	    popReferent ("validateConstructorProto");
	    	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};		
	    }
	    
	    auto frame = ConstructorProto::init (func.getName (), func.getRealName (), sym, cl, params, throwers);	    
	    frame = ConstructorProto::init (frame.to <ConstructorProto> (), func.getMangledName ());	    
	    return frame;
	}

	Generator Visitor::validateMethodProto (const semantic::Function & func, const Generator & classType_, const Generator & trait) {
	    enterForeign ();	    
	    std::vector <Generator> params;
	    static std::list <lexing::Word> __validating__;
	    auto & function = func.getContent ();
	    std::list <Ymir::Error::ErrorMsg> errors;
	    bool no_value = false;
	    Generator retType (Generator::empty ());
	    for (auto func_loc : __validating__) {
		if (func_loc.isSame (func.getName ())) no_value = true;
	    }
	    
	    auto classType = Type::init (function.getLocation (), ClassPtr::init (function.getLocation (), classType_).to <Type> ().toDeeplyMutable ().to <Type> (), true, false);

	    __validating__.push_back (func.getName ());
	    enterBlock ();
	    this-> insertLocal (Keys::SELF, ProtoVar::init (func.getName (), classType, Generator::empty (), true, 1, true));

	    try {
		auto & __params = function.getPrototype ().getParameters ();
		auto fakeParams = std::vector <syntax::Expression> (__params.begin () + 1, __params.end ());
		auto proto = syntax::Function::Prototype::init (fakeParams, function.getPrototype ().getType (), false);
		validatePrototypeForProto (func.getName (), proto, no_value, params, retType);
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    } 

	    {
		try {
		    this-> discardAllLocals ();
		    this-> quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    std::vector <Generator> throwers;
	    for (auto &it : func.getThrowers ()) {
		try {
		    throwers.push_back (validateType (it));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    __validating__.pop_back ();
	    exitForeign ();
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};		
	    }

	    if (!function.getPrototype ().getParameters ()[0].to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT))
		classType = Type::init (classType.getLocation (), classType.to <Type> (), false, false);
	    
	    auto frame = MethodProto::init (function.getLocation (), func.getComments (), func.getRealName (), retType, params, false,
					    classType,
					    function.getPrototype ().getParameters ()[0].to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT), function.getBody ().isEmpty (), func.isFinal (), func.isSafe (), trait, throwers);
	    
	    return FrameProto::init (frame.to <FrameProto> (), func.getMangledName (), Frame::ManglingStyle::Y);	    
	    return frame;
	}

	

	void Visitor::validatePrototypeForFrame (const lexing::Word &, const syntax::Function::Prototype & proto,  std::vector <Generator> & params, generator::Generator & retType) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector <Generator> addedParams;
	    addedParams.reserve (proto.getParameters ().size ());
	    for (auto & param : proto.getParameters ()) {
		try {
		    auto var = param.to <syntax::VarDecl> ();
		    Generator type (Generator::empty ());
		    Generator value (Generator::empty ());
		    if (!var.getType ().isEmpty ()) {
			type = validateType (var.getType ());
		    }
		
		    if (!var.getValue ().isEmpty ()) {
			value = validateValue (var.getValue ());
			if (!type.isEmpty ())
			    verifyCompatibleTypeWithValue (type.getLocation (), type, value);
			else {
			    type = Type::init (value.to <Value> ().getType ().to <Type> (), false);
			}
		    }
			
		    bool isMutable = false;
		    bool isRef = false;
		    bool dmut = false;
			
		    type = applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut);
						
		    verifyMutabilityRefParam (var.getLocation (), type, MUTABLE_CONST_PARAM);
			
		    if (!value.isEmpty ()) {		    
			verifyMemoryOwner (value.getLocation (), type, value, true);
		    }

		    if (type.is <NoneType> () || type.is<Void> ()) {
			Ymir::Error::occur (var.getLocation (), ExternalError::get (VOID_VAR));
		    } else if (type.is <generator::LambdaType> ()) {
			Ymir::Error::occur (type.getLocation (), ExternalError::get (INCOMPLETE_TYPE), type.prettyString ());
		    }
		
		    addedParams.push_back (ParamVar::init (var.getName (), type, isMutable, false));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    // We insert them after validation to avoid cross referencing value of param
	    for (auto & param : addedParams) {
		if (param.getLocation () != Keys::UNDER) {
		    verifyShadow (param.getLocation ());		
		    insertLocal (param.getLocation ().getStr (), param);
		}
	    }
	    
	    params.insert (params.end (), addedParams.begin (), addedParams.end ());
	    
	    try {
		if (!proto.getType ().isEmpty ()) {
		    retType = validateType (proto.getType (), true);
		    if (retType.to <Type> ().isRef ()) {
			Ymir::Error::occur (retType.getLocation (), ExternalError::get (REF_RETURN_TYPE), retType.prettyString ());
		    }		    
		}		
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	    

	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};		
	    }
	    
	}	
	
	void Visitor::validatePrototypeForProto (const lexing::Word & loc, const syntax::Function::Prototype & proto, bool no_value, std::vector <Generator> & params, generator::Generator & retType) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector <Generator> addedParams;
	    addedParams.reserve (proto.getParameters ().size ());
	    
	    for (auto & param : proto.getParameters ()) {
		try {
		    auto var = param.to <syntax::VarDecl> ();
		    Generator type (Generator::empty ()); // C++ macros are a mystery to me.
		    // You just can't declare two vars in a row !!

		    Generator value (Generator::empty ());
		    
		    if (!var.getType ().isEmpty ()) {						
			type = validateType (var.getType ());
		    }
		    		
		    if (!var.getValue ().isEmpty () && !no_value) {
			value = validateValue (var.getValue ());
			if (!type.isEmpty ()) 
			    verifyCompatibleTypeWithValue (param.getLocation (), type, value);
		    	else {
		    	    type = value.to <Value> ().getType ();
		    	}
		    }

		    if (var.getType ().isEmpty () && no_value && !var.getValue ().isEmpty ()) {
			Ymir::Error::occur (var.getLocation (), ExternalError::get (FORWARD_REFERENCE_VAR));
		    }
				
		    bool isMutable = false;
		    bool isRef = false;
		    bool dmut = false;
		    type = applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut);
		    
		    verifyMutabilityRefParam (var.getLocation (), type, MUTABLE_CONST_PARAM);
		
		    if (!value.isEmpty ()) {		    
		    	verifyMemoryOwner (value.getLocation (), type, value, true);
		    }

		    if (type.is <NoneType> () || type.is<Void> ()) {
			Ymir::Error::occur (var.getLocation (), ExternalError::get (VOID_VAR));
		    }
		    
		    if (type.is <generator::LambdaType> ()) {
			Ymir::Error::occur (type.getLocation (), ExternalError::get (INCOMPLETE_TYPE), type.prettyString ());
		    }

		    int nb_consumed = 1;
		    if (!var.getType ().isEmpty () && var.getType ().is <TemplateSyntaxList> ()) {
			nb_consumed = var.getType ().to <TemplateSyntaxList> ().getContents ().size ();
		    }
		    
		    addedParams.push_back (ProtoVar::init (var.getName (), type, value, isMutable, nb_consumed, false));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    errors.push_back (Ymir::Error::createNote (param.getLocation ()));
		} 
	    }

	    // We insert them after validation to avoid cross referencing value of param
	    for (auto & param : addedParams) {
		if (param.getName () != Keys::UNDER) {
		    verifyShadow (param.getLocation ());		
		    insertLocal (param.getLocation ().getStr (), param);
		}
	    }

	    params.insert (params.end (), addedParams.begin (), addedParams.end ());
	    
	    try {
		if (!proto.getType ().isEmpty ()) {
		    retType = validateType (proto.getType (), true);
		    if (retType.to <Type> ().isRef ()) {
			Ymir::Error::occur (retType.getLocation (), ExternalError::get (REF_RETURN_TYPE), retType.prettyString ());
		    }		    
		} else retType = Void::init (loc);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
	    
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};		
	    }

	}
	
	Generator Visitor::validateVarDeclValue (const syntax::VarDecl & var, bool needInitValue) {
	    if (var.getName () != Keys::UNDER)
		verifyShadow (var.getName ());

	    Generator value (Generator::empty ());
	    if (!var.getValue ().isEmpty ()) {
		value = validateValue (var.getValue ());
	    }

	    if (var.getValue ().isEmpty () && var.getType ().isEmpty ()) {
		Error::occur (var.getLocation (), ExternalError::get (VAR_DECL_WITH_NOTHING));
	    }

	    Generator type (Generator::empty ());
	    if (!var.getType ().isEmpty ()) {
		type = validateType (var.getType ());
	    } else {
		type = Type::init (value.to <Value> ().getType ().to <Type> (), false, false);
	    }

	    
	    if (var.getValue ().isEmpty () && needInitValue) {
		Error::occur (var.getLocation (), ExternalError::get (VAR_DECL_WITHOUT_VALUE));
	    } 
		    
	    bool isMutable = false, isRef = false, dmut = false;
	    type = applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut);
	    
	    if (!value.isEmpty ()) {
		// We do not check the lambdatype complete type if the var is not mutable
		if (isMutable || !type.is <LambdaType> ())
		    verifyMemoryOwner (var.getLocation (), type, value, true);
	    }

	    if (type.is<NoneType> () || type.is<Void> ()) {
		Ymir::Error::occur (var.getLocation (), ExternalError::get (VOID_VAR));
	    }
	    
	    
	    auto ret = generator::VarDecl::init (var.getLocation (), var.getName ().getStr (), type, value, isMutable);
	    if (var.getName () != Keys::UNDER)
		insertLocal (var.getName ().getStr (), ret);
	    return ret;
	}
	
	Generator Visitor::validateDecoratedExpression (const syntax::DecoratedExpression & dec_expr) {
	    Generator inner = Generator::empty ();
	    if (dec_expr.hasDecorator (syntax::Decorator::CTE)) {
		inner = validateCteValue (dec_expr.getContent ());
	    } else {
		inner = validateValue (dec_expr.getContent ());
	    }
	    
	    if (dec_expr.hasDecorator (syntax::Decorator::MUT)) {
		if (!inner.to<Value> ().getType ().to<Type> ().isMutable ()) 
		    Ymir::Error::occur (dec_expr.getDecorator (syntax::Decorator::MUT).getLocation (),
					ExternalError::get (DISCARD_CONST)
		    );
		else
		    Ymir::Error::warn (dec_expr.getDecorator (syntax::Decorator::MUT).getLocation (),
				       ExternalError::get (USELESS_DECORATOR)
			);
	    } 

	    if (dec_expr.hasDecorator (syntax::Decorator::DMUT)) {
		if (!inner.to<Value> ().getType ().to<Type> ().isMutable ()) 
		    Ymir::Error::occur (dec_expr.getDecorator (syntax::Decorator::DMUT).getLocation (),
					ExternalError::get (DISCARD_CONST)
		    );
		else
		    Ymir::Error::warn (dec_expr.getDecorator (syntax::Decorator::DMUT).getLocation (),
				       ExternalError::get (USELESS_DECORATOR)
		    );
	    } 
	    
	    if (dec_expr.hasDecorator (syntax::Decorator::REF)) {
		if (inner.to <Value> ().isLvalue ()) {
		    if (inner.is <VarRef> () && inner.to<VarRef> ().isSelf ()) {
			Ymir::Error::occur (inner.getLocation (),
					    ExternalError::get (REF_SELF)
			);
		    }
		    
		    this-> verifyLockAlias (inner);
		    
		    // if (!inner.to <Value> ().getType ().to <Type> ().isMutable ()) {
		    // 	Ymir::Error::occur (inner.getLocation (), ExternalError::get (IMMUTABLE_LVALUE));
		    // } // We allow this, since we want to pass element by const reference to function, or variable
		    // The mutability will verify if we are allowed to do a reference of the element

		    if (!inner.is<Referencer> ()) {
			auto type = inner.to <Value> ().getType ();
			type = Type::init (type.to <Type> (), type.to <Type> ().isMutable (), true);
			inner = Referencer::init (dec_expr.getLocation (), type, inner);
		    } 
		} else {
		    Ymir::Error::occur (inner.getLocation (),
					ExternalError::get (NOT_A_LVALUE)
		    );
		}
	    }

	    if (dec_expr.hasDecorator (syntax::Decorator::CONST)) {
		auto type = Type::init (inner.to<Value> ().getType ().to <Type> (), false);
		inner = Value::init (inner.to<Value> (), type);
	    }

	    return inner;
	}

	Generator Visitor::validateIfExpression (const syntax::If & _if) {
	    Generator test (Generator::empty ());
	    if (!_if.getTest ().isEmpty ()) { // Has a test if it is not an else
		test = validateValue (_if.getTest ());
		
		if (!test.to<Value> ().getType ().is <Bool> ()) {
		    Ymir::Error::occur (test.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					test.to <Value> ().getType ().to <Type> ().getTypeName (),
					Bool::NAME
		    );
		}
	    }

	    auto content = validateValue (_if.getContent (), false, false, false);
	    auto type = content.to <Value> ().getType ();

	    if (!_if.getElsePart ().isEmpty ()) {
		auto _else = validateValue (_if.getElsePart (), false, false, false);
		if (!_else.to <Value> ().isReturner () && !_else.to <Value> ().isBreaker ()) {
		    if (!_else.to <Value> ().getType ().to <Type> ().isCompatible (type)) {
			auto anc = getCommonAncestor (_else.to <Value> ().getType (), type);
			if (!anc.isEmpty ())
			    type = anc;
		    }

		    try {
			type = this-> deduceTypeBranching (content.getLocation (), _else.getLocation (), type, _else.to<Value> ().getType ());
		    } catch (Error::ErrorList list) {
			Ymir::Error::occurAndNote (_if.getLocation (), list.errors, ExternalError::get (BRANCHING_VALUE));
		    }
		}

		if (content.to <Value> ().isReturner () || content.to <Value> ().isBreaker ()) type = _else.to <Value> ().getType ();
		return Conditional::init (_if.getLocation (), type, test, content, _else);	    
	    } else {
		verifyCompatibleType (_if.getLocation (), _if.getLocation (), Void::init (_if.getLocation ()), type);
		// We check if the type is void, it impose to add a ; at the end of if expression
		
		return Conditional::init (_if.getLocation (), type, test, content, Generator::empty ());
	    }
	}

	
	Generator Visitor::validateCteIfExpression (const syntax::If & _if) {
	    Generator test (Generator::empty ());
	    if (!_if.getTest ().isEmpty ()) {
		auto test = validateValue (_if.getTest ());
		auto value = retreiveValue (test);
		if (!value.is<BoolValue> ()) {
		    Ymir::Error::occur (test.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					test.to <Value> ().getType ().to <Type> ().getTypeName (),
					Bool::NAME
		    );
		    return Generator::empty ();
		}
		if (value.to <BoolValue> ().getValue ()) {
		    return validateValue (_if.getContent (), false, false, false);
		} else if (!_if.getElsePart ().isEmpty ()) {
		    return validateValue (_if.getElsePart ());
		} else return Block::init (_if.getLocation (), Void::init (_if.getLocation ()), {});
	    } else return validateValue (_if.getContent (), false, false, false);
	}	
	
	Generator Visitor::validateWhileExpression (const syntax::While & _wh) {
	    Generator test (Generator::empty ());
	    if (!_wh.getTest ().isEmpty ()) {
		test = validateValue (_wh.getTest ());
		if (!test.to <Value> ().getType ().is <Bool> ()) {
		    Ymir::Error::occur (test.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					test.to <Value> ().getType ().to <Type> ().getTypeName (),
					Bool::NAME
		    );
		}
	    }

	    std::list <Ymir::Error::ErrorMsg> errors;
	    enterLoop ();
	    Generator content (Generator::empty ());
	    try {
		content = validateValue (_wh.getContent (), false, false, false);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	    
	    
	    auto breakType = quitLoop ();
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    Generator type (Generator::empty ());
	    if (!test.isEmpty ()) {
		type = content.to <Value> ().getType ();

		if (!breakType.isEmpty () && !content.to <Value> ().isBreaker () && !type.equals (breakType)) {
		    type = this-> deduceTypeBranching (content.getLocation (), breakType.getLocation (), content.to <Value> ().getType (), breakType);
		} else if (content.to <Value> ().isBreaker ()) {
		    type = breakType;
		}
	    } else {
		if (breakType.isEmpty ()) type = Void::init (_wh.getLocation ());
		else type = breakType;
	    }

	    return Loop::init (_wh.getLocation (), type, test, content, _wh.isDo ());	    
	}	

	Generator Visitor::validateForExpression (const syntax::For & _for, bool isCte) {
	    auto forVisitor = ForVisitor::init (*this);
	    Generator content (Generator::empty ());
	    enterLoop ();
	    std::list <Ymir::Error::ErrorMsg> errors;
	    try {
		if (isCte)
		    content = forVisitor.validateCte (_for);
		else
		    content = forVisitor.validate (_for);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	    
	    
	    auto breakType = quitLoop ();
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    auto type = content.to <Value> ().getType ();
	    if (!breakType.isEmpty () && !content.to <Value> ().isBreaker () && !type.equals (breakType)) {
		type = this-> deduceTypeBranching (content.getLocation (), breakType.getLocation (), content.to <Value> ().getType (), breakType);			    
	    }
	    
	    return Value::init (content.to <Value> (), type);
	}
	
	Generator Visitor::validateBreak (const syntax::Break & _break) {
	    if (!this-> isInLoop ())
		Ymir::Error::occur (_break.getLocation (), ExternalError::get (BREAK_NO_LOOP));
				    
	    Generator value = Generator::empty ();
	    Generator type = Generator::empty ();
	    if (!_break.getValue ().isEmpty ()) {
		value = validateValue (_break.getValue ());
		type = Type::init (_break.getLocation (), value.to <Value> ().getType ().to <Type> ());
	    } else type = Void::init (_break.getLocation ());

	    bool set = false;
	    auto loop_type = getCurrentLoopType ();
	    if (loop_type.isEmpty ()) {
		setCurrentLoopType (type);
		loop_type = type;
		set = true;
	    } else if (!loop_type.equals (type)) {
		auto anc = getCommonAncestor (loop_type, type);
		if (!anc.isEmpty ()) {
		    loop_type = anc;
		    setCurrentLoopType (anc);
		} else {		
		    auto note = Ymir::Error::createNote (loop_type.getLocation ());
		    Ymir::Error::occurAndNote (value.getLocation (), note, ExternalError::get (INCOMPATIBLE_TYPES),
					       type.to <Type> ().getTypeName (),
					       loop_type.to <Type> ().getTypeName ()
		    );
		}
	    }

	    if (!loop_type.is<Void> ()) {
		if (set) {
		    try {
		    	verifyMemoryOwner (_break.getLocation (), loop_type, value, false);
		    } catch (Error::ErrorList err) { // Maybe implicit alias problem, we set the type to non mutable, to check
		    	loop_type = Type::init (loop_type.to <Type> (), false, loop_type.to <Type> ().isRef ());
		    	setCurrentLoopType (loop_type);
		    	verifyMemoryOwner (_break.getLocation (), loop_type, value, false); // if this pass, the loop type is const, and it is ok
		    }		    
		} else {
		    loop_type = this-> deduceTypeBranching (_break.getLocation (), loop_type.getLocation (), value.to <Value> ().getType (), loop_type);
		    setCurrentLoopType (loop_type);
		}
	    }
	    
	    return Break::init (_break.getLocation (), Void::init (_break.getLocation ()), value);
	}

	Generator Visitor::validateReturn (const syntax::Return & rt) {
	    Generator value = Generator::empty ();
	    Generator type = Generator::empty ();
	    if (!rt.getValue ().isEmpty ()) {
	    	value = validateValue (rt.getValue ());
	    	type = value.to <Value> ().getType ();
	    } else type = Void::init (rt.getLocation ());

	    auto fn_type = getCurrentFuncType ();

	    /** In case of lambda proto, we are able to validate it with the function return type */
	    if (type.is <LambdaType> () && (fn_type.is <FuncPtr> () || fn_type.is <Delegate> ())) {
		std::vector <Generator> paramTypes;
		if (fn_type.is <FuncPtr> ()) paramTypes = fn_type.to <FuncPtr> ().getParamTypes ();
		else paramTypes = fn_type.to <Delegate> ().getInners ()[0].to <FuncPtr> ().getParamTypes ();

		if (value.is <VarRef> ()) {
		    value = validateLambdaProto (value.to <VarRef> ().getValue ().to <LambdaProto> (), paramTypes);
		    type = value.to <Value> ().getType ();
		} else if (value.is <LambdaProto> ()) {
		    value = validateLambdaProto (value.to <LambdaProto> (), paramTypes);
		    type = value.to <Value> ().getType ();
		} // This is working exactly like findParameter for function, maybe we can merge those two blocks
	    }

	    try {
		if (fn_type.isEmpty ()) {
		    this-> setCurrentFuncType (type);
		    fn_type = type;
		} else if (!value.isEmpty ()) { // If value, it can be a mut [mut void]
		    verifyCompatibleTypeWithValue (fn_type.getLocation (), fn_type, value);
		} else
		    verifyCompatibleType (fn_type.getLocation (), type.getLocation (), fn_type, type);
	    } catch (Error::ErrorList list) {
		list.errors.back ().addNote (Ymir::Error::createNote (rt.getLocation()));
		throw list;
	    }
	    
	    // 	if (!fn_type.equals (type)) {
	    // 	auto note = Ymir::Error::createNote (fn_type.getLocation ());
	    // 	Ymir::Error::occurAndNote (value.getLocation (), note, ExternalError::get (INCOMPATIBLE_TYPES),
	    // 				   type.to <Type> ().getTypeName (),
	    // 				   fn_type.to <Type> ().getTypeName ()
	    // 	);				    
	    // }

	    if (!fn_type.is<Void> ()) {
		verifyMemoryOwner (rt.getLocation (), fn_type, value, true);
	    }
	    
	    return Return::init (rt.getLocation (), Void::init (rt.getLocation ()), fn_type, value);
	}
	    
	Generator Visitor::validateList (const syntax::List & list) {
	    if (list.isArray ()) return validateArray (list);
	    if (list.isTuple ()) return validateTuple (list);
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator Visitor::validateListTemplate (const TemplateSyntaxList & lst) {
	    std::vector <syntax::Expression> exprs;
	    for (auto & it : lst.getContents ())
		exprs.push_back (TemplateSyntaxWrapper::init (it.getLocation (), it));
	    return validateTuple (syntax::List::init (lst.getLocation (), lst.getLocation (), exprs).to <syntax::List> ());
	}
	
	Generator Visitor::validateString (const syntax::String & str, bool forceUtf8) {
	    Generator inner (Generator::empty ());
	    if (str.getSuffix () == Keys::S8 || forceUtf8) inner = Char::init (str.getLocation (), 8);
	    else inner = Char::init (str.getLocation (), 32);
	    inner = Type::init (inner.to <Type> (), false);
	    
	    auto visitor = UtfVisitor::init (*this);
	    int len = 0;
	    auto value = visitor.convertString (str.getLocation (), str.getSequence (), inner.to <Char> ().getSize (), len);

	    auto type = Array::init (str.getLocation (), inner, len);
	    type = Type::init (type.to <Type> (), true);

	    auto sliceType = Slice::init (str.getLocation (), inner);
	    sliceType = Type::init (sliceType.to <Type> (), true);
	    
	    return Aliaser::init (
		str.getLocation (),
		sliceType, 
		StringValue::init (str.getLocation (), type, value, len)
	    );
	}
	
	Generator Visitor::validateArray (const syntax::List & list) {
	    std::vector <Generator> params;
	    Generator innerType (Generator::empty ());
	    for (auto it : list.getParameters ()) {		
		auto val = validateValueNonVoid (it);
		if (val.is<List> ()) {
		    for (auto & g_it : val.to <List> ().getParameters ()) {
			params.push_back (g_it);
			if (innerType.isEmpty ()) {
			    innerType = params [0].to <Value> ().getType ();
			} else {
			    innerType = this-> deduceTypeBranching (params [0].getLocation (), params.back ().getLocation (), innerType, params.back ().to <Value> ().getType ());
			}
			//verifyMemoryOwner (params.back ().getLocation (), params [0].to <Value> ().getType (), params.back (), false);
		    }
		} else {
		    params.push_back (val);
		    if (innerType.isEmpty ()) {
			innerType = params [0].to <Value> ().getType ();
		    } else {
			innerType = this-> deduceTypeBranching (params [0].getLocation (), params.back ().getLocation (), innerType, params.back ().to <Value> ().getType ());
		    }
		    //verifyMemoryOwner (params.back ().getLocation (), params [0].to <Value> ().getType (), params.back (), false);
		}
	    }

	    if (innerType.isEmpty ()) {
		innerType = Void::init (list.getLocation ());
	    }
	    	    
	    innerType = Type::init (innerType.to <Type> (), innerType.to <Type> ().isMutable (), false);
	    
	    // An array literal is always static
	    auto type = Array::init (list.getLocation (), innerType, params.size ());
	    type = Type::init (type.to <Type> (), true); // Array constant are mutable by default (not lvalue), to ease simple affectation
	    innerType = Type::init (innerType.to <Type> (), true);
	    
	    auto slc = Slice::init (list.getLocation (), innerType);
	    slc = Type::init (slc.to <Type> (), true);
	    
	    if (!innerType.is <Void> ()) {
		return Copier::init (list.getLocation (),
				     slc,
				     Aliaser::init (list.getLocation (), slc,
						    ArrayValue::init (list.getLocation (), type.to <Type> ().toDeeplyMutable (), params)
				     )
		);
	    } else
		return
		    Aliaser::init (list.getLocation (), slc,
				   ArrayValue::init (list.getLocation (), type.to <Type> ().toDeeplyMutable (), params)
		    );
	}	
	
	Generator Visitor::validateTuple (const syntax::List & list) {
	    std::vector <Generator> params;
	    std::vector <Generator> types;
	    for (auto it : list.getParameters ()) {
		auto val = validateValue (it, false, false);
		if (val.is <List> ()) {
		    for (auto & g_it : val.to<List> ().getParameters ()) {
			if (g_it.to <Value> ().getType ().is<NoneType> () || g_it.to <Value> ().getType ().is <Void> ()) {
			    Ymir::Error::occur (g_it.getLocation (), ExternalError::get (VOID_VALUE));
			}
			
			params.push_back (g_it);
			auto type = params.back ().to <Value> ().getType ();
			try {
			    verifyMemoryOwner (params.back ().getLocation (), type, params.back (), false);
			    types.push_back (Type::init (type.to <Type> (), type.to <Type> ().isMutable (), false));
			} catch (Error::ErrorList ATTRIBUTE_UNUSED lst) { // maybe there was an implicit alias, that is not a problem
			    type = Type::init (type.to <Type> (), false); // we just put it as a const one
			    verifyMemoryOwner (params.back ().getLocation (), type, params.back (), false);
			    types.push_back (Type::init (type.to <Type> (), type.to <Type> ().isMutable (), false));
			}
		    }
		} else {
		    if (val.to <Value> ().getType ().is<NoneType> () || val.to <Value> ().getType ().is <Void> ()) {
			Ymir::Error::occur (val.getLocation (), ExternalError::get (VOID_VALUE));
		    }
		    params.push_back (val);
		    auto type = params.back ().to <Value> ().getType ();
		    try {
			verifyMemoryOwner (params.back ().getLocation (), type, params.back (), false);
			types.push_back (Type::init (type.to <Type> (), type.to <Type> ().isMutable (), false));
		    } catch (Error::ErrorList ATTRIBUTE_UNUSED lst) { // maybe there was an implicit alias, that is not a problem
			type = Type::init (type.to <Type> (), false); // we just put it as a const one
			verifyMemoryOwner (params.back ().getLocation (), type, params.back (), false);
			types.push_back (Type::init (type.to <Type> (), type.to <Type> ().isMutable (), false));
		    }
		}
	    }
	    
	    auto type = Tuple::init (list.getLocation (), types);
	    type = Type::init (type.to <Type> (), true); // Tuple are mutable by default (not lvalue)
	    
	    return TupleValue::init (list.getLocation (), type, params);	    
	}

	Generator Visitor::validateTemplateChecker (const syntax::TemplateChecker & check) {
	    std::vector<Generator> params;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    for (auto & it : check.getCalls ()) {
		bool succeed = true;
		std::list <Ymir::Error::ErrorMsg> locErrors;
		try {
		    params.push_back (validateType (it, true));
		} catch (Error::ErrorList list) {
		    locErrors.insert (locErrors.begin (), list.errors.begin (), list.errors.end ());
		    succeed = false;
		} 

		if (!succeed) {
		    succeed = true;
		    try {
			auto val = validateValue (it);
			auto rvalue = retreiveValue (val);
			params.push_back (rvalue);			
		    } catch (Error::ErrorList list) {
			succeed = false;
		    } 		    
		}
		
		if (!succeed)
		    errors.insert (errors.end (), locErrors.begin (), locErrors.end ());
	    }

	    if (errors.size () != 0)
		throw Error::ErrorList {errors};

	    bool succeed = false; // again, due to longjmp
	    
	    try {
		auto visitor = TemplateVisitor::init (*this);
		auto mapper = visitor.validateFromExplicit (check.getParameters (), params);
		succeed = mapper.succeed;
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 

	    return BoolValue::init (check.getLocation (), Bool::init (check.getLocation ()), succeed);
	}

	Generator Visitor::validateThrow (const syntax::Throw & thr) {
	    auto inner = this-> validateValue (thr.getValue ());
	    auto type = inner.to <Value> ().getType ();
	    
	    auto uniq = UniqValue::init (thr.getLocation (), type, inner);
	    
	    auto syntaxType = createClassTypeFromPath (thr.getLocation (), {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (EXCEPTION_TYPE)});
	    auto ancType = validateType (syntaxType);

	    verifyCompatibleType (thr.getLocation (), type.getLocation (), ancType, type);
	    
	    auto loc = thr.getLocation ();
	    auto bin = syntax::Binary::init (lexing::Word::init (loc, Token::DCOLON),
					     TemplateSyntaxWrapper::init (loc, uniq),
					     syntax::Var::init (lexing::Word::init (loc, SubVisitor::__TYPEINFO__)),
					     syntax::Expression::empty ()
	    );
	    auto info = validateValue (bin);
	    
	    return Throw::init (thr.getLocation (), info, uniq);
	}

	Generator Visitor::validateMatch (const syntax::Match & matcher) {
	    auto visitor = MatchVisitor::init (*this);
	    return visitor.validate (matcher);
	}

	Generator Visitor::validateCatchOutOfScope (const syntax::Catch & cat) {
	    Ymir::Error::occur (cat.getLocation (), ExternalError::get (CATCH_OUT_OF_SCOPE));
	    return Generator::empty ();
	}

	Generator Visitor::validateScopeOutOfScope (const syntax::Scope & scope) {
	    Ymir::Error::occur (scope.getLocation (), ExternalError::get (SCOPE_OUT_OF_SCOPE));
	    return Generator::empty ();
	}

	Generator Visitor::validateAssert (const syntax::Assert & assert) {
	    auto test = validateValue (assert.getTest ());
	    if (!test.to <Value> ().getType ().is <Bool> ()) {
		Ymir::Error::occur (test.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    test.to <Value> ().getType ().to <Type> ().getTypeName (),
				    Bool::NAME
		);
	    }

	    std::vector <syntax::Expression> params;
	    params.push_back (TemplateSyntaxWrapper::init (test.getLocation (), test));
	    if (!assert.getMsg ().isEmpty ()) {
		params.push_back (assert.getMsg ());
	    }

	    auto loc = assert.getLocation ();
	    auto func = createVarFromPath (assert.getLocation (), {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (ASSERT_FUNC)});
	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		func,
		params
		);
	    
	    auto ret = validateValue (call);
	    try {
	    	auto val = retreiveValue (test);
	    	ret = Value::initBrRet (ret.to <Value> (), ret.to <Value> ().isBreaker (), !val.to <BoolValue> ().getValue (), assert.getLocation (), assert.getLocation ());
	    } catch (Error::ErrorList list) {
	    } 
	    
	    return ret;
	}
		
	Generator Visitor::validateCteAssert (const syntax::Assert & assert) {
	    auto test = validateValue (assert.getTest ());
	    if (!test.to <Value> ().getType ().is <Bool> ()) {
		Ymir::Error::occur (test.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    test.to <Value> ().getType ().to <Type> ().getTypeName (),
				    Bool::NAME
		);
	    }

	    auto val = retreiveValue (test);
	    if (!val.to <BoolValue> ().getValue ()) {
		std::string msg;
		if (!assert.getMsg ().isEmpty ()) {
		    try {
			auto msgVal = retreiveValue (validateValue (assert.getMsg ()));
			std::vector <char> text;
			if (msgVal.is <StringValue> ()) {
			    text = msgVal.to <StringValue> ().getValue ();
			} else if (msgVal.is <Aliaser> () && msgVal.to <Aliaser> ().getWho ().is <StringValue> ()) {
			    text = msgVal.to <Aliaser> ().getWho ().to <StringValue> ().getValue ();
			}
			if (text.size () != 0) {
			    Ymir::OutBuffer buf;
			    text = UtfVisitor::utf32_to_utf8 (text);
			    for (int i = 0 ; i < (int) text.size () - 1; i++) buf.write (text [i]);
			    msg = buf.str ();
			} else {
			    msg = msgVal.prettyString ();
			}
		    } catch (Ymir::Error::ErrorList) {
			msg = assert.getMsg ().prettyString ();
		    }
		}
		Ymir::Error::occur (assert.getLocation (), ExternalError::get (ASSERT_FAILED), msg);
	    }
	    
	    return None::init (assert.getLocation ());
	}

	Generator Visitor::validateMacroCall (const syntax::MacroCall & call) {
	    auto visitor = MacroVisitor::init (*this);
	    return visitor.validate (call);
	}	

	Generator Visitor::validatePragma (const syntax::Pragma & prg) {
	    auto visitor = PragmaVisitor::init (*this);
	    return visitor.validate (prg);	    
	}

	Generator Visitor::validateDollar (const syntax::Dollar & dl) {
	    auto loc = dl.getLocation ();
	    if (this-> _dollars.size () != 0) {
	    	auto left = this-> _dollars.back ();
	    	if (left.to <Value> ().getType ().is <Slice> ()) {
	    	    return StructAccess::init (loc,
	    				       Integer::init (dl.getLocation (), 64, false),
	    				       left, Slice::LEN_NAME
	    		);
	    	} else if (left.to <Value> ().getType ().is <Array> ()) {
	    	    return ufixed (left.to <Value> ().getType ().to <Array> ().getSize ());
	    	} else if (left.to <Value> ().getType ().is <ClassPtr> ()) {
	    	    auto leftSynt = TemplateSyntaxWrapper::init (loc, left);
	    	    auto bin = syntax::Binary::init (
	    		lexing::Word::init (loc, Token::DOT),
	    		leftSynt,
	    		syntax::Var::init (lexing::Word::init (loc, CoreNames::get (DOLLAR_OP_OVERRIDE))),
	    		syntax::Expression::empty ()
	    		);

	    	    auto call = syntax::MultOperator::init (
	    		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
	    		bin,
	    		{}, false
	    	    );
		    
	    	    return validateValue (call);
	    	} 		  
	    }
	    
	    Ymir::Error::occur (dl.getLocation (), ExternalError::get (DOLLAR_OUSIDE_CONTEXT));
	    return Generator::empty ();
	}

	Generator Visitor::validateTry (const syntax::Try & tr) {
	    auto inner = this-> validateValue (tr.getContent ());
	    auto syntaxType = createClassTypeFromPath (tr.getLocation (), {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (EXCEPTION_TYPE)});
	    auto errType = Type::init (validateType (syntaxType).to <Type> (), false, false);
	    
	    auto optionType = Option::init (tr.getLocation (), inner.to <Value> ().getType (), errType);

	    bool needAlias = false;
	    if (inner.to <Value> ().getType ().to <Type> ().isMutable ()) {
		optionType = Type::init (optionType.to <Type> (), true);
		needAlias = true;
	    }
		    
	    auto throwsType = inner.getThrowers ();
	    inner = OptionValue::init (tr.getLocation (), optionType, inner, true);
	    if (needAlias)
		inner = Aliaser::init (tr.getLocation (), optionType, inner);
	    if (throwsType.size () != 0) {
		auto jmp_buf_type = validateType (syntax::Var::init (lexing::Word::init (tr.getLocation (), global::CoreNames::get (JMP_BUF_TYPE))));

		auto loc = tr.getLocation ();
		auto varDecl = generator::VarDecl::init (lexing::Word::init (loc, "#catch"), "#catch", errType, Generator::empty (), false);
		auto typeInfo = validateTypeInfo (loc, errType);
		auto vref = VarRef::init (loc, "#catch", errType, varDecl.getUniqId (),  false, Generator::empty ());

		auto outer = OptionValue::init (tr.getLocation (), optionType, vref, false);

		auto ret = ExitScope::init (tr.getLocation (), optionType, jmp_buf_type, inner, {}, {}, varDecl, typeInfo, outer);		
		if (needAlias)
		    ret = Aliaser::init (tr.getLocation (), optionType, ret);
		return ret;
	    }
	    
	    return inner;
	}

	Generator Visitor::validateWith (const syntax::With & wh) {
	    enterBlock ();
	    std::vector <Generator> varDecls;
	    std::vector <Generator> exits;
	    
	    std::list <Error::ErrorMsg> errors;
	    for (auto & it : wh.getDecls ()) {
		try {
		    varDecls.push_back (this-> validateVarDeclValue (it.to <syntax::VarDecl> (), true));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    }

	    for (auto & it : varDecls) {
		try {
		    auto vdecl = it.to <generator::VarDecl> ();
		    auto loc = vdecl.getLocation ();
		    auto trait = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (DISPOSING_MODULE), CoreNames::get (DISPOSABLE_TRAITS)});		
		    auto impl = validateType (trait);
		    
		    if (!vdecl.getVarType ().is <ClassPtr> ()) {
			Ymir::Error::occur (vdecl.getLocation (), ExternalError::get (NOT_IMPL_TRAIT), vdecl.getVarType ().prettyString (), impl.prettyString ());	
		    } else {
			verifyClassImpl (vdecl.getLocation (), vdecl.getVarType (), impl);
		    }

		    auto vRef = VarRef::init (it.getLocation (), vdecl.getName (), vdecl.getVarType (), vdecl.getUniqId (), vdecl.isMutable (), Generator::empty ());
		    auto intr = syntax::Intrinsics::init (lexing::Word::init (loc, Keys::ALIAS), TemplateSyntaxWrapper::init (loc, vRef));
		    auto call = syntax::MultOperator::init (
			lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
			syntax::Binary::init (lexing::Word::init (loc, Token::DOT),
					      intr,
					      syntax::Var::init (lexing::Word::init (loc, global::CoreNames::get (DISPOSE_OP_OVERRIDE))),
					      syntax::Expression::empty ()),
			{}, false
			);
		    exits.push_back (this-> validateValue (call));
		} catch (Error::ErrorList list) {
		    list.errors.back ().addNote (Error::createNote (wh.getLocation ()));
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    }		

	    std::vector <syntax::Expression> scopes;
	    syntax::Expression catcher (syntax::Expression::empty ());
	    
	    Generator ret (Generator::empty ());
	    try {
		ret = this-> validateValue (wh.getContent ());
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }

	    try {
		if (errors.size () != 0)
		    discardAllLocals ();
		
		quitBlock ();
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }
	    
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    auto jmp_buf_type = validateType (syntax::Var::init (lexing::Word::init (wh.getLocation (), global::CoreNames::get (JMP_BUF_TYPE))));
	    for (auto it : Ymir::r (exits.size (), 0)) {
		ret = ExitScope::init (wh.getLocation (), ret.to <Value> ().getType (), jmp_buf_type, ret, {exits [it - 1]}, {}, Generator::empty (), Generator::empty (), Generator::empty ());
		ret = Block::init (wh.getLocation (), ret.to<Value> ().getType (), {varDecls [it - 1], ret});		
	    }
	    
	    return ret;
	}

	Generator Visitor::validateAtomic (const syntax::Atomic & atom) {
	    if (atom.getWho ().isEmpty ()) {
		auto loc = atom.getLocation ();
		auto mutexType = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (ATOMIC_MODULE), CoreNames::get (MUTEX_TYPE)});
		auto mutexInitValue = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (ATOMIC_MODULE), CoreNames::get (MUTEX_INIT)});
				
		auto type = validateType (mutexType);
		auto initValue = this-> validateValue (mutexInitValue);
		auto ptrType = Pointer::init (atom.getLocation (), type);		
		
		
		auto name = Ymir::format ("__atom%", atom.getLocation ().getLine ());
		auto glbVar = GlobalVar::init (lexing::Word::init (atom.getLocation (), name), name, "", false, type, initValue);
		auto vRef = VarRef::init (lexing::Word::init (atom.getLocation (), name), name, type, glbVar.getUniqId (), false, Generator::empty ());
		
		insertNewGenerator (glbVar);
		enterBlock ();
		
		auto inner = this-> validateValue (atom.getContent ());
		inner = Block::init (atom.getLocation (), inner.to <Value> ().getType (),
				     {
					 AtomicLocker::init (atom.getLocation (), Addresser::init (atom.getLocation (), ptrType, vRef), false),
					 inner
				     }
		    );
		auto jmp_buf_type = validateType (syntax::Var::init (lexing::Word::init (atom.getLocation (), global::CoreNames::get (JMP_BUF_TYPE))));
		auto exit = ExitScope::init (atom.getLocation (), inner.to <Value> ().getType (), jmp_buf_type, inner, {
			AtomicUnlocker::init (atom.getLocation (), Addresser::init (atom.getLocation (), ptrType, vRef), false)
		    }, {}, Generator::empty (), Generator::empty (), Generator::empty ()
		    );
		return exit;
	    } else {
		auto value = this-> validateValue (atom.getWho ());
		if (!value.to <Value> ().getType ().is <ClassPtr> ()) {
		    Ymir::Error::occur (atom.getLocation (), ExternalError::get (MONITOR_NON_CLASS), value.to <Value> ().getType ().prettyString ());
		}

		auto inner = this-> validateValue (atom.getContent ());
		inner = Block::init (atom.getLocation (), inner.to <Value> ().getType (),
				     {
					 AtomicLocker::init (atom.getLocation (), value, true),
					 inner
				     }
		    );

		auto jmp_buf_type = validateType (syntax::Var::init (lexing::Word::init (atom.getLocation (), global::CoreNames::get (JMP_BUF_TYPE))));
		auto exit = ExitScope::init (atom.getLocation (), inner.to <Value> ().getType (), jmp_buf_type, inner, {
			AtomicUnlocker::init (atom.getLocation (), value, true)
			    }, {}, Generator::empty (), Generator::empty (), Generator::empty ()
		    );
		return exit;
	    }
	}	
	
	Generator Visitor::validateTypeInfo (const lexing::Word & loc, const Generator & type_) {
	    auto type = Type::init (type_.to <Type> (), false, false);
	    if (type.is <ClassPtr> ())
		type = type.to <ClassPtr> ().getInners ()[0];
	    
	    auto typeInfo = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (TYPE_INFO_MODULE), CoreNames::get (TYPE_INFO)});
		
	    auto str = validateType (typeInfo);

	    auto typeIDs = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (TYPE_INFO_MODULE), CoreNames::get (TYPE_IDS)});
	    auto en_m = validateValue (typeIDs);
	       
	    std::vector <Generator> types = {
		Integer::init (loc, 32, false),
		Integer::init (loc, 0, false),
		Slice::init (loc, str),
		Slice::init (loc, Char::init (loc, 32))
	    };

	    std::vector <Generator> innerTypes;
	    if (!type.is <ClassRef> ()) {
		if (type.to <Type> ().isComplex ()) {
		    for (auto & it : type.to <Type> ().getInners ())
			innerTypes.push_back (validateTypeInfo (loc, it));
		}
	    } else {		
		if (!type.to <ClassRef> ().getAncestor ().isEmpty ()) {
		    innerTypes.push_back (validateTypeInfo (loc, type.to <ClassRef> ().getAncestor ()));
		}
	    }
	    
	    auto arrayType = Array::init (loc, str, innerTypes.size ());
	    auto stringLit = syntax::String::init (loc, loc, lexing::Word::init (loc, type.prettyString ()), lexing::Word::eof ());
	    auto name = validateValue (stringLit);
	    auto constName = Mangler::init ().mangle (type) + "_" + "name";
	    auto constNameInner = Mangler::init ().mangle (type) + "_" + "nameInner";
	    auto sliceType = Slice::init (loc, str);
	    auto inner = name.to <Aliaser> ().getWho ();

	    std::vector <Generator> values = {
		en_m.to <generator::Enum> ().getFieldValue (typeInfoName (type)),
		SizeOf::init (loc, Integer::init (loc, 0, false), type),
		Copier::init (loc, sliceType, Aliaser::init (loc, sliceType, ArrayValue::init (loc, arrayType, innerTypes))),	       
		GlobalConstant::init (loc, constName, name.to <Value> ().getType (),
				      Aliaser::init (loc, name.to <Value> ().getType (),
						     GlobalConstant::init (
							 loc, constNameInner, inner.to<Value> ().getType (), inner
						     )
				      )
		)
	    };
	    
	    return StructCst::init (
		loc,
		str,
		str.to <StructRef> ().getRef ().to <semantic::Struct> ().getGenerator (),
		types,
		values
	    );
	}

	std::string Visitor::typeInfoName (const Generator & type) {
	    // Maybe that's enhanceable
	    match (type) {
		of_u (Void) return "VOID";
		elof_u (Array) return "ARRAY";
		elof_u (Bool) return "BOOL";
		elof_u (Char) return "CHAR";
		elof_u (Closure) return "CLOSURE";
		elof_u (Float) return "FLOAT";
		elof_u (FuncPtr) return "FUNC_PTR";
		elof (Integer,  i) {
		    if (i.isSigned ()) return "SIGNED_INT";
		    return "UNSIGNED_INT";
		}
		elof_u (Pointer) return "POINTER";
		elof_u (Slice) return "SLICE";
		elof_u (StructRef) return "STRUCT";
		elof_u (Tuple) return "TUPLE";
		elof_u (ClassRef) return "OBJECT";
		fo;
	    }
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return "";
	}

	// Generator Visitor::validateDotTemplateCall (const syntax::TemplateCall & bin) {
	//     auto right = bin.getContent ().to <syntax::Binary> ().getRight ();
	//     auto rightTmpl = syntax::TemplateCall::init (bin.getLocation (), bin.getParameters (), right);
	//     auto leftTmpl = bin.getContent ().to<syntax::Binary> ().getLeft ();
	//     auto bin = syntax::Binary::init (bin.getContent ().getLocation (),
	// 				     leftTmpl,
	// 				     rightTmpl,
	// 				     syntax::Expression::empty ()
	// 	);
	//     try {
		
	//     }
	// }

	// I hate the error handling of this function,
	// TODO refactor all that an factorise it, the errors seem to be handled the same for multsym and simple value
	Generator Visitor::validateTemplateCall (const syntax::TemplateCall & tcl) {	    
	    auto value = this-> validateValue (tcl.getContent (), false, true);

	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector <Generator> params;
	    for (auto & it : tcl.getParameters ()) {
		bool succeed = true;
		std::list <Ymir::Error::ErrorMsg> locErrors;
		try {
		    params.push_back (validateType (it, true));
		} catch (Error::ErrorList list) {
		    locErrors.insert (locErrors.begin (), list.errors.begin (), list.errors.end ());
		    succeed = false;
		} 

		if (!succeed) {
		    succeed = true;
		    try {
			auto val = validateValue (it);
			auto rvalue = retreiveValue (val);
			params.push_back (rvalue);			
		    } catch (Error::ErrorList list) {
			auto note = Ymir::Error::createNoteOneLine (ExternalError::get (TEMPLATE_VALUE_TRY));
			for (auto & it : list.errors)
			    note.addNote (it);
			
			locErrors.back ().addNote (note);
			succeed = false;
		    } 		    
		}
		
		if (!succeed)
		    errors.insert (errors.end (), locErrors.begin (), locErrors.end ());
	    }
	    
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    if (value.is <TemplateRef> ()) {
		value = MultSym::init (value.getLocation (), {value});
	    }
	    
	    if (value.is<MultSym> ()) {
		int all_score = -1; 
		Symbol final_sym (Symbol::empty ());
		std::map <int, std::vector <Symbol>> loc_scores;
		std::map <int, std::vector <Generator>> loc_elem;
		
		for (auto & elem : value.to <MultSym> ().getGenerators ()) {
		    if (elem.is<TemplateRef> ()) {
			int local_score = 0;
			Symbol local_sym (Symbol::empty ());
			bool succeed = true;
			try {
			    auto templateVisitor = TemplateVisitor::init (*this);
			    local_sym = templateVisitor.validateFromExplicit (elem.to <TemplateRef> (), params, local_score);
			    if (!local_sym.isEmpty ()) {
				loc_scores [local_score].push_back (local_sym);
				loc_elem [local_score].push_back (elem);
			    }
			} catch (Error::ErrorList list) {
			    auto note = Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), elem.getLocation (), elem.prettyString ());
			    for (auto & it : list.errors)			
				note.addNote (it);
			    errors.push_back (note);
			    succeed = false;
			} 

			if (succeed) {
			    if (local_score > all_score) {
				all_score = local_score;
				final_sym = local_sym;
			    } else if (local_sym.isEmpty ()) {
				errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), elem.getLocation (), elem.prettyString ()));
			    }
			}
		    }
		}
		
		if (loc_scores.size () != 0) {
		    errors = {};
		    Generator ret (Generator::empty ());
		    auto &element_on_scores = loc_scores.find ((int) all_score)-> second;
		    auto &location_elems = loc_elem.find ((int) all_score)-> second;
		    std::vector <Symbol> syms;
		    std::vector <Generator> aux;
		    for (auto it : Ymir::r (0, element_on_scores.size ())) {
			std::vector <Generator> types;
			Visitor::__CALL_NB_RECURS__ += 1;
			try {
			    this-> validateTemplateSymbol (element_on_scores [it], loc_elem.find ((int) all_score)-> second [it]);
			    if (location_elems [it].is<MethodTemplateRef> ()) { 
			    	if (element_on_scores [it].is <TemplateSolution> ()) {
			    	    auto self = location_elems [it].to <MethodTemplateRef> ().getSelf ();
			    	    auto proto = this-> validateTemplateSolutionMethod (element_on_scores [it], self);
			    	    auto delType = Delegate::init (proto.getLocation (), proto);
			    	    aux.push_back (				    
			    		DelegateValue::init (proto.getLocation (),
			    				     delType,
			    				     proto.to<MethodProto> ().getClassType (),
			    				     self,
			    				     proto
					    )
					);
			    	} else { // Not finalized template call
			    	    auto & tmpRef = location_elems [it];
			    	    aux.push_back (
			    		MethodTemplateRef::init (tmpRef.getLocation (), element_on_scores [it], tmpRef.to <MethodTemplateRef> ().getSelf ())
					);
			    	}
			    } else {
			    	syms.push_back (element_on_scores [it]);
			    }
			} catch (Error::ErrorList list) {			    
			    static std::list <Error::ErrorMsg> __last_error__;
			    if (Visitor::__CALL_NB_RECURS__ == 2 && !global::State::instance ().isVerboseActive ()) {
				list.errors.insert (list.errors.begin (), format ("     : %(B)", "..."));
				list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (OTHER_CALL)));
			    } else if ((Visitor::__CALL_NB_RECURS__ < 2 || global::State::instance ().isVerboseActive ()) && !Visitor::__LAST_TEMPLATE__) {
				list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", element_on_scores [it].getName (), element_on_scores [it].getRealName ()));
				list.errors.insert (list.errors.begin (), Ymir::Error::createNote (tcl.getLocation (), ExternalError::get (IN_TEMPLATE_DEF)));
				Visitor::__LAST_TEMPLATE__ = true;

				std::vector<std::string> names;
				for (auto & it : params)
				    names.push_back (it.prettyString ());

				std::string leftName = value.getLocation ().getStr ();				
				list.errors = {Ymir::Error::makeOccurAndNote (
					tcl.getLocation (),
					list.errors,
					ExternalError::get (UNDEFINED_TEMPLATE_OP),
					leftName,
					names
					)};
				
				__last_error__ = {};
			    } else if (Visitor::__LAST_TEMPLATE__) {
				list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", element_on_scores [it].getName (), element_on_scores [it].getRealName ()));
				list.errors.insert (list.errors.begin (), Ymir::Error::createNote (tcl.getLocation (), ExternalError::get (IN_TEMPLATE_DEF)));
				
				Visitor::__LAST_TEMPLATE__ = false;
				std::vector<std::string> names;
				for (auto & it : params)
				    names.push_back (it.prettyString ());

				std::string leftName = value.getLocation ().getStr ();
				
				list.errors = {Ymir::Error::makeOccurAndNote (
					tcl.getLocation (),
					list.errors,
					ExternalError::get (UNDEFINED_TEMPLATE_OP),
					leftName,
					names
					)
				};				
				__last_error__ = list.errors;
			    } else {
				if (__last_error__.size () != 0)
				    list.errors = __last_error__;
			    }
			    
			    errors = list.errors;
			}
			Visitor::__CALL_NB_RECURS__ -= 1;
		    }
		    		    
		    if (errors.size () == 0) {
			
			if (syms.size () != 0) { 
			    ret = this-> validateMultSym (value.getLocation (), syms);
			    if (ret.is <MultSym> ()) 
				aux.insert (aux.end (), ret.to <MultSym> ().getGenerators ().begin (), ret.to <MultSym> ().getGenerators ().end ());
			    else aux.push_back (ret);
			}
			
			if (aux.size () == 1)
			    ret = aux [0];
			else ret = MultSym::init (value.getLocation (), aux);		    
		    		    
			if (!ret.isEmpty ())
			    return ret;
		    }
		    
		} 
	    }

	    std::vector<std::string> names;
	    for (auto & it : params)
		names.push_back (it.prettyString ());
		    
	    std::string leftName = value.getLocation ().getStr ();
	    Ymir::Error::occurAndNote (
		tcl.getLocation (),
		errors,
		ExternalError::get (UNDEFINED_TEMPLATE_OP),
		leftName,
		names
		);   	    
	    
	    return Generator::empty ();	    
	}

	Generator Visitor::validateCast (const syntax::Cast & cast) {
	    auto visitor = CastVisitor::init (*this);
	    return visitor.validate (cast);
	}

	Generator Visitor::validateArrayAlloc (const syntax::ArrayAlloc & alloc) {
	    if (alloc.isDynamic ()) {
		auto value = validateValueNonVoid (alloc.getLeft ());
		verifyMemoryOwner (alloc.getLocation (), value.to <Value> ().getType (), value, false);
		auto size = SizeOf::init (
		    alloc.getLocation (),
		    Integer::init (alloc.getLocation (), 0, false),
		    value.to <Value> ().getType ()
		);
		
		auto len = validateValueNonVoid (alloc.getSize ());
		auto type = Slice::init (alloc.getLocation (), value.to<Value> ().getType ());
		type = Type::init (type.to <Type> (), true);
		
		return ArrayAlloc::init (alloc.getLocation (), type.to<Type> ().toDeeplyMutable (), value, size, len);
	    } else {
		auto value = validateValueNonVoid (alloc.getLeft ());
		verifyMemoryOwner (alloc.getLocation (), value.to <Value> ().getType (), value, false);
		
		auto size = SizeOf::init (
		    alloc.getLocation (),
		    Integer::init (alloc.getLocation (), 0, false),
		    value.to <Value> ().getType ()
		);
		
		auto len = retreiveValue (validateValueNonVoid (alloc.getSize ()));
		if (!len.is <Fixed> () || (len.to<Fixed> ().getType ().to <Integer> ().isSigned () && len.to <Fixed> ().getUI ().i < 0)) {
		    Ymir::Error::occur (alloc.getSize ().getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					value.to <Value> ().getType ().to <Type> ().getTypeName (),
					(Integer::init (lexing::Word::eof (), 64, false)).to<Type> ().getTypeName ()
		    );
		}
		
		auto type = Array::init (alloc.getLocation (), value.to<Value> ().getType (), len.to <Fixed> ().getUI ().u);
		type = Type::init (type.to <Type> (), true);
		
		return ArrayAlloc::init (alloc.getLocation (), type.to <Type> ().toDeeplyMutable (), value, size, len.to <Fixed> ().getUI ().u);
	    }
	}

	Generator Visitor::validateDestructDecl (const syntax::DestructDecl & decl) {
	    auto value = validateValue (syntax::Intrinsics::init (lexing::Word::init (decl.getLocation (), Keys::EXPAND), decl.getValue ()));
	    match (value) {
		of (List, lst) {
		    if ((decl.isVariadic () && lst.getParameters ().size () < decl.getParameters ().size ())
			|| (!decl.isVariadic () && lst.getParameters ().size () != decl.getParameters ().size ())) {
			Ymir::Error::occur (decl.getLocation (),
					    ExternalError::get (MISMATCH_ARITY),
					    decl.getParameters ().size (),
					    lst.getParameters ().size ());			    
		    }

		    std::vector <Generator> values;
		    Generator type (Void::init (decl.getLocation ()));
		    for (int i = 0 ; i < (int) decl.getParameters ().size () ; i++) {
			if (i != (int) decl.getParameters ().size () - 1 || lst.getParameters ().size () == decl.getParameters ().size ()) {
			    auto varDecl = decl.getParameters ()[i].to <syntax::VarDecl> ();
			    auto auxDecl = syntax::VarDecl::init (varDecl.getName (), varDecl.getDecorators (), varDecl.getType (), TemplateSyntaxWrapper::init (lst.getLocation (), lst.getParameters ()[i]));
			    values.push_back (validateValue (auxDecl));
				
			} else {
			    std::vector <Generator> rest (lst.getParameters ().begin () + i, lst.getParameters ().end ());
			    std::vector <Generator> types;
			    for (auto & it : rest) {
				types.push_back (it.to<Value> ().getType ());
			    }
			    auto tupleType = Tuple::init (lst.getLocation (), types);
			    tupleType = Type::init (tupleType.to <Type> (), true);
				
			    auto varDecl = decl.getParameters ()[i].to <syntax::VarDecl> ();
			    auto auxDecl = syntax::VarDecl::init (varDecl.getName (), varDecl.getDecorators (), varDecl.getType (), 
								  TemplateSyntaxWrapper::init (
								      lst.getLocation (),
								      TupleValue::init (
									  lst.getLocation (),
									  tupleType,
									  rest
									  ))
				);
				
			    values.push_back (validateValue (auxDecl));				
			}			    
		    }
		    return Set::init (decl.getLocation (), type, values);
		}
		elfo {
		    if (decl.getParameters ().size () != 1) {
			Ymir::Error::occur (decl.getLocation (),
					    ExternalError::get (OVERFLOW_ARITY),
					    decl.getParameters ().size (),
					    1);
		    }
		    auto varDecl = decl.getParameters () [0].to<syntax::VarDecl> ();
		    auto auxDecl = syntax::VarDecl::init (varDecl.getName (), varDecl.getDecorators (), varDecl.getType (), 
							  TemplateSyntaxWrapper::init (value.getLocation (), value)
			);
		    return validateValue (auxDecl);
		}
	    }
	    
	    return Generator::empty ();
	}

	Generator Visitor::validateLambda (const syntax::Lambda & function) {
	    auto name = format ("_%", function.getUniqId ());
	    auto frameName = this-> _referent.back ().getRealName () + "::" + name;
	    auto lambdaStored = this-> _lambdas.find (frameName);
	    if (lambdaStored != this-> _lambdas.end ()) { // We want to avoid multiple time validation
		return lambdaStored-> second;
	    }
	    
	    std::vector <Generator> params;
	    std::vector <Generator> paramsProto;
	    std::vector <Generator> paramTypes;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    Generator body (Generator::empty ());
	    Generator retType (Generator::empty ());

	    bool uncomplete = false; // idem
	    auto syms = this-> _symbols.size () - 1; // index of the last symbol is the current enclosure of the frame
	    
	    enterForeign ();
	    enterBlock ();
	    {
		try {
		    for (auto & param : function.getPrototype ().getParameters ()) {
			auto var = param.to <syntax::VarDecl> ();
			Generator type (Generator::empty ());
			if (!var.getType ().isEmpty ()) {
			    type = validateType (var.getType ());
			} else {
			    uncomplete = true;
			}

			bool isMutable = false;
			bool isRef = false;
			bool dmut = false;
			if (!type.isEmpty ()) {
			    type = applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut);
			    
			    verifyMutabilityRefParam (var.getLocation (), type, MUTABLE_CONST_PARAM);
			    if (type.is <NoneType> () || type.is<Void> ()) {
				Ymir::Error::occur (var.getLocation (), ExternalError::get (VOID_VAR));
			    }
			}
		
			params.push_back (ParamVar::init (var.getName (), type, isMutable, false));
			paramsProto.push_back (ProtoVar::init (var.getName (), type, Generator::empty (), isMutable, 1, false));
			paramTypes.push_back (type);
			if (var.getName () != Keys::UNDER) {
			    verifyShadow (var.getName ());		
			    insertLocal (var.getName ().getStr (), params.back ());
			}		
		    }
	   
		    if (!function.getPrototype ().getType ().isEmpty ()) {
			retType = validateType (function.getPrototype ().getType (), true);		
		    }

		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    this-> discardAllLocals ();

	    {
		try {
		    quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    exitForeign ();
		
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    auto proto = LambdaProto::init (function.getLocation (), frameName, retType, paramsProto, function.getContent (), function.isRefClosure (), function.isMoveClosure (), syms);
	    proto = LambdaProto::init (proto.to<LambdaProto>(), format ("%%%", this-> _referent.back ().getMangledName (), name.length (), name), Frame::ManglingStyle::Y);	    

	    if (!uncomplete) {
		return validateLambdaProto (proto.to <LambdaProto> (), paramTypes);
	    } else {
		return proto;
	    }
	}

	Generator Visitor::validateLambdaProto (const LambdaProto & proto, const std::vector <Generator> & types) {
	    std::vector <Generator> params;
	    std::vector <Generator> paramsProto;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    Generator body (Generator::empty ());
	    Generator retType (proto.getReturnType ());

	    bool needFinalReturn = false;// mmmh, not understanding why, but gcc doesn't like it otherwise
	    enterForeign ();
	    enterContext ({});
	    enterBlock ();
	    {
		try {
		    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
			auto var = proto.getParameters ()[it].to <ProtoVar> ();
			if (it >= (long) types.size () || types [it].isEmpty ()) {
			    Ymir::Error::occur (var.getLocation (), ExternalError::get (UNKNOWN_LAMBDA_TYPE), var.prettyString ());
			} else {
			    bool isMutable = types [it].to <Type> ().isMutable ();
			    if (!var.getType ().isEmpty ()) {
				this-> verifyCompatibleType (var.getType ().getLocation (), types [it].getLocation (), var.getType (), types [it]);
			    }
			    params.push_back (ParamVar::init (var.getLocation (), types [it], isMutable, false));
			    paramsProto.push_back (ProtoVar::init (var.getLocation (), types [it], Generator::empty (), isMutable, 1, false));
			    if (var.getName () != Keys::UNDER) {
				insertLocal (var.getName (), params.back ());
			    }
			}
		    }
		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 		    
	    }
	    
	    uint refId = 0;
	    {
		try {		    
		    this-> setCurrentFuncType (retType);
		    refId = generator::VarDecl::__lastId__;
		    generator::VarDecl::__lastId__ += 1;
		    if (proto.isRefClosure () || proto.isMoveClosure ())
			this-> enterClosure (proto.isRefClosure (), refId, proto.getClosureIndex ());
			    
		    body = validateValue (proto.getContent ());		    
		    retType = this-> getCurrentFuncType ();
	    
		    if (!body.to<Value> ().isReturner ()) {
			if (!retType.isEmpty ()) {
			    verifyMemoryOwner (body.getLocation (), retType, body, true);		    
			    needFinalReturn = !retType.is<Void> ();
			} else {
			    needFinalReturn = !body.to <Value> ().getType ().is<Void> ();
			    retType = body.to <Value> ().getType ();
			}
		    }
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 		    
	    }

	    Generator closure (Generator::empty ());
	    if (proto.isRefClosure () || proto.isMoveClosure ()) {
		closure = this-> exitClosure ();
		if (closure.to <Closure> ().getNames ().size () != 0) {
		    params.insert (params.begin (), ParamVar::init (lexing::Word::init (lexing::Word::eof (), "#_closure"), closure, false, false));
		    params [0].setUniqId (refId);
		} else closure = Generator::empty ();
	    }

	    if (!body.isEmpty ()) {
		if (body.getThrowers ().size () != 0) {
		    std::list <Ymir::Error::ErrorMsg> notes;
		    for (auto &it : body.getThrowers ()) {
			notes.push_back (Ymir::Error::createNote (it.getLocation (), it.prettyString ()));		
		    }
		    errors.push_back (Error::makeOccurAndNote (proto.getLocation (), notes, ExternalError::get (THROWS_IN_LAMBDA)));
		}
	    }
	    
	    {
		try { // We want to guarantee that we exit the foreign at the end of this function 
		    quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    exitForeign ();
	    exitContext ();
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};

	    auto frame = Frame::init (proto.getLocation (), proto.getName (), params, retType, body, needFinalReturn);
	    frame.to <Frame> ().isWeak (true);
	    frame.to <Frame> ().setMangledName (proto.getMangledName ());

	    insertNewGenerator (frame);
	    
	    auto frameProto = FrameProto::init (proto.getLocation (), proto.getName (), retType, paramsProto, false, false, {});
	    frameProto = FrameProto::init (frameProto.to<FrameProto>(), proto.getMangledName (), Frame::ManglingStyle::Y);
		
	    auto funcType = Type::init (FuncPtr::init (proto.getLocation (), frameProto.to <FrameProto> ().getReturnType (), types).to <Type> (), true);
	    auto addr = Addresser::init (proto.getLocation (), funcType, frameProto);
	    insert_or_assign (this-> _lambdas, proto.getName (), addr);
	    
	    if (!closure.isEmpty ()) {
		auto closureValue = validateClosureValue (closure, proto.isRefClosure (), proto.getClosureIndex ());
		auto tupleType = Delegate::init (proto.getLocation (), funcType);		
		auto tuple = DelegateValue::init (proto.getLocation (), tupleType, closureValue.to <Value> ().getType (), closureValue, addr);
						       
		return tuple;
	    } else {		
		return addr;
	    }
	}

	Generator Visitor::validateMultSymProto (const Generator & sym, const std::vector <Generator> &types) {
	    std::vector <Generator> valueParams;
	    for (auto it : Ymir::r (0, types.size ())) {
		valueParams.push_back (FakeValue::init (types [it].getLocation (), types [it]));
	    }
	    
	    std::list <Ymir::Error::ErrorMsg> errors;
	    int score;
	    auto call = CallVisitor::init (*this);	    
	    auto ret = call.validate (sym.getLocation (), sym, valueParams, score, errors);
	    if (ret.isEmpty ()) 
		call.error (lexing::Word::init (sym.getLocation (), ""), lexing::Word::init (sym.getLocation (), ""), sym, valueParams, errors);
	    
	    return ret.to <Call> ().getFrame ();
	}

	Generator Visitor::validateClosureValue (const Generator & closureType, bool isRefClosure, uint closureIndex) {
	    std::vector <Generator> innerTypes;
	    std::vector <Generator> innerValues;
	    auto loc = closureType.getLocation ();
	    for (auto & name : closureType.to <Closure> ().getNames ()) {
		auto & syms = this-> _symbols [closureIndex];
		for (auto _it : Ymir::r (0, syms.size ())) {
		    auto ptr = syms [_it].find (name);
		    if (ptr != syms [_it].end ()) {
			if (ptr-> second.is <ParamVar> ()) {
			    auto type = ptr-> second.to <Value> ().getType ();
			    auto varRef = VarRef::init (loc, name, type, ptr-> second.getUniqId (), false, Generator::empty (), ptr-> second.to <ParamVar> ().isSelf ());
			    if (isRefClosure) {
				innerValues.push_back (Referencer::init (loc, Type::init (type.to <Type> (), type.to <Type> ().isMutable (), true), varRef));
			    } else
				innerValues.push_back (varRef);

			    innerTypes.push_back (type);
			} else if (ptr-> second.is <generator::VarDecl> ()) {
			    auto type = ptr-> second.to <generator::VarDecl> ().getVarType ();
			    auto varRef = VarRef::init (loc, name, type, ptr-> second.getUniqId (), false, Generator::empty ());
			    if (isRefClosure) {
				innerValues.push_back (Referencer::init (loc, Type::init (type.to <Type> (), type.to <Type> ().isMutable (), true), varRef));
			    } else {
				Generator value (Generator::empty ());
				if (!ptr-> second.to <generator::VarDecl> ().isMutable ())
				    value = ptr-> second.to <generator::VarDecl> ().getVarValue ();
				varRef = VarRef::init (loc, name, type, ptr-> second.getUniqId (), false, value);
				innerValues.push_back (varRef);
			    }
			    
			    innerTypes.push_back (type);
			} else if (ptr-> second.is <StructAccess> ()) {
			    innerValues.push_back (ptr-> second);
			    
			    auto type = ptr-> second.to <Value> ().getType ();
			    innerTypes.push_back (type);
			}
			break; // We found it, go to the the next enclosure
		    }
		}
	    }
	    
	    auto tupleType = TupleClosure::init (loc, innerTypes);
	    auto tupleValue = TupleValue::init (loc, tupleType, innerValues);
	    return Copier::init (loc, Pointer::init (loc, Void::init (loc)), tupleValue);
	}	
	
	Generator Visitor::validateFuncPtr (const syntax::FuncPtr & ptr) {
	    std::vector <Generator> params;
	    if (ptr.getLocation () == Keys::FUNCTION) {
		for (auto & it : ptr.getParameters ()) {
		    params.push_back (validateType (it, true));
		}
		
		return FuncPtr::init (ptr.getLocation (), validateType (ptr.getRetType (), true), params);
	    } else if (ptr.getLocation () == Keys::DELEGATE) {
		for (auto & it : ptr.getParameters ()) {
		    params.push_back (validateType (it, true));
		}
		
		return Delegate::init (ptr.getLocation (), FuncPtr::init (ptr.getLocation (), validateType (ptr.getRetType (), true), params));
	    } else {
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		return Generator::empty ();
	    }
	}

	Generator Visitor::validateNullValue (const syntax::Null & nl) {
	    auto type = Pointer::init (nl.getLocation (), Void::init (nl.getLocation ()));
	    return NullValue::init (nl.getLocation (), type);
	}
	
	Generator Visitor::validateIntrinsics (const syntax::Intrinsics & intr) {
	    if (intr.isCopy ()) return validateCopy (intr);
	    if (intr.isAlias ()) return validateAlias (intr);
	    if (intr.isExpand ()) return validateExpand (intr);
	    if (intr.isDeepCopy ()) return validateDeepCopy (intr);
	    if (intr.isTypeof ()) {
		auto elem = validateValue (intr.getContent ());
		return elem.to <Value> ().getType ();
	    }
	    if (intr.isSizeof ()) {
		auto elem = validateType (intr.getContent (), true);
		return SizeOf::init (intr.getLocation (), Integer::init (intr.getLocation (), 0, false), elem);
	    }
	    
	    if (intr.isMove ()) {
		Ymir::Error::occur (intr.getLocation (),
				    ExternalError::get (MOVE_ONLY_CLOSURE));				    
	    }
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}
       	
	Generator Visitor::validateCopy (const syntax::Intrinsics & intr) {
	    auto content = validateValue (intr.getContent ());

	    if (content.to <Value> ().getType ().is <Array> () || content.to <Value> ().getType ().is <Slice> ()) {
		auto type = content.to<Value> ().getType ();
		type = Type::init (type.to <Type> (), false);
		type = type.to <Type> ().toMutable ();
		
		if (type.is <Array> ()) {
		    if (type.is <Array> ()) {
			type = Type::init (Slice::init (intr.getLocation (), type.to<Array> ().getInners () [0]).to <Type> (), true);
		    }
		    
		    content = Aliaser::init (intr.getLocation (), type, content);
		}
		
		// The copy is done on the first level, so we don't have the right to change the mutability of inner data
		return Copier::init (intr.getLocation (), type, content);
	    } else {
		Ymir::Error::occur (
		    intr.getLocation (),
		    ExternalError::get (NO_COPY_EXIST),
		    content.to<Value> ().getType ().to <Type> ().getTypeName ()
		);
		
		return Generator::empty ();		
	    }	    
	}

	Generator Visitor::validateAlias (const syntax::Intrinsics & intr) {
	    auto content = validateValue (intr.getContent ());

	    if (content.to <Value> ().getType ().is <Array> () || content.to <Value> ().getType ().is <Slice> ()) {
		auto type = content.to <Value> ().getType ();
		if (type.is <Array> ()) {
		    type = Type::init (Slice::init (intr.getLocation (), type.to<Array> ().getInners () [0]).to <Type> (), 
				       content.to <Value> ().getType ().to <Type> ().isMutable ());
		}
		return Aliaser::init (intr.getLocation (), type, content);
	    }
	    
	    auto type = content.to <Value> ().getType ();
	    if (!type.to <Type> ().isMutable ())
		Ymir::Error::occur (content.getLocation (),
				    ExternalError::get (NOT_A_LVALUE)
		);

	    this-> verifyLockAlias (content);
	    
	    return Aliaser::init (intr.getLocation (), type, content);	    	    
	}

	Generator Visitor::validateDeepCopy (const syntax::Intrinsics & intr) {
	    auto inner = validateValue (intr.getContent ());
	    syntax::Expression call (syntax::Expression::empty ());
	    auto loc = intr.getLocation ();
	    if (inner.to <Value> ().getType ().is <ClassPtr> ()) {
		auto trait = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (DUPLICATION_MODULE), CoreNames::get (DCOPY_TRAITS)});		
		auto impl = validateType (trait);
		
		verifyClassImpl (intr.getLocation (), inner.to <Value> ().getType (), impl);		
		call = syntax::MultOperator::init (
		    lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		    syntax::Binary::init (lexing::Word::init (loc, Token::DOT),
					  TemplateSyntaxWrapper::init (inner.getLocation (), inner), 	       
					  syntax::Var::init (lexing::Word::init (loc, global::CoreNames::get (DCOPY_OP_OVERRIDE))),
					  syntax::Expression::empty ()
			),
		    {}, false
		);
	    } else {
		auto func = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (DUPLICATION_MODULE), CoreNames::get (DCOPY_OP_OVERRIDE)});
		
		call = syntax::MultOperator::init (
		    lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		    func,
		    {TemplateSyntaxWrapper::init (inner.getLocation (), inner)}	       
		    );
	    }

	    auto val = validateValue (call);
	    return Aliaser::init (intr.getLocation (), val.to <Value> ().getType (), val);
	}
	
	Generator Visitor::validateExpand (const syntax::Intrinsics & intr) {
	    std::vector <Generator> values;
	    auto content = validateValue (intr.getContent ());
	    auto type = content.to <Value> ().getType ();
	    auto rref = UniqValue::init (intr.getLocation (), type, content);
	    
	    if (content.to<Value> ().getType ().is<Tuple> ()) {
		auto type = Void::init (intr.getLocation ());
		std::vector <Generator> expanded;
		auto & tu_inners = content.to <Value> ().getType ().to<Tuple> ().getInners ();
		for (auto it : Ymir::r (0, tu_inners.size ())) {
		    auto type = tu_inners [it];
		    if (content.to<Value> ().isLvalue () &&
			content.to <Value> ().getType ().to <Type> ().isMutable () &&
			type.to <Type> ().isMutable ())
			type = Type::init (type.to <Type> (), true);
		    else type = Type::init (type.to<Type> (), false);
		    
		    expanded.push_back (TupleAccess::init (intr.getLocation (), type, rref, it));
		}
		return List::init (intr.getLocation (), type, expanded);
	    } else {
		return content;
	    }
	}

	Generator Visitor::validateMultOperator (const syntax::MultOperator & op) {
	    if (op.getEnd () == Token::RCRO) {
		auto bracketVisitor = BracketVisitor::init (*this);
		return bracketVisitor.validate (op);
	    } else if (op.getEnd () == Token::RPAR) {
		auto callVisitor = CallVisitor::init (*this);
		return callVisitor.validate (op);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	Generator Visitor::validateType (const syntax::Expression & expr, bool lock) {
	    auto type = validateType (expr);
	    if (lock && !type.to<Type> ().isMutable ())
		return Type::init (type.to <Type> (), false);
	    return type;		
	}

	Generator Visitor::validateTypeClassRef (const syntax::Expression & expr, bool lock) {
	    auto type = validateTypeClassRef (expr);
	    if (lock && !type.to<Type> ().isMutable ())
		return Type::init (type.to <Type> (), false);
	    return type;		
	}

	Generator Visitor::validateTypeClassRef (const syntax::Expression & type) {
	    Generator val (Generator::empty ());
	    match (type) {
		of (syntax::Var, var) 
		    val = validateTypeVar (var);
		
		elof (syntax::DecoratedExpression, dec_expr)
		    val = validateTypeDecorated (dec_expr, true);
				
		elof (TemplateSyntaxWrapper, tmplSynt)
		    val =  tmplSynt.getContent ();
		
		fo;		
	    }

	    if (val.isEmpty ()) {		
		val = validateValue (type, true, true); // Can't make a implicit call validation if we are looking for a type
	    }
		
	    if (val.is<ClassRef> ()) return val;	   
	    if (val.is <generator::Class> ()) {
		return val.to <generator::Class> ().getClassRef ();
	    }
	    
	    Ymir::Error::occur (type.getLocation (), ExternalError::get (USE_AS_TYPE));
	    return Generator::empty ();	  
	}
	
	Generator Visitor::validateType (const syntax::Expression & type) {
	    Generator val (Generator::empty ());
	    match (type) {
		of (syntax::ArrayAlloc, array)
		    val = validateTypeArrayAlloc (array);
		
		elof (syntax::Var, var)
		    val = validateTypeVar (var);		

		elof (syntax::DecoratedExpression, dec_expr)
		    val = validateTypeDecorated (dec_expr);		

		elof (syntax::Unary, un)
		    val = validateTypeUnary (un);		

		elof (syntax::Try, tr)
		    val = validateTypeTry (tr);		
		
		elof (syntax::List, list) {
		    if (list.isArray ())
			val = validateTypeSlice (list);
		    if (list.isTuple ())
			val = validateTypeTuple (list);
		}

		elof (TemplateSyntaxList, tmplSynt)
		    val = validateTypeTupleTemplate (tmplSynt);		
		
		elof (TemplateSyntaxWrapper, tmplSynt)
		    val =  tmplSynt.getContent ();
		
		elof (syntax::TemplateCall, tmpCall) 
		    val = validateTypeTemplateCall (tmpCall);
		
		fo;		
	    }

	    if (val.isEmpty ()) {		
		val = validateValue (type, true, true, false, true); // Can't make a implicit call validation if we are looking for a type
	    }

	    if (val.is <ClassRef> () || val.is <generator::Class> ()) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (FORGET_TOKEN), Token::AND);
		Ymir::Error::occurAndNote (type.getLocation (), note, ExternalError::get (USE_AS_TYPE));
	    }
		
	    if (val.is<Type> ()) return val;
	    if (val.is<generator::Struct> ())
		return StructRef::init (type.getLocation (), val.to <generator::Struct> ().getRef ());
	    
	    // if (val.is <StructCst> ()) return val.to <StructCst> ().getStr ();, Why?

	    Ymir::Error::occur (type.getLocation (), ExternalError::get (USE_AS_TYPE));
	    return Generator::empty ();	    	   
	}

	Generator Visitor::validateTypeVar (const syntax::Var & var) {
	    if (std::find (Integer::NAMES.begin (), Integer::NAMES.end (), var.getName ().getStr ()) != Integer::NAMES.end ()) {
		auto size = var.getName ().getStr ().substr (1);
		
		// According to c++ documentation atoi return 0, if the conversion failed
		return Integer::init (var.getName (), std::atoi (size.c_str ()), var.getName ().getStr ()[0] == 'i');
	    } else if (var.getName ().getStr () == Void::NAME) {
		return Void::init (var.getName ());
	    } else if (var.getName ().getStr () == Bool::NAME) {
		return Bool::init (var.getName ());
	    } else if (std::find (Float::NAMES.begin (), Float::NAMES.end (), var.getName ().getStr ()) != Float::NAMES.end ()) {
		auto size = var.getName ().getStr ().substr (1);
		return Float::init (var.getName (), std::atoi (size.c_str ())); 
	    } else if (std::find (Char::NAMES.begin (), Char::NAMES.end (), var.getName ().getStr ()) != Char::NAMES.end ()) {
		return Char::init (var.getName (), std::atoi (var.getName ().getStr ().substr (1).c_str ()));
	    } else {		
		auto syms = getGlobal (var.getName ().getStr ());
		// println( var.getName ().getStr (), " ", this-> _referent.back ().formatTree ());
		// for (auto & it : syms)
		//     println (it.getRealName ());
		if (!syms.empty ()) {
		    auto ret = validateMultSymType (var.getLocation (), syms);		    
		    if (!ret.isEmpty ()) return ret;		    
		} else {
		    syms = getGlobalPrivate (var.getName ().getStr ());
		    std::list <Ymir::Error::ErrorMsg> notes;
		    for (auto it : Ymir::r (0, syms.size ())) {
			notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), syms [it].getName (), syms [it].getRealName ()));
		    }
		    if (var.getName ().getStr () == "U")
			Ymir::Error::halt ("", "");
		    
		    Error::occurAndNote (var.getName (), notes, ExternalError::get (UNDEF_TYPE), var.getName ().getStr ());
		}
	    }

	    if (var.getName ().getStr () == "U")
		Ymir::Error::halt ("", "");
	    Error::occur (var.getName (), ExternalError::get (UNDEF_TYPE), var.getName ().getStr ());
	    return Generator::empty ();
	}

	Generator Visitor::validateTypeUnary (const syntax::Unary & un) {
	    auto op = un.getOperator ();
	    if (op == Token::AND) { // Pointer
		try {
		    auto inner = validateType (un.getContent (), true);		
		    if (!inner.isEmpty ()) return Pointer::init (un.getLocation (), inner);
		} catch (Error::ErrorList list) {
		    try {
			auto inner = validateTypeClassRef (un.getContent (), true);
			if (inner.is <ClassRef> ()) {
			    auto ret =  ClassPtr::init (un.getLocation (), inner);
			    return ret;
			}
		    } catch (Error::ErrorList ignore) {
			throw list;
		    }
		}
	    }

	    return Generator::empty ();
	}

	Generator Visitor::validateTypeTry (const syntax::Try & tr) {
	    auto inner = validateType (tr.getContent (), true);
	    auto syntaxType = createClassTypeFromPath (tr.getLocation (), {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (EXCEPTION_TYPE)});
	    auto errType = Type::init (validateType (syntaxType).to <Type> (), false, false);
	    
	    if (!inner.isEmpty ()) return Option::init (tr.getLocation (), inner, errType);
	    return Generator::empty ();
	}

	Generator Visitor::validateTypeDecorated (const syntax::DecoratedExpression & expr, bool canBeClassRef) {
	    Generator type (Generator::empty ());
	    if (canBeClassRef) 
		type = validateTypeClassRef (expr.getContent ());
	    else
		type = validateType (expr.getContent ());
	    
	    lexing::Word gotConstOrMut (lexing::Word::eof ());
	    for (auto & deco : expr.getDecorators ()) {
		switch (deco.getValue ()) {
		case syntax::Decorator::REF : type = Type::init (type.to<Type> (), type.to <Type> ().isMutable (), true); break;
		case syntax::Decorator::CONST : {
		    if (!gotConstOrMut.isEof ()) {
			auto note = Ymir::Error::createNote (gotConstOrMut);
			Ymir::Error::occurAndNote (expr.getDecorator (syntax::Decorator::CONST).getLocation (), note, ExternalError::get (CONFLICT_DECORATOR));
		    }
		    gotConstOrMut = expr.getDecorator (syntax::Decorator::CONST).getLocation ();
		    type = Type::init (type.to<Type> (), false); break;
		}
		case syntax::Decorator::MUT : {
		    if (!gotConstOrMut.isEof ()) {
			auto note = Ymir::Error::createNote (gotConstOrMut);
			Ymir::Error::occur (expr.getDecorator (syntax::Decorator::MUT).getLocation (), ExternalError::get (CONFLICT_DECORATOR));
		    }
		    gotConstOrMut = expr.getDecorator (syntax::Decorator::MUT).getLocation ();
		    type = Type::init (type.to<Type> (), true); break;
		}
		case syntax::Decorator::DMUT : {
		    if (!gotConstOrMut.isEof ()) {
			auto note = Ymir::Error::createNote (gotConstOrMut);
			Ymir::Error::occur (expr.getDecorator (syntax::Decorator::DMUT).getLocation (), ExternalError::get (CONFLICT_DECORATOR));		    
		    }
		    gotConstOrMut = expr.getDecorator (syntax::Decorator::DMUT).getLocation ();
		    type = type.to<Type> ().toDeeplyMutable (); break;
		}
		default :
		    Ymir::Error::occur (deco.getLocation (),
					ExternalError::get (DECO_OUT_OF_CONTEXT),
					deco.getLocation ().getStr ()
		    );		
		}
	    }
	    	   	    
	    return type;
	}

	Generator Visitor::validateTypeArrayAlloc (const syntax::ArrayAlloc & alloc) {
	    if (alloc.isDynamic ())
		Ymir::Error::occur (alloc.getLocation (), ExternalError::get (USE_AS_TYPE));

	    auto type = validateType (alloc.getLeft (), true);
	    auto size = validateValue (alloc.getSize ());

	    Generator value = retreiveValue (size);
	    if (!value.is <Fixed> () || (value.to<Fixed> ().getType ().to <Integer> ().isSigned () && value.to <Fixed> ().getUI ().i < 0)) {
		Ymir::Error::occur (alloc.getSize ().getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    value.to <Value> ().getType ().to <Type> ().getTypeName (),
				    (Integer::init (lexing::Word::eof (), 64, false)).to<Type> ().getTypeName ()
		);
	    }
	    
	    return Array::init (alloc.getLocation (), type, value.to <Fixed> ().getUI ().u);	    
	}

	Generator Visitor::validateTypeSlice (const syntax::List & list) {
	    if (list.getParameters ().size () != 1)
		Ymir::Error::occur (list.getLocation (), ExternalError::get (USE_AS_TYPE));

	    auto type = validateType (list.getParameters () [0], true);
	    return Slice::init (list.getLocation (), type);	    
	}
	
	Generator Visitor::validateTypeTuple (const syntax::List & list) {
	    std::vector <Generator> params;
	    for (auto & it : list.getParameters ()) {
		params.push_back (validateType (it, true));
	    }
	    
	    return Tuple::init (list.getLocation (), params);
	}	

	Generator Visitor::validateTypeTupleTemplate (const TemplateSyntaxList & lst) {
	    std::vector<Generator> params;
	    for (auto & it : lst.getContents ()) {
		params.push_back (validateType (TemplateSyntaxWrapper::init (it.getLocation (), it), true));
	    }

	    return Tuple::init (lst.getLocation (), params);
	}
	
	Generator Visitor::validateTypeTemplateCall (const syntax::TemplateCall & call) {	    
	    auto left = call.getContent ();
	    match (left) {
		of (syntax::Var, var) {
		    Generator innerType (Generator::empty ());
		    if (call.getParameters ().size () == 1) {
			innerType = validateType (call.getParameters ()[0], true);
		    }
		    
		    if (var.getName ().getStr () == Range::NAME) {
			return Range::init (var.getName (), innerType);
		    }		       
		} fo;		
	    }
	    
	    return Generator::empty ();
	}
	
	const std::vector <generator::Generator> & Visitor::getGenerators () const {
	    return this-> _list;
	}
	
	void Visitor::insertNewGenerator (const Generator & generator) {
	    this-> _list.push_back (generator);
	}
	
	void Visitor::enterBlock () {
	    this-> _usedSyms.back ().push_back ({});
	    this-> _symbols.back ().push_back ({});
	}
	
	void Visitor::quitBlock () {	    
	    if (this-> _symbols.back ().empty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");

	    std::list <Ymir::Error::ErrorMsg> errors;
	    for (auto & sym : this-> _symbols.back ().back ()) {
		if (sym.first != Keys::SELF) { // SELF is like "_", we don't need it to be used
		    if (this-> _usedSyms.back ().back ().find (sym.first) == this-> _usedSyms.back ().back ().end ()) {
			errors.push_back (Error::makeWarn (sym.second.getLocation (), ExternalError::get (NEVER_USED), sym.second.getName ()));
		    }
		}
	    }

	    this-> _usedSyms.back ().pop_back ();
	    this-> _symbols.back ().pop_back ();

	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	}

	void Visitor::lockAliasing (const lexing::Word & loc, const generator::Generator & gen_) {
	    if (gen_.to <Value> ().getType ().to <Type> ().isMutable ()) { // not necessary to store immutable element
		auto gen = gen_;
		while (gen.is <Aliaser> () || gen.is<Referencer> ()) {
		    if (gen.is <Aliaser> ()) gen = gen.to <Aliaser> ().getWho ();
		    else if (gen.is <Referencer> ()) gen = gen.to <Referencer> ().getWho ();
		}
		
		this-> _lockedAlias.push_back (gen);
		this-> _lockedAliasLoc.push_back (loc);
	    }
	}

	void Visitor::unlockAliasing (const generator::Generator & gen_) {
	    if (this-> _lockedAlias.size () != 0 && gen_.to <Value> ().getType ().to <Type> ().isMutable ()) {
		std::vector <Generator> keeps;
		std::vector <lexing::Word> keepLoc;
		auto gen = gen_;
		while (gen.is <Aliaser> () || gen.is<Referencer> ()) {
		    if (gen.is <Aliaser> ()) gen = gen.to <Aliaser> ().getWho ();
		    else if (gen.is <Referencer> ()) gen = gen.to <Referencer> ().getWho ();
		}

		int i = 0;
		for (auto & it : this-> _lockedAlias) {
		    if (!it.equals (gen)) {
			keeps.push_back (it);
			keepLoc.push_back (this-> _lockedAliasLoc [i]);
		    }
		    i += 1;
		}
		
		this-> _lockedAlias = keeps;
		this-> _lockedAliasLoc = keepLoc;
	    }	    
	}

	void Visitor::verifyLockAlias (const generator::Generator & gen_) {
	    if (this-> _lockedAlias.size () != 0 && gen_.to <Value> ().getType ().to <Type> ().isMutable ()) {
		auto gen = gen_;
		while (gen.is <Aliaser> () || gen.is<Referencer> ()) {
		    if (gen.is <Aliaser> ()) gen = gen.to <Aliaser> ().getWho ();
		    else if (gen.is <Referencer> ()) gen = gen.to <Referencer> ().getWho ();
		}

		int i = 0;
		for (auto & it : this-> _lockedAlias) {		    
		    if (it.equals (gen)) {
			auto note = Ymir::Error::createNote (this-> _lockedAliasLoc [i]);
			Ymir::Error::occurAndNote (gen_.getLocation (), note, ExternalError::get (LOCKED_CONTEXT), gen.to <Value> ().getType ().prettyString ());
		    }
		    i += 1;
		}		
	    }	    
	}
	
	std::map <std::string, generator::Generator> Visitor::discardAllLocals () {
	    auto ret = this-> _symbols.back ().back ();
	    this-> _symbols.back ().back () = {};
	    return ret;
	}
	
	void Visitor::enterForeign () {
	    this-> _usedSyms.push_back ({});
	    this-> _symbols.push_back ({});
	    this-> _loopSaved.push_back (this-> _loopBreakTypes);
	    this-> _loopBreakTypes = {};
	}
	
	void Visitor::exitForeign () {
	    this-> _usedSyms.pop_back ();
	    this-> _symbols.pop_back ();
	    this-> _loopBreakTypes = this-> _loopSaved.back ();
	    this-> _loopSaved.pop_back ();
	}

	void Visitor::enterClosure (bool isRef, uint refId, uint index) {
	    insert_or_assign (this-> _symbols.back ()[0], "#{CLOSURE}", BoolValue::init (lexing::Word::eof (), Bool::init (lexing::Word::eof ()), isRef));
	    this-> _usedSyms.back ()[0].insert ("#{CLOSURE}");

	    insert_or_assign (this-> _symbols.back ()[0], "#{CLOSURE-TYPE}", Closure::init (lexing::Word::eof (), {}, {}, index));
	    this-> _usedSyms.back ()[0].insert ("#{CLOSURE-TYPE}");

	    insert_or_assign (this-> _symbols.back ()[0], "#{CLOSURE-VARREF}", VarRef::init (lexing::Word::eof (), "#{CLOSURE-VARREF}", Void::init (lexing::Word::eof ()), refId, false, Generator::empty ()));
	    this-> _usedSyms.back () [0].insert ("#{CLOSURE-VARREF}");
	    this-> _enclosed.push_back ({});	    
	}

	Generator Visitor::getInClosure (const std::string & name) {
	    if (!isInClosure ()) return Generator::__empty__;
	    auto closureType = this-> getLocal ("#{CLOSURE-TYPE}", false);
	    //bool isRefClosure = this-> getLocal ("#{CLOSURE}").to <BoolValue> ().getValue ();
	    auto field = closureType.to <Closure> ().getField (name);
	    if (field.isEmpty ()) { // need to get it from upper closure
		auto & syms = this-> _symbols [closureType.to <Closure> ().getIndex ()];
		auto & used = this-> _usedSyms [closureType.to <Closure> ().getIndex ()]; 
		for (auto _it : Ymir::r (0, syms.size ())) {
		    auto ptr = syms [_it].find (name);
		    if (ptr != syms [_it].end ()) {
			used [_it].insert (name);
			Generator type (Generator::empty ());
			if (ptr-> second.is <generator::VarDecl> ()) {
			    type = ptr-> second.to <generator::VarDecl> ().getVarType ();
			} else type = ptr-> second.to <Value> ().getType ();
			
			auto types = closureType.to <Type> ().getInners ();
			
			auto names = closureType.to <Closure> ().getNames ();
			types.push_back (type);
			names.push_back (name);

			closureType = Closure::init (lexing::Word::eof (), types, names, closureType.to <Closure> ().getIndex ());
			closureType = Type::init (closureType.to <Type> (), closureType.to <Type> ().isMutable (), true);
			
			insert_or_assign (this-> _symbols.back ()[0], "#{CLOSURE-TYPE}", closureType);
			auto closureRef = this-> getLocal ("#{CLOSURE-VARREF}", false);
			insert_or_assign (this-> _symbols.back ()[0], "#{CLOSURE-VARREF}", VarRef::init (lexing::Word::eof (), "#{CLOSURE-VARREF}", closureType, closureRef.to <VarRef> ().getRefId (), false, Generator::empty ()));
			
			return StructAccess::init (lexing::Word::eof (), type, this-> getLocal ("#{CLOSURE-VARREF}", false), name);
		    }
		}
	    } else {
		auto closureRef = this-> getLocal ("#{CLOSURE-VARREF}", false);
		return StructAccess::init (lexing::Word::eof (), field, closureRef, name);
	    }

	    return Generator::__empty__;
	}	
	
	bool Visitor::isInClosure () {
	    auto ret = this-> getLocal ("#{CLOSURE}", false);
	    return !ret.isEmpty ();
	}
	
	bool Visitor::isInRefClosure () {
	    auto ret = this-> getLocal ("#{CLOSURE}", false);
	    return !ret.isEmpty () && ret.to<BoolValue> ().getValue ();
	}
	
	Generator Visitor::exitClosure () {
	    return this-> getLocal ("#{CLOSURE-TYPE}", false);
	}
	
	void Visitor::enterContext (const std::vector <lexing::Word> & Cas) {
	    this-> _contextCas.push_back (Cas);
	}

	void Visitor::exitContext () {
	    if (this-> _contextCas.empty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");

	    this-> _contextCas.pop_back ();
	}

	void Visitor::enterClassDef (const semantic::Symbol & sym) {
	    this-> _classContext.push_back (sym);
	}

	void Visitor::exitClassDef (const semantic::Symbol & sym) {
	    if (this-> _classContext.back ().getPtr () != sym.getPtr ()) {
		Ymir::Error::halt ("", "");
	    }
	    
	    this-> _classContext.pop_back ();
	}
              
	void Visitor::getClassContext (const semantic::Symbol & cl, bool & isPrivate, bool & isProtected) {
	    isPrivate = false;
	    isProtected = false;
	    if (this-> _classContext.size () != 0) {
		isPrivate = this-> _classContext.back ().equals (cl);
		if (isPrivate) isProtected = true;
		else {
		    Symbol clSym (Symbol::empty ());
		    if (this-> _classContext.back ().is <semantic::Class> ()) { // Maybe we are inside a macro 
			auto ancestor = this-> _classContext.back ().to <semantic::Class> ().getGenerator ().to <generator::Class> ().getClassRef ().to <ClassRef> ().getAncestor ();
			if (!ancestor.isEmpty ())
			    clSym = ancestor.to <ClassRef> ().getRef ();
		    
			while (!clSym.isEmpty ()) {
			    if (clSym.equals (cl)) {
				isProtected = true;
				break;
			    } else {
				auto ancestor = clSym.to <semantic::Class> ().getGenerator ().to <generator::Class> ().getClassRef ().to <ClassRef> ().getAncestor ();
				if (!ancestor.isEmpty ())
				    clSym = ancestor.to <ClassRef> ().getRef ();
				else break;
			    }
			}
		    }
		}
	    }
	}

	void Visitor::getMacroContext (const semantic::Symbol & cl, bool & isPrivate) {
	    isPrivate = false;
	    if (this-> _classContext.size () != 0) {
		isPrivate = this-> _classContext.back ().equals (cl);
	    }	    
	}

	bool Visitor::getModuleContext (const semantic::Symbol & cl) {
	    auto module = cl;
	    if (module.is <ModRef> ()) module = module.to <ModRef> ().getModule ();
	    
	    if (this-> _referent.size () != 0) {
		auto curr = this-> _referent.back ();
		while (!curr.isEmpty () && !module.isEmpty ()) {
		    if (curr.equals (module)) return true;		    		    
		    curr = curr.getReferent ();
		}
	    }
	    return false;
	}
	
	bool Visitor::isInContext (const std::string & context) {
	    if (!this-> _contextCas.empty ()) {
		for (auto & it : this-> _contextCas.back ())
		    if (it.getStr () == context) return true;
	    }
	    return false;
	}

	bool Visitor::isInTrusted () const {
	    return this-> _referent.back ().isTrusted ();
	}
	
	void Visitor::enterLoop () {
	    this-> _loopBreakTypes.push_back (Generator::empty ());	    
	}

	Generator Visitor::quitLoop () {
	    auto last = this-> _loopBreakTypes.back ();
	    this-> _loopBreakTypes.pop_back ();
	    return last;
	}

	void Visitor::enterDollar (const Generator & gen) {
	    this-> _dollars.push_back (gen);
	}

	void Visitor::quitDollar () {
	    this-> _dollars.pop_back ();
	}
	
	const Generator & Visitor::getCurrentLoopType () const {
	    return this-> _loopBreakTypes.back ();
	}

	void Visitor::setCurrentLoopType (const Generator & type) {
	    this-> _loopBreakTypes.back () = type;
	}

	Generator Visitor::getCurrentFuncType () {
	    return getLocal ("#{RET}", false);
	}

	void Visitor::setCurrentFuncType (const Generator & type) {
	    insert_or_assign (this-> _symbols.back () [0], "#{RET}", type);
	    this-> _usedSyms.back () [0].insert ("#{RET}");
	}
	
	bool Visitor::isInLoop () const {
	    return !this-> _loopBreakTypes.empty ();
	}
	
	void Visitor::insertLocal (const std::string & name, const Generator & gen) {
	    if (this-> _symbols.back ().empty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    if (this-> _symbols.back ().back ().find (name) == this-> _symbols.back ().back ().end ()) {
		insert_or_assign (this-> _symbols.back ().back (), name, gen);
	    } else {
		insert_or_assign (this-> _symbols.back ().back (), name, gen);
	    }
	}       

	
	Generator Visitor::getLocal (const std::string & name, bool canBeInClosure) {
	    for (auto _it : Ymir::r (0, this-> _symbols.back ().size ())) {
		auto it = (this-> _symbols.back ().size () - _it) - 1;
		auto ptr = this-> _symbols.back () [it].find (name); 		    
		if (ptr != this-> _symbols.back () [it].end ()) {
		    this-> _usedSyms.back () [it].insert (name);
		    return ptr-> second;
		}		
	    }

	    if (canBeInClosure)
		return this-> getInClosure (name);
	    
	    return Generator::__empty__;
	}

	void Visitor::printLocal () const {
	    for (auto _it : Ymir::r (0, this-> _symbols.back ().size ())) {
		println (Ymir::format ("%* {", (int) _it, '\t'));
		for (auto it : this-> _symbols.back () [_it]) {		    
		    println (Ymir::format ("%* %-> %", (int) (_it + 1), '\t', it.first, it.second.prettyString ()));
		}
		println (Ymir::format ("%* }", (int)_it, '\t'));
	    }
	}	

	void Visitor::verifyMemoryOwner (const lexing::Word & loc, const Generator & type, const Generator & gen, bool construct, bool checkTypes, bool inMatch) {
	    if (checkTypes) 
		verifyCompatibleTypeWithValue (loc, type, gen);

	    verifyCompleteType (loc, type);
	    verifyImplicitAlias (loc, type, gen);

	    // if (gen.is<Aliaser> () && llevel <= 1 && !inMatch && !gen.to <Aliaser> ().getWho ().is<StringValue>()) // special case for string literal
	    // 	Ymir::Error::warn (gen.getLocation (), ExternalError::get (ALIAS_NO_EFFECT));
	    
	    // Verify Implicit referencing
	    if ((!construct || !type.to <Type> ().isRef ()) && gen.is<Referencer> ()) {
		if (!inMatch)
		    Ymir::Error::warn (gen.getLocation (), ExternalError::get (REF_NO_EFFECT));
	    } else {
		if (type.to <Type> ().isRef () && construct) {
		    verifySameType (type, gen.to <Value> ().getType ());
		    
		    if (!gen.is<Referencer> ()) {
			if (gen.to<Value> ().isLvalue () || gen.is <Aliaser> ()) {
			    Ymir::Error::occur (gen.getLocation (), ExternalError::get (IMPLICIT_REFERENCE),
						gen.to<Value> ().getType ().to <Type> ().getTypeName ()
			    );
			} else {
			    auto note = Ymir::Error::createNote (loc);
			    Ymir::Error::occurAndNote (gen.getLocation (), note, ExternalError::get (NOT_A_LVALUE));
			}
		    }
		}
	    }

	    if (type.is<LambdaType> ()) {
		if (!construct || !gen.is<LambdaProto> ()) {
		    auto note = Ymir::Error::createNote (loc);
		    Ymir::Error::occurAndNote (gen.getLocation (), note, ExternalError::get (USE_AS_VALUE));
		} else {
		    verifyMutabilityLevel (loc, gen.getLocation (), type, gen.to <Value> ().getType (), construct);
		}
	    } else if (type.is<Pointer> ()) {
		auto llevel = type.to <Type> ().mutabilityLevel ();
		auto rlevel = gen.to <Value> ().getType ().to <Type> ().mutabilityLevel ();
		if (llevel > std::max (1, rlevel) && !gen.is <NullValue> ()) {
		    auto note = Ymir::Error::createNote (gen.getLocation ());
		    Ymir::Error::occurAndNote (loc, note, ExternalError::get (DISCARD_CONST_LEVEL),
					       llevel, std::max (1, rlevel)
			);
		}		
	    } else {
		verifyMutabilityLevel (loc, gen.getLocation (), type, gen.to <Value> ().getType (), construct);
	    }

	    // TODO Verify locality
	    // if (!type.to<Type> ().isLocal () && gen.to <Value> ().getType ().to <Type> ().isLocal ()) {
	    // 	Ymir::Error::occur (loc, ExternalError::get (DISCARD_LOCALITY));				    
	    // }
	}

	void Visitor::verifyMutabilityLevel (const lexing::Word & loc, const lexing::Word & rloc, const Generator & leftType, const Generator & rightType, bool construct) {
	    auto llevel = leftType.to <Type> ().mutabilityLevel ();
	    // Tuple are different, mutability level does not work with them, we need to verify each fields
	    if (rightType.is <Tuple> ()) {
		for (auto it : Ymir::r (0, leftType.to <Tuple> ().getInners ().size ())) {
		    try {
			verifyMutabilityLevel (leftType.to <Tuple> ().getInners ()[it].getLocation (),
					       rightType.to <Tuple> ().getInners ()[it].getLocation (),
					       leftType.to <Tuple> ().getInners ()[it],
					       rightType.to <Tuple> ().getInners ()[it], false);
		    } catch (Error::ErrorList list) {
			auto note = Ymir::Error::createNote (rloc);
			for (auto &it : list.errors) note.addNote (it);
			Ymir::Error::occurAndNote (loc, note, ExternalError::get (DISCARD_CONST));
		    }
		}
	    } else if (rightType.is <Range> () || leftType.is<Array> ()) {
		auto rlevel = rightType.to <Type> ().mutabilityLevel ();

		// In case of void array, the mutabilityLevel is set to the left operand, as the right operand is necessarily a constant and the left operand can have very deep level
		// Exemple : let dmut a : [[[[[[i32]]]]]] = []; // Ok
		
		if (isVoidArrayType (rightType)) rlevel = llevel;
		    
		if (llevel > std::max (1, rlevel)) { // left operand can be mutable, but it can't modify inner right operand values
		    auto note = Ymir::Error::createNote (rloc);
		    Ymir::Error::occurAndNote (loc, note, ExternalError::get (DISCARD_CONST_LEVEL),
					       llevel, std::max (1, rlevel)
			);
		}		
	    } else if (leftType.is <StructRef> () && !leftType.to <StructRef> ().getRef ().to <semantic::Struct> ().getGenerator ().to <generator::Struct> ().hasComplexField ()) {
		auto rlevel = rightType.to <Type> ().mutabilityLevel ();
		if (llevel > std::max (1, rlevel)) { // left operand can be mutable, but it can't modify inner right operand values
		    auto note = Ymir::Error::createNote (rloc);
		    Ymir::Error::occurAndNote (loc, note, ExternalError::get (DISCARD_CONST_LEVEL),
					       llevel, std::max (1, rlevel)
			);
		}		
	    } else if (leftType.is<Pointer> ()) {
		auto rlevel = rightType.to <Type> ().mutabilityLevel ();
		if (llevel > std::max (1, rlevel)) {
		    auto note = Ymir::Error::createNote (rloc);
		    Ymir::Error::occurAndNote (loc, note, ExternalError::get (DISCARD_CONST_LEVEL),
					       llevel, std::max (1, rlevel)
			);
		}		
	    } else if (leftType.is<ClassPtr> ()) {
		auto rlevel = rightType.to <Type> ().mutabilityLevel ();
		if (llevel > std::max (1, rlevel)) {
		    auto note = Ymir::Error::createNote (rloc);
		    Ymir::Error::occurAndNote (loc, note, ExternalError::get (DISCARD_CONST_LEVEL),
					       llevel, std::max (1, rlevel)
			);
		}		
	    } else if (leftType.is<FuncPtr> ()) { // Yes, i know that's ugly, but easier to understand actually	
	    } else if (leftType.is<ClassRef> ()) {		
		auto rlevel = rightType.to <Type> ().mutabilityLevel ();

		if (llevel > std::max (1, rlevel)) { // left operand can be mutable, but it can't modify inner right operand values
		    auto note = Ymir::Error::createNote (rloc);
		    Ymir::Error::occurAndNote (loc, note, ExternalError::get (DISCARD_CONST_LEVEL),
					       llevel, std::max (1, rlevel)
			);
		}		
	    } else if (leftType.is<LambdaType> ()) {
		if (leftType.to <Type> ().isMutable ())
		    Ymir::Error::occur (rloc, ExternalError::get (DISCARD_CONST_LEVEL),
					1, 0
			);	 
	    } else {
		
		// Verify mutability
		if (leftType.to<Type> ().isComplex () || leftType.to <Type> ().isRef ()) {
		    auto rlevel = rightType.to <Type> ().mutabilityLevel ();
		    
		    // In case of void array, the mutabilityLevel is set to the left operand, as the right operand is necessarily a constant and the left operand can have very deep level
		    // Exemple : let dmut a : [[[[[[i32]]]]]] = []; // Ok
		    
		    if (isVoidArrayType (rightType)) rlevel = llevel; 
		    
		    if ((leftType.to <Type> ().isRef () && construct && llevel > std::max (0, rlevel))) { // If it is the construction of a ref
			auto note = Ymir::Error::createNote (rloc);
			Ymir::Error::occurAndNote (loc, note, ExternalError::get (DISCARD_CONST_LEVEL),
						   llevel, std::max (0, rlevel)
			    );		
		    } else if (llevel > std::max (1, rlevel)) {
			auto note = Ymir::Error::createNote (rloc);
			Ymir::Error::occurAndNote (loc, note, ExternalError::get (DISCARD_CONST_LEVEL),
						   llevel, std::max (1, rlevel)
			    );
		    }
		}
	    }
	}
	

	bool Visitor::isVoidArrayType (const Generator & gen) {
	    if (!gen.is <Array> () && !gen.is <Slice> ()) return false;
	    if (!gen.to <Type> ().getInners () [0].is <Void> ()) return false;
	    return true;
	}

	bool Visitor::canImplicitAlias (const Generator & value) {
	    if (value.is<Copier> () || value.is <Aliaser> () || value.is <Referencer> ()) return true; // It is aliased or copied, that's ok
	    
	    {
		// Totally ok for implicit alias 
		match (value) {
		    s_of_u (ArrayValue) return true;
		    s_of_u (StructCst) return true;
		    s_of_u (ClassCst) return true;
		    s_of_u (ArrayAlloc) return true;
		    s_of_u (NullValue) return true;
		}
	    }
	    
	    return false;
	}
	
	void Visitor::verifyImplicitAlias (const lexing::Word & loc, const Generator & type, const Generator & gen) {
	    if (!type.to <Type> ().needExplicitAlias ()) return; // No need to explicitly alias
	    auto llevel = type.to <Type> ().mutabilityLevel ();
	    	    
	    if (gen.is<Copier> () || gen.is <Aliaser> () || gen.is <Referencer> ()) return; // It is aliased or copied, that's ok
	    auto max_level = 1;

	    {
		// Totally ok for implicit alias 
		match (gen) {
		    s_of_u (ArrayValue) return;
		    s_of_u (StructCst) return;
		    s_of_u (ClassCst) return;
		    s_of_u (ArrayAlloc) return;
		    s_of_u (NullValue) return;
		}
	    }
	    {
		// Ok for implicit alias, but the mutability must be checked
		match (gen) {
		    of (Block, arr) {			
			if (!arr.isLvalue ()) // Implicit alias of LBlock is not allowed
			max_level = arr.getType ().to <Type> ().mutabilityLevel ();
		    }		    	       
		    elof (Conditional,  arr) max_level = arr.getType ().to <Type> ().mutabilityLevel ();
		    elof (ExitScope,    arr) max_level = arr.getType ().to <Type> ().mutabilityLevel ();
		    elof (SuccessScope, arr) max_level = arr.getType ().to <Type> ().mutabilityLevel ();
		    elof (Call,         arr) max_level = arr.getType ().to <Type> ().mutabilityLevel ();
		    fo;
		}
	    }
	    

	    max_level = std::max (1, max_level);
		
	    // If the type is totally immutable, it's it not necessary to make an explicit alias 
	    if (llevel > max_level) {
		std::list <Ymir::Error::ErrorMsg> notes;
		notes.push_back (Ymir::Error::createNote (gen.getLocation (), ExternalError::get (IMPLICIT_ALIAS),
							  gen.to <Value> ().getType ().to <Type> ().getTypeName ()));

		if (type.is <StructRef> ()) {
		    auto & varDecl = type.to <StructRef> ().getExplicitAliasTypeLoc ();
		    notes.push_back (Ymir::Error::createNote (varDecl.getLocation (), ExternalError::get (IMPLICIT_ALIAS),
							      varDecl.to <generator::VarDecl> ().getVarType ().prettyString ()));
		}
		
		Ymir::Error::occurAndNote (loc, notes, ExternalError::get (DISCARD_CONST_LEVEL),
					   llevel, max_level
		);
	    }
	}

	void Visitor::verifyCompleteType (const lexing::Word & loc, const Generator &type) {
	    if (type.is <LambdaType> ()) {
		if (!loc.isSame (type.getLocation ())) {
		    auto note = Ymir::Error::createNote (loc);
		    Ymir::Error::occurAndNote (type.getLocation (), note, ExternalError::get (INCOMPLETE_TYPE), type.prettyString ());
		} else
		    Ymir::Error::occur (type.getLocation (), ExternalError::get (INCOMPLETE_TYPE), type.prettyString ());		
	    }
	}
	
	Generator Visitor::applyDecoratorOnVarDeclType (const std::vector <syntax::DecoratorWord> & decos, const Generator & type, bool & isRef, bool & isMutable, bool & dmut) {
	    isMutable = false;
	    isRef = false;
	    for (auto & deco : decos) {
		switch (deco.getValue ()) {
		case syntax::Decorator::REF : {
		    isRef = true;
		} break;
		case syntax::Decorator::MUT : {
		    isMutable = true;
		} break;
		case syntax::Decorator::DMUT : {
		    dmut = true;
		} break;
		default :
		    Ymir::Error::occur (deco.getLocation (),
					ExternalError::get (DECO_OUT_OF_CONTEXT),
					deco.getLocation ().getStr ()
		    );
		}
	    }

	    auto retType = Type::init (type.to <Type> (), isMutable, isRef);
	    if (dmut) retType = retType.to <Type> ().toDeeplyMutable ();
	    
	    return retType;
	}

	void Visitor::verifyMutabilityRefParam (const lexing::Word & loc, const Generator & type, Ymir::ExternalErrorValue error) {
	    // Exception slice can be mutable even if it is not a reference, that is the only exception
	    if (type.to<Type> ().isMutable () && !type.to<Type> ().isRef () && !type.to <Type> ().needExplicitAlias ()) {
		Ymir::Error::occur (loc, ExternalError::get (error));
	    }	    
	}

	void Visitor::verifySameType (const Generator & left, const Generator & right) {	    
	    if (!left.equals (right)) {
		if (left.getLocation ().getLine () == right.getLocation ().getLine () && left.getLocation ().getColumn () == right.getLocation ().getColumn ()) 
		    Ymir::Error::occur (left.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					left.to<Type> ().getTypeName (),
					right.to <Type> ().getTypeName ()
		    );
		else {
		    auto note = Ymir::Error::createNote (right.getLocation ());
		    Ymir::Error::occurAndNote (left.getLocation (), note, ExternalError::get (INCOMPATIBLE_TYPES),
					       left.to<Type> ().getTypeName (),
					       right.to <Type> ().getTypeName ()
		    );
		}
	    }
	}
	
	void Visitor::verifyCompleteSameType (const Generator & left, const Generator & right) {	    
	    if (!left.to <Type> ().completeEquals (right)) {		
		if (left.getLocation ().getLine () == right.getLocation ().getLine () && left.getLocation ().getColumn () == right.getLocation ().getColumn ()) 
		    Ymir::Error::occur (left.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					left.to<Type> ().getTypeName (),
					right.to <Type> ().getTypeName ()
		    );
		else {
		    auto note = Ymir::Error::createNote (right.getLocation ());
		    Ymir::Error::occurAndNote (left.getLocation (), note, ExternalError::get (INCOMPATIBLE_TYPES),
					       left.to<Type> ().getTypeName (),
					       right.to <Type> ().getTypeName ()
		    );
		}
	    }
	}

	Generator Visitor::validateTypeClassContext (const lexing::Word & loc, const Generator & cl, const syntax::Expression & type) {
	    auto sym = cl.to <ClassPtr> ().getInners ()[0].to <ClassRef> ().getRef ();
	    Generator gen (Generator::empty ());
	    std::list <Error::ErrorMsg> errors;
	    
	    pushReferent (sym, "validateTypeClContext");	    
	    try {
		gen = this-> validateType (type, true);
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    }
	    popReferent ("validateTypeClContext");
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    return gen;	    
	}

	void Visitor::verifyClassImpl (const lexing::Word & loc, const Generator & cl, const syntax::Expression & trait) {
	    auto type = this-> validateTypeClassContext (loc, cl, trait);
	    return verifyClassImpl (loc, cl, type);
	}
	
	void Visitor::verifyClassImpl (const lexing::Word & loc, const Generator & cl, const Generator & trait) {
	    if (!trait.is <TraitRef> ()) {
		Ymir::Error::occur (trait.getLocation (), ExternalError::get (IMPL_NO_TRAIT), trait.prettyString ());
	    }

	    auto sym = cl.to <ClassPtr> ().getInners ()[0].to <ClassRef> ().getRef ();
	    while (!sym.isEmpty ()) {
		for (auto & it : sym.to <semantic::Class> ().getAllInner ()) {
		    bool succeed = false;
		    std::list <Ymir::Error::ErrorMsg> errors;
		    match (it) {
			of (semantic::Impl, im) {
			    pushReferent (sym, "verifyClassImpl");
			    enterForeign ();
			    
			    try {
				auto sec_trait = this-> validateType (im.getTrait ());
				if (trait.equals (sec_trait)) succeed = true;
			    } catch (Error::ErrorList list) {
				errors = list.errors;
			    }
			    
			    exitForeign ();
			    popReferent ("verifyClassImpl");
			    
			    if (errors.size () != 0)
			    throw Error::ErrorList {errors};
			    if (succeed) return;
			} fo;
		    }		
		}
		
		auto ancestor = sym.to <semantic::Class> ().getGenerator ().to <generator::Class> ().getClassRef ().to <ClassRef> ().getAncestor ();
		if (!ancestor.isEmpty ())
		    sym = ancestor.to <ClassRef> ().getRef ();
		else break;
	    } 

	    auto note = Ymir::Error::createNote (loc);
	    Ymir::Error::occurAndNote (cl.getLocation (), note, ExternalError::get (NOT_IMPL_TRAIT), cl.prettyString (), trait.prettyString ());
	}

	std::vector <Generator> Visitor::getAllImplClass (const Generator &cl) {
	    Symbol sym (Symbol::empty ());
	    Generator classType (Generator::empty ());
	    if (cl.is <ClassPtr> ()) {
		sym = cl.to<ClassPtr> ().getClassRef ().getRef ();
		classType = cl.to <ClassPtr> ().getInners ()[0];
	    } else {
		sym = cl.to <ClassRef> ().getRef ();
		classType = cl;
	    }
	    
	    std::vector <Generator> traits;
	    if (!classType.to <ClassRef> ().getAncestor ().isEmpty ())
		traits = getAllImplClass (classType.to <ClassRef> ().getAncestor ());
	    
	    for (auto & it : sym.to <semantic::Class> ().getAllInner ()) {
		std::list <Ymir::Error::ErrorMsg> errors;
		match (it) {
		    of (semantic::Impl, im) {
			pushReferent (sym, "getAllImplClass");
			enterForeign ();
			try {
			    auto sec_trait = this-> validateType (im.getTrait ());
			    traits.push_back (sec_trait);
			} catch (Error::ErrorList list) {
			    errors = list.errors;
			}
			    
			exitForeign ();
			popReferent ("getAllImplClass");
			if (errors.size () != 0)
			throw Error::ErrorList {errors};
		    } fo;
		}
	    }
	    return traits;
	}
	
	void Visitor::verifyCompatibleTypeWithValue (const lexing::Word & loc, const Generator & type, const Generator & gen) {
	    if (gen.is <NullValue> () && type.is <Pointer> ())  return;
	    else if (gen.to <Value> ().getType ().is <Slice> () && gen.to <Value> ().getType ().to <Type> ().getInners () [0].is<Void> () && type.is <Slice> ()) return;
	    // else if (type.is <Integer> () && this-> isIntConstant (gen)) return; // We allow int implicit cast if the operand is knwon at compile time and is a int value
	    // else if (type.is <Float> () && this-> isFloatConstant (gen)) return; // Idem for float const
	    
	    verifyCompatibleType (loc, gen.getLocation (), type, gen.to <Value> ().getType ());
	}	

	
	void Visitor::verifyCompatibleType (const lexing::Word & loc, const lexing::Word & rightLoc, const Generator & left, const Generator & right, bool fromObject) {
	    bool error = false;
	    std::string leftName;	    
	    if (!left.to<Type> ().isCompatible (right)) {
		// It can be compatible with an ancestor of right
		error = !isAncestor (left, right);
		if (!error) return;
		
		leftName = left.to<Type> ().getTypeName ();
	    }
	    	    
	    if (right.is <ClassPtr> () && fromObject) {
		auto syntaxType = this-> createClassTypeFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (OBJECT_MODULE), CoreNames::get (OBJECT_TYPE)});
		auto objectType = this-> validateType (syntaxType);
		if (objectType.to <Type> ().isCompatible (left)) return;
	    }
	    
	    if (!left.to <Type> ().getProxy ().isEmpty () && !left.to <Type> ().getProxy ().to <Type> ().isCompatible (right.to <Type> ().getProxy ())) {
		error = true;
		leftName = left.to<Type> ().getProxy ().to <Type> ().getTypeName ();
	    }

	    if (error) {
		if (loc.getLine () == rightLoc.getLine ()) 
		    Ymir::Error::occur (loc, ExternalError::get (INCOMPATIBLE_TYPES),
					leftName, 
					right.to <Type> ().getTypeName ()
		    );
		else {
		    auto note = Ymir::Error::createNote (rightLoc);
		    Ymir::Error::occurAndNote (loc, note, ExternalError::get (INCOMPATIBLE_TYPES),
					       leftName,
					       right.to <Type> ().getTypeName ()
		    );
		}
	    }
	}

	void Visitor::throwIncompatibleTypes (const lexing::Word & loc, const lexing::Word & rightLoc, const Generator & left, const Generator & right, const std::list <Error::ErrorMsg> & notes) {
	    std::string leftName = left.prettyString ();	    
	    if (!left.to <Type> ().getProxy ().isEmpty () && !left.to <Type> ().getProxy ().to <Type> ().isCompatible (right.to <Type> ().getProxy ())) {
		leftName = left.to<Type> ().getProxy ().to <Type> ().getTypeName ();
	    }

	    if (loc.getLine () == rightLoc.getLine ()) {
		Ymir::Error::occurAndNote (loc, notes, ExternalError::get (INCOMPATIBLE_TYPES),
				    leftName, 
				    right.to <Type> ().getTypeName ()
		    );
	    } else {
		std::list <Error::ErrorMsg> auxNotes = {Ymir::Error::createNote (rightLoc)};
		auxNotes.insert (auxNotes.end (), notes.begin (), notes.end ());
		
		Ymir::Error::occurAndNote (loc, auxNotes, ExternalError::get (INCOMPATIBLE_TYPES),
					   leftName,
					   right.to <Type> ().getTypeName ()
		    );
	    }
	}

	Generator Visitor::deduceTypeBranching (const lexing::Word & lloc, const lexing::Word & rloc, const generator::Generator & left, const generator::Generator & right) {
	    /**
	     * Proxy verification 
	     */
	    if (!left.to <Type> ().getProxy ().isEmpty () || !right.to <Type> ().getProxy ().isEmpty ()) {
		if (left.to <Type> ().isCompatible (right)) {
		    if (left.to <Type> ().getProxy ().isEmpty () || right.to <Type> ().getProxy ().isEmpty ()) {
			if (left.to <Type> ().mutabilityLevel () < right.to <Type> ().mutabilityLevel ()) {
			    return Type::init (Type::init (left.to <Type> (), left.to <Type> ().isMutable ()).to<Type> (), Generator::empty ());
			} else {
			    return Type::init (Type::init (right.to <Type> (), right.to <Type> ().isMutable ()).to<Type> (), Generator::empty ());
			}
		    }
		    else {		    
			if (!left.to<Type> ().getProxy ().to <Type> ().isCompatible (right.to<Type> ().getProxy ())) {
			    if (left.to <Type> ().mutabilityLevel () < right.to <Type> ().mutabilityLevel ()) {
				return Type::init (Type::init (left.to <Type> (), left.to <Type> ().isMutable ()).to <Type> (), Generator::empty ()); // remove the proxy
			    } else {
				return Type::init (Type::init (right.to <Type> (), right.to <Type> ().isMutable ()).to <Type> (), Generator::empty ()); // remove the proxy
			    }
			} else {
			    if (left.to <Type> ().mutabilityLevel () < right.to <Type> ().mutabilityLevel ()) {
				return Type::init (Type::init (left.to<Type> (), left.to <Type> ().isMutable ()).to <Type> (), left.to<Type> ().getProxy ());
			    } else {
				return Type::init (Type::init (right.to <Type> (), right.to <Type> ().isMutable ()).to <Type> (), left.to <Type> ().getProxy ()); // remove the proxy
			    }
			}
		    }
		} else { // the type are incompatible, the proxy has no impact, we just remove it 
		    auto ltype = Type::init (left.to <Type> (), Generator::empty ());
		    auto rtype = Type::init (right.to <Type> (), Generator::empty ());
		    return deduceTypeBranching (lloc, rloc, ltype, rtype); // and retry
		}
	    }

	    if (left.to <Type> ().isCompatible (right)) {
		if (left.to <Type> ().mutabilityLevel () < right.to <Type> ().mutabilityLevel ()) return left;
		else return right;
	    } else {
		match (left) {
		    of (Slice, ls) {
			if (right.is<Slice> ()) {
			    auto & rs = right.to <Slice> ();
			    if (ls.getInners ()[0].is <Void> ()) return right;
			    if (rs.getInners ()[0].is <Void> ()) return left;
			    else {
				try {
				    auto slc = Slice::init (lloc, this-> deduceTypeBranching (lloc, rloc, ls.getInners ()[0], rs.getInners ()[0]));
				    if (ls.mutabilityLevel () < rs.mutabilityLevel ()) {
					slc = Type::init (slc.to<Type> (), ls.isMutable ());
				    } else {
					slc = Type::init (slc.to<Type> (), rs.isMutable ());
				    }
				    return slc;
				} catch (Error::ErrorList list) {
				    throwIncompatibleTypes (lloc, rloc, left, right, list.errors);
				}
			    }
			} 
		    } elof (ClassPtr, lp) {
			if (right.is <ClassPtr> ()) {			    
			    auto & rp = right.to <ClassPtr> ();
			    Generator ancestor = Generator::empty ();
			    if (isAncestor (left, right)) { // right over left
				ancestor = left;
			    } else if (isAncestor (right, left)) {
				ancestor = right;
			    } else {
				ancestor = this-> getCommonAncestor (left, right);
			    }
			    			
			    if (ancestor.isEmpty ()) {
				this-> verifyCompatibleType (lloc, rloc, left, right);
			    } else {
				if (lp.mutabilityLevel () < rp.mutabilityLevel ()) {
				    auto inner = Type::init (ancestor.to <Type> ().getInners ()[0].to <Type> (), lp.getInners ()[0].to <Type> ().isMutable ());
				    return Type::init (ClassPtr::init (ancestor.getLocation (), inner).to <Type> (), lp.isMutable ());
				} else  {
				    auto inner = Type::init (ancestor.to <Type> ().getInners ()[0].to <Type> (), rp.getInners ()[0].to <Type> ().isMutable ());
				    return Type::init (ClassPtr::init (ancestor.getLocation (), inner).to<Type> (), rp.isMutable ());
				}			    
			    }
			}
		    } elof (Tuple, lt) {
			if (right.is <Tuple> ()) {
			    auto & rt = right.to <Tuple> ();
			    if (lt.getInners ().size () != rt.getInners ().size ()) {
				this-> verifyCompatibleType (lloc, rloc, left, right);
			    }
			    
			    try {
				std::vector <Generator> innerParams;
				for (auto it : Ymir::r (0, rt.getInners ().size ())) {
				    innerParams.push_back (this-> deduceTypeBranching (
							       lt.getInners ()[it].getLocation (),
							       rt.getInners ()[it].getLocation (),
							       lt.getInners ()[it], rt.getInners ()[it]));
				}
				bool isMutable = lt.isMutable () && rt.isMutable ();
				return Type::init (Tuple::init (lloc, innerParams).to <Type> (), isMutable);
			    } catch (Error::ErrorList list) {
				throwIncompatibleTypes (lloc, rloc, left, right, list.errors);
			    }
			}
		    } fo;
		}
	    }

	    // To throw the error only
	    throwIncompatibleTypes (lloc, rloc, left, right, {});
	    return Generator::empty ();
	}	

	bool Visitor::isIntConstant (const Generator & value) {
	    if (!value.to <Value> ().getType ().is <Integer> ()) return false;
	    try {
		auto val = retreiveValue (value);
		return val.is <Fixed> ();
	    } catch (Error::ErrorList err) {
		return false;
	    }
	}

	bool Visitor::isFloatConstant (const Generator & value) {
	    if (!value.to <Value> ().getType ().is <Float> ()) return false;
	    try {
		auto val = retreiveValue (value);
		return val.is <FloatValue> ();		
	    } catch (Error::ErrorList list) {
		return false;
	    }
	}

	bool Visitor::isAncestor (const Generator & left_, const Generator & right_) {
	    auto right = right_;
	    auto left = left_;
	    
	    if (right.is <ClassPtr> ()) right = right.to <Type>().getInners ()[0];
	    if (left.is <ClassPtr> ()) left = left.to <Type>().getInners ()[0];
	    
	    if (right.is <ClassRef> () && !right.to <ClassRef> ().getRef ().to <semantic::Class> ().getAncestor ().isEmpty ()) {
		auto ancestor = right.to <ClassRef> ().getAncestor ();
		while (!ancestor.isEmpty ()) {
		    if (left.to <Type> ().isCompatible (ancestor)) return true;
		    else {
			if (!ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getAncestor ().isEmpty ())
			    ancestor = ancestor.to <ClassRef> ().getAncestor ();
			else ancestor = Generator::empty ();
		    }
		}
	    }
	    return false;
	}
	
	void Visitor::verifyShadow (const lexing::Word & name) {
	    verifyNotIsType (name);
	    
	    auto gen = getLocal (name.getStr ());	    
	    if (!gen.isEmpty ()) {		
		auto note = Ymir::Error::createNote (gen.getLocation ());		
		Error::occurAndNote (name, note, ExternalError::get (SHADOWING_DECL), name.getStr ());
	    }	    
	}

	void Visitor::verifyNotIsType (const lexing::Word & name) {
	    if (std::find (Integer::NAMES.begin (), Integer::NAMES.end (), name.getStr ()) != Integer::NAMES.end ()) {
		Error::occur  (name, ExternalError::get (IS_TYPE), name.getStr ());
	    } else if (name.getStr () == Void::NAME) {
		Error::occur  (name, ExternalError::get (IS_TYPE), name.getStr ());
	    } else if (name.getStr () == Bool::NAME) {
		Error::occur  (name, ExternalError::get (IS_TYPE), name.getStr ());
	    } else if (std::find (Float::NAMES.begin (), Float::NAMES.end (), name.getStr ()) != Float::NAMES.end ()) {
		Error::occur  (name, ExternalError::get (IS_TYPE), name.getStr ());
	    } else if (std::find (Char::NAMES.begin (), Char::NAMES.end (), name.getStr ()) != Char::NAMES.end ()) {
		Error::occur  (name, ExternalError::get (IS_TYPE), name.getStr ());
	    }
	}
	

	void Visitor::verifyThrows (const std::vector <Generator> & types, const std::vector <Generator> & rethrow, std::vector <Generator> & unused, std::vector <Generator> & notfound) {	    
	    // This function is ugly, maybe there is a better way to do this
	    // But still this is working
	    std::vector <Generator> used;
	    for (auto &it : types) {
		bool found = false;
		for (auto & j : used) {
		    if (it.to<Type> ().isCompatible (j)) {
			found = true;
			break;
		    }
		}
		
		if (!found) {
		    for (auto &j : rethrow) {
			if (it.to<Type> ().isCompatible (j)) {
			    found = true;
			    used.push_back (j);
			    break;
			}
		    }
		    
		    if (!found) notfound.push_back (it);
		}
	    }

	    for (auto & it : rethrow) {
		bool found = false;
		for (auto & j : used) {
		    if (it.to <Type> ().isCompatible (j)) {
			found = true;
			break;
		    }
		}
		if (!found) unused.push_back (it);
	    }
	}
	
	Generator Visitor::retreiveValue (const Generator & gen) {
	    auto compile_time = CompileTime::init (*this);
	    return compile_time.execute (gen);
	}

	const Generator & Visitor::retreiveFrameFromProto (const FrameProto & proto) {
	    auto name = Mangler::init ().mangleFrameProto (proto);
	    for (auto & it : this-> _list) {
		if (it.is<Frame> ()) {
		    auto sec_name = Mangler::init ().mangleFrame (it.to <Frame> ());
		    if (sec_name == name) return it;
		}
	    }
	    return Generator::__empty__;
	}

	bool Visitor::insertTemplateSolution (const Symbol & sol) {
	    for (auto & it : this-> _templateSolutions) {
		if (it.equals (sol)) return false;		
	    }
	    
	    this-> _templateSolutions.push_back (sol);
	    return true;
	}

	void Visitor::removeTemplateSolution (const Symbol & sol) {
	    std::vector <Symbol> res;
	    for (auto & it : this-> _templateSolutions) {
		if (!it.equals (sol)) res.push_back (it);
	    }

	    this-> _templateSolutions = res;
	}
	
	std::vector <Symbol> Visitor::getGlobal (const std::string & name) {					       
	    return this-> _referent.back ().get (name);
	}

	std::vector <Symbol> Visitor::getGlobalPrivate (const std::string & name) {
	    return this-> _referent.back ().getPrivate (name);
	}	
	
	void Visitor::pushReferent (const semantic::Symbol & sym, const std::string &) {
	    // print (this-> _referent.size (), ' ');
	    // println ("IN : ", msg, " => ", sym.getRealName ());
	    this-> _referent.push_back (sym);
	}

	void Visitor::popReferent (const std::string &) {
	    // print (this-> _referent.size ());
	    // println ("Out : ", msg, " => ", this-> _referent.back ().getRealName ());
	    this-> _referent.pop_back ();
	}
       	
	syntax::Expression Visitor::createVarFromPath (const lexing::Word & loc, const std::vector <std::string> & names_) {
	    auto names = names_;
	    syntax::Expression last = syntax::Var::init (lexing::Word::init (loc, names [0]));
	    names = std::vector <std::string> (names.begin () + 1, names.end ());
	    while (names.size () > 0) {
		last = syntax::Binary::init (
		    lexing::Word::init (loc, Token::DCOLON),
		    last,
		    syntax::Var::init (lexing::Word::init (loc, names [0])),
		    syntax::Expression::empty ()
		);
		names = std::vector <std::string> (names.begin () + 1, names.end ());
	    }	    
	    return last;
	}

	syntax::Expression Visitor::createClassTypeFromPath (const lexing::Word & loc, const std::vector <std::string> & names_) {
	    auto last = createVarFromPath (loc, names_);
	    return syntax::Unary::init (
		lexing::Word::init (loc, Token::AND),
		last
		);
	}

	Generator Visitor::addElseToConditional (const Generator & gen, const Generator & _else) {
	    match (gen) {
		of (Conditional, cd) {
		    if (cd.getElse ().isEmpty ()) {
			return Conditional::init (cd.getLocation (), cd.getType (), cd.getTest (), cd.getContent (), _else, cd.isComplete ());
		    } else {
			auto addElse_ = addElseToConditional (cd.getElse (), _else);
			return Conditional::init (cd.getLocation (), cd.getType (), cd.getTest (), cd.getContent (), addElse_, cd.isComplete ());
		    }
		}
		elfo {
		    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");   
		}
	    }
	    return Generator::empty ();
	}

	Generator Visitor::setCompleteConditional (const Generator & gen) {
	    match (gen) {
		of (Conditional, cd) {
		    if (cd.getElse ().isEmpty ()) {
			return Conditional::init (cd.getLocation (), cd.getType (), cd.getTest (), cd.getContent (), Generator::empty (), true);
		    } else {
			auto _else = setCompleteConditional (cd.getElse ());
			return Conditional::init (cd.getLocation (), cd.getType (), cd.getTest (), cd.getContent (), _else, cd.isComplete ());
		    }
		} fo;		
	    }	    
	    return gen;
	}

	Generator Visitor::getCommonAncestor (const Generator & leftType, const Generator & rightType_) {
	    auto ancestor = leftType;
	    auto rightType = rightType_;
	    
	    if (leftType.is <ClassPtr> ())  ancestor = ancestor.to <ClassPtr> ().getInners ()[0];
	    if (rightType.is <ClassPtr> ()) rightType = rightType.to <ClassPtr> ().getInners ()[0];
	    
	    if (ancestor.is <ClassRef> ()) {
		while (!ancestor.isEmpty ()) {
		    if (isAncestor (ancestor, rightType))
			return ClassPtr::init (leftType.getLocation (), ancestor);
		    else {
			if (!ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getAncestor ().isEmpty ())
			    ancestor = ancestor.to <ClassRef> ().getAncestor ();
			else ancestor = Generator::empty ();
		    }
		}
	    }
	    return Generator::empty ();
	}
	
	bool Visitor::isUseless (const Generator & value) {
	    match (value) {
		s_of_u (Affect)  return false;
		s_of_u (Block)  return false;
		s_of_u (Break)  return false;
		s_of_u (Call)  return false;
		s_of_u (Conditional) return false;
		s_of_u (Loop)  return false;
		s_of_u (generator::VarDecl) return false;
		s_of_u (Return) return false;
		s_of_u (Set) return false;
		s_of_u (Throw) return false;
		s_of_u (ExitScope) return false;
	    }
	    return true;
	}	
	
    }
    
}
