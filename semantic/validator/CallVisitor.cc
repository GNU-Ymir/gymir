#include <ymir/semantic/validator/CallVisitor.hh>
#include <ymir/semantic/validator/TemplateVisitor.hh>
#include <ymir/global/State.hh>

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
	    std::vector <std::string> errors;
	    {
		TRY (
		    left = this-> _context.validateValue (expression.getLeft (), false, true);
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;
	    }

	    std::vector <Generator> rights;
	    if (left.isEmpty ()) {
		left = this-> validateDotCall (expression.getLeft (), rights, errors);
	    }
	    
	    for (auto & it : expression.getRights ()) {
		auto val = this-> _context.validateValue (it);
		if (val.is<List> ()) {
		    for (auto & g_it : val.to <List> ().getParameters ())
			rights.push_back (g_it);
		} else 
		    rights.push_back (val);
	    }

	    int score = 0;
	    errors = {};
	    auto ret = validate (expression.getLocation (), left, rights, score, errors);
	    
	    if (ret.isEmpty ()) {
		this-> error (expression.getLocation (), expression.getEnd (), left, rights, errors);
		return Generator::empty ();
	    } else return ret;
	}

	generator::Generator CallVisitor::validate (const lexing::Word & location , const generator::Generator & left, const std::vector <generator::Generator> & rights, int & score, std::vector <std::string> & errors) {
	    if (left.is <FrameProto> ()) {
		auto gen = validateFrameProto (location, left.to<FrameProto> (), rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <LambdaProto> ()) {
		auto gen = validateLambdaProto (location, left.to <LambdaProto> (), rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is<VarRef> () && left.to<Value> ().getType ().is<LambdaType> ()) { // We stored the lambdaproto in a varref
		auto gen = validateLambdaProto (location, left.to <VarRef> ().getValue ().to<LambdaProto> (), rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <generator::Struct> ()) {
		auto gen = validateStructCst (location, left.to <generator::Struct> (), rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <MultSym> ()) {
		auto gen = validateMultSym (location, left.to <MultSym> (), rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    } else if (left.to<Value> ().getType ().is<FuncPtr> ()) {
		auto gen = validateFunctionPointer (location, left, rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <generator::TemplateRef> ()) {
		Symbol sym (Symbol::empty ());
		Generator proto_gen (Generator::empty ());
		auto gen = validateTemplateRef (location, left.to <TemplateRef> (), rights, score, errors, sym, proto_gen);
		if (!gen.isEmpty ()) {
		    TRY (
			this-> _context.validateTemplateSymbol (sym);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			msgs.insert (msgs.begin (), Ymir::Error::createNoteOneLine ("% -> %", proto_gen.getLocation (), proto_gen.prettyString ()));
			msgs.insert (msgs.begin (), Ymir::Error::createNote (location, ExternalError::get (IN_TEMPLATE_DEF)));
			THROW (ErrorCode::EXTERNAL, msgs);
		    } FINALLY;
		    return gen;
		}
	    }
	    
	    if (!left.is<MultSym> ()) 
		errors.insert (errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), left.getLocation (), left.prettyString ()));	    
	    return Generator::empty ();
	}
	
	generator::Generator CallVisitor::validateFrameProto (const lexing::Word & location, const FrameProto & proto, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params;
	    std::vector <Generator> addParams;
	    std::vector <Generator> rights = rights_;
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		auto param = findParameter (rights, proto.getParameters () [it].to<ProtoVar> ());
		if (param.isEmpty ()) return Generator::empty ();
		params.push_back (param);
	    }
	    
	    if (rights.size () != 0 && !proto.isCVariadic ()) return Generator::empty ();
	    addParams = rights;
	    
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		{
		    TRY (
			this-> _context.verifyCompatibleType (
			    proto.getParameters () [it].to <Value> ().getType (),
			    params [it].to <Value> ().getType ()
			);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors = {};
			return Generator::empty ();
		    } FINALLY;
		}
		
		{
		    TRY (		    
			this-> _context.verifyMemoryOwner (
			    params [it].getLocation (),
			    proto.getParameters () [it].to <Value> ().getType (),
			    params [it],
			    true
			);
			types.push_back (proto.getParameters () [it].to <Value> ().getType ());
			score += Scores::SCORE_TYPE;		   
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto.getParameters () [it].to <Value> ().getLocation (), proto.prettyString ()));
		    } FINALLY;
		}
	    }

	    if (errors.size () != 0) return Generator::empty ();	   
	    return Call::init (location, proto.getReturnType (), proto.clone (), types, params, addParams);
	}
       
	generator::Generator CallVisitor::findParameter (std::vector <Generator> & params, const ProtoVar & var) {
	    for (auto  it : Ymir::r (0, params.size ())) {
		if (params [it].is <NamedGenerator> ()) {
		    auto name = params [it].to <NamedGenerator> ().getLocation ();
		    if (name.str == var.getLocation ().str) {
			auto toRet = params [it].to <NamedGenerator> ().getContent ();
			params.erase (params.begin () + it);
			return toRet;
		    }
		}
	    }

	    // If the var has a value, it is an optional argument
	    if (!var.getValue ().isEmpty ()) return var.getValue ();
	    
	    else if (params.size () == 0) return Generator::empty ();
	    // If it does not have a value, it is a mandatory var, and its name cannot be the same as params [0] (we just verify that in the for loop)
	    else if (params [0].is <NamedGenerator> ()) return Generator::empty ();
	    
	    auto toRet = params [0];
	    params.erase (params.begin ());
	    return toRet;	    
	}

	generator::Generator CallVisitor::validateLambdaProto (const lexing::Word & location, const LambdaProto & proto, const std::vector <Generator> & rights, int & score, std::vector <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> types;
	    if (rights.size () != proto.getParameters ().size ()) return Generator::empty ();
	    
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		if (proto.getParameters ()[it].to <Value> ().getType ().isEmpty ()) {
		    types.push_back (rights [it].to<Value> ().getType ());
		    types.back ().to <Type> ().isMutable (false);
		    types.back ().to <Type> ().isRef (false);
		} else {		    
		    {
			TRY (
			    this-> _context.verifyCompatibleType (
				proto.getParameters () [it].to <Value> ().getType (),
				rights [it].to <Value> ().getType ()
			    );
			) CATCH (ErrorCode::EXTERNAL) {
			    GET_ERRORS_AND_CLEAR (msgs);
			    errors = {};
			    return Generator::empty ();
			} FINALLY;
		    }
		    
		    {
			TRY (		    
			    this-> _context.verifyMemoryOwner (
				rights [it].getLocation (),
				proto.getParameters () [it].to <Value> ().getType (),
				rights [it],
				true
			    );
			    types.push_back (proto.getParameters () [it].to <Value> ().getType ());
			    score += Scores::SCORE_TYPE;		   
			) CATCH (ErrorCode::EXTERNAL) {
			    GET_ERRORS_AND_CLEAR (msgs);
			    errors.insert (errors.end (), msgs.begin (), msgs.end ());
			    errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto.getParameters () [it].to <Value> ().getLocation (), proto.prettyString ()));
			} FINALLY;
		    }
		}
	    }

	    if (errors.size () != 0) return Generator::empty ();
	    
	    TRY (
		auto frameProto = this-> _context.validateLambdaProto (proto, types);
		return Call::init (location, frameProto.to<FrameProto> ().getReturnType (), frameProto.clone (), types, rights, {});
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors.insert (errors.end (), msgs.begin (), msgs.end ());
	    } FINALLY;
	    return Generator::empty ();
	}

	
	generator::Generator CallVisitor::validateStructCst (const lexing::Word & location, const generator::Struct & str, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    std::vector <Generator> params;
	    std::vector <Generator> rights = rights_;
	    for (auto it : str.getFields ()) {
		auto param = findParameterStruct (rights, it.to<generator::VarDecl> ());
		if (param.isEmpty ()) return Generator::empty ();
		params.push_back (param);
	    }

	    if (rights.size () != 0) return Generator::empty ();
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, str.getFields ().size ())) {
		TRY (
		    this-> _context.verifyMemoryOwner (
			params [it].getLocation (),
			str.getFields () [it].to <generator::VarDecl> ().getVarType (),
			params [it],
			true
		    );
		    types.push_back (str.getFields () [it].to <generator::VarDecl> ().getVarType ());
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;
	    }

	    if (errors.size () != 0) return Generator::empty ();
	    
	    score = 0;
	    return StructCst::init (location, StructRef::init (str.getLocation (), str.getRef ()), str.clone (), types, params);
	}	

	generator::Generator CallVisitor::findParameterStruct (std::vector <Generator> & params, const generator::VarDecl & var) {
	    for (auto  it : Ymir::r (0, params.size ())) {
		if (params [it].is <NamedGenerator> ()) {
		    auto name = params [it].to <NamedGenerator> ().getLocation ();
		    if (name.str == var.getLocation ().str) {
			auto toRet = params [it].to <NamedGenerator> ().getContent ();
			params.erase (params.begin () + it);
			return toRet;
		    }
		}
	    }

	    // If the var has a value, it is an optional argument
	    if (!var.getVarValue ().isEmpty ()) return var.getVarValue ();
	    
	    else if (params.size () == 0) return Generator::empty ();
	    // If it does not have a value, it is a mandatory var, and its name cannot be the same as params [0] (we just verify that in the for loop)
	    else if (params [0].is <NamedGenerator> ()) return Generator::empty ();
	    
	    auto toRet = params [0];
	    params.erase (params.begin ());
	    return toRet;	    
	}

	generator::Generator CallVisitor::validateFunctionPointer (const lexing::Word & location, const Generator & gen, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params = rights_;
	    auto funcType = gen.to <Value> ().getType ().to <FuncPtr> ();
	    if (params.size () != funcType.getParamTypes ().size ()) return Generator::empty ();
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, funcType.getParamTypes ().size ())) {
		{
		    TRY (
			this-> _context.verifyCompatibleType (
			    funcType.getParamTypes () [it],
			    params [it].to <Value> ().getType ()
			);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors = {};
			return Generator::empty ();
		    } FINALLY;
		}
		
		{
		    TRY (		    
			this-> _context.verifyMemoryOwner (
			    params [it].getLocation (),
			    funcType.getParamTypes () [it],
			    params [it],
			    true
			);
			types.push_back (funcType.getParamTypes () [it]);
			score += Scores::SCORE_TYPE;		   
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), it, funcType.getTypeName ()));
		    } FINALLY;
		}
	    }

	    if (errors.size () != 0) return Generator::empty ();	   
	    return Call::init (location,
			       funcType.getReturnType (),
			       gen,
			       types,
			       params,
			       {}
	    );
	}
	
	generator::Generator CallVisitor::validateMultSym (const lexing::Word & location, const MultSym & sym, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    Generator final_gen (Generator::empty ());	    
	    Generator used_gen (Generator::empty ());
	    Generator proto_gen (Generator::empty ());
	    Symbol templSym (Symbol::empty ());
	    score = -1;
	    int nbCand = 0;
	    bool fromTempl = true;
	    std::map <int, std::vector <Generator>> nonTemplScores;
	    std::map <int, std::vector <Symbol>> templScores;
	    for (auto & it : sym.getGenerators ()) {
		int current = 0;
		std::vector <std::string> local_errors;
		if (it.is <FrameProto> ()) {
		    auto gen = validateFrameProto (location, it.to <FrameProto> (), rights_, current, local_errors);
		    if (!gen.isEmpty ()) nonTemplScores[current].push_back (gen);
		    if (!gen.isEmpty () && (current > score || fromTempl)) {// simple function can take the token on 1. less scored 2. every templates
			score = current;
			final_gen = gen;
			used_gen = it;
			fromTempl = false;
		    } else if (gen.isEmpty ()) {
			local_errors.insert (local_errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.getLocation (), it.prettyString ()));
			insertCandidate (nbCand, errors, local_errors);
		    }
		} else if (it.is <TemplateRef> ()) {
		    Symbol _sym (Symbol::empty ());
		    Generator _proto_gen (Generator::empty ());
		    auto gen = validateTemplateRef (location, it.to <TemplateRef> (), rights_, current, local_errors, _sym, _proto_gen);
		    if (!gen.isEmpty ()) templScores [current].push_back (_sym);
		    if (!gen.isEmpty () && current > score && fromTempl) { // Can only take the token over less scored template function
			score = current;
			final_gen = gen;
			used_gen = it;
			proto_gen = _proto_gen;
			fromTempl = true;
			templSym = _sym;
		    } else if (gen.isEmpty ()) {
			local_errors.insert (local_errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.to <TemplateRef> ().getTemplateRef ().getName (), it.prettyString ()));
			insertCandidate (nbCand, errors, local_errors);
		    }
		} else if (it.is <VarRef> () && it.to <VarRef> ().getValue ().is<LambdaProto> ()) {
		    auto gen = validateLambdaProto (location, it.to<VarRef> ().getValue ().to <LambdaProto> (), rights_, current, local_errors);
		    if (!gen.isEmpty ()) nonTemplScores[current].push_back (gen);
		    if (!gen.isEmpty () && (current > score || fromTempl)) {
			// simple function can take the token on 1. less scored 2. every templates
			
			score = current;
			final_gen = gen;
			used_gen = it;
			fromTempl = false;
		    } else if (gen.isEmpty ()) {
			local_errors.insert (local_errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.getLocation (), it.prettyString ()));
			insertCandidate (nbCand, errors, local_errors);
		    }
   
		}
	    }
	    
	    if (!templSym.isEmpty () && fromTempl) {
		auto element_on_scores = templScores.find (score);
		if (element_on_scores-> second.size () != 1) {
		    std::string leftName = sym.getLocation ().str;
		    std::vector<std::string> names;
		    for (auto & it : rights_)
			names.push_back (it.prettyString ());
			
		    std::string note;
		    for (auto & it : element_on_scores-> second)
			note += Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.getName (), this-> _context.validateMultSym (sym.getLocation (), {it}).prettyString ()) + '\n';
		    Ymir::Error::occurAndNote (location,
					       note,
					       ExternalError::get (SPECIALISATION_WOTK_WITH_BOTH),
					       leftName,
					       names);

		}
		
		TRY (
		    this-> _context.validateTemplateSymbol (templSym);
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    msgs.insert (msgs.begin (), Ymir::Error::createNoteOneLine ("% -> %", proto_gen.getLocation (), proto_gen.prettyString ()));
		    msgs.insert (msgs.begin (), Ymir::Error::createNote (location, ExternalError::get (IN_TEMPLATE_DEF)));
		    THROW (ErrorCode::EXTERNAL, msgs);
		} FINALLY;
	    } else if (!final_gen.isEmpty ()) {
		auto element_on_scores = nonTemplScores.find (score);
		if (element_on_scores-> second.size () != 1) {
		    std::string leftName = sym.getLocation ().str;
		    std::vector<std::string> names;
		    for (auto & it : rights_)
			names.push_back (it.prettyString ());
			
		    std::string note;
		    for (auto & it : element_on_scores-> second)
			note += Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.getLocation (), it.prettyString ()) + '\n';
		    Ymir::Error::occurAndNote (location,
					       note,
					       ExternalError::get (SPECIALISATION_WOTK_WITH_BOTH),
					       leftName,
					       names);

		}
	    }
	    return final_gen;
	}

	generator::Generator CallVisitor::validateTemplateRef (const lexing::Word & location, const generator::TemplateRef & ref, const std::vector <generator::Generator> & rights_, int & score, std::vector <std::string> & errors, Symbol & _sym, Generator & proto_gen) {
	    const Symbol & sym = ref.getTemplateRef ();
	    if (!sym.to<semantic::Template> ().getDeclaration ().is <syntax::Function> ()) return Generator::empty ();
	    
	    std::vector <Generator> typeParams;
	    std::vector <Generator> valueParams;
	    std::vector <Generator> rights = rights_;

	    for (auto & it : sym.to <semantic::Template> ().getDeclaration ().to <syntax::Function> ().getPrototype ().getParameters ()) {
		Generator value (Generator::empty ());
		auto var = it.to<syntax::VarDecl> ();
		volatile bool failure = false;
		
		this-> _context.enterForeign ();		    // We enter a foreign, the value of a parameter var has no local context
		TRY (
		    this-> _context.enterBlock (); // it has no context but it need a block to be validated
		    if (!var.getValue ().isEmpty ())
			value = this-> _context.validateValue (var.getValue ());
		    this-> _context.quitBlock ();

		    auto type = Void::init (var.getName ());
		    bool isMutable = false;
		    bool isRef = false;
		    this-> _context.applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable);
		    
		    auto param = findParameter (rights, ProtoVar::init (var.getName (), Generator::empty (), value, isMutable).to<ProtoVar> ());
		    if (param.isEmpty ())
		    	failure = true;
		    else {
			typeParams.push_back (param.to <Value> ().getType ());
			valueParams.push_back (param);
		    }
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.begin (), msgs.begin (), msgs.end ());
		    return Generator::empty ();
		} FINALLY;		     		

		this-> _context.exitForeign (); // exiting the context to return to the context of the local frame
		if (errors.size () != 0)
		    THROW (ErrorCode::EXTERNAL, errors);

		if (failure) return Generator::empty ();
	    }

	    for (auto & it : rights) { // Add the rests, for variadic templates 
		typeParams.push_back (it.to <Value> ().getType ());
		valueParams.push_back (it);
	    }

	    auto templateVisitor = TemplateVisitor::init (this-> _context);
	    std::vector <Generator> finalParams;
	    Generator soluce (Generator::empty ());
	    
	    TRY (
		// The solution is a function transformed generated by template specialisation (if it succeed)
		soluce = templateVisitor.validateFromImplicit (ref, valueParams, typeParams, score, _sym, finalParams);
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors.insert (errors.begin (), msgs.begin (), msgs.end ());
		return Generator::empty ();
	    } FINALLY;
	    
	    if (!soluce.isEmpty ()) {
		int _score;
		proto_gen = soluce;
		auto res = validateFrameProto (location, soluce.to<FrameProto> (), finalParams, _score, errors);
		score += _score;		
		return res;
	    }
	    
	    return Generator::empty ();	    
	}							   

	Generator CallVisitor::validateDotCall (const syntax::Expression & left, std::vector <Generator> & params, const std::vector <std::string> & errors) {
	    match (left) {
		of (syntax::Binary, bin, {
			if (bin.getLocation () == Token::DOT) {
			    bool success = true;
			    Generator right (Generator::empty ());
			    TRY (				
				auto param = this-> _context.validateValue (bin.getLeft ());
				right = this-> _context.validateValue (bin.getRight ());
				params.push_back (param);
			    ) CATCH (ErrorCode::EXTERNAL) {
				GET_ERRORS_AND_CLEAR (msgs);
				success = false;
			    } FINALLY;
			    if (success) return right;
			}
		    }
		);
	    }
	    
	    THROW (ErrorCode::EXTERNAL, errors);
	    return Generator::empty ();
	}
	
	void CallVisitor::error (const lexing::Word & location, const Generator & left, const std::vector <Generator> & rights, std::vector <std::string> & errors) {	    
	    std::vector <std::string> names;
	    for (auto & it : rights)
		names.push_back (it.to <Value> ().getType ().to <Type> ().getTypeName ());

	    std::string leftName;
	    match (left) {
		of (FrameProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())
		else of (MultSym,    sym,   leftName = sym.getLocation ().str)
		else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ())
	    }
	    
	    errors.insert (errors.begin (), Ymir::Error::makeOccur (
		location,
		ExternalError::get (UNDEFINED_CALL_OP),
		leftName,
		names
	    ));
	    
	    
	    THROW (ErrorCode::EXTERNAL, errors);
	}

	void CallVisitor::error (const lexing::Word & location, const lexing::Word & end, const Generator & left, const std::vector <Generator> & rights, std::vector <std::string> & errors) {	    
	    std::vector <std::string> names;
	    for (auto & it : rights)
		names.push_back (it.to <Value> ().getType ().to <Type> ().getTypeName ());

	    std::string leftName;
	    match (left) {
		of (FrameProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())
		else of (MultSym,    sym,   leftName = sym.getLocation ().str)
		else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ())
	    }
	    
	    errors.insert (errors.begin (), Ymir::Error::makeOccur (
		location,
		end,
		ExternalError::get (UNDEFINED_CALL_OP),
		leftName,
		names
	    ));
	    
	    
	    THROW (ErrorCode::EXTERNAL, errors);
	}

	
	void CallVisitor::insertCandidate (int & nb, std::vector <std::string> & errors, const std::vector <std::string> & candErrors) {	    
	    if (nb == 3 && !global::State::instance ().isVerboseActive ()) {
		errors.push_back (format ("     : %(B)", "..."));
		errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (OTHER_CANDIDATES)));
		nb += 1;
	    } else if (nb < 3 || global::State::instance ().isVerboseActive ()) {
		errors.insert (errors.begin (), candErrors.begin (), candErrors.end ());
		nb += 1;
	    }
	}
	

    }

}
