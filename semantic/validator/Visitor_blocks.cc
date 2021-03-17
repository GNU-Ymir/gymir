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

	
	Generator Visitor::validateCatchOutOfScope (const syntax::Catch & cat) {
	    Ymir::Error::occur (cat.getLocation (), ExternalError::get (CATCH_OUT_OF_SCOPE));
	    return Generator::empty ();
	}

	Generator Visitor::validateScopeOutOfScope (const syntax::Scope & scope) {
	    Ymir::Error::occur (scope.getLocation (), ExternalError::get (SCOPE_OUT_OF_SCOPE));
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


	
    }

}
