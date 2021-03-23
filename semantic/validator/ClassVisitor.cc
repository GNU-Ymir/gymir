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

	std::list <bool> ClassVisitor::__fast_validation__;
	
	ClassVisitor::ClassVisitor (Visitor & context) :
	    _context (context),
	    _funcVisitor (FunctionVisitor::init (context))
	{}
	
	ClassVisitor ClassVisitor::init (Visitor & context) {
	    return ClassVisitor (context);
	}
	
	generator::Generator ClassVisitor::validate (const semantic::Symbol & cls, bool inModule) {
	    auto sym = cls; // some cheating on c++ const, to store the generator inside the class symbol
	    // and avoid validating multiple times the same class
	    
	    auto validated = !cls.to <semantic::Class> ().getGenerator ().isEmpty ();
	    if (validated && cls.to <semantic::Class> ().getGenerator ().is <generator::Class> ()) {
		validated = !cls.to <semantic::Class> ().getGenerator ().to <generator::Class> ().getClassRef ().to <generator::ClassRef> ().isFast ();
	    }
	    if (!validated || inModule) {
		this-> validateClassContent (cls, inModule);
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

	void ClassVisitor::validateClassContent (const semantic::Symbol & cls, bool inModule) {
	    auto sym = cls; // some cheating on c++ const, to store the generator inside the class symbol
	    // and avoid validating multiple times the same class
	    
	    std::list <Error::ErrorMsg> errors;
	    if (__fast_validation__.empty () || !__fast_validation__.back () || inModule) { // if inModule but __fast_validation__, then the class must be a template
		auto ancestor = this-> validateAncestor (cls);
		auto gen = generator::Class::init (cls.getName (), sym, ClassRef::init (cls.getName (), ancestor, sym));
		// To avoid recursive validation 
		sym.to <semantic::Class> ().setGenerator (gen);
		__fast_validation__.push_back (false);
		    
		this-> validateCtes (cls); // this throws if an error occur anyway,
		// we don't wan't to validate the class if an assertion failed

		// The validation of the vtable, places the vtable inside the symbol 
		this-> validateVtable (cls, ancestor, errors);

		// The validation of the field of the class
		this-> validateFields (cls, ancestor, inModule, errors);

		__fast_validation__.pop_back ();
		    
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
	    } else {
		auto gen = generator::Class::init (cls.getName (), sym, ClassRef::init (cls.getName (), Generator::empty (), sym, true));
		// To avoid recursive validation 
		sym.to <semantic::Class> ().setGenerator (gen);
	    }
	}
	

	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================     FIELDS     ===============================
	 * ================================================================================
	 * ================================================================================
	 */


	void ClassVisitor::validateFields (const semantic::Symbol & cls, const Generator & ancestor, bool inModule, std::list <Error::ErrorMsg> & errors) {
	    auto sym = cls;
	    auto gen = cls.to <semantic::Class> ().getGenerator ();
	    try {
		// Validate the local fields 
		auto localFields = std::move (this-> validateLocalFields (cls, inModule, errors));
		
		std::vector <Generator> allFields;
		if (!ancestor.isEmpty ()) {
		    auto ancestorFields = ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <generator::Class> ().getFields ();
		    this-> verifyCollisions (cls, localFields, ancestorFields);

		    allFields = ancestorFields;
		    allFields.insert (allFields.end (), localFields.begin (), localFields.end ());		    
		} else allFields = localFields;
		

		gen = generator::Class::initFields (gen.to <generator::Class> (), allFields, localFields);
		sym.to <semantic::Class> ().setGenerator (gen);
	    } catch (Error::ErrorList &list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
	}    


	std::vector <generator::Generator> ClassVisitor::validateLocalFields (const semantic::Symbol & cls, bool inModule, std::list <Error::ErrorMsg> & errors) {
	    std::vector <generator::Generator> syms;
	    		
	    this-> _context.pushReferent (cls, "validateClass");
	    this-> _context.enterForeign ();
		
	    this-> _context.enterBlock (); // we enter a block, to avoid a crash of the visitor
	    // It needs a block (context) to validate vardecls
		    
	    std::vector <std::string> fields;
	    std::vector <generator::Generator> types;
	    for (auto & it : cls.to<semantic::Class> ().getFields ()) {
		try {
		    auto fast = !inModule;
		    auto prv = cls.to <semantic::Class> ().isMarkedPrivate (it.to <syntax::VarDecl> ().getName ().getStr ());
		    prv = prv || (cls.to <semantic::Class> ().isFinal () && cls.to <semantic::Class> ().isMarkedProtected (it.to <syntax::VarDecl> ().getName ().getStr ()));

		    if (fast && prv) {
			ClassVisitor::__fast_validation__.push_back (true);
			try {
			    syms.push_back (this-> _context.validateVarDeclValue (it.to <syntax::VarDecl> (), false));
			} catch (Error::ErrorList &list) {
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			}
			ClassVisitor::__fast_validation__.pop_back ();
		    } else {
			syms.push_back (this-> _context.validateVarDeclValue (it.to <syntax::VarDecl> (), false));
		    }
		} catch (Error::ErrorList &list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}		    
	    }

	    try {
		// we don't want an error when quitting the block
		// it is normal, that none of the declarations are used
		this-> _context.discardAllLocals (); 
		    
		this-> _context.quitBlock ();
	    } catch (Error::ErrorList &list) {
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

	    } catch (Error::ErrorList &list) {
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
		} catch (Error::ErrorList &list) {
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
		} catch (Error::ErrorList &list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    // For all the implementations
	    for (auto it : implemented) {
		try {
		    this-> validateVtableImpl (it.first.to <semantic::Impl> (), it.second, classType, ancestor, vtable, protection, implVtable, addMethods);
		} catch (Error::ErrorList &list) {
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
			auto fptr = this-> _funcVisitor.validateFunctionType  (vtable [i]);
			auto protoFptr = this-> _funcVisitor.validateFunctionType  (vtable [j]);
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
				auto proto = this-> _funcVisitor.validateMethodProto (ft.to <semantic::Function> (), classType, trait);
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
		} catch (Error::ErrorList &list) {		    
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
	    auto proto = this-> _funcVisitor.validateMethodProto (func, classType, trait);
	    auto protoFptr = this-> _funcVisitor.validateFunctionType  (proto);
	    for (auto i : Ymir::r (0, ancVtable.size ())) {
		if (Ymir::Path (ancVtable[i].to <FrameProto> ().getName (), "::").fileName ().toString () == func.getName ().getStr ()) {		    
		    auto fptr = this-> _funcVisitor.validateFunctionType  (ancVtable [i]);
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
	    auto proto = this-> _funcVisitor.validateMethodProto (func, classType, trait);	    
	    auto protoFptr = this-> _funcVisitor.validateFunctionType  (proto);
	    for (auto i : Ymir::r (0, ancVtable.size ())) {
		if (Ymir::Path (ancVtable[i].to <FrameProto> ().getName (), "::").fileName ().toString () == func.getName ().getStr ()) {		    
		    auto fptr = this-> _funcVisitor.validateFunctionType  (ancVtable [i]);
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
			    } catch (Error::ErrorList &list) {
				auto note = Ymir::Error::createNote (ancVtable [i].getLocation ());
				Ymir::Error::occurAndNote (func.getName (), note, ExternalError::get (WRONG_IMPLEMENT), ancVtable [i].prettyString (), ancVtable [i].to<MethodProto> ().getTrait ().prettyString (), trait.prettyString ());
			    }
			}				   

			std::vector <Generator> unused, notfound;
			this-> _funcVisitor.computeThrows (proto.getThrowers (), ancVtable[i].getThrowers (), unused, notfound);
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
		    auto fptr = this-> _funcVisitor.validateFunctionType  (vtable [i]);
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
				this-> _funcVisitor.validateMethod (func, clRef, cls.isWeak ()); // We need to pass weak here
				// The method could have been imported from a trait that is not weak
			    }
			}
			elof_u (semantic::Constructor) {
			    this-> _funcVisitor.validateConstructor (it, clRef, ancestor, ancestorFields);
			} fo;			
		    }
		} catch (Error::ErrorList &list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 

		this-> _context.popReferent ("validate::innerClass");			
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
