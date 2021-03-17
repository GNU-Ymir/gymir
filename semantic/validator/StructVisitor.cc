#include <ymir/semantic/validator/_.hh>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	
	StructVisitor::StructVisitor (Visitor & context) :
	    _context (context)
	{}

	StructVisitor StructVisitor::init (Visitor & context) {
	    return StructVisitor (context);
	}

	Generator StructVisitor::validate (const semantic::Symbol & str) {
	    auto sym = str;
	    
	    if (str.to <semantic::Struct> ().getGenerator ().isEmpty ()) {
		this-> validateStructContent (str);
	    }

	    match (str.to <semantic::Struct> ().getGenerator ()) {
		of_u (generator::Struct) {
		    return StructRef::init (str.getName (), str);
		} elof (ErrorType, err) {
		    if (!this-> _context.isInContext ({PragmaVisitor::PRAGMA_COMPILE_CONTEXT})) {
			// in pragma compile the errors are not printed, so we need to keep them in case of retry
			sym.to <semantic::Struct> ().setGenerator (NoneType::init (str.getName ()));
		    }
		    Ymir::Error::occurAndNote (str.getName (), err.getErrors (), ExternalError::get (VALIDATING), str.getRealName ());
		    return Generator::empty ();
		} elfo {
		    Ymir::Error::occur (str.getName (), ExternalError::get (INCOMPLETE_TYPE_CLASS), str.getRealName ());
		    return Generator::empty ();
		}
	    }
	}

	void StructVisitor::validateStructContent (const semantic::Symbol & str) {
	    auto sym = str;
	    auto gen = generator::Struct::init (sym.getName (), sym);
	    sym.to <semantic::Struct> ().setGenerator (gen); // empty generator to avoid recursive validation
		
	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector <Generator> fieldsDecl;
		
	    this-> _context.pushReferent (sym, "validateStruct"); // we are in the scope of the structure
	    this-> _context.enterForeign (); // but we don't have access to the scope of the thing using the structure
	    this-> _context.enterBlock (); // we enter a block, because we will validate var decls

	    for (auto & it : sym.to<semantic::Struct> ().getFields ()) { // validate the var decl of all fields
		try {
		    auto field = this-> _context.validateVarDeclValue (it.to <syntax::VarDecl> (), false); 
		    if (str.to <semantic::Struct> ().isUnion () && !it.to <syntax::VarDecl> ().getValue ().isEmpty ())
		    errors.push_back (Ymir::Error::makeOccur (it.to <syntax::VarDecl> ().getValue ().getLocation (), ExternalError::get (UNION_INIT_FIELD)));
			
		    fieldsDecl.push_back (field);		    	    		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    }
		
	    this-> _context.discardAllLocals (); // we don't use the fields, that's normal, so we discard them
	    this-> _context.quitBlock (); // no error possible, no need to catch		
		
	    this-> _context.exitForeign ();	       
	    this-> _context.popReferent ("validateStruct");
		
	    if (errors.size () != 0) {
		if (!this-> _context.isInContext ({PragmaVisitor::PRAGMA_COMPILE_CONTEXT})) {
		    // in pragma compile the errors are not printed, so we need to keep them in case of retry
		    sym.to <semantic::Struct> ().setGenerator (NoneType::init (sym.getName ()));
		} else {
		    sym.to <semantic::Struct> ().setGenerator (ErrorType::init (sym.getName (), sym.getRealName (), errors));
		}
		Ymir::Error::occurAndNote (sym.getName (), errors, ExternalError::get (VALIDATING), sym.getRealName ());

	    }
				
	    gen = generator::Struct::init (gen.to <generator::Struct> (), fieldsDecl);
	    this-> verifyRecursivity (sym.getName (), gen, sym, true);		
		
	    sym.to <semantic::Struct> ().setGenerator (gen);
	}
	

	void StructVisitor::verifyRecursivity (const lexing::Word & loc, const generator::Generator & gen, const semantic::Symbol & sym, bool fst) const {
	    match (gen) {
		of (StructRef, str_ref) {
		    if (str_ref.isRefOf (sym) && !fst) {
			auto note = Ymir::Error::createNote (sym.getName ());
			Ymir::Error::occurAndNote (loc, note, ExternalError::get (NO_SIZE_FORWARD_REF));
		    } else {
			auto & str = str_ref.getRef ().to <semantic::Struct> ().getGenerator ();
			for (auto & it : str.to<generator::Struct> ().getFields ()) {
			    this-> verifyRecursivity (loc, it.to <generator::VarDecl> ().getVarType (), sym, false);
			}
		    }
		}
		elof_u (Pointer) {} // No forward problem on pointer types		    
		elof_u (Slice) {} // No problem for slice, their size can be 0			 
		elof (Type, t) {
		    if (t.isComplex ()) {
			for (auto & it : t.getInners ()) this-> verifyRecursivity (loc, it, sym, false);
		    }
		} fo;
	    }
	}

    }    

}
