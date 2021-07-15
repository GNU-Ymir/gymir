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
	    // There is no reason to have a catch scope out of a block
	    // THe catcher is validated inside the validateBlock function
	    Ymir::Error::occur (cat.getLocation (), ExternalError::CATCH_OUT_OF_SCOPE);
	    return Generator::empty ();
	}

	Generator Visitor::validateScopeOutOfScope (const syntax::Scope & scope) {
	    // Similarely, there is no reason to have a scope out of a block
	    Ymir::Error::occur (scope.getLocation (), ExternalError::SCOPE_OUT_OF_SCOPE);
	    return Generator::empty ();
	}

	Generator Visitor::validateBlock (const syntax::Block & block, const std::vector <Generator> & init) {
	    std::vector <Generator> values = init;
	    
	    Generator type (Void::init (block.getLocation ()));
	    lexing::Word valueLoc (block.getLocation ());
	    
	    bool breaker = false, returner = false;
	    lexing::Word brLoc = lexing::Word::eof (), rtLoc = lexing::Word::eof ();
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    this-> enterBlock (); // A block is composed of four parts,
	    // A declarator module, containing the symbol declared inside the block
	    auto decl = this-> validateInnerModule (block.getDeclModule (), errors);	    
	    
	    if (!decl.isEmpty ()) pushReferent (decl, "validateBlock"); // we need to add the declarations as the referent of the block	    
	    // The second part is the block itself, (list of values)
	    this-> validateInnerBlock (block, values, type, valueLoc, returner, rtLoc, breaker, brLoc, errors);

	    if (!decl.isEmpty ()) popReferent ("validateBlock"); // at the end of the block, the symbols declared inside it does not exists anymore	    
	    
	    // If there are some errors, no need to add the warning about the unused vars
	    // Moreover, they may be not pertinent 
	    if (errors.size () != 0) {
		this-> discardAllLocals (); 
	    }

	    this-> quitBlock (errors.size () == 0, errors);
	    
	    auto ret = Value::initBrRet (Block::init (valueLoc, type, values).to <Value> (), breaker, returner, brLoc, rtLoc);	    
	    Generator catchVar (Generator::empty ()), catchInfo (Generator::empty ()), catchAction (Generator::empty ());	    
	    if (errors.size () == 0) { // The third part is a catcher
		this-> validateCatcher (block, catchVar, catchInfo, catchAction, type, ret.getThrowers (), errors);
	    }

	    std::vector <Generator> onExit, onSuccess, onFailure; // and then the guard scopes
	    this-> validateScopes (block, onExit, onSuccess, onFailure, returner, breaker, ret.getThrowers ().size () != 0, errors);
	    	    
	    if (errors.size () != 0) throw Error::ErrorList {errors};
	   	    
	    if (onSuccess.size () != 0) ret = SuccessScope::init (valueLoc, type, ret, onSuccess); // if there is no catcher (failure, exit or catch), no need add a catcher
	    if (onExit.size () != 0 || onFailure.size () != 0 || !catchVar.isEmpty ()) { // Otherwise an exit scope, ensure that a list of code is executed no matter what happens
		auto jmp_buf_type = validateType (syntax::Var::init (lexing::Word::init (valueLoc, global::CoreNames::get (JMP_BUF_TYPE))));		
		auto ex = ExitScope::init (valueLoc, type, jmp_buf_type, ret, onExit, onFailure, catchVar, catchInfo, catchAction);
		return ex;
	    }
	    return ret;
	}

	void Visitor::validateInnerBlock (const syntax::Block & block, std::vector <Generator> & values, Generator & type, lexing::Word & valueLoc, bool & returner, lexing::Word & rtLoc, bool & breaker, lexing::Word & brLoc, std::list <Error::ErrorMsg> & errors) {
	    // This function simply validates the inner part of a block
	    for (int i = 0 ; i < (int) block.getContent ().size () ; i ++) {
		try {
		    if ((returner || breaker) && !block.getContent ()[i].is <syntax::Unit> ()) { // if there is a expression after a break or a return, then we throw an error			
			Error::occur (block.getContent () [i].getLocation (), ExternalError::UNREACHBLE_STATEMENT);
		    }

		    auto value = validateValue (block.getContent () [i], false, false, false); // the value can't be a type, is not from a call, and we don't check if it is a break statement
		    bool isMutable = value.to <Value> ().getType ().to <Type> ().isMutable (); 
		    if (value.to <Value> ().isReturner ()) { returner = true; rtLoc = value.to<Value> ().getReturnerLocation (); } // store the return location inside the block
		    if (value.to <Value> ().isBreaker ()) { breaker = true; brLoc = value.to<Value> ().getBreakerLocation (); } // store the break location inside the block
		    if (!canImplicitAlias (value)) isMutable = false; // the block is not mutable if we can't implicitely alias the value
		    
		    type = value.to <Value> ().getType (); 
		    type = Type::init (block.getContent() [i].getLocation (), type.to <Type> (), isMutable, false);
		    valueLoc = value.getLocation ();
		    
		    values.push_back (value);		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    try {
		if (!type.is<Void> ()) {
		    verifyMemoryOwner (block.getEnd (), type, values.back(), false); // If the block has a value, we check for implicitely aliases or else
		} else if (type.is<Void> () && values.size () != 0 && !values.back ().is <None> () && isUseless (values.back ())) { 
		    Ymir::Error::occur (block.getContent ().back ().getLocation (), ExternalError::USE_UNIT_FOR_VOID); // TODO, this error does not work
		}
	    } catch (Error::ErrorList list) {  
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	    	    
	}
	
	
	void Visitor::validateCatcher (const syntax::Block & block, Generator & varDecl, Generator & typeInfo, Generator & action, generator::Generator & typeBlock, const std::vector <Generator> & throwsTypes, std::list <Ymir::Error::ErrorMsg> & errors) {
	    auto catcher = block.getCatcher ();
	    if (catcher.isEmpty ()) return; // if there is no catcher to validate, do nothing
	    if (throwsTypes.size () == 0) { // If there is a catcher, but no throwers inside the block
		errors.push_back (Error::makeOccur (catcher.getLocation (), ExternalError::NOTHING_TO_CATCH));
		return;
	    }
	    
	    this-> enterBlock (); // the catcher declares variable that must be inside a block (the var #catch)
	    
	    try {
		auto loc = catcher.getLocation ();
		auto type = Type::init (this-> _cache.exceptionType.getValue ().to <Type> (), false, false);

		varDecl = generator::VarDecl::init (lexing::Word::init (loc, "#catch"), "#catch", type, Generator::empty (), false); // creation of a temp variable
		this-> insertLocal ("#catch", varDecl); 
		typeInfo = this-> validateTypeInfo (loc, type);
		auto vref = VarRef::init (loc, "#catch", type, varDecl.getUniqId (),  false, Generator::empty ()); // Get a ref to that variable

		auto visitor = MatchVisitor::init (*this); // Catchers are validated by a MatchVisitor

		action = visitor.validateCatcher (vref, throwsTypes, catcher.to <syntax::Catch> ());
		    
		if (!action.to <Value> ().isReturner () && !action.to <Value> ().isBreaker ()) { // If the action has a type
		    // We must infer the type from the type of the block, and the type of the catcher
		    typeBlock = this-> inferTypeBranching (action.getLocation (), typeBlock.getLocation (), action.to <Value> ().getType (), typeBlock);
		}
		
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
	    
	    
	    this-> discardAllLocals (); // We discard the var #catch
	    this-> quitBlock (errors.size () == 0, errors);	    
	}

	void Visitor::validateScopes (const syntax::Block & block, std::vector <Generator> & onExit, std::vector <Generator> & onSuccess, std::vector <Generator> & onFailure, bool & returner, bool & breaker, bool hasThrowers, std::list <Error::ErrorMsg> & errors) {	    	    
	    try {
		for (auto & scope_ : block.getScopes ()) { // There are three type of scope gards
		    auto scope = scope_.to <syntax::Scope> ();
		    if (scope.isExit ()) { // Exiters , that can make the block break or return (because they happens all the time)
			onExit.push_back (validateValue (scope.getContent()));
			if (onExit.back ().to <Value> ().isReturner ()) returner = true;
			if (onExit.back ().to <Value> ().isBreaker ()) breaker = true;
		    } else if (scope.isSuccess ()) { // Success
			onSuccess.push_back (validateValue (scope.getContent ()));
		    } else if (scope.isFailure ()) { // Failure
			if (!hasThrowers) { // If there is no throwers inside the block, then there is no reason to fail
			    Ymir::Error::occur (scope.getLocation (), ExternalError::FAILURE_NO_THROW);
			}
			onFailure.push_back (validateValue (scope.getContent ()));
		    } else Ymir::Error::occur (scope.getLocation (), ExternalError::UNDEFINED_SCOPE_GUARD, scope.getLocation ().getStr ());
		    // This last error can't really happen actually, but well it guards the compiler from adding things and forgetting some modifications
		} 
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
	}    


	Symbol Visitor::validateInnerModule (const syntax::Declaration & decl, std::list <Error::ErrorMsg> & errors) {
	    if (decl.isEmpty ()) return Symbol::empty (); // This function is really simple, we just create a module thanks to declarator visitor
	    auto sym = declarator::Visitor::init ().visit (decl, false);
	    if (!sym.isEmpty ()) {
		this-> _referent.back ().insert (sym);
		this-> enterForeign (); // Symbols does not have access to the scope of the block
		try {
		    this-> validate (sym); // and we validate all the declared symbols
		} catch (Error::ErrorList list) {		    
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 		
		this-> exitForeign ();						
	    }
	    	    
	    if (errors.size () != 0) {
		return Symbol::empty ();
	    }
	    return sym;
	}
	
	
	Generator Visitor::validateSet (const syntax::Set & set) {
	    std::vector <Generator> values; // A set is almost a block, but does not enter a block
	    Generator type (Void::init (set.getLocation ()));
	    bool breaker = false, returner = false;
	    // TODO, this function seems really close to validateInnerBlock, maybe they can be merged
	    // This is not done for the moment due to a problem of types (Set, and Block)
	    std::list <Ymir::Error::ErrorMsg> errors;
	    for (int i = 0 ; i < (int) set.getContent ().size () ; i ++) {
		try {
		    if (returner || breaker) {		
			Error::occur (set.getContent () [i].getLocation (), ExternalError::UNREACHBLE_STATEMENT);
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

	    try {
		if (!type.is<Void> ()) {
		    verifyMemoryOwner (set.getLocation (), type, values.back(), false); // If the block has a value, we check for implicitely aliases or else
		} 
	    } catch (Error::ErrorList list) {  
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }

	    return Set::init (set.getLocation (), type, values);
	}

	
    }

}
