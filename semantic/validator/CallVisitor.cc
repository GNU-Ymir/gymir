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
	    if (left.isEmpty () && expression.canBeDotCall ()) {
		left = this-> validateDotCall (expression.getLeft (), rights, errors);		
	    } else if (left.isEmpty ())
		THROW (ErrorCode::EXTERNAL, errors);

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
	    } else 
		return ret;	    
	}

	generator::Generator CallVisitor::validate (const lexing::Word & location , const generator::Generator & left, const std::vector <generator::Generator> & rights, int & score, std::vector <std::string> & errors) {
	    bool checked = false;
	    if (left.is <FrameProto> ()) {
		auto gen = validateFrameProto (location, left.to<FrameProto> (), rights, score, errors);
		checked = true;
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <ConstructorProto> ()) {
		auto gen = validateConstructorProto (location, left.to <ConstructorProto> (), rights, score, errors);
		checked = true;
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <LambdaProto> ()) {
		auto gen = validateLambdaProto (location, left.to <LambdaProto> (), rights, score, errors);
		checked = true;
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is<VarRef> () && left.to<Value> ().getType ().is<LambdaType> ()) { // We stored the lambdaproto in a varref
		auto gen = validateLambdaProto (location, left.to <VarRef> ().getValue ().to<LambdaProto> (), rights, score, errors);
		checked = true;
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <generator::Struct> ()) {
		auto gen = validateStructCst (location, left.to <generator::Struct> (), rights, score, errors);
		checked = true;
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <MultSym> ()) {
		auto gen = validateMultSym (location, left.to <MultSym> (), rights, score, errors);
		checked = true;
		if (!gen.isEmpty ()) return gen;
	    } else if (left.to<Value> ().getType ().is<FuncPtr> ()) {
		auto gen = validateFunctionPointer (location, left, rights, score, errors);
		checked = true;
		if (!gen.isEmpty ()) return gen;
	    } else if (left.to <Value> ().getType ().is <Delegate> ()) {
		auto gen = validateDelegate (location, left, rights, score, errors);
		checked = true;
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <generator::TemplateRef> ()) {
		checked = true;
		Symbol sym (Symbol::empty ());
		Generator proto_gen (Generator::empty ());
		auto gen = validateTemplateRef (location, left, rights, score, errors, sym, proto_gen);
		if (!gen.isEmpty ()) {
		    std::vector <std::string> local_errors;
		    TRY (
			this-> _context.validateTemplateSymbol (sym, left);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			msgs.insert (msgs.begin (), Ymir::Error::createNoteOneLine ("% -> %", proto_gen.getLocation (), proto_gen.prettyString ()));
			msgs.insert (msgs.begin (), Ymir::Error::createNote (location, ExternalError::get (IN_TEMPLATE_DEF)));
			errors = msgs;
			gen = Generator::empty ();
		    } FINALLY;
		    
		    return gen;
		}
	    }
	    
	    if (!left.is<MultSym> () && checked) 
		errors.insert (errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), realLocation (left), prettyName (left)));	    
	    return Generator::empty ();
	}
	
	generator::Generator CallVisitor::validateFrameProto (const lexing::Word & location, const FrameProto & proto, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params;
	    std::vector <Generator> addParams;
	    std::vector <Generator> rights = rights_;
	    TRY (
		for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		    auto param = findParameter (rights, proto.getParameters () [it].to<ProtoVar> ());
		    if (param.isEmpty ()) return Generator::empty ();
		    params.push_back (param);
		}
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors = msgs;
	    } FINALLY;
	    
	    if (errors.size () != 0) return Generator::empty ();	    
	    if (rights.size () != 0 && !proto.isCVariadic ()) return Generator::empty ();
	    addParams = rights;
	    
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		{
		    bool succeed = true;
		    TRY (			
			this-> _context.verifyCompatibleType (
			    params [it].getLocation (),
			    proto.getParameters () [it].to <Value> ().getType (),
			    params [it].to <Value> ().getType ()
			);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto.getParameters () [it].to <Value> ().getLocation (), proto.prettyString ()));
			succeed = false;
		    } FINALLY;
		    
		    if (!succeed) return Generator::empty ();
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
			auto llevel = params [it].to <Value> ().getType ().to <Type> ().mutabilityLevel ();
			auto rlevel = proto.getParameters () [it].to <Value> ().getType ().to <Type> ().mutabilityLevel () + 1;
			score += rlevel - llevel;

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

	generator::Generator CallVisitor::validateConstructorProto (const lexing::Word & location, const ConstructorProto & proto, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params;
	    std::vector <Generator> rights = rights_;
	    TRY (
		for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		    auto param = findParameter (rights, proto.getParameters () [it].to<ProtoVar> ());
		    if (param.isEmpty ()) return Generator::empty ();
		    params.push_back (param);
		}
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors = msgs;
	    } FINALLY;

	    if (errors.size () != 0) return Generator::empty ();	    
	    if (rights.size () != 0) return Generator::empty ();
	    
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		{
		    bool succeed = true;
		    TRY (			
			this-> _context.verifyCompatibleType (
			    params [it].getLocation (),
			    proto.getParameters () [it].to <Value> ().getType (),
			    params [it].to <Value> ().getType ()
			);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto.getParameters () [it].to <Value> ().getLocation (), proto.prettyString ()));
			succeed = false;
		    } FINALLY;
		    
		    if (!succeed) return Generator::empty ();
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
			auto llevel = params [it].to <Value> ().getType ().to <Type> ().mutabilityLevel ();
			auto rlevel = proto.getParameters () [it].to <Value> ().getType ().to <Type> ().mutabilityLevel () + 1;
			score += rlevel - llevel;

		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto.getParameters () [it].to <Value> ().getLocation (), proto.prettyString ()));
		    } FINALLY;
		}
	    }

	    if (errors.size () != 0) return Generator::empty ();	   
	    return ClassCst::init (location, proto.getReturnType (), proto.clone (), types, params);
	}	
       
	generator::Generator CallVisitor::findParameter (std::vector <Generator> & params, const ProtoVar & var) {
	    Generator ret (Generator::empty ());
	    for (auto  it : Ymir::r (0, params.size ())) {
		if (params [it].is <NamedGenerator> ()) {
		    auto name = params [it].to <NamedGenerator> ().getLocation ();
		    if (name.str == var.getLocation ().str) {
			auto toRet = params [it].to <NamedGenerator> ().getContent ();
			params.erase (params.begin () + it);
			ret = toRet;
			break;
		    }
		}
	    }

	    if (ret.isEmpty ()) {
		// If the var has a value, it is an optional argument
		if (!var.getValue ().isEmpty ()) ret = var.getValue ();	    
		// If it does not have a value, it is a mandatory var, so we take the first param that is not a NamedExpression
		// No NamedExpression can have the same name as var, it is already checked in the first for loop
		else {
		    for (auto it : Ymir::r (0, params.size ())) {
			if (!params [it].is<NamedGenerator> ()) {
			    ret = params [it];
			    params.erase (params.begin () + it);
			    break;
			}
		    }
		    if (ret.isEmpty ()) return Generator::empty ();
		}
	    }
	    
	    if (ret.to <Value> ().getType ().is <LambdaType> () && (var.getType ().is <FuncPtr> () || var.getType ().is <Delegate> ())) {
		std::vector <Generator> paramTypes;
		if (var.getType ().is <FuncPtr> ()) paramTypes = var.getType ().to <FuncPtr> ().getParamTypes ();
		else paramTypes = var.getType ().to <Delegate> ().getInners ()[0].to <FuncPtr> ().getParamTypes ();
		
	    	if (ret.is <VarRef> ()) {
	    	    return this-> _context.validateLambdaProto (ret.to <VarRef> ().getValue ().to <LambdaProto> (), paramTypes);
	    	} else if (ret.is <LambdaProto> ()) 		    
	    	    return this-> _context.validateLambdaProto (ret.to<LambdaProto> (), paramTypes);		     	    
	    }
	    
	    return ret;	    
	}

	generator::Generator CallVisitor::validateLambdaProto (const lexing::Word & location, const LambdaProto & proto, const std::vector <Generator> & rights, int & score, std::vector <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> types;
	    std::vector <Generator> params = rights;
	    if (rights.size () != proto.getParameters ().size ()) return Generator::empty ();
	    
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		if (proto.getParameters ()[it].to <Value> ().getType ().isEmpty ()) {
		    types.push_back (rights [it].to<Value> ().getType ());
		    types.back ().to <Type> ().isMutable (false);
		    types.back ().to <Type> ().isRef (false);
		} else {		    
		    {
			bool succeed = true;
			TRY (
			    this-> _context.verifyCompatibleType (
				params [it].getLocation (),
				proto.getParameters () [it].to <Value> ().getType (),
				params [it].to <Value> ().getType ()
			    );
			) CATCH (ErrorCode::EXTERNAL) {
			    GET_ERRORS_AND_CLEAR (msgs);
			    succeed = false;
			} FINALLY;
			if (!succeed) return Generator::empty ();
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
			    auto llevel = params [it].to <Value> ().getType ().to <Type> ().mutabilityLevel ();
			    auto rlevel = proto.getParameters () [it].to <Value> ().getType ().to <Type> ().mutabilityLevel () + 1;
			    score += rlevel - llevel;
			    
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
		auto gen = this-> _context.validateLambdaProto (proto, types);
		match (gen) {
		    of (Addresser, addr, // Simple proto, no closure
			return Call::init (location, addr.getWho ().to<FrameProto> ().getReturnType (), addr.getWho ().clone (), types, rights, {});
		    ) else of (DelegateValue, dl,
			return Call::init (location, dl.getFuncPtr ().to <Addresser> ().getWho ().to <FrameProto> ().getReturnType (), gen, types, rights, {});
		    );
		}
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		return Generator::empty ();
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors.insert (errors.end (), msgs.begin (), msgs.end ());
	    } FINALLY;
	    return Generator::empty ();
	}

	
	generator::Generator CallVisitor::validateStructCst (const lexing::Word & location, const generator::Struct & str, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    std::vector <Generator> params;
	    std::vector <Generator> rights = rights_;
	    std::vector <Generator> types;
	    
	    if (str.getRef ().to <semantic::Struct> ().isUnion ()) {
		lexing::Word name;
		TRY (
		    if (rights.size () != 1)
			Ymir::Error::occur (location, ExternalError::get (UNION_CST_MULT));

		    if (rights [0].is <NamedGenerator> ()) { // If it is a named gen, we get the field with the same name
			name = rights [0].to <NamedGenerator> ().getLocation ();
			auto param = rights [0].to <NamedGenerator> ().getContent ();
			for (auto it : str.getFields ()) {
			    if (it.to <generator::VarDecl> ().getLocation ().str == name.str) {				
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
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;		
		if (params.size () == 0) return Generator::empty ();

		if (errors.size () != 0) return Generator::empty ();
	    
		score = 0;
		return UnionCst::init (location, StructRef::init (str.getLocation (), str.getRef ()), str.clone (), name.str, types[0], params[0]);
	    } else {
		for (auto it : str.getFields ()) {
		    auto param = findParameterStruct (rights, it.to<generator::VarDecl> ());
		    if (param.isEmpty ()) return Generator::empty ();
		    params.push_back (param);
		}
		
		if (rights.size () != 0) return Generator::empty ();
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
	
	generator::Generator CallVisitor::validateFunctionPointer (const lexing::Word & location, const Generator & gen, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params = rights_;
	    auto funcType = gen.to <Value> ().getType ().to <FuncPtr> ();
	    if (params.size () != funcType.getParamTypes ().size ()) return Generator::empty ();
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, funcType.getParamTypes ().size ())) {
		{
		    bool succeed = true;
		    TRY (
			this-> _context.verifyCompatibleType (
			    params [it].getLocation (),
			    funcType.getParamTypes () [it],
			    params [it].to <Value> ().getType ()
			);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors = {};
			succeed = false;
		    } FINALLY;
		    
		    if (!succeed) return Generator::empty ();
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
			
			auto llevel = params [it].to <Value> ().getType ().to <Type> ().mutabilityLevel ();
			auto rlevel = funcType.getParamTypes ()[it].to <Type> ().mutabilityLevel () + 1;
			score += rlevel - llevel;
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

	generator::Generator CallVisitor::validateDelegate (const lexing::Word & location, const Generator & gen, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params;
	    std::vector <Generator> rights = rights_;
	    std::vector <Generator> paramTypes;
	    Generator retType (Generator::empty ());
	    std::string typeName;
	    if (gen.to <Value> ().getType ().to <Type> ().getInners ()[0].is <FuncPtr> ()) {
		auto funcType = gen.to <Value> ().getType ().to <Type> ().getInners ()[0].to <FuncPtr> ();
		typeName = funcType.getTypeName ();
		params = rights;
		rights = {};
		paramTypes = funcType.getParamTypes ();
		retType = funcType.getReturnType ();
		if (params.size () != paramTypes.size ()) return Generator::empty ();
	    } else {
		auto proto = gen.to <Value> ().getType ().to <Type> ().getInners ()[0];
		TRY (
		    if (proto.is <MethodProto> ()) {
			auto meth = proto.to <MethodProto> ();
			auto type = meth.getClassType ();
			type.to <Type> ().isMutable (meth.isMutable ());
			this-> _context.verifyImplicitAlias (location, type, gen.to <DelegateValue> ().getClosure ());
			auto llevel = gen.to <DelegateValue> ().getClosure ().to <Value> ().getType ().to <Type> ().mutabilityLevel ();
			auto rlevel = type.to <Type> ().mutabilityLevel () + 1;
			score += rlevel - llevel;
		    }
		    
		    if (proto.to <FrameProto> ().getParameters ().size () != rights.size ()) return Generator::empty ();
		    for (auto it : Ymir::r (0, proto.to <FrameProto> ().getParameters ().size ())) {
			auto param = findParameter (rights, proto.to <FrameProto> ().getParameters () [it].to<ProtoVar> ());
			if (param.isEmpty ()) return Generator::empty ();
			params.push_back (param);
			paramTypes.push_back (proto.to <FrameProto> ().getParameters ()[it].to <ProtoVar> ().getType ());
		    }
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors = msgs;
		} FINALLY;
		
		retType = proto.to <FrameProto> ().getReturnType ();
		if (errors.size () != 0) return Generator::empty ();	    
	    }
	    
	    if (rights.size () != 0) return Generator::empty ();
	    
		    
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, paramTypes.size ())) {
		{
		    bool succeed = true;
		    TRY (			
			this-> _context.verifyCompatibleType (
			    params [it].getLocation (),
			    paramTypes [it],
			    params [it].to <Value> ().getType ()
			);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors = {};
			succeed = false;
		    } FINALLY;
		    if (!succeed) return Generator::empty ();
		}
		
		{
		    TRY (		    
			this-> _context.verifyMemoryOwner (
			    params [it].getLocation (),
			    paramTypes [it],
			    params [it],
			    true
			);
			types.push_back (paramTypes [it]);
			score += Scores::SCORE_TYPE;
			auto llevel = params [it].to <Value> ().getType ().to <Type> ().mutabilityLevel ();
			auto rlevel = paramTypes [it].to <Type> ().mutabilityLevel () + 1;
			score += rlevel - llevel;

		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), it, typeName));
		    } FINALLY;
		}
	    }

	    if (errors.size () != 0) return Generator::empty ();	   
	    return Call::init (location,
			       retType,
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
		if (it.is <FrameProto> () || it.is <ConstructorProto> () || (it.is <Addresser> () && it.to<Addresser> ().getType ().is <FuncPtr> ()) || it.is <DelegateValue> ()) {
		    auto func = it;
		    if (it.is <Addresser> ()) func = it.to <Addresser> ().getWho ();
		    
		    auto gen = validate (location, func, rights_, current, local_errors);
		    if (!gen.isEmpty ()) nonTemplScores[current].push_back (gen);
		    if (!gen.isEmpty () && (current > score || fromTempl)) {// simple function can take the token on 1. less scored 2. every templates
			score = current;
			final_gen = gen;
			used_gen = it;
			fromTempl = false;
		    } else if (gen.isEmpty ()) {
			//local_errors.insert (local_errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.getLocation (), it.prettyString ()));
			insertCandidate (nbCand, errors, local_errors);
		    }
		} else if (it.is <TemplateRef> ()) {
		    Symbol _sym (Symbol::empty ());
		    Generator _proto_gen (Generator::empty ());
		    auto gen = validateTemplateRef (location, it, rights_, current, local_errors, _sym, _proto_gen);
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
			names.push_back (prettyName (it));
			
		    std::string note;
		    for (auto & it : element_on_scores-> second) {
			auto gen = this-> _context.validateMultSym (sym.getLocation (), {it});
			note += Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), realLocation (gen), prettyName (gen)) + '\n';
		    }
		    Ymir::Error::occurAndNote (location,
					       note,
					       ExternalError::get (SPECIALISATION_WORK_WITH_BOTH),
					       leftName,
					       names);

		}
		
		std::vector <std::string> local_errors;
		TRY (
		    this-> _context.validateTemplateSymbol (templSym, used_gen);		    
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    msgs.insert (msgs.begin (), Ymir::Error::createNoteOneLine ("% -> %", proto_gen.getLocation (), proto_gen.prettyString ()));
		    msgs.insert (msgs.begin (), Ymir::Error::createNote (location, ExternalError::get (IN_TEMPLATE_DEF)));
		    errors = msgs;
		    final_gen = Generator::empty ();
		} FINALLY;
		
	    } else if (!final_gen.isEmpty ()) {
		auto element_on_scores = nonTemplScores.find (score);
		if (element_on_scores-> second.size () != 1) {
		    std::string leftName = sym.getLocation ().str;
		    std::vector<std::string> names;
		    for (auto & it : rights_)
			names.push_back (prettyName (it));
			
		    std::string note;
		    for (auto & it : element_on_scores-> second)
			note += Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), realLocation (it), prettyName (it)) + '\n';
		    Ymir::Error::occurAndNote (location,
					       note,
					       ExternalError::get (SPECIALISATION_WORK_WITH_BOTH),
					       leftName,
					       names);

		}
	    }
	    return final_gen;
	}

	generator::Generator CallVisitor::validateTemplateRef (const lexing::Word & location, const Generator & ref, const std::vector <generator::Generator> & rights_, int & score, std::vector <std::string> & errors, Symbol & _sym, Generator & proto_gen) {
	    const Symbol & sym = ref.to <TemplateRef> ().getTemplateRef ();
	    if (!sym.to<semantic::Template> ().getDeclaration ().is <syntax::Function> ()) return Generator::empty ();
	    
	    std::vector <Generator> typeParams;
	    std::vector <Generator> valueParams;
	    std::vector <Generator> rights = rights_;
	    
	    if (ref.is <MethodTemplateRef> ())
		rights.insert (rights.begin (), ref.to <MethodTemplateRef> ().getSelf ());
	    
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
		    failure = true;
		} FINALLY;
		
		this-> _context.exitForeign (); // exiting the context to return to the context of the local frame
		if (failure) return Generator::empty ();
	    }

	    for (auto & it : rights) { // Add the rests, for variadic templates 
		typeParams.push_back (it.to <Value> ().getType ());
		valueParams.push_back (it);
	    }	    
	    
	    auto templateVisitor = TemplateVisitor::init (this-> _context);
	    std::vector <Generator> finalParams;
	    bool succeed = true;
	    
	    TRY (
		// The solution is a function transformed generated by template specialisation (if it succeed)
		proto_gen = templateVisitor.validateFromImplicit (ref, valueParams, typeParams, score, _sym, finalParams);		
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors.insert (errors.begin (), msgs.begin (), msgs.end ());
		succeed = false;
	    } FINALLY;
	    
	    if (succeed) {
		int _score;
		Generator ret (Generator::empty ());
		if (ref.is <MethodTemplateRef> ()) {
		    // Remove the first argument, (that is self)
		    finalParams = std::vector <Generator> (finalParams.begin () + 1, finalParams.end ());
		    auto self = ref.to <MethodTemplateRef> ().getSelf ();
		    auto delType = Delegate::init (proto_gen.getLocation (), proto_gen);
		    auto delValue = DelegateValue::init (proto_gen.getLocation(),
							 delType, proto_gen.to <MethodProto> ().getClassType (),
							 self, proto_gen);
		    ret = validateDelegate (location, delValue, finalParams, _score, errors);
		} else {
		    ret = validateFrameProto (location, proto_gen.to <FrameProto> (), finalParams, _score, errors);
		}
		score += _score;		
		return ret;
	    } 
	    
	    return Generator::empty ();	    
	}							   

	Generator CallVisitor::validateDotCall (const syntax::Expression & exp, std::vector <Generator> & params, const std::vector <std::string> & errors) {
	    Generator right (Generator::empty ());
	    Generator left (Generator::empty ());
	    if (!exp.is <syntax::Binary> () || exp.to <syntax::Binary> ().getLocation () != Token::DOT) THROW (ErrorCode::EXTERNAL, errors);
	    auto bin = exp.to <syntax::Binary> ();

	    TRY (		
		left = this-> _context.validateValue (bin.getLeft ());
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);			       
	    } FINALLY;

	    if (left.isEmpty ())  
		THROW (ErrorCode::EXTERNAL, errors);	    
	    else if (left.to <Value> ().getType ().is <ClassRef> ()) {
		volatile bool succ = false;
		std::vector <std::string> localErrors;
		TRY (
		    match (bin.getRight ()) {		    
			of (syntax::TemplateCall, cl, {
				auto loc = left.getLocation ();
				auto inner_bin = syntax::Binary::init (
				    {loc, Token::DOT},  TemplateSyntaxWrapper::init (loc, left),
				    cl.getContent (), Expression::empty ()
				);
				auto inner_value = this-> _context.validateValue (inner_bin);
				succ = true; // If we are here, then the class has field named cl.getContent ()
				
				auto n_bin = TemplateCall::init (				    
				    cl.getLocation (), cl.getParameters (),
				    TemplateSyntaxWrapper::init (inner_value.getLocation (), inner_value)
				);
				right = this-> _context.validateValue (n_bin);
			    }
			);
		    }
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    if (succ) { // If we failed, we failed after the DotOp, so the failure is due to template call
			localErrors = msgs;
		    }
		} FINALLY;
		
		if (succ && localErrors.size () != 0) { // Errors, when processing the template call
		    THROW (ErrorCode::EXTERNAL, localErrors);
		}
	    }

	    
	    if (right.isEmpty ()) {
		TRY (
		    right = this-> _context.validateValue (bin.getRight ());		    
		    params.push_back (left);
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);		    
		} FINALLY;

		if (right.isEmpty ())
		    THROW (ErrorCode::EXTERNAL, errors);
	    }
	    
	    return right;
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
		else of (ModuleAccess, acc, leftName = acc.prettyString ())
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
		else of (ConstructorProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())			 
		else of (MultSym,    sym,   leftName = sym.getLocation ().str)
		else of (ModuleAccess, acc, leftName = acc.prettyString ())
		else of (Value,      val,  leftName = val.getType ().to <Type> ().getTypeName ()
		);
	    }

	    OutBuffer buf;
	    for (auto & it : errors)
		buf.write (it, "\n");
	    
	    Ymir::Error::occurAndNote (
		location,
		end,
		buf.str (),
		ExternalError::get (UNDEFINED_CALL_OP),
		leftName,
		names
	    );
	    	    
	    //THROW (ErrorCode::EXTERNAL, errors);
	}

	std::string CallVisitor::prettyName (const Generator & gen) {
	    match (gen) {
		of (DelegateValue, dg, {
			return dg.getType ().to <Type> ().getInners ()[0].prettyString ();		    
		    }
		) else of (Call, cl, {
			return prettyName (cl.getFrame ());
		    }
		) else of (FrameProto, p, return p.prettyString ();
		) else of (ConstructorProto, c, return c.prettyString ();
		) else of (generator::Struct, str, return str.getName ()			 
		) else of (MultSym,    sym,   return sym.getLocation ().str
		) else of (ModuleAccess, acc, return acc.prettyString ()
		) else of (Value,      val,  return val.getType ().to <Type> ().getTypeName ());
	    }
	    return gen.getLocation().str;
	}

	lexing::Word CallVisitor::realLocation (const Generator & gen) {
	    match (gen) {
		of (DelegateValue, dg, {
			return dg.getType ().getLocation ();
		    }
		) else of (Call, cl, {
			return realLocation (cl.getFrame ());
		    }
		);
	    }
	    return gen.getLocation ();
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
