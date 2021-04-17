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
	    } catch (Error::ErrorList  list) {
		if (!canBeType || fromValidateType) throw list;
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

		s_of (syntax::TemplateCall, cl) 
		    return validateTemplateCall (cl);

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

	       
	Generator Visitor::retreiveValue (const Generator & gen) {
	    auto compile_time = CompileTime::init (*this);
	    return compile_time.execute (gen);
	    // println (gen, " => ", ret);
	    // return ret;
	}

	
    }

}
