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

	Generator Visitor::applyDecoratorOnVarDeclType (const std::vector <syntax::DecoratorWord> & decos, const Generator & type, bool & isRef, bool & isMutable, bool & dmut, bool canBeRef, bool canBeDmut, bool canBeMut) {
	    isMutable = false;
	    isRef = false;
	    dmut = false;
	    for (auto & deco : decos) {
		switch (deco.getValue ()) {
		case syntax::Decorator::REF : {
		    if (canBeRef) { 
			isRef = true;
		    } else {
			Ymir::Error::occur (deco.getLocation (),
					    ExternalError::DECO_OUT_OF_CONTEXT,
					    deco.getLocation ().getStr ()
			    );
		    }
		} break;
		case syntax::Decorator::MUT : {
		    if (canBeMut) {
			isMutable = true;
		    } else {
			Ymir::Error::occur (deco.getLocation (),
					    ExternalError::DECO_OUT_OF_CONTEXT,
					    deco.getLocation ().getStr ()
			    );
		    }
		} break;
		case syntax::Decorator::DMUT : {
		    if (canBeDmut) {
			dmut = true;
		    } else {
			Ymir::Error::occur (deco.getLocation (),
					    ExternalError::DECO_OUT_OF_CONTEXT,
					    deco.getLocation ().getStr ()
			    );
		    }
		} break;
		default :
		    Ymir::Error::occur (deco.getLocation (),
					ExternalError::DECO_OUT_OF_CONTEXT,
					deco.getLocation ().getStr ()
		    );
		}
	    }

	    auto retType = Type::init (type.to <Type> (), isMutable, isRef);
	    if (dmut) retType = retType.to <Type> ().toDeeplyMutable ();
	    
	    return retType;
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
				Ymir::Error::createNoteOneLine (ExternalError::PRIVATE_IN_THIS_CONTEXT, gen.getName (), FunctionVisitor::init (*this).validateConstructorProto (gen).prettyString ())					    
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
				Ymir::Error::createNoteOneLine (ExternalError::PRIVATE_IN_THIS_CONTEXT, gen.getName (), gen.getName ().getStr ())					    
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
				    Ymir::Error::createNoteOneLine (ExternalError::PRIVATE_IN_THIS_CONTEXT, gen.getName (), gen.getName ().getStr ())
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
	    if (module.is <ModRef> ()) module = module.to <ModRef> ().getModule ().getValue ();
	    
	    if (this-> _referent.size () != 0) {
		auto curr = this-> _referent.back ();
		while (!curr.isEmpty () && !module.isEmpty ()) {
		    if (curr.equals (module)) return true;		    		    
		    curr = curr.getReferent ();
		}
	    }
	    return false;
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

	Generator Visitor::getTypeInfoType () {	    
	    auto typeInfo = createVarFromPath (lexing::Word::eof (), {CoreNames::get (CORE_MODULE), CoreNames::get (TYPE_INFO_MODULE), CoreNames::get (TYPE_INFO)});
	    return this-> validateType (typeInfo);
	}

	Generator Visitor::getTypeInfoIds () {
	    auto typeIDs = createVarFromPath (lexing::Word::eof (), {CoreNames::get (CORE_MODULE), CoreNames::get (TYPE_INFO_MODULE), CoreNames::get (TYPE_IDS)});
	    return this-> validateValue (typeIDs);
	}

	Generator Visitor::getOutOfArrayCall (const lexing::Word & loc) {
	    auto func = this-> createVarFromPath (lexing::Word::eof (), {CoreNames::get (CORE_MODULE), CoreNames::get (ARRAY_MODULE), CoreNames::get (OUT_OF_ARRAY)});
	    auto proto = this-> validateValue (func);
	    auto callvisitor = CallVisitor::init (*this);
	    std::list <Error::ErrorMsg> errors;

	    if (proto.is <FrameProto> ()) {
		int score = 0;
		auto ret = callvisitor.validateFrameProto (loc, proto, {}, score, errors);
		if (errors.size () != 0) {
		    throw Error::ErrorList {errors};
		}
		return ret;
	    } else {
		Error::occur (loc, ExternalError::UNDEF_VAR, func.prettyString ());
		return Generator::empty ();
	    }
	}
	
	Generator Visitor::getDisposeTrait () {
	    auto trait = this-> createVarFromPath (lexing::Word::eof (), {CoreNames::get (CORE_MODULE), CoreNames::get (DISPOSING_MODULE), CoreNames::get (DISPOSABLE_TRAITS)});		
	    return this-> validateType (trait);
	}

	Generator Visitor::getExceptionType () {
	    auto syntaxType = this-> createClassTypeFromPath (lexing::Word::eof (), {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (EXCEPTION_TYPE)});
	    return this-> validateType (syntaxType);
	}

	Generator Visitor::getObjectType () {
	    auto syntaxType = this-> createClassTypeFromPath (lexing::Word::eof (), {CoreNames::get (CORE_MODULE), CoreNames::get (OBJECT_MODULE), CoreNames::get (OBJECT_TYPE)});
	    return this-> validateType (syntaxType);
	}

	Generator Visitor::getSegFault () {
	    auto syntaxType = this-> createClassTypeFromPath (lexing::Word::eof (), {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (SEG_FAULT_TYPE)});
	    return this-> validateType (syntaxType);
	}

	const VisitorCache & Visitor::getCache () const {
	    return this-> _cache;
	}
	
	
    }
}
