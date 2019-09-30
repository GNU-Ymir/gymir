#include <ymir/semantic/validator/CallVisitor.hh>
#include <ymir/semantic/validator/TemplateVisitor.hh>

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
	    auto left = this-> _context.validateValue (expression.getLeft ());
	    std::vector <Generator> rights;
	    for (auto & it : expression.getRights ()) {
		auto val = this-> _context.validateValue (it);
		if (val.is<List> ()) {
		    for (auto & g_it : val.to <List> ().getParameters ())
			rights.push_back (g_it);
		} else 
		    rights.push_back (val);
	    }

	    int score = 0;
	    std::vector <std::string> errors;
	    auto ret = validate (expression, left, rights, score, errors);
	    
	    if (ret.isEmpty ()) {
		this-> error (expression, left, rights, errors);
		return Generator::empty ();
	    } else return ret;
	}


	generator::Generator CallVisitor::validate (const syntax::MultOperator & expression, const generator::Generator & left, const std::vector <generator::Generator> & rights, int & score, std::vector <std::string> & errors) {
	    if (left.is <FrameProto> ()) {
		auto gen = validateFrameProto (expression, left.to<FrameProto> (), rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <generator::Struct> ()) {
		auto gen = validateStructCst (expression, left.to <generator::Struct> (), rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <MultSym> ()) {
		auto gen = validateMultSym (expression, left.to <MultSym> (), rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <generator::TemplateRef> ()) {
		Symbol sym (Symbol::empty ());
		Generator proto_gen (Generator::empty ());
		auto gen = validateTemplateRef (expression, left.to <TemplateRef> (), rights, score, errors, sym, proto_gen);
		if (!gen.isEmpty ()) {
		    TRY (
			this-> _context.validateTemplateSymbol (sym);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			msgs.insert (msgs.begin (), Ymir::Error::createNoteOneLine ("% -> %", proto_gen.getLocation (), proto_gen.prettyString ()));
			msgs.insert (msgs.begin (), Ymir::Error::createNote (expression.getLocation (), ExternalError::get (IN_TEMPLATE_DEF)));
			THROW (ErrorCode::EXTERNAL, msgs);
		    } FINALLY;
		    return gen;
		}
	    }
	    
	    if (!left.is<MultSym> ()) 
		errors.insert (errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), left.getLocation (), left.prettyString ()));	    
	    return Generator::empty ();
	}

	generator::Generator CallVisitor::validateFrameProto (const syntax::MultOperator & expression, const FrameProto & proto, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params;
	    std::vector <Generator> rights = rights_;
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		auto param = findParameter (rights, proto.getParameters () [it].to<ProtoVar> ());
		if (param.isEmpty ()) return Generator::empty ();
		params.push_back (param);
	    }
	    
	    if (rights.size () != 0) return Generator::empty ();
	    
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
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

	    if (errors.size () != 0) return Generator::empty ();	   
	    return Call::init (expression.getLocation (), proto.getReturnType (), proto.clone (), types, params);
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

	generator::Generator CallVisitor::validateStructCst (const syntax::MultOperator & expression, const generator::Struct & str, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
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
	    return StructCst::init (expression.getLocation (), StructRef::init (str.getLocation (), str.getRef ()), str.clone (), types, params);
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
	
	generator::Generator CallVisitor::validateMultSym (const syntax::MultOperator & expression, const MultSym & sym, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    Generator final_gen (Generator::empty ());	    
	    Generator used_gen (Generator::empty ());
	    Generator proto_gen (Generator::empty ());
	    Symbol templSym (Symbol::empty ());
	    score = -1;
	    bool fromTempl = true;
	    std::map <int, std::vector <Generator>> nonTemplScores;
	    std::map <int, std::vector <Symbol>> templScores;
	    for (auto & it : sym.getGenerators ()) {
		int current = 0;
		std::vector <std::string> local_errors;
		if (it.is <FrameProto> ()) {
		    auto gen = validateFrameProto (expression, it.to <FrameProto> (), rights_, current, local_errors);
		    if (!gen.isEmpty ()) nonTemplScores[current].push_back (gen);
		    if (!gen.isEmpty () && (current > score || fromTempl)) {// simple function can take the token on 1. less scored 2. every templates
			score = current;
			final_gen = gen;
			used_gen = it;
			fromTempl = false;
		    } else if (gen.isEmpty ()) {
			local_errors.insert (local_errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.getLocation (), it.prettyString ()));
			errors.insert (errors.begin (), local_errors.begin (), local_errors.end ());
		    }
		} else if (it.is <TemplateRef> ()) {
		    Symbol _sym (Symbol::empty ());
		    Generator _proto_gen (Generator::empty ());
		    auto gen = validateTemplateRef (expression, it.to <TemplateRef> (), rights_, current, local_errors, _sym, _proto_gen);
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
			errors.insert (errors.begin (), local_errors.begin (), local_errors.end ());
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
		    Ymir::Error::occurAndNote (expression.getLocation (),
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
		    msgs.insert (msgs.begin (), Ymir::Error::createNote (expression.getLocation (), ExternalError::get (IN_TEMPLATE_DEF)));
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
		    Ymir::Error::occurAndNote (expression.getLocation (),
					       note,
					       ExternalError::get (SPECIALISATION_WOTK_WITH_BOTH),
					       leftName,
					       names);

		}
	    }
	    return final_gen;
	}

	generator::Generator CallVisitor::validateTemplateRef (const syntax::MultOperator & expression, const generator::TemplateRef & ref, const std::vector <generator::Generator> & rights_, int & score, std::vector <std::string> & errors, Symbol & _sym, Generator & proto_gen) {
	    const Symbol & sym = ref.getTemplateRef ();
	    if (!sym.to<semantic::Template> ().getDeclaration ().is <syntax::Function> ()) return Generator::empty ();
	    
	    std::vector <Generator> typeParams;
	    std::vector <Generator> valueParams;
	    std::vector <Generator> rights = rights_;
	    for (auto & it : sym.to <semantic::Template> ().getDeclaration ().to <syntax::Function> ().getPrototype ().getParameters ()) {
		Generator value (Generator::empty ());
		auto var = it.to<syntax::VarDecl> ();
		
		if (!var.getValue ().isEmpty ())
		    value = this-> _context.validateValue (var.getValue ());
		
		auto type = Void::init (var.getName ());
		bool isMutable = false, isRef = false;
		this-> _context.applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable);
		
		auto param = findParameter (rights, ProtoVar::init (var.getName (), Generator::empty (), value, isMutable).to<ProtoVar> ());
		if (param.isEmpty ()) return Generator::empty ();
		typeParams.push_back (param.to <Value> ().getType ());
		valueParams.push_back (param);
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
		auto res = validateFrameProto (expression, soluce.to<FrameProto> (), finalParams, _score, errors);
		score += _score;		
		return res;
	    }
	    
	    return Generator::empty ();	    
	}							   
	
	void CallVisitor::error (const syntax::MultOperator & expression, const Generator & left, const std::vector <Generator> & rights, std::vector <std::string> & errors) {	    
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
		expression.getLocation (),
		expression.getEnd (),
		ExternalError::get (UNDEFINED_CALL_OP),
		leftName,
		names
	    ));
	    
	    
	    THROW (ErrorCode::EXTERNAL, errors);
	}

	

    }

}
