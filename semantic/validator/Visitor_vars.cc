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

	Generator Visitor::validateVar (const syntax::Var & var) {
	    auto gen = getLocal (var.getName ().getStr ());	    
	    
	    if (gen.isEmpty ()) {
		auto sym = getGlobal (var.getName ().getStr ());		
		if (sym.empty ()) {
		    sym = getGlobalPrivate (var.getName ().getStr ());
		    std::list <Ymir::Error::ErrorMsg> notes;
		    for (auto it : Ymir::r (0, sym.size ())) {
			notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::PRIVATE_IN_THIS_CONTEXT, sym[it].getName (), sym [it].getRealName ()));
		    }
		    Error::occurAndNote (var.getLocation (), notes, ExternalError::UNDEF_VAR, var.getName ().getStr ());
		}

		auto ret = validateMultSym (var.getLocation (), sym);
		if (ret.is <MultSym> () && ret.to <MultSym> ().getGenerators ().size () == 0) {
		    Error::occur (var.getLocation (), ExternalError::UNDEF_VAR, var.getName ().getStr ());
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
				gens.push_back (FunctionVisitor::init (*this).validateFunctionProto (sym));			    
			    }
			    succ = true;
			}
			elof_u (semantic::Constructor) {
			    gens.push_back (FunctionVisitor::init (*this).validateConstructorProto (sym));		    
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
			elof_u (semantic::TemplatePreSolution) {
			    auto loc_gens = validateMultSym (loc, this-> validateTemplatePreSolution (sym, Generator::empty ()).to <semantic::TemplateSolution> ().getAllLocal ());
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
			    validateVarDecl (sym, false);
			    auto gen = decl.getGenerator ().to <GlobalVar> ();
			    Generator value (Generator::empty ());
			    if (!gen.isMutable ())
			    value = gen.getValue ();
			    gens.push_back (VarRef::init (decl.getName (), decl.getName ().getStr (), gen.getType (), gen.getUniqId (), gen.isMutable (), value));
			    succ = true;
			}
			elof_u (semantic::Aka) {
			    auto al_ref = validateAka (sym);
			    gens.push_back (al_ref);
			    succ = true;
			} fo;			
		    }
		} catch (Error::ErrorList list) {
		    errors = list.errors;
		    errors.back ().addNote (
			Ymir::Error::createNote (loc)
			);
		} 		

		popReferent ("validateMultSym");
		if (errors.size () != 0) {
		    throw Error::ErrorList {errors};
		}
		
		if (!succ) {
		    println (sym.formatTree ());
		    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		}
	    }
	    
	    if (gens.size () == 1) return gens [0];
	    else {
		if (gens.size () == 0) {
		    println ("ici ?", " ", multSym.size ());
		    for (auto & it : multSym) println (it.getRealName (), " ", it.getName ());
		}
		return MultSym::init (loc, gens);
	    }
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
			    Ymir::Error::occur (loc, ExternalError::USE_AS_TYPE);
			}							     
			elof_u (semantic::Module) {
			    Ymir::Error::occur (loc, ExternalError::USE_AS_TYPE);
			}
			elof_u (semantic::ModRef) {
			    Ymir::Error::occur (loc, ExternalError::USE_AS_TYPE);
			}
			elof_u (semantic::Aka) {
			    locGen = validateAka (multSym [it]);
			}
			elof (semantic::Function, func) {
			    if (!func.isMethod ()) {
				locGen = FunctionVisitor::init (*this).validateFunctionProto (multSym [it]);
			    }
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
		    notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::CANDIDATE_ARE, gen.getLocation (), gen.prettyString ()));
		    notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::CANDIDATE_ARE, locGen.getLocation (), locGen.prettyString ()));
			
		    Ymir::Error::occurAndNote (loc,
					       notes,
					       ExternalError::SPECIALISATION_WORK_TYPE_BOTH);
		}
		
		if (!locGen.isEmpty ())
		gen = locGen;
	    }
	    
	    if (errors.size () != 0 && gen.isEmpty ())
	    throw Error::ErrorList {errors};
	    
	    return gen;
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
					ExternalError::DISCARD_CONST
		    );
		else
		    Ymir::Error::warn (dec_expr.getDecorator (syntax::Decorator::MUT).getLocation (),
				       ExternalError::USELESS_DECORATOR
			);
	    } 

	    if (dec_expr.hasDecorator (syntax::Decorator::DMUT)) {
		if (!inner.to<Value> ().getType ().to<Type> ().isMutable ()) 
		    Ymir::Error::occur (dec_expr.getDecorator (syntax::Decorator::DMUT).getLocation (),
					ExternalError::DISCARD_CONST
		    );
		else
		    Ymir::Error::warn (dec_expr.getDecorator (syntax::Decorator::DMUT).getLocation (),
				       ExternalError::USELESS_DECORATOR
		    );
	    } 
	    
	    if (dec_expr.hasDecorator (syntax::Decorator::REF)) {
		if (inner.to <Value> ().isLvalue ()) {
		    if (inner.is <VarRef> () && inner.to<VarRef> ().isSelf ()) {
			Ymir::Error::occur (inner.getLocation (),
					    ExternalError::REF_SELF
			);
		    }
		    
		    this-> verifyLockAlias (inner);
		    
		    // if (!inner.to <Value> ().getType ().to <Type> ().isMutable ()) {
		    // 	Ymir::Error::occur (inner.getLocation (), ExternalError::IMMUTABLE_LVALUE);
		    // } // We allow this, since we want to pass element by const reference to function, or variable
		    // The mutability will verify if we are allowed to do a reference of the element

		    if (!inner.is<Referencer> ()) {
			auto type = inner.to <Value> ().getType ();
			type = Type::init (type.to <Type> (), type.to <Type> ().isMutable (), true);
			inner = Referencer::init (dec_expr.getLocation (), type, inner);
		    } 
		} else {
		    Ymir::Error::occur (inner.getLocation (),
					ExternalError::NOT_A_LVALUE
		    );
		}
	    }

	    if (dec_expr.hasDecorator (syntax::Decorator::CONST)) {
		auto type = Type::init (inner.to<Value> ().getType ().to <Type> (), false);
		inner = Value::init (inner.to<Value> (), type);
	    }

	    if (dec_expr.hasDecorator (syntax::Decorator::PURE)) {
		auto deco = dec_expr.getDecorator (syntax::Decorator::PURE);
		Ymir::Error::occur (deco.getLocation (),
				    ExternalError::DECO_OUT_OF_CONTEXT,
				    deco.getLocation ().getStr ()
		    );			
	    }
	    
	    return inner;
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

	
    }
}
