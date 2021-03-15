#include <ymir/semantic/validator/ClassVisitor.hh>
#include <ymir/semantic/generator/value/_.hh>
#include <ymir/errors/_.hh>
#include <ymir/utils/Path.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/semantic/validator/PragmaVisitor.hh>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	
	ClassVisitor::ClassVisitor (Visitor & context) :
	    _context (context)
	{}
	
	ClassVisitor ClassVisitor::init (Visitor & context) {
	    return ClassVisitor (context);
	}
	
	generator::Generator ClassVisitor::validate (const semantic::Symbol & cls, bool inModule) {
	    auto sym = cls; // some cheating on c++ const, to store the generator inside the class symbol
	    // and avoid validating multiple times the same class
	    
	    if (cls.to <semantic::Class> ().getGenerator ().isEmpty () || inModule) {
		std::list <Error::ErrorMsg> errors;
		auto ancestor = this-> validateAncestor (cls);
		auto gen = generator::Class::init (cls.getName (), sym, ClassRef::init (cls.getName (), ancestor, sym));
		// To avoid recursive validation 
		sym.to <semantic::Class> ().setGenerator (gen);
		
		this-> validateCtes (cls); // this throws if an error occur anyway,
		// we don't wan't to validate the class if an assertion failed

		// The validation of the vtable, places the vtable inside the symbol 
		this-> validateVtable (cls, ancestor, errors);

		// The validation of the field of the class
		this-> validateFields (cls, ancestor, errors);

		if (errors.size () != 0) { // we caught the errors, to display the fields and vtable errors at the same time
		    if (!this-> _context.isInContext ({PragmaVisitor::PRAGMA_COMPILE_CONTEXT})) {  // in pragma compile the errors are not printed, so we need to keep them in case of retry
			sym.to <semantic::Class> ().setGenerator (NoneType::init (cls.getName ()));
		    } else {
			sym.to <semantic::Class> ().setGenerator (ErrorType::init (cls.getName (), cls.getRealName (), errors));
		    }
		    Ymir::Error::occurAndNote (cls.getName (), errors, ExternalError::get (VALIDATING), cls.getRealName ());
		}

		if (inModule) { // if we are in the module that declared the class, then we have to validate the inner symbols
		    this-> validateInnerClass (cls, errors);
		
		    if (errors.size () != 0) { // caught the error to add a note
			if (!this-> _context.isInContext ({PragmaVisitor::PRAGMA_COMPILE_CONTEXT})) { 
			    sym.to <semantic::Class> ().setGenerator (NoneType::init (cls.getName ()));
			} else {
			    sym.to <semantic::Class> ().setGenerator (ErrorType::init (cls.getName (), cls.getRealName (), errors));
			}
			Ymir::Error::occurAndNote (cls.getName (), errors, ExternalError::get (VALIDATING), cls.getRealName ());
		    }
		    
		    this-> _context.insertNewGenerator (cls.to <semantic::Class> ().getGenerator ());
		    
		}
	    }

	    match (cls.to <semantic::Class> ().getGenerator ()) {
		of (generator::Class, cl) {
		    return cl.getClassRef ();
		} elof (ErrorType, err) {
		    // if we are still in a pragma compile, we don't wan't to lose the errors
		    if (!this-> _context.isInContext ({PragmaVisitor::PRAGMA_COMPILE_CONTEXT})) {  
			sym.to <semantic::Class> ().setGenerator (NoneType::init (cls.getName ()));
		    }
		    Ymir::Error::occurAndNote (cls.getName (), err.getErrors (), ExternalError::get (VALIDATING), cls.getRealName ());
		    return Generator::empty (); 
		} elfo {
		    Ymir::Error::occur (cls.getName (), ExternalError::get (INCOMPLETE_TYPE_CLASS), cls.getRealName ());
		    return Generator::empty ();
		}
	    }
	}

	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================     FIELDS     ===============================
	 * ================================================================================
	 * ================================================================================
	 */


	void ClassVisitor::validateFields (const semantic::Symbol & cls, const Generator & ancestor, std::list <Error::ErrorMsg> & errors) {
	    auto sym = cls;
	    auto gen = cls.to <semantic::Class> ().getGenerator ();
	    try {
		// Validate the local fields 
		auto localFields = std::move (this-> validateLocalFields (cls, errors));
		
		std::vector <Generator> allFields;
		if (!ancestor.isEmpty ()) {
		    auto ancestorFields = ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <generator::Class> ().getFields ();
		    this-> verifyCollisions (cls, localFields, ancestorFields);

		    allFields = ancestorFields;
		    allFields.insert (allFields.end (), localFields.begin (), localFields.end ());		    
		} else allFields = localFields;
		

		gen = generator::Class::initFields (gen.to <generator::Class> (), allFields, localFields);
		sym.to <semantic::Class> ().setGenerator (gen);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
	}    


	std::vector <generator::Generator> ClassVisitor::validateLocalFields (const semantic::Symbol & cls, std::list <Error::ErrorMsg> & errors) {
	    std::vector <generator::Generator> syms;
	    		
	    this-> _context.pushReferent (cls, "validateClass");
	    this-> _context.enterForeign ();
		
	    try {
		this-> _context.enterBlock (); // we enter a block, to avoid a crash of the visitor
		// It needs a block (context) to validate vardecls
		    
		std::vector <std::string> fields;
		std::vector <generator::Generator> types;
		for (auto & it : cls.to<semantic::Class> ().getFields ()) {
		    syms.push_back (this-> _context.validateVarDeclValue (it.to <syntax::VarDecl> (), false));
		}

		// we don't want an error when quitting the block
		// it is normal, that none of the declarations are unused
		this-> _context.discardAllLocals (); 
		    
		this-> _context.quitBlock ();
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 

	    this-> _context.exitForeign ();			
	    this-> _context.popReferent ("validateClass");
	    
	    if (errors.size () != 0) {
		return {};
	    } else return  syms;
	}	


	void ClassVisitor::verifyCollisions (const semantic::Symbol & cls, const std::vector <Generator> & locals, const std::vector <Generator> & ancetors) {
	    for (auto & it : locals) { // Verify shadowing
		for (auto & jt : ancetors) {
		    if (it.to <generator::VarDecl> ().getName () == jt.to <generator::VarDecl> ().getName ()) {
			auto note = Ymir::Error::createNote (it.getLocation ());
			Error::occurAndNote (jt.getLocation (), note, ExternalError::get (SHADOWING_DECL), it.to <generator::VarDecl> ().getName ());
		    }
		}
	    }
	}

	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================     ASSERT     ===============================
	 * ================================================================================
	 * ================================================================================
	 */
	
	void ClassVisitor::validateCtes (const semantic::Symbol & cls) { 
	    if (cls.to <semantic::Class> ().getAssertions ().size () != 0) {
		std::list <Error::ErrorMsg> errors;
		auto sym = cls; // we need a symbol, no just a class for referent push
		this-> _context.pushReferent (sym, "validateClassAssertions");
		this-> _context.enterForeign ();
		    
		for (auto & it : cls.to <semantic::Class> ().getAssertions ()) {
		    try {
			this-> _context.validateCteValue (it);
		    } catch (Error::ErrorList & list) {
			errors = std::move (list.errors);
		    }
		}
		    
		this-> _context.exitForeign ();
		this-> _context.popReferent ("validateClassAssertions");
		
		if (!errors.empty ()) {
		    throw Error::ErrorList {errors};
		}
	    }
	}

	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================     VTABLE     ===============================
	 * ================================================================================
	 * ================================================================================
	 */
	
	void ClassVisitor::validateVtable (const semantic::Symbol & cls, const Generator & ancestor, std::list <Error::ErrorMsg> & errors) {
	    auto sym = cls;
	    auto gen = cls.to <semantic::Class> ().getGenerator ();
	    try {
		std::vector <Symbol> addMethods;
		std::vector <generator::Class::MethodProtection> protections;
		
		// Create the vtable from the class declarations, and ancestor
		auto vtable = this-> validateClassDeclarations (sym, ClassRef::init (cls.getName (), ancestor, sym), ancestor, protections, addMethods);
		
		// for the moment the __dtor is never set, but everything is prepared for it to appear
		auto nulltype = Pointer::init (gen.getLocation (), Void::init (gen.getLocation ()));

		// Create a generator with a vtable from the template of the generator without vtable
		gen = generator::Class::initVtable (gen.to <generator::Class> (), vtable, protections, NullValue::init (gen.getLocation (), nulltype));
			
		sym.to <semantic::Class> ().setGenerator (gen);
		sym.to <semantic::Class> ().setTypeInfo (this-> _context.validateTypeInfo (gen.getLocation (), ClassRef::init (cls.getName (), ancestor, sym)));
			
		// Add methods is the list of methods that have been added by trait implementation
		sym.to <semantic::Class> ().setAddMethods (addMethods); // We don't put them in the table of the symbol, because they are not declared in it

	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
	}

       
	std::vector<generator::Generator> ClassVisitor::validateClassDeclarations (const semantic::Symbol & cls, const Generator & classType, const Generator & ancestor, std::vector <generator::Class::MethodProtection>  & protection, std::vector <Symbol> & addMethods)  {
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
		    
		    addMethods.push_back (Symbol::empty ()); // we add empty symbol, for the addMethods vec to be the right size
		}
	    }

	    // Create a vector with all implementations
	    std::vector <std::pair<semantic::Symbol, generator::Generator> > implemented;	    
	    for (auto it : cls.to <semantic::Class> ().getAllInner ()) {
		try {
		    match (it) {
			of (semantic::Impl, im) {
			    // Add the method defined in the traits inside the vtable
			    auto ret = this-> validateVtableTrait (im, classType, ancestor, vtable, protection, ancVtable, addMethods);			    
			    implemented.push_back (std::pair<semantic::Symbol, generator::Generator> (it, ret));
			} fo;
		    }		    		
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    // Validate the vtable of all the methods declared immediately inside the class (not inside an impl)
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

	    // For all the implementations
	    for (auto it : implemented) {
		try {
		    this-> validateVtableImpl (it.first.to <semantic::Impl> (), it.second, classType, ancestor, vtable, protection, implVtable, addMethods);
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 	
		
	    }

	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }

	    this-> verifyCollisionVtable (cls, classType, vtable);
	    

	    /** Save only the symbol that are not empty */
	    auto aux = std::move (addMethods);
	    addMethods = {};
	    for (auto & it : aux)
		if (!it.isEmpty ()) addMethods.push_back (it);
		
	    return vtable;
	}

	void ClassVisitor::verifyCollisionVtable (const semantic::Symbol & cls, const generator::Generator & classType, const std::vector <Generator> & vtable) {
	    for (auto i : Ymir::r (0, vtable.size ())) {
		for (auto j : Ymir::r (0, vtable.size ())) {
		    // Verification of the collision (since all reimplemented function are marked override)
		    if (i != j && Ymir::Path (vtable[i].to <FrameProto> ().getName (), "::").fileName ().toString () ==
			Ymir::Path (vtable [j].to <FrameProto> ().getName (), "::").fileName ().toString ()) {
			auto fptr = this-> _context.validateFunctionType  (vtable [i]);
			auto protoFptr = this-> _context.validateFunctionType  (vtable [j]);
			if (protoFptr.equals (fptr) && vtable [i].to<MethodProto> ().isMutable () == vtable [j].to<MethodProto> ().isMutable ()) {
			    auto note = Ymir::Error::createNote (vtable [i].getLocation ());
			    Ymir::Error::occurAndNote (vtable [j].getLocation (), note, ExternalError::get (CONFLICTING_DECLARATIONS), vtable [i].prettyString ());
			}
		    }
		}
	    }
	    
	       
	    if (!cls.to <semantic::Class> ().isAbs ()) {
		for (auto & it : vtable) {
		    if (it.to <MethodProto> ().isEmptyFrame ()) {
			auto note = Ymir::Error::createNote (it.getLocation ());
			auto loc = classType.getLocation ();
			Ymir::Error::occurAndNote (loc, note, ExternalError::get (NOT_ABSTRACT_NO_OVER), classType.prettyString (), it.prettyString ());
		    }
		}
	    }
	}

	void ClassVisitor::validateVtableImpl (const semantic::Impl & impl, const generator::Generator & trait, const Generator & classType, const Generator & ancestor, std::vector <Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <Generator> & implVtable, std::vector<Symbol> & addMethods) {
	    for (auto jt : impl.getAllInner ()) { // For all the overriden method of the impl
		if (jt.is <semantic::Function> ()) {
		    auto & func = jt.to <semantic::Function> ();
		    auto index = this-> validateVtableMethod (func, classType, ancestor, vtable, protection, implVtable, trait);
		    if (func.getContent ().getBody ().isEmpty ()) {
			Ymir::Error::occur (vtable [index].getLocation (), ExternalError::get (NO_BODY_METHOD), vtable [index].prettyString ());
		    }
					
		    // Definition of a new method in implement is forbidden
		    if (index >= (int) addMethods.size ()) { // the index is returned by vtablemethod, if the index is > as addMethods, then is must mean that the symbol is not in the vtable
			std::list <Ymir::Error::ErrorMsg> names;
			for (auto & ft : trait.to <TraitRef> ().getRef ().to <semantic::Trait> ().getAllInner ()) { // It is necessarily a trait, we verified that earlier
			    if (ft.getName ().getStr () == func.getName ().getStr () && ft.is <semantic::Function> ()) {
				auto proto = this-> _context.validateMethodProto (ft.to <semantic::Function> (), classType, trait);
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
	}

    
	generator::Generator ClassVisitor::validateVtableTrait (const semantic::Impl & impl, const Generator & classType, const Generator & ancestor, std::vector <Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <Generator> & ancVtable, std::vector<Symbol> & addMethods) {
	    auto trait = this-> _context.validateType (impl.getTrait ());
	    if (!trait.is <TraitRef> ()) {
		Ymir::Error::occur (impl.getTrait ().getLocation (), ExternalError::get (IMPL_NO_TRAIT), trait.prettyString ());
	    }

	    std::list <Ymir::Error::ErrorMsg> errors;
	    for (auto it : trait.to <TraitRef> ().getRef ().to <semantic::Trait> ().getAllInner ()) {
		this-> _context.pushReferent (it, "vtableImplement");
		try {
		    if (it.is <semantic::Template> ()) {
			Ymir::Error::occur (it.getName (), ExternalError::get (TEMPLATE_IN_TRAIT));
		    } else if (it.is <semantic::VarDecl> ()) {
			Ymir::Error::occur (it.getName (), ExternalError::get (VAR_DECL_IN_TRAIT));
		    } else if (it.is <semantic::Function> ()) {
			auto & func = it.to <semantic::Function> ();
			int index = this-> validateVtableTraitMethod (func, classType, ancestor, vtable, protection, ancVtable, trait);
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
		this-> _context.popReferent ("vtableImplement");
	    }
	    
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    return trait;
	}

	int ClassVisitor::validateVtableTraitMethod (const semantic::Function & func, const Generator & classType, const Generator &, std::vector <Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <Generator> & ancVtable, const generator::Generator & trait) {
	    auto proto = this-> _context.validateMethodProto (func, classType, trait);
	    auto protoFptr = this-> _context.validateFunctionType  (proto);
	    for (auto i : Ymir::r (0, ancVtable.size ())) {
		if (Ymir::Path (ancVtable[i].to <FrameProto> ().getName (), "::").fileName ().toString () == func.getName ().getStr ()) {		    
		    auto fptr = this-> _context.validateFunctionType  (ancVtable [i]);
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
	
	int ClassVisitor::validateVtableMethod (const semantic::Function & func, const Generator & classType, const Generator &, std::vector <Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <Generator> & ancVtable, const Generator & trait) {	    
	    auto proto = this-> _context.validateMethodProto (func, classType, trait);	    
	    auto protoFptr = this-> _context.validateFunctionType  (proto);
	    for (auto i : Ymir::r (0, ancVtable.size ())) {
		if (Ymir::Path (ancVtable[i].to <FrameProto> ().getName (), "::").fileName ().toString () == func.getName ().getStr ()) {		    
		    auto fptr = this-> _context.validateFunctionType  (ancVtable [i]);
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
				this-> _context.verifyCompatibleType (func.getName (), ancVtable [i].to <MethodProto> ().getTrait ().getLocation (), trait, ancVtable [i].to <MethodProto> ().getTrait ());
			    } catch (Error::ErrorList list) {
				auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
				Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (WRONG_IMPLEMENT), ancVtable [i].prettyString (), ancVtable [i].to<MethodProto> ().getTrait ().prettyString (), trait.prettyString ());
			    }
			}				   

			std::vector <Generator> unused, notfound;
			this-> _context.verifyThrows (proto.getThrowers (), ancVtable[i].getThrowers (), unused, notfound);
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
		    auto fptr = this-> _context.validateFunctionType  (vtable [i]);
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
	
	/**
	 * ================================================================================
	 * ================================================================================
	 * ================================      INNER      ===============================
	 * ================================================================================
	 * ================================================================================
	 */


	void ClassVisitor::validateInnerClass (const semantic::Symbol & cls, std::list <Ymir::Error::ErrorMsg> & errors) {
	    auto & clRef = cls.to <semantic::Class> ().getGenerator ().to <generator::Class> ().getClassRef ();
	    auto & ancestor = clRef.to <ClassRef> ().getAncestor ();
	    auto & addMethods = cls.to <semantic::Class> ().getAddMethods ();
	    
	    std::vector <Generator> ancestorFields;
	    if (!ancestor.isEmpty ())
		ancestorFields = ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <generator::Class> ().getFields ();
	    
	    auto allInners = cls.to <semantic::Class> ().getAllInner ();
	    allInners.insert (allInners.end (), addMethods.begin (), addMethods.end ());

	    for (auto & it : allInners) {
		this-> _context.pushReferent (it, "validate::innerClass");
		try {
		    match (it) {
			of (semantic::Function, func) {
			    if (!func.getContent ().getBody ().isEmpty ()) {				
				this-> validateMethod (func, clRef, cls.isWeak ()); // We need to pass weak here
				// The method could have been imported from a trait that is not weak
			    }
			}
			elof_u (semantic::Constructor) {
			    this-> validateConstructor (it, clRef, ancestor, ancestorFields);
			} fo;			
		    }
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 

		this-> _context.popReferent ("validate::innerClass");			
	    }
	    	    
	}
	
	void ClassVisitor::validateConstructor (const semantic::Symbol & sym, const Generator & classType_, const Generator & ancestor, const std::vector <Generator> & ancestorFields) {
	    auto & cs = sym.to <Constructor> ();
	    auto constr = cs.getContent ();
	    std::vector <Generator> params;
	    Generator retType (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto classType = classType_;
	    
	    auto proto = this-> _context.validateConstructorProto (sym);
	    this-> verifyConstructionLoop (proto.getLocation (), proto);

	    auto currentClassDef = classType_.to <ClassRef> ().getRef ();
	    this-> _context.enterClassDef (currentClassDef);
	    this-> _context.enterContext (cs.getCustomAttributes ());
	    
	    classType = Type::init (proto.getLocation (), ClassPtr::init (proto.getLocation (), Type::init (proto.getLocation (), classType.to <Type> (), true, false)).to <Type> (), true, false);
	    this-> _context.enterForeign ();

	    std::vector <Generator> throwers;
	    for (auto &it : constr.getThrowers ()) {
		try {
		    throwers.push_back (Generator::init (it.getLocation (), this-> _context.validateType (it)));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    this-> _context.enterBlock ();
	    try {
		this-> _context.validatePrototypeForFrame (cs.getName (), constr.getPrototype (), params, retType);
		retType = classType.to <ClassPtr> ().getInners ()[0].to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <Value> ().getType ();
		params.insert (params.begin (), ParamVar::init (cs.getName (), classType, true, true));
		this-> _context.insertLocal (params [0].getName (), params [0]);
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    } 

	    Generator body (Generator::empty ());
	    {
		try {
		    auto preConstruct = this-> validatePreConstructor (cs, classType_, ancestor, ancestorFields);
		    this-> _context.setCurrentFuncType (retType);
		    body = this-> _context.validateValue (constr.getBody ());
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
		this-> _context.verifyThrows (body.getThrowers (), throwers, unused, notfound);		    		    
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
		    this-> _context.quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    this-> _context.exitForeign ();
	    this-> _context.exitContext ();
	    this-> _context.exitClassDef (currentClassDef);
	    
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    
	    auto frame = Frame::init (constr.getLocation (), cs.getRealName (), params, classType, body, false);
	    frame.to <Frame> ().setMangledName (cs.getMangledName ());
	    frame.to <Frame> ().isWeak (cs.isWeak ());
	    this-> _context.insertNewGenerator (frame);
	}

	void ClassVisitor::validateMethod (const semantic::Function & func, const Generator & classType_, bool isWeak) {
	    auto function = func.getContent ();
	    std::vector <Generator> params;
	    Generator retType (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    auto classType = classType_;
	    auto & cs = classType.to <ClassRef> ().getRef ().to <semantic::Class> ();
	    auto currentClassDef = classType.to <ClassRef> ().getRef ();
	    this-> _context.enterClassDef (currentClassDef);
	    this-> _context.enterContext (function.getCustomAttributes ());
	    
	    classType = Type::init (function.getLocation (), ClassPtr::init (function.getLocation (), classType).to <Type> ().toDeeplyMutable ().to <Type> (), true, false);
	    this-> _context.enterForeign ();

	    std::vector <Generator> throwers;
	    for (auto &it : func.getThrowers ()) {
		try {
		    throwers.push_back (Generator::init (it.getLocation (), this-> _context.validateType (it)));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }	    
	    
	    this-> _context.enterBlock ();
	    
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
		this-> _context.insertLocal (params [0].getName (), params [0]);		

		auto fakeParams = std::vector <syntax::Expression> (__params.begin () + 1, __params.end ());
		auto proto = syntax::Function::Prototype::init (fakeParams, function.getPrototype ().getType (), false);
		
		this-> _context.validatePrototypeForFrame (cs.getName (), proto, params, retType);
		if (retType.isEmpty ()) retType = Void::init (func.getName ());
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    } 
	    
	    bool needFinalReturn = false;
	    Generator body (Generator::empty ());
	    if (errors.size () == 0) 
		{
		    try {
			this-> _context.setCurrentFuncType (retType);
			body = this-> _context.validateValue (function.getBody ());
		
			if (!body.to<Value> ().isReturner ()) {
			    this-> _context.verifyMemoryOwner (body.getLocation (), retType, body, true);		    
			    needFinalReturn = !retType.is<Void> ();
			}
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 
		}


	    if (!body.isEmpty ()) {
		std::vector <Generator> unused, notfound;		    
		this-> _context.verifyThrows (body.getThrowers (), throwers, unused, notfound);		    		    
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
			this-> _context.discardAllLocals ();
		    
		    this-> _context.quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    this-> _context.exitForeign ();
	    this-> _context.exitContext ();
	    this-> _context.exitClassDef (currentClassDef);

	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
		
	    auto frame = Frame::init (function.getLocation (), func.getRealName (), params, retType, body, needFinalReturn);
	    frame.to <Frame> ().isWeak (func.isWeak () || isWeak);
	    frame.to <Frame> ().setMangledName (func.getMangledName ());

	    this-> _context.insertNewGenerator (frame);		
	}

	generator::Generator ClassVisitor::validatePreConstructor (const semantic::Constructor & cs, const Generator & classType, const Generator & ancestor, const std::vector<Generator> & ancestorFields) {
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
		    cstrs = this-> _context.getClassConstructors (loc, classR.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator (), lexing::Word::eof ());
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
		    auto result = this-> _context.validateValue (call);
		    instructions.push_back (ClassCst::init (result.to <ClassCst> (), this-> _context.validateValue (syntax::Var::init (lexing::Word::init (loc, Keys::SELF)))));
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
			cstrs = this-> _context.getClassConstructors (loc, ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator (), lexing::Word::eof ());
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
			auto result = this-> _context.validateValue (call);
			instructions.push_back (ClassCst::init (result.to <ClassCst> (), this-> _context.validateValue (syntax::Var::init (lexing::Word::init (loc, Keys::SELF)))));
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
		    
			auto left = this-> _context.validateValue (access);
			auto right = this-> _context.validateValue (it.second);
			this-> _context.verifyMemoryOwner (left.getLocation (), left.to <Value> ().getType (), right, true);
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
			    auto left = this-> _context.validateValue (access);
			    auto right = this-> _context.validateValue (it.to <syntax::VarDecl> ().getValue ());
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

	void ClassVisitor::verifyConstructionLoop (const lexing::Word & location, const Generator & call) {
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

	    this-> _context.pushReferent (sym, "verifyConstructionLoop");
	    protos.push_back (sym);
	    gen_protos.push_back (current_proto);
	    locs.push_back (location);
	    
	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector <Generator> params;
	    Generator retType (Generator::empty ());

	    auto currentClassDef = clRef.to <ClassRef> ().getRef ();
	    this-> _context.enterClassDef (currentClassDef);
	    this-> _context.enterForeign ();
	    this-> _context.enterBlock ();

	    {
		try {
		    this-> _context.validatePrototypeForFrame (cs.getName (), cs.getContent ().getPrototype (), params, retType);
		    retType = clRef.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <Value> ().getType ();
		    params.insert (params.begin (), ParamVar::init (cs.getName (), clRef, true, true));
		    this-> _context.insertLocal (params [0].getName (), params [0]);
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
			    cstrs = this-> _context.getClassConstructors (loc, ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator (), lexing::Word::eof ());
			} catch (Error::ErrorList list) {
			    errors = list.errors;
			    return;
			}
			
			if (!cstrs.isEmpty ()) {			    
			    auto superBin = TemplateSyntaxWrapper::init (loc, cstrs);				      			    
			    auto call = syntax::MultOperator::init (lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR), superBin, superParams);			    
			    auto result = this-> _context.validateValue (call);
			    this-> verifyConstructionLoop (loc, result);
			}
		    }

		    for (auto & it : cs.getContent ().getFieldConstruction ()) {
			auto right = this-> _context.validateValue (it.second);
			if (right.to <Value> ().getType ().is <ClassPtr> ()) {
			    locs.back () = it.second.getLocation ();
			    this-> verifyConstructionLoop (it.second.getLocation (), right);
			    validated.emplace (it.first.getStr ());
			}
		    }

		    for (auto & it : clRef.to<ClassRef> ().getRef ().to <semantic::Class> ().getFields ()) {
			if (validated.find (it.to<syntax::VarDecl> ().getName ().getStr ()) == validated.end ()) {
			    if (!it.to <syntax::VarDecl> ().getValue ().isEmpty ()) {
				auto right = this-> _context.validateValue (it.to <syntax::VarDecl> ().getValue ());
				if (right.to <Value> ().getType ().is <ClassPtr> ()) {
				    auto loc = it.to <syntax::VarDecl> ().getValue ().getLocation ();
				    locs.back () = loc;
				    this-> verifyConstructionLoop (loc, right);
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
		    this-> _context.discardAllLocals ();
		    this-> _context.quitBlock ();
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    this-> _context.exitForeign ();
	    this-> _context.exitClassDef (currentClassDef);	    	    
	    protos.pop_back ();
	    gen_protos.pop_back ();
	    locs.pop_back ();	    
	    this-> _context.popReferent ("verifyConstructionLoop");

	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	}	

	
	/**
	 * ================================================================================
	 * ================================================================================
	 * ================================    ANCESTOR     ===============================
	 * ================================================================================
	 * ================================================================================
	 */

	generator::Generator ClassVisitor::validateAncestor (const semantic::Symbol & cls) {
	    if (!cls.to <semantic::Class> ().getAncestor ().isEmpty ()) {
		auto ancestor = this-> _context.validateValue (cls.to <semantic::Class> ().getAncestor (), true, false);
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
		
		return ancestor;
	    } return Generator::empty ();
	}
	

    }    

}
