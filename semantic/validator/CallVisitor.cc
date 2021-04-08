#include <ymir/semantic/validator/CallVisitor.hh>
#include <ymir/semantic/validator/TemplateVisitor.hh>
#include <ymir/global/State.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <chrono>
#include <time.h>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	using namespace syntax;

	CallVisitor::CallVisitor (Visitor & context) :
	    _context (context)
	{}

	CallVisitor CallVisitor::init (Visitor & context) {
	    return CallVisitor {context};
	}

	generator::Generator CallVisitor::validate (const syntax::MultOperator & expression) {
	    Generator left (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    try { // First we try to validate the left operand
		left = this-> _context.validateValue (expression.getLeft (), false, true);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }

	    std::vector <Generator> rights;
	    if (left.isEmpty () && expression.canBeDotCall ()) { // if the validation failed, then maybe it is a dot call
		left = this-> validateDotCall (expression.getLeft (), rights, errors); // so we try a dot call validation
	    } else if (left.isEmpty ()) {
		this-> error (expression.getLocation (), expression.getEnd (), left, rights, errors);
	    }

	    for (auto & it : expression.getRights ()) { // Validation of the rights operands
		auto val = this-> _context.validateValue (it);
		if (val.is<List> ()) { // in case of expand
		    for (auto & g_it : val.to <List> ().getParameters ())
		    rights.push_back (g_it);
		} else {
		    rights.push_back (val);
		}
	    }
	    
	    int score = 0;
	    errors = {}; // clear the errors about left, if there was any, if we are here, then we at least validated a dot call
	    // Perform the validation
	    auto ret = validate (expression.getLocation (), left, rights, score, errors);
	    
	    if (ret.isEmpty ()) {
		this-> error (expression.getLocation (), expression.getEnd (), left, rights, errors);
		return Generator::empty ();
	    } else {
		return ret;
	    }
	}

	generator::Generator CallVisitor::validate (const lexing::Word & location , const generator::Generator & left, const std::vector <generator::Generator> & rights, int & score, std::list <Ymir::Error::ErrorMsg> & errors) {
	    bool checked = true;
	    Generator gen (Generator::empty ());
	    match (left) {
		of_u (FrameProto) {
		    gen = validateFrameProto (location, left, rights, score, errors);
		} elof_u (ConstructorProto) {
		    gen = validateConstructorProto (location, left, rights, score, errors);
		} elof_u (LambdaProto) {
		    gen = validateLambdaProto (location, left, rights, score, errors);
		} elof (VarRef, vref) {
		    if (left.to <Value> ().getType ().is <LambdaType> ()) gen = validateLambdaProto (location, vref.getValue (), rights, score, errors);
		    else if (vref.getValue ().is <LambdaProto> ()) gen = validateLambdaProto (location, vref.getValue (), rights, score, errors);
		    else checked = false;
		} elof (generator::Struct, str) {
		    gen = validateStructCst (location, str, rights, score, errors);
		} elof (MultSym, msym) {
		    gen = validateMultSym (location, msym, rights, score, errors);
		} elof_u (generator::TemplateRef) {
		    Symbol sym (Symbol::empty ());
		    Generator proto_gen (Generator::empty ());
		    gen = validateTemplate (location, left, rights, score, errors, sym, proto_gen);
		} elfo {
		    checked = false;
		}
		
		if (gen.isEmpty ()) {
		    s_of (Value, v) {
			if (v.getType ().is <FuncPtr> ()) gen = validateFunctionPointer (location, left, rights, score, errors);
			else if (v.getType ().is <Delegate> ()) gen = validateDelegate (location, left, rights, score, errors);
		    }
		}
	    }
	    
	    if (!gen.isEmpty ()) {
		return gen;
	    }
 	    
	    if (!left.is<MultSym> () && checked) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), realLocation (left), prettyName (left));
		for (auto & it : errors) {
		    note.addNote (it);
		}
		errors = {note};
	    }
	    return Generator::empty ();
	}

	generator::Generator CallVisitor::validateMultSym (const lexing::Word & location, const MultSym & sym, const std::vector <Generator> & rights_, int & score, std::list <Ymir::Error::ErrorMsg> & errors) {
	    Generator final_gen (Generator::empty ());	 // The generator inside which a call generator will be given
	    Generator used_gen (Generator::empty ()); // The prototype used by the call generator
	    Generator proto_gen (Generator::empty ()); // The proto generator created by a template specialization
	    Symbol templSym (Symbol::empty ()); // The symbol created by a template specialization
	    bool fromTempl = true; // If this bool is true, the used gen is a template generator
	    std::map <int, std::vector <Generator>> nonTemplScores; // the list of score on non templates symbols
	    std::map <int, std::vector <Symbol>> templScores; // The list of score on template symbols

	    // Compute the score of the different symbol inside the multsym
	    this-> computeScoreOfMultSym (location, sym, rights_, score, nonTemplScores, templScores, fromTempl, final_gen, used_gen, templSym, proto_gen, errors);

	    // Verify that there is only one possibility
	    this-> validateScoreOfMultSym (location, sym, rights_, score, nonTemplScores, templScores, fromTempl, final_gen, used_gen, templSym, proto_gen, errors);

	    // If final gen is empty, the errors are already thrown in validateScoreOfMultSym anyway, we can't reach this point then
	    return final_gen;
	}

	void CallVisitor::computeScoreOfMultSym (const lexing::Word & location, const generator::MultSym & sym,  const std::vector <Generator> & rights_, int & score, std::map <int, std::vector <Generator>> & nonTemplScores, std::map <int, std::vector <Symbol>> & templScores, bool & fromTempl, generator::Generator & final_gen, generator::Generator & used_gen, Symbol & templSym, generator::Generator & proto_gen, std::list <Ymir::Error::ErrorMsg> & errors) {
	    fromTempl = true;
	    score = -1;
	    for (auto & it : sym.getGenerators ()) {
		int current = 0;
		std::list <Ymir::Error::ErrorMsg> local_errors;
		Symbol _sym (Symbol::empty ());
		Generator _proto_gen (Generator::empty ());
		Generator gen (Generator::empty ());
		
		if (it.is <TemplateRef> () && fromTempl) { // If this is a template we need to store the symbol created and the prototype
		    gen = validateTemplate (location, it, rights_, current, local_errors, _sym, _proto_gen);
		} else { // Otherwise it will be simply return a call expression, and that's enough
		    gen = validate (location, it, rights_, current, local_errors);
		}
		
		if (!gen.isEmpty ()) {
		    if (it.is <TemplateRef> ()) { // If the validation was from a template
			templScores [current].push_back (_sym);
			if (current > score && fromTempl) { // If the score is better than the last one, and the last one was also a template
			    score = current;
			    final_gen = gen;
			    used_gen = it;
			    proto_gen = _proto_gen;
			    fromTempl = true;
			    templSym = _sym;
			}
		    } else {
			nonTemplScores [current].push_back (gen);
			// Non template symbol always have priority on template symbols
			if (current > score || fromTempl) { // If the score is better than the last one, or the last one was a template
			    score = current;
			    final_gen = gen;
			    used_gen = it;
			    fromTempl = false;
			}
		    }
		    
		} else {
		    if (it.is <TemplateRef> ()) { // If the validation was on a template, the note about candidate was not added
			auto note = Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.to <TemplateRef> ().getTemplateRef ().getName (), it.prettyString ());
			for (auto & it : local_errors) {
			    note.addNote (it);
			}
			
			errors.push_back (note);
		    } else { // in the method validation we already added the note
			errors.insert (errors.end (), local_errors.begin (), local_errors.end ());
		    }
		}
	    }

	}


	void CallVisitor::validateScoreOfMultSym (const lexing::Word & location, const generator::MultSym & sym,  const std::vector <Generator> & rights_, int & score, std::map <int, std::vector <Generator>> & nonTemplScores, std::map <int, std::vector <Symbol>> & templScores, bool & fromTempl, generator::Generator & final_gen, generator::Generator & used_gen, Symbol & templSym, generator::Generator & proto_gen, std::list <Ymir::Error::ErrorMsg> & errors) {
	    if (!templSym.isEmpty () && fromTempl) { // if the validated symbol is a template
		auto element_on_scores = templScores.find (score); // unfortunately, we can't merge this block and the block for non template, due to type problems (symbol and generator)
		// Maybe with a template function, but this is working anyway
		if (element_on_scores-> second.size () != 1) { // If there is more than one symbol with the highest score
		    std::string leftName = sym.prettyString ();
		    std::vector<std::string> names;
		    for (auto & it : rights_) {
			names.push_back (prettyName (it));
		    }
			
		    std::list <Ymir::Error::ErrorMsg> notes;
		    for (auto & it : element_on_scores-> second) {
			auto gen = this-> _context.validateMultSym (sym.getLocation (), {it});
			notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), realLocation (gen), prettyName (gen)));
		    }
		    Ymir::Error::occurAndNote (location,
					       notes,
					       ExternalError::get (SPECIALISATION_WORK_WITH_BOTH),
					       leftName,
					       names);

		}
		
		std::list <Ymir::Error::ErrorMsg> local_errors;
		try { // Template symbol must be validated to be available at symbol linking time
		    Visitor::__CALL_NB_RECURS__ += 1;
		    this-> _context.validateTemplateSymbol (templSym, used_gen);
		} catch (Error::ErrorList list) {
		    // The errors of template validation can be quite huge, so in non verbose mode we print only some of them
		    // TODO, Maybe that's not a good idea, and that can be managed directly inside the error system, when an error has more than 3 sub errors
		    // Yes, it would probably be better
		    errors = this-> computeLastErrorList (location, list, proto_gen, used_gen);
		    final_gen = Generator::empty ();
		}
		
		Visitor::__CALL_NB_RECURS__ -= 1;
		
	    } else if (!final_gen.isEmpty ()) { // almost the same, as with template, if there is more than one valid symbol, throw an error
		auto element_on_scores = nonTemplScores.find (score);
		if (element_on_scores-> second.size () != 1) {
		    std::string leftName = sym.prettyString ();
		    std::vector<std::string> names;
		    for (auto & it : rights_)
		    names.push_back (prettyName (it));
			
		    std::list <Ymir::Error::ErrorMsg> notes;
		    for (auto & it : element_on_scores-> second) {
			notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), realLocation (it), prettyName (it)));
		    }
		    Ymir::Error::occurAndNote (location,
					       notes,
					       ExternalError::get (SPECIALISATION_WORK_WITH_BOTH),
					       leftName,
					       names);

		}
	    }

	}

	std::list <Ymir::Error::ErrorMsg> CallVisitor::computeLastErrorList (const lexing::Word & location, Error::ErrorList & list, const Generator & proto_gen, const Generator & used_gen) {
	    auto note = Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), used_gen.to <TemplateRef> ().getTemplateRef ().getName (), used_gen.prettyString ());
	    for (auto & it : list.errors) {
		note.addNote (it);
	    }
	    
	    list.errors = {note};
	    list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", proto_gen.getLocation (), proto_gen.prettyString ()));
	    list.errors.insert (list.errors.begin (), Ymir::Error::createNote (location, ExternalError::get (IN_TEMPLATE_DEF)));

	    return list.errors;
	}
	
	
	
	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================      PROTOS     ==============================
	 * ================================================================================
	 * ================================================================================
	 */
	
	generator::Generator CallVisitor::validateFrameProto (const lexing::Word & location, const Generator & proto, const std::vector <Generator> & rights_, int & score, std::list <Ymir::Error::ErrorMsg> & errors) {
	    score = 0;
	    auto & fProto = proto.to <FrameProto> ();
	    std::vector <Generator> addParams;
	    std::vector <Generator> rights = rights_;
	    auto list = std::list <Generator> (rights.begin (), rights.end ());
	    
	    // Compute the list of parameters
	    std::vector <Generator> params = this-> validateParameterList (fProto.getParameters (), list, errors);

	    if (errors.size () != 0) return Generator::empty ();
	    if (list.size () != 0 && !fProto.isCVariadic ()) return Generator::empty ();
	    if (params.size () < fProto.getParameters ().size ()) return Generator::empty ();
	    addParams = std::vector <Generator> (list.begin (), list.end ());
	    
	    // Compute the list of types, while verifying that the types are compatible, and the memory is correctly borrowed
	    // FrameProtos are not lambda, all the type of the parameters are known
	    auto types = this-> validateTypeParameterList (proto, fProto.getParameters (), params, score, true, true, false, false, errors);
	    
	    // If everything succeded, then the call is valid
	    if (errors.size () != 0) return Generator::empty ();
	    return Call::init (location, fProto.getReturnType (), proto, types, params, addParams);
	}

	generator::Generator CallVisitor::validateConstructorProto (const lexing::Word & location, const Generator & proto, const std::vector <Generator> & rights_, int & score, std::list <Ymir::Error::ErrorMsg> & errors) {
	    score = 0; // Calling a constructor prototype is pretty similar to calling a function
	    auto & cProto = proto.to <ConstructorProto> ();
	    std::vector <Generator> rights = rights_;
	    auto list = std::list <Generator> (rights.begin (), rights.end ());
	    std::vector <Generator> params = this-> validateParameterList (proto.to <ConstructorProto> ().getParameters (), list, errors);

	    if (errors.size () != 0) return Generator::empty ();
	    if (list.size () != 0) return Generator::empty ();
	    if (params.size () < cProto.getParameters ().size ()) return Generator::empty ();
	    // Unlike functions, we cannot have C variadic parameters in a constructor
	    // ConstructorProtos are not lambda, all the type of the parameters are known
	    std::vector <Generator> types = this-> validateTypeParameterList (proto, cProto.getParameters (), params, score, true, true, false, false, errors);
	    
	    if (errors.size () != 0) return Generator::empty ();
	    // And we return a ClassConstructor instead of a simple call for generation
	    return ClassCst::init (location, cProto.getReturnType (), proto, types, params);
	}

	generator::Generator CallVisitor::validateLambdaProto (const lexing::Word & location, const Generator & proto, const std::vector <Generator> & rights, int & score, std::list <Ymir::Error::ErrorMsg> & errors) {
	    auto & lProto = proto.to <LambdaProto> ();
	    if (rights.size () != lProto.getParameters ().size ()) return Generator::empty ();

	    score = 0;
	    
	    // LambdaProtos are lambda, some of the parameters can have an unknown type
	    auto types = this-> validateTypeParameterList (proto, lProto.getParameters (), rights, score, true, true, true, false, errors);
	    if (errors.size () != 0) return Generator::empty ();
	    
	    try {
		// Validate the prototype with the computed types
		auto gen = this-> _context.validateLambdaProto (lProto, types);
		match (gen) {
		    of (Addresser, addr) { // Simple proto, no closure
			return Call::init (location, addr.getWho ().to<FrameProto> ().getReturnType (), addr.getWho (), types, rights, {});
		    } elof (DelegateValue, dl) { // Delegate value with a closure
			return Call::init (location, dl.getFuncPtr ().to <Addresser> ().getWho ().to <FrameProto> ().getReturnType (), gen, types, rights, {});
		    } fo;
		} // There cannot be other things returned by context.validateLambdaProto
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		return Generator::empty ();
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }
	    
	    return Generator::empty ();
	}


	/**
	 * ================================================================================
	 * ================================================================================
	 * ==================================     STRUCT     ==============================
	 * ================================================================================
	 * ================================================================================
	 */

	generator::Generator CallVisitor::validateStructCst (const lexing::Word & location, const generator::Struct & str, const std::vector <Generator> & rights_, int & score, std::list <Ymir::Error::ErrorMsg> & errors) {
	    if (str.getRef ().to <semantic::Struct> ().isUnion ()) {
		return this-> validateUnionCst (location, str, rights_, score, errors);
	    }

	    std::vector <Generator> params;
	    std::vector <Generator> rights = rights_;
	    std::vector <Generator> types;
		
	    for (auto it : str.getFields ()) {
		auto param = findParameterStruct (rights, it.to<generator::VarDecl> ());
		if (param.isEmpty ()) return Generator::empty ();
		params.push_back (param);
	    }
		
	    if (rights.size () != 0) return Generator::empty ();
	    for (auto it : Ymir::r (0, str.getFields ().size ())) {
		try {
		    this-> _context.verifyMemoryOwner (
			params [it].getLocation (),
			str.getFields () [it].to <generator::VarDecl> ().getVarType (),
			params [it],
			true
			);
		    types.push_back (str.getFields () [it].to <generator::VarDecl> ().getVarType ());
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    }
	    

	    if (errors.size () != 0) return Generator::empty ();
	    
	    score = 0;
	    return StructCst::init (location, StructRef::init (str.getLocation (), str.getRef ()), str.clone (), types, params);
	}

	Generator CallVisitor::validateUnionCst (const lexing::Word & location, const generator::Struct & str, const std::vector <Generator> & rights, int & score, std::list <Ymir::Error::ErrorMsg> & errors) {
	    lexing::Word name = lexing::Word::eof ();
	    std::vector <Generator> params;
	    std::vector <Generator> types;
	    try {
		if (rights.size () != 1) {
		    Ymir::Error::occur (location, ExternalError::get (UNION_CST_MULT));
		}

		if (rights [0].is <NamedGenerator> ()) { // If it is a named gen, we get the field with the same name
		    name = rights [0].to <NamedGenerator> ().getLocation ();
		    auto param = rights [0].to <NamedGenerator> ().getContent ();
		    for (auto it : str.getFields ()) {
			if (it.to <generator::VarDecl> ().getLocation ().getStr () == name.getStr ()) {
			    this-> _context.verifyMemoryOwner (
				name,
				it.to <generator::VarDecl> ().getVarType (),
				param,
				true
				);
				
			    types.push_back (it.to <generator::VarDecl> ().getVarType ());
			    params.push_back (param);
			    break;
			}
		    }
		} // No else, we must name the field we want to construct
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }
	    
	    if (params.size () == 0) return Generator::empty ();
	    if (errors.size () != 0) return Generator::empty ();
	    
	    score = 0;
	    return UnionCst::init (location, StructRef::init (str.getLocation (), str.getRef ()), str.clone (), name.getStr (), types[0], params[0]);
	}

	
	/**
	 * ================================================================================
	 * ================================================================================
	 * ================================     POINTERS     ==============================
	 * ================================================================================
	 * ================================================================================
	 */

	generator::Generator CallVisitor::validateFunctionPointer (const lexing::Word & location, const Generator & gen, const std::vector <Generator> & rights, int & score, std::list <Ymir::Error::ErrorMsg> & errors) {
	    score = 0;
	    auto funcType = gen.to <Value> ().getType ().to <FuncPtr> ();
	    if (rights.size () != funcType.getParamTypes ().size ()) return Generator::empty ();

	    // We must verify the types, the ownership, it is not a lambda, and the prototype contains only types
	    auto types = this-> validateTypeParameterList (gen, funcType.getParamTypes (), rights, score, true, true, false, true, errors);
	    
	    if (errors.size () != 0) return Generator::empty ();
	    return Call::init (location,
			       funcType.getReturnType (),
			       gen,
			       types,
			       rights,
			       {}
		);
	}

	generator::Generator CallVisitor::validateDelegate (const lexing::Word & location, const Generator & gen, const std::vector <Generator> & rights, int & score, std::list <Ymir::Error::ErrorMsg> & errors) {
	    score = 0;
	    if (gen.to <Value> ().getType ().to <Type> ().getInners ()[0].is <FuncPtr> ()) { // If this is just a function pointer, then there is no first parameter to pass
		auto & funcType = gen.to <Value> ().getType ().to <Type> ().getInners ()[0].to <FuncPtr> ();
		auto retType = funcType.getReturnType ();
		if (rights.size () != funcType.getParamTypes ().size ()) return Generator::empty ();
		
		auto types = this-> validateTypeParameterList (gen, funcType.getParamTypes (), rights, score, true, true, false, true, errors);
		
		if (errors.size () != 0) return Generator::empty ();
		return Call::init (location, retType, gen, types, rights, {});
	    } else {
		auto list = std::list <Generator> (rights.begin (), rights.end ());
		
		auto proto = gen.to <Value> ().getType ().to <Type> ().getInners ()[0];
		auto & fProto = proto.to <FrameProto> ();
		
		try {
		    if (proto.is <MethodProto> ()) {
			auto meth = proto.to <MethodProto> ();
			Generator type (Generator::empty ());
			if (meth.isMutable ())
			type = meth.getClassType ().to<Type> ().toDeeplyMutable ();
			else type = Type::init (meth.getClassType ().to <Type> (), meth.isMutable ());
			
			this-> _context.verifyImplicitAlias (location, type, gen.to <DelegateValue> ().getClosure ());
			this-> _context.verifyMemoryOwner   (location, type, gen.to <DelegateValue> ().getClosure (), true);
			auto llevel = gen.to <DelegateValue> ().getClosure ().to <Value> ().getType ().to <Type> ().mutabilityLevel ();
			auto rlevel = type.to <Type> ().mutabilityLevel () + 1;
			score += rlevel - llevel;
		    }

		} catch (Error::ErrorList list) {
		    errors = std::move (list.errors);
		}
		
		auto params = this-> validateParameterList (fProto.getParameters (), list, errors);
		if (list.size () != 0 || params.size () != fProto.getParameters ().size ()) return Generator::empty ();
		if (errors.size () != 0) return Generator::empty ();
		
		auto types = this-> validateTypeParameterList (gen, fProto.getParameters (), params, score, true, true, false, false, errors);
		
		if (errors.size () != 0) return Generator::empty ();
		
		auto retType = fProto.getReturnType ();
		return Call::init (location, retType, gen, types, params, {});
	    }
	}

	
	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================    TEMPLATES    ==============================
	 * ================================================================================
	 * ================================================================================
	 */

	generator::Generator CallVisitor::validateTemplate (const lexing::Word & location, const Generator & ref, const std::vector <generator::Generator> & rights, int & score, std::list <Ymir::Error::ErrorMsg> & errors, Symbol & sym, Generator & proto_gen) {
	    Generator gen (Generator::empty ());
	    if (ref.is <TemplateClassCst> ()) {
		gen = validateTemplateClassCst (location, ref, rights, score, errors, sym, proto_gen);
	    } else {
		gen = validateTemplateRef (location, ref, rights, score, errors, sym, proto_gen);
	    }
		
	    if (!gen.isEmpty ()) {
		std::list <Ymir::Error::ErrorMsg> local_errors;
		try {
		    Visitor::__CALL_NB_RECURS__ += 1;
		    this-> _context.validateTemplateSymbol (sym, ref);
		} catch (Error::ErrorList list) {
		    list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", proto_gen.getLocation (), proto_gen.prettyString ()));
		    list.errors.insert (list.errors.begin (), Ymir::Error::createNote (location, ExternalError::get (IN_TEMPLATE_DEF)));
		    errors = std::move (list.errors);
		    gen = Generator::empty ();
		}
		Visitor::__CALL_NB_RECURS__ -= 1;
	    }
	    
	    return gen;
	}
	
	generator::Generator CallVisitor::validateTemplateRef (const lexing::Word & location, const Generator & ref, const std::vector <generator::Generator> & rights_, int & score, std::list <Ymir::Error::ErrorMsg> & errors, Symbol & _sym, Generator & proto_gen) {
	    const Symbol & sym = ref.to <TemplateRef> ().getTemplateRef ();
	    if (!sym.to<semantic::Template> ().getDeclaration ().is <syntax::Function> ()) return Generator::empty ();
	    std::vector <Generator> typeParams;
	    std::vector <Generator> valueParams;
	    std::vector <Generator> rights = rights_;

	    if (ref.is <MethodTemplateRef> ()) {
		rights.insert (rights.begin (), ref.to <MethodTemplateRef> ().getSelf ());
	    }
	    
	    auto list = std::list <Generator> (rights.begin (), rights.end ());
	    
	    for (auto & it : sym.to <semantic::Template> ().getDeclaration ().to <syntax::Function> ().getPrototype ().getParameters ()) {
		Generator value (Generator::empty ());
		auto var = it.to<syntax::VarDecl> ();
		bool failure = false;
		this-> _context.enterForeign ();		    // We enter a foreign, the value of a parameter var has no local context
		try {
		    this-> _context.enterBlock (); // it has no context but it need a block to be validated
		    if (!var.getValue ().isEmpty ()) {
			try {
			    value = this-> _context.validateValue (var.getValue ());
			} catch (Error::ErrorList err) {
			    // If it failed it can be a template, that will be validated later, so we add something
			    // To tell the templateVisitor, that there is something, but we can't use it for the moment
			    value = None::init (var.getValue ().getLocation ());
			}
		    }
		    this-> _context.quitBlock (true);

		    auto type = Void::init (var.getName ());
		    bool isMutable = false;
		    bool isRef = false;
		    bool dmut = false;
		    type = this-> _context.applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut);
		    auto param = findParameter (list, ProtoVar::init (var.getName (), Generator::empty (), value, isMutable, 1, false).to<ProtoVar> ());

		    if (param.isEmpty ()) {
			failure = true;
		    } else {
			typeParams.push_back (param.to <Value> ().getType ());
			valueParams.push_back (param);
		    }
		} catch (Error::ErrorList list) {
		    errors.insert (errors.begin (), list.errors.begin (), list.errors.end ());
		    failure = true;
		}
		
		this-> _context.exitForeign (); // exiting the context to return to the context of the local frame
		if (failure) return Generator::empty ();
		
	    }

	    for (auto & it : list) { // Add the rests, for variadic templates
		typeParams.push_back (it.to <Value> ().getType ());
		valueParams.push_back (it);
	    }

	    auto templateVisitor = TemplateVisitor::init (this-> _context);
	    std::vector <Generator> finalParams;
	    bool succeed = true;

	    try {
		// The solution is a function transformed generated by template specialisation (if it succeed)
		proto_gen = templateVisitor.validateFromImplicit (ref, valueParams, typeParams, score, _sym, finalParams);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.begin (), list.errors.begin (), list.errors.end ());
		succeed = false;
	    }


	    if (succeed) {
		int _score;
		Generator ret (Generator::empty ());
		finalParams = rights_;
		if (ref.is <MethodTemplateRef> ()) {
		    auto self = ref.to <MethodTemplateRef> ().getSelf ();
		    auto delType = Delegate::init (proto_gen.getLocation (), proto_gen);
		    auto delValue = DelegateValue::init (proto_gen.getLocation(),
							 delType, proto_gen.to <MethodProto> ().getClassType (),
							 self, proto_gen);
		    
		    ret = validateDelegate (location, delValue, finalParams, _score, errors);
		} else {
		    ret = validateFrameProto (location, proto_gen, finalParams, _score, errors);
		}
		score += _score;

		return ret;
	    }
	    
	    return Generator::empty ();
	}

	generator::Generator CallVisitor::validateTemplateClassCst (const lexing::Word & loc, const Generator & ref, const std::vector <generator::Generator> & rights_, int & score, std::list <Ymir::Error::ErrorMsg> & errors, Symbol & _sym, Generator & proto_gen) {
	    for (auto & it : ref.to <TemplateClassCst> ().getPrototypes ()) {
		int local_score = 0;
		auto gen = validateTemplateClassCst (loc, ref, it, rights_, local_score, errors, _sym);
		if (!gen.isEmpty ()) {
		    if (gen.is <ClassRef> ()) gen = ClassPtr::init (loc, gen);
		    auto bin = syntax::Binary::init (lexing::Word::init (loc, Token::DCOLON),
						     TemplateSyntaxWrapper::init (loc, gen),
						     Var::init (lexing::Word::init (loc, ClassRef::INIT_NAME)),
						     Expression::empty ());
		    
		    std::vector <syntax::Expression> params;
		    for (auto & it : rights_)
		    params.push_back (TemplateSyntaxWrapper::init (it.getLocation (), it));
					  
		    auto expr = syntax::MultOperator::init (lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
							    bin,
							    params, false
			);
		    
		    score = local_score;
		    bool succeed = true;
		    try {
			proto_gen = this-> _context.validateValue (expr);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			succeed = false;
		    }
		    
		    if (succeed) errors = {};
		    return proto_gen;
		}
	    }
	    
	    _sym = Symbol::empty ();
	    proto_gen = Generator::empty ();
	    score = 0;
	    return Generator::empty ();
	}
	
	generator::Generator CallVisitor::validateTemplateClassCst (const lexing::Word &, const Generator & ref, const syntax::Function::Prototype & prototype, const std::vector <generator::Generator> & rights_, int & score, std::list <Ymir::Error::ErrorMsg> & errors, Symbol & _sym) {
	    std::vector <Generator> typeParams;
	    std::vector <Generator> valueParams;
	    std::vector <Generator> rights = rights_;
	    auto list = std::list <Generator> (rights.begin (), rights.end ());
	    
	    for (auto & it : prototype.getParameters ()) {
		Generator value (Generator::empty ());
		auto var = it.to<syntax::VarDecl> ();
		bool failure = false;
		this-> _context.enterForeign ();		    // We enter a foreign, the value of a parameter var has no local context
		try {
		    this-> _context.enterBlock (); // it has no context but it need a block to be validated
		    if (!var.getValue ().isEmpty ()) {
			try {
			    value = this-> _context.validateValue (var.getValue ());
			} catch (Error::ErrorList err) {
			    value = None::init (var.getValue ().getLocation ());
			}
		    }
		    this-> _context.quitBlock (true);

		    auto type = Void::init (var.getName ());
		    bool isMutable = false;
		    bool isRef = false;
		    bool dmut = false;
		    type = this-> _context.applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut);
		    
		    auto param = findParameter (list, ProtoVar::init (var.getName (), Generator::empty (), value, isMutable, 1, false).to<ProtoVar> ());
		    if (param.isEmpty ())
		    failure = true;
		    else {
			typeParams.push_back (param.to <Value> ().getType ());
			valueParams.push_back (param);
		    }
		} catch (Error::ErrorList list) {
		    errors.insert (errors.begin (), list.errors.begin (), list.errors.end ());
		    failure = true;
		}
				
		this-> _context.exitForeign (); // exiting the context to return to the context of the local frame
		if (failure) return Generator::empty ();
	    }

	    for (auto & it : list) { // Add the rests, for variadic templates
		typeParams.push_back (it.to <Value> ().getType ());
		valueParams.push_back (it);
	    }
	    
	    auto templateVisitor = TemplateVisitor::init (this-> _context);
	    std::vector <Generator> finalParams;
	    Generator clGen (Generator::empty ());
	    bool succeed = true;
	    try {
		// The solution is a function transformed generated by template specialisation (if it succeed)
		clGen = templateVisitor.validateFromImplicit (ref, valueParams, typeParams, prototype.getParameters (), score, _sym, finalParams);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.begin (), list.errors.begin (), list.errors.end ());
		succeed = false;
	    }

	    if (succeed) return clGen;
	    else return Generator::empty ();
	}


	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================    ARGUMENTS    ==============================
	 * ================================================================================
	 * ================================================================================
	 */

	
	std::vector <Generator> CallVisitor::validateParameterList (const std::vector <Generator> & proto, std::list <Generator> & list, std::list<Error::ErrorMsg> & errors) {
	    std::vector <Generator> params;
	    try {
		for (auto it : Ymir::r (0, proto.size ())) { /// for all parameter of the function, we try to find the associated argument
		    auto param = findParameter (list, proto [it].to<ProtoVar> ());
		    if (param.isEmpty ()) return {};
		    params.push_back (param);
		}
	    } catch (Error::ErrorList list) {
		errors = std::move (list.errors);
	    }

	    if (errors.size () == 0) {
		return params;
	    } else return {};
	}

	std::vector <Generator> CallVisitor::validateTypeParameterList (const Generator & frame, const std::vector <Generator> & proto, const std::vector <Generator> & params, int & score, bool verifyType, bool verifyMemory,  bool isLambda, bool typeList, std::list<Error::ErrorMsg> & errors) {
	    std::vector <Generator> types;
	    
	    for (auto it : Ymir::r (0, proto.size ())) {
		// For the list of argument that has been processed, we check the types and values
		
		auto locType = Generator::empty ();
		if (typeList) locType = proto [it];
		else locType = proto [it].to <Value> ().getType ();
		if (isLambda && locType.isEmpty ()) {
		    types.push_back (Type::init (params [it].to<Value> ().getType ().to <Type> (), false, false));
		} else {
		    
		    if (verifyType) {
			try {
			    this-> _context.verifyCompatibleTypeWithValue (
				params [it].getLocation (),
				locType,
				params [it]
				);
			} catch (Error::ErrorList  list) {
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			    errors.push_back (
				Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto [it].to <Value> ().getLocation (), frame.prettyString ())
				);
			    return {};
			}
		    }

		    if (verifyMemory) {
			try {
			    this-> _context.verifyMemoryOwner (
				params [it].getLocation (),
				locType,
				params [it],
				true, false // we don't check the type again its done above
				);
		    
			} catch (Error::ErrorList  list) {
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			    errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto [it].getLocation (), frame.prettyString ()));
			    return {};
			}
		    }
		

		    types.push_back (locType);
		    score += Scores::SCORE_TYPE;
		    auto llevel = params [it].to <Value> ().getType ().to <Type> ().mutabilityLevel ();
		    auto rlevel = locType.to <Type> ().mutabilityLevel () + 1;
		    score += rlevel - llevel;
		}
	    }

	    return types;
	}
	
	generator::Generator CallVisitor::findSingleParameter (std::list <Generator> & params, const ProtoVar & var) {
	    Generator ret (Generator::empty ()), fst (Generator::empty ());
	    int i = 0, iFst = 0;
	    match_forall (params) { // We traverse the lis of arguments given to the call
		of (NamedGenerator, nmg) { // If one of them is a namedexpression, and has the same name as the protovar
		    if (nmg.getLocation ().getStr () == var.getLocation ().getStr ()) {
			auto toRet = nmg.getContent (); // Then the argument must be its content
			ret = toRet;
			break;
		    }
		} elfo {
		    if (fst.isEmpty ()) {
			fst = ref; // we store the first not named argument to avoid redoing a for loop to find it
			iFst = i;  // (ref, is the var declared by match)
		    }
		}
		i += 1;
	    }

	    if (ret.isEmpty ()) { // If there is no NamedExpression inside the argument list
		// If the var has a value, it is an optional argument, and must be called with a named expression
		if (!var.getValue ().isEmpty ()) {
		    i = -1;
		    ret = var.getValue ();
		} else {
		    // If it does not have a value, it is a mandatory var, so we take the first param that is not a NamedExpression
		    // No NamedExpression can have the same name as var, it is already checked in the first for loop
		    ret = fst;
		    i = iFst;
		    if (ret.isEmpty ()) return Generator::empty ();
		}
	    }
		
	    if (!ret.isEmpty ()) {
		if (i != -1) {
		    auto pt = params.begin (); // We remove it from the argument list
		    std::advance (pt, i);
		    params.erase (pt);
		}
		    
		if (ret.to <Value> ().getType ().is <LambdaType> () && (var.getType ().is <FuncPtr> () || var.getType ().is <Delegate> ())) {
		    std::vector <Generator> paramTypes; // If the argument is a lambda prototype, we need to validate it using the types in the parameter
		    if (var.getType ().is <FuncPtr> ()) paramTypes = var.getType ().to <FuncPtr> ().getParamTypes ();
		    else paramTypes = var.getType ().to <Delegate> ().getInners ()[0].to <FuncPtr> ().getParamTypes ();
		
		    if (ret.is <VarRef> ()) {
			return this-> _context.validateLambdaProto (ret.to <VarRef> ().getValue ().to <LambdaProto> (), paramTypes);
		    } else if (ret.is <LambdaProto> ()) {
			return this-> _context.validateLambdaProto (ret.to<LambdaProto> (), paramTypes);
		    }
		}
	    }

	    return ret;
	}

	
	generator::Generator CallVisitor::findParameter (std::list <Generator> & params, const ProtoVar & var) {
	    if (var.getNbConsume () == 1) {
		return this-> findSingleParameter (params, var);
	    } else { // if it consume more than one argument, then it has been validated by a template specialization
		int i = 0;
		std::vector <Generator> tupleValues;
		std::vector <Generator> tupleTypes;
		std::list <Generator> rest;
		
		tupleTypes.reserve (var.getNbConsume ());
		tupleValues.reserve (var.getNbConsume ());
		
		for (auto & ret : params) {
		    if (!ret.is<NamedGenerator> ()) {
			if (ret.to <Value> ().getType ().is <LambdaType> () && (var.getType ().to <Type> ().getInners () [i].is <FuncPtr> () || var.getType ().to <Type> ().getInners () [i].is <Delegate> ())) {
			    std::vector <Generator> paramTypes;
			    if (var.getType ().to <Type> ().getInners () [i].is <FuncPtr> ()) paramTypes = var.getType ().to <Type> ().getInners () [i].to <FuncPtr> ().getParamTypes ();
			    else paramTypes = var.getType ().to <Delegate> ().getInners ()[0].to <FuncPtr> ().getParamTypes ();
		
			    if (ret.is <VarRef> ()) {
				tupleValues.push_back (this-> _context.validateLambdaProto (ret.to <VarRef> ().getValue ().to <LambdaProto> (), paramTypes));
			    } else if (ret.is <LambdaProto> ())
			    tupleValues.push_back (this-> _context.validateLambdaProto (ret.to<LambdaProto> (), paramTypes));
			} else {
			    tupleValues.push_back (ret);
			}
			tupleTypes.push_back (tupleValues.back ().to <Value> ().getType ());
			
			i += 1;
			if (i == var.getNbConsume ()) break;
		    } else {
			rest.push_back (ret);
		    }
		}
		
		params = rest;
				
		if (i == var.getNbConsume ()) {
		    auto tupleType = Tuple::init (tupleTypes [0].getLocation (), tupleTypes);
		    return TupleValue::init (tupleValues [0].getLocation (), tupleType, tupleValues);
		} else return Generator::empty ();
	    }
	}


	
	

	generator::Generator CallVisitor::findParameterStruct (std::vector <Generator> & params, const generator::VarDecl & var) {
	    for (auto  it : Ymir::r (0, params.size ())) {
		if (params [it].is <NamedGenerator> ()) {
		    auto name = params [it].to <NamedGenerator> ().getLocation ();
		    if (name.getStr () == var.getLocation ().getStr ()) {
			auto toRet = params [it].to <NamedGenerator> ().getContent ();
			params.erase (params.begin () + it);
			return toRet;
		    }
		}
	    }

	    // If the var has a value, it is an optional argument
	    if (!var.getVarValue ().isEmpty ()) return var.getVarValue ();
	    // Cf, find Parameter
	    for (auto it : Ymir::r (0, params.size ())) {
		if (!params [it].is <NamedGenerator> ()) {
		    auto toRet = params [it];
		    params.erase (params.begin () + it);
		    return toRet;
		}
	    }
	    
	    return Generator::empty ();
	}
	
	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================     DOTCALL     ==============================
	 * ================================================================================
	 * ================================================================================
	 */

	
	Generator CallVisitor::validateDotCall (const syntax::Expression & exp, std::vector <Generator> & params, const std::list <Ymir::Error::ErrorMsg> & errors) {
	    if (exp.is <syntax::Binary> () && exp.getLocation () == Token::DOT_AND) {
		auto intr = syntax::Intrinsics::init (lexing::Word::init (exp.getLocation (), Keys::ALIAS), exp.to<syntax::Binary> ().getLeft ());
		auto n_bin = syntax::Binary::init (lexing::Word::init (exp.getLocation (), Token::DOT), intr, exp.to <syntax::Binary> ().getRight (), exp.to <syntax::Binary> ().getType ());
		return validateDotCall (n_bin, params, errors);
	    }
	    
	    Generator right (Generator::empty ());
	    Generator left (Generator::empty ());
	    syntax::Expression synthBin (syntax::Expression::empty ());
	    
	    if (exp.is <syntax::TemplateCall> () && exp.to <syntax::TemplateCall> ().getContent ().is <syntax::Binary> () && (exp.to <syntax::TemplateCall> ().getContent ().getLocation () == Token::DOT ||
															      exp.to <syntax::TemplateCall> ().getContent ().getLocation () == Token::DOT_AND)) {
		auto leftBin = exp.to <syntax::TemplateCall> ().getContent ().to <syntax::Binary> ().getLeft ();
		if (exp.to <syntax::TemplateCall> ().getContent ().getLocation () == Token::DOT_AND) {
		    leftBin = syntax::Intrinsics::init (lexing::Word::init (exp.to <syntax::TemplateCall> ().getContent ().getLocation (), Keys::ALIAS), leftBin);
		}
		auto rightBin = exp.to <syntax::TemplateCall> ().getContent ().to <syntax::Binary> ().getRight ();
		auto rightTmpl = syntax::TemplateCall::init (exp.getLocation (), exp.to <syntax::TemplateCall> ().getParameters (), rightBin);
		synthBin = syntax::Binary::init (lexing::Word::init (exp.to <syntax::TemplateCall> ().getContent ().getLocation (), Token::DOT),
						 leftBin,
						 rightTmpl,
						 syntax::Expression::empty ());
		
	    } else if  (exp.is <syntax::Binary> () && exp.to <syntax::Binary> ().getLocation () == Token::DOT) {
		synthBin = exp;
	    } else {
		this-> error (exp.getLocation (), left, params, errors);
	    }
	    
	    auto bin = synthBin.to <syntax::Binary> ();

	    try {
		left = this-> _context.validateValue (bin.getLeft ());
	    } catch (Error::ErrorList list) {}
	    
	    if (left.isEmpty ()) {
		this-> error (exp.getLocation (), left, params, errors);
	    }
	    
	    if (right.isEmpty ()) {
		try {
		    right = this-> _context.validateValue (bin.getRight ());
		    params.push_back (left);
		} catch (Error::ErrorList list) {
		    std::list <Ymir::Error::ErrorMsg> copyErrors = std::move (errors);
		    copyErrors.back ().addNote (Ymir::Error::createNoteOneLine (ExternalError::get (UFC_REWRITING)));
		    for (auto & it : list.errors) {
			copyErrors.back ().addNote (it);
		    }

		    throw Error::ErrorList {copyErrors};
		}

		if (right.isEmpty ()) {
		    this-> error (exp.getLocation (), left, params, errors);
		}
	    }
	    
	    return right;
	}
	
	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================      ERRORS     ==============================
	 * ================================================================================
	 * ================================================================================
	 */
	    
	
	void CallVisitor::error (const lexing::Word & location, const Generator & left, const std::vector <Generator> & rights, const std::list <Ymir::Error::ErrorMsg> & errors) {
	    std::list <std::string> names;
	    for (auto & it : rights)
	    names.push_back (it.to <Value> ().getType ().to <Type> ().getTypeName ());

	    std::string leftName;
	    if (left.isEmpty ()) leftName = (NoneType::init (location)).prettyString ();
	    else {
		match (left) {
		    of (FrameProto,          proto) leftName = proto.getName ();
		    elof (generator::Struct, str)   leftName = str.getName ();
		    elof (MultSym,           sym)   leftName = sym.getLocation ().getStr ();
		    elof (TemplateRef,       cl)    leftName = cl.prettyString ();
		    elof (TemplateClassCst,  cl)    leftName = cl.prettyString ();
		    elof (ModuleAccess,      acc)   leftName = acc.prettyString ();
		    elof (Value,             val)   leftName = val.getType ().to <Type> ().getTypeName ();
		    fo;
		}
	    }
	    
	    auto err = Ymir::Error::makeOccurAndNote (
		location,
		errors,
		ExternalError::get (UNDEFINED_CALL_OP),
		leftName,
		names
		);
	    
	    err.setWindable (true);
	    throw Error::ErrorList {{err}};
	    // throw Error::ErrorList {errors};
	}

	void CallVisitor::error (const lexing::Word & location, const lexing::Word & end, const Generator & left, const std::vector <Generator> & rights, const std::list <Ymir::Error::ErrorMsg> & errors) {
	    std::list <std::string> names;
	    for (auto & it : rights)
	    names.push_back (it.to <Value> ().getType ().to <Type> ().getTypeName ());

	    std::string leftName;
	    if (left.isEmpty ()) leftName = (NoneType::init (location)).prettyString ();
	    else {
		match (left) {
		    of (FrameProto, proto) leftName = proto.getName ();
		    elof (ConstructorProto, proto) leftName = proto.getName ();
		    elof (generator::Struct, str) leftName = str.getName ();
		    elof (MultSym,    sym)  leftName = sym.prettyString ();
		    elof (ModuleAccess, acc) leftName = acc.prettyString ();
		    elof (TemplateRef, cl) leftName = cl.prettyString ();
		    elof (TemplateClassCst, cl) leftName = cl.prettyString ();
		    elof (Value,      val)  leftName = val.getType ().to <Type> ().getTypeName ();
		    fo;
		}
	    }
	    
	    auto err = Ymir::Error::makeOccurAndNote (
		location,
		end,
		errors,
		ExternalError::get (UNDEFINED_CALL_OP),
		leftName,
		names
		);

	    err.setWindable (true);
	    throw Error::ErrorList {{err}};
	    //throw Error::ErrorList {errors};
	}

	std::string CallVisitor::prettyName (const Generator & gen) {
	    match (gen) {
		of (DelegateValue, dg) {
		    return dg.getType ().to <Type> ().getInners ()[0].prettyString ();
		}
		elof (Call, cl) {
		    return prettyName (cl.getFrame ());
		}
		elof (FrameProto, p) return p.prettyString ();
		elof (ConstructorProto, c) return c.prettyString ();
		elof (generator::Struct, str) return str.getName ();
		elof (MultSym,    sym)   return sym.prettyString ();
		elof (ModuleAccess, acc) return acc.prettyString ();
		elof (TemplateRef, cl) return cl.prettyString ();
		elof (TemplateClassCst, cl) return cl.prettyString ();
		elof (ClassCst, cs) return prettyName (cs.getFrame ());
		elof (Value,      val)  return val.getType ().to <Type> ().getTypeName ();
		fo;
	    }
	    return gen.getLocation().getStr ();
	}

	lexing::Word CallVisitor::realLocation (const Generator & gen) {
	    match (gen) {
		of (DelegateValue, dg) {
		    return dg.getType ().getLocation ();
		}
		elof (Call, cl) {
		    return realLocation (cl.getFrame ());
		}
		elof (ClassCst, cl) {
		    return realLocation (cl.getFrame ());
		} fo;
	    }
	    return gen.getLocation ();
	}
	
	
    }

}
