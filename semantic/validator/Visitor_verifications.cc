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
	
	void Visitor::verifyMemoryOwner (const lexing::Word & loc, const Generator & type, const Generator & gen, bool construct, bool checkTypes, bool inMatch) {
	    if (checkTypes) {
		verifyCompatibleTypeWithValue (loc, type, gen);
	    }


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
		    s_of_u (TupleValue) return;
		    s_of_u (OptionValue) return;
		}
	    }
	    {
		// Ok for implicit alias, but the mutability must be checked
		match (gen) {
		    of (Block, arr) {			
			if (!arr.isLvalue ()) {// Implicit alias of LBlock is not allowed
			    max_level = arr.getType ().to <Type> ().mutabilityLevel ();
			}
		    }		    	       
		    elof (Conditional,  arr) max_level = arr.getType ().to <Type> ().mutabilityLevel ();
		    elof (ExitScope,    arr) max_level = arr.getType ().to <Type> ().mutabilityLevel ();
		    elof (SuccessScope, arr) max_level = arr.getType ().to <Type> ().mutabilityLevel ();
		    elof (Call,         arr) max_level = arr.getType ().to <Type> ().mutabilityLevel ();	    
		    fo;
		}
	    }
	    

	    // int min_level = 1;
	    // if (type.is <StructRef> ()) min_level = 0;
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
	

	void Visitor::verifyClassImpl (const lexing::Word & loc, const Generator & cl, const syntax::Expression & trait) {
	    auto type = this-> validateTypeClassContext (loc, cl, trait);
	    verifyClassImpl (loc, cl, type);
	}
	
	bool Visitor::verifyClassImpl (const lexing::Word & loc, const Generator & cl, const Generator & trait, bool thr) {
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
			    
			    if (errors.size () != 0) {
				if (thr) throw Error::ErrorList {errors};
				else return false;
			    }
			    if (succeed) return true;
			} fo;
		    }		
		}
		
		auto ancestor = sym.to <semantic::Class> ().getGenerator ().to <generator::Class> ().getClassRef ().to <ClassRef> ().getAncestor ();
		if (!ancestor.isEmpty ()) {
		    sym = ancestor.to <ClassRef> ().getRef ();
		}
		else break;
	    }
	    
	    if (thr) {
		auto note = Ymir::Error::createNote (loc);
		Ymir::Error::occurAndNote (cl.getLocation (), note, ExternalError::get (NOT_IMPL_TRAIT), cl.prettyString (), trait.prettyString ());
	    }
	    
	    return false;
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
		auto objectType = this-> _cache.objectType.getValue ();
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


    }
}
