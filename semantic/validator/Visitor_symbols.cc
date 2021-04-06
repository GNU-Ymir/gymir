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

	void Visitor::validateModule (const semantic::Module & mod) {
	    std::list <Error::ErrorMsg> errors;
	    if (!mod.isExtern ()) {
		const std::vector <Symbol> & syms = mod.getAllLocal ();
		for (auto & it : syms) { // a module is a just a list of symbol
		    try {
			validate (it); // we simple validate all the symbol inside the module
		    } catch (Error::ErrorList lst) {
			errors.insert (errors.end (), lst.errors.begin (), lst.errors.end ());
		    }
		}
	    }
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	}
	
	void Visitor::validateFunction (const semantic::Function & func) {
	    auto visitor = FunctionVisitor::init (*this); // there is a specific validator for functions
	    return visitor.validate (func);
	}


	void Visitor::validateVarDecl (const semantic::Symbol & sym) { // this global var decls are close to vardecl value, but insert a generator 
	    if (sym.to <semantic::VarDecl> ().getGenerator ().isEmpty ()) {
		auto var = sym.to <semantic::VarDecl> ();
		auto elemSym = sym; // c++ cheating on mutability
		
		Generator type (Generator::empty ()), value (Generator::empty ());
	    
		if (!var.getType ().isEmpty ()) { // validate the type of the expression, if specified in the source code
		    type = this-> validateType (var.getType ());
		}

		// validate the value, if specified in the source code
		if (!var.getValue ().isEmpty ()) { // The value can't be of type void, 
		    value = this-> validateValueNonVoid (var.getValue ());		
		}

		// if no value, then the variable cannot be validated, it must have a default value
		// In ymir, we wan't to ensure that everything is initialised
		if (var.getValue ().isEmpty () && !var.isExtern ()) { 
		    Error::occur (var.getName (), ExternalError::get (VAR_DECL_WITHOUT_VALUE));
		} 
		
		if (type.isEmpty ()) { // If the type is empty, then we have at least a value, and we take the type of the value
		    type = Type::init (value.to <Value> ().getType ().to<Type> (), false, false);
		}

		bool isMutable = false, isRef = false, dmut = false;;
		type = this-> applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut, false);
		if (!isMutable) type = Type::init (type.to <Type> (), false);

		if (!value.isEmpty ()) {
		    if (var.isExtern ()) {// can't have a value on external, there are just reference 
			Ymir::Error::occur (value.getLocation (), ExternalError::get (EXTERNAL_VAR_WITH_VALUE), var.getRealName ());
		    }
		    
		    if (!type.is <LambdaType> ()) {// if the var has a real type (not a lambda type), we have to check the mutability 
			this-> verifyMemoryOwner (var.getName (), type, value, true);
		    }
		}
		
		auto glbVar = GlobalVar::init (var.getName (), var.getRealName (), var.getExternalLanguage (), isMutable, type, value);		
		elemSym.to<semantic::VarDecl> ().setGenerator (glbVar);	 // we store the variable inside the global, to avoid revalidation	
		
		insertNewGenerator (glbVar);	// we insert the generator, to generate something in the generation phase	
	    }
	}


	generator::Generator Visitor::validateAka (const semantic::Symbol & sym) { 
	    if (sym.to <semantic::Aka> ().getGenerator ().isEmpty ()) { // We don't want to validate multiple times the same symbol
		auto aka = sym.to <semantic::Aka> (); 
		auto elemSym = sym; // cheating on c++ mutability

		Generator elem (Generator::empty ());
		try {
		    elem = validateValue (aka.getValue (), true);
		} catch (Error::ErrorList &list) {
		    elem = Generator::empty ();
		} 
		
		if (elem.isEmpty ())
		elem = validateType (aka.getValue (), true);
		
		if (elem.is <Value> ()) {
		    auto type = Type::init (elem.to <Value> ().getType ().to <Type> (), false, false);
		    elem = Value::init (elem.to <Value> (), type);
		}
		
		elemSym.to <semantic::Aka> ().setGenerator (elem);
	    }

	    return sym.to <semantic::Aka> ().getGenerator ();
	}

	generator::Generator Visitor::validateStruct (const semantic::Symbol & str) {	    
	    auto visitor = StructVisitor::init (*this);
	    return visitor.validate (str);
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
		    } catch (Error::ErrorList &list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }
		}
		    
		exitForeign ();
		popReferent ("validateClassAssertions");
	    }

	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};
	}

	
	generator::Generator Visitor::validateClass (const semantic::Symbol & cls, bool inModule) {	    
	    auto visitor = ClassVisitor::init (*this);
	    return visitor.validate (cls, inModule);
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
			} catch (Error::ErrorList &list) {
			    auto note = Ymir::Error::createNote (it.getLocation ());
			    for (auto it : list.errors) 
				note.addNote (it);
			    errors.push_back (note);
			}
		    }
		    
		} catch (Error::ErrorList &list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 

		{
		    try {
			syms = this-> discardAllLocals ();
			this-> quitBlock (errors.size () == 0);
		    } catch (Error::ErrorList &list) {
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
	
    }
}

