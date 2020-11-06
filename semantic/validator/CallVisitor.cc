#include <ymir/semantic/validator/CallVisitor.hh>
#include <ymir/semantic/validator/TemplateVisitor.hh>
#include <ymir/global/State.hh>
#include <chrono>

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
	    std::list <std::string> errors;
	    {
		try {
		    left = this-> _context.validateValue (expression.getLeft (), false, true);
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    std::vector <Generator> rights;
	    if (left.isEmpty () && expression.canBeDotCall ()) {
		left = this-> validateDotCall (expression.getLeft (), rights, errors);		
	    } else if (left.isEmpty ())
		throw Error::ErrorList {errors};

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

	generator::Generator CallVisitor::validate (const lexing::Word & location , const generator::Generator & left, const std::vector <generator::Generator> & rights, int & score, std::list <std::string> & errors) {
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
		Generator gen (Generator::empty ());
		
		if (left.is <TemplateClassCst> ())
		    gen = validateTemplateClassCst (location, left, rights, score, errors, sym, proto_gen);
		else
		    gen = validateTemplateRef (location, left, rights, score, errors, sym, proto_gen);
		
		if (!gen.isEmpty ()) {
		    std::list <std::string> local_errors;
		    try {
			Visitor::__CALL_NB_RECURS__ += 1;
			this-> _context.validateTemplateSymbol (sym, left);
		    } catch (Error::ErrorList list) {
			if (Visitor::__CALL_NB_RECURS__ == 3 && !global::State::instance ().isVerboseActive ()) {
			    list.errors.push_back (format ("     : %(B)", "..."));
			    list.errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (OTHER_CALL)));
			} else if (Visitor::__CALL_NB_RECURS__ <  3 || global::State::instance ().isVerboseActive ()) {
			    list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", proto_gen.getLocation (), proto_gen.prettyString ()));
			    list.errors.insert (list.errors.begin (), Ymir::Error::createNote (location, ExternalError::get (IN_TEMPLATE_DEF)));
			    Visitor::__LAST__ = true;
			} else if (Visitor::__LAST__) {			    
			    Visitor::__LAST__ = false;
			} else list.errors = {};
			
			errors = list.errors;
			gen = Generator::empty ();
		    } 
		    Visitor::__CALL_NB_RECURS__ -= 1;
		    return gen;
		}
	    }
	    
	    if (!left.is<MultSym> () && checked) 
		errors.insert (errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), realLocation (left), prettyName (left)));	    
	    return Generator::empty ();
	}
	
	generator::Generator CallVisitor::validateFrameProto (const lexing::Word & location, const FrameProto & proto, const std::vector <Generator> & rights_, int & score, std::list <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params;
	    std::vector <Generator> addParams;
	    std::vector <Generator> rights = rights_;
	    auto list = std::list <Generator> (rights.begin (), rights.end ());
	    try {
		for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		    auto param = findParameter (list, proto.getParameters () [it].to<ProtoVar> ());		    
		    if (param.isEmpty ()) return Generator::empty ();		    
		    params.push_back (param);
		}
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    } 
	    
	    if (errors.size () != 0) return Generator::empty ();	    
	    if (list.size () != 0 && !proto.isCVariadic ()) return Generator::empty ();
	    addParams = std::vector <Generator> (list.begin (), list.end ());
	    
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		{
		    bool succeed = true;
		    try {			
			this-> _context.verifyCompatibleType (
			    params [it].getLocation (),
			    proto.getParameters () [it].to <Value> ().getType (),
			    params [it].to <Value> ().getType ()
			);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto.getParameters () [it].to <Value> ().getLocation (), proto.prettyString ()));
			succeed = false;
		    } 
		    
		    if (!succeed) return Generator::empty ();
		}


		{
		    try {		    
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

		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto.getParameters () [it].to <Value> ().getLocation (), proto.prettyString ()));
		    } 
		}
	    }
	    
	    if (errors.size () != 0) return Generator::empty ();	   
	    return Call::init (location, proto.getReturnType (), proto.clone (), types, params, addParams);
	}

	generator::Generator CallVisitor::validateConstructorProto (const lexing::Word & location, const ConstructorProto & proto, const std::vector <Generator> & rights_, int & score, std::list <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params;
	    std::vector <Generator> rights = rights_;
	    auto list = std::list <Generator> (rights.begin (), rights.end ());
	    
	    try {
		for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		    auto param = findParameter (list, proto.getParameters () [it].to<ProtoVar> ());
		    if (param.isEmpty ()) return Generator::empty ();
		    params.push_back (param);
		}
	    } catch (Error::ErrorList list) {		
		errors = list.errors;
	    } 

	    if (errors.size () != 0) return Generator::empty ();	    
	    if (list.size () != 0) return Generator::empty ();
	    
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		{
		    bool succeed = true;
		    try {			
			this-> _context.verifyCompatibleType (
			    params [it].getLocation (),
			    proto.getParameters () [it].to <Value> ().getType (),
			    params [it].to <Value> ().getType ()
			);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto.getParameters () [it].to <Value> ().getLocation (), proto.prettyString ()));
			succeed = false;
		    } 
		    
		    if (!succeed) return Generator::empty ();
		}
		
		{
		    try {		    
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

		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto.getParameters () [it].to <Value> ().getLocation (), proto.prettyString ()));
		    } 
		}
	    }

	    if (errors.size () != 0) return Generator::empty ();	   
	    return ClassCst::init (location, proto.getReturnType (), proto.clone (), types, params);
	}	
       
	generator::Generator CallVisitor::findParameter (std::list <Generator> & params, const ProtoVar & var) {
	    if (var.getNbConsume () == 1) {
		Generator ret (Generator::empty ());
		int i = 0;
		for (auto it : params) {
		    if (it.is <NamedGenerator> ()) {
			auto name = it.to <NamedGenerator> ().getLocation ();
			if (name.str == var.getLocation ().str) {
			    auto toRet = it.to <NamedGenerator> ().getContent ();
			    ret = toRet;
			    break;
			}
		    }
		    i += 1;
		}

		if (ret.isEmpty ()) {
		    // If the var has a value, it is an optional argument
		    if (!var.getValue ().isEmpty ()) {
			i = -1;
			ret = var.getValue ();
			
			// If it does not have a value, it is a mandatory var, so we take the first param that is not a NamedExpression
			// No NamedExpression can have the same name as var, it is already checked in the first for loop
		    } else {
			i = 0;
			for (auto & it : params) {
			    if (!it.is<NamedGenerator> ()) {
				ret = it;				
				break;
			    }
			    i += 1;
			}
			if (ret.isEmpty ()) return Generator::empty ();
		    }
		}
		
		if (!ret.isEmpty ()) {

		    if (i != -1) {
			auto pt = params.begin ();
			std::advance (pt, i);
			params.erase (pt);
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
		}

		return ret;
	    } else {
		int i = 0;
		std::vector <Generator> tupleValues;
		std::vector <Generator> tupleTypes;
		std::list <Generator> rest = params;
		
		tupleTypes.reserve (var.getNbConsume ());
		tupleValues.reserve (var.getNbConsume ());
		int it = 0;
		
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

			auto pt = rest.begin ();
			std::advance (pt, it);
			rest.erase (pt);
			
			// Don't increment it, because we have remove a element
			i += 1;
			if (i == var.getNbConsume ()) break;
		    } else {
			it += 1;
		    }
		}
		
		params = rest;
				
		if (i == var.getNbConsume ()) {
		    auto tupleType = Tuple::init (tupleTypes [0].getLocation (), tupleTypes);
		    return TupleValue::init (tupleValues [0].getLocation (), tupleType, tupleValues);
		} else return Generator::empty ();
	    }
	}

	generator::Generator CallVisitor::validateLambdaProto (const lexing::Word & location, const LambdaProto & proto, const std::vector <Generator> & rights, int & score, std::list <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> types;
	    std::vector <Generator> params = rights;
	    if (rights.size () != proto.getParameters ().size ()) return Generator::empty ();
	    
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		if (proto.getParameters ()[it].to <Value> ().getType ().isEmpty ()) {
		    types.push_back (Type::init (rights [it].to<Value> ().getType ().to <Type> (), false, false));
		} else {		    
		    {
			bool succeed = true;
			try {
			    this-> _context.verifyCompatibleType (
				params [it].getLocation (),
				proto.getParameters () [it].to <Value> ().getType (),
				params [it].to <Value> ().getType ()
			    );
			} catch (Error::ErrorList list) {			   
			    succeed = false;
			} 
			if (!succeed) return Generator::empty ();
		    }
		    
		    {
			try {		    
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
			    
			} catch (Error::ErrorList list) {
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			    errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), proto.getParameters () [it].to <Value> ().getLocation (), proto.prettyString ()));
			} 
		    }
		}
	    }

	    if (errors.size () != 0) return Generator::empty ();
	    
	    try {
		auto gen = this-> _context.validateLambdaProto (proto, types);
		match (gen) {
		    of (Addresser, addr, // Simple proto, no closure
			return Call::init (location, addr.getWho ().to<FrameProto> ().getReturnType (), addr.getWho (), types, rights, {});
		    ) else of (DelegateValue, dl,
			       return Call::init (location, dl.getFuncPtr ().to <Addresser> ().getWho ().to <FrameProto> ().getReturnType (), gen, types, rights, {});
		    );
		}
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		return Generator::empty ();
	    } catch (Error::ErrorList list) {		
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 
	    return Generator::empty ();
	}

	
	generator::Generator CallVisitor::validateStructCst (const lexing::Word & location, const generator::Struct & str, const std::vector <Generator> & rights_, int & score, std::list <std::string> & errors) {
	    std::vector <Generator> params;
	    std::vector <Generator> rights = rights_;
	    std::vector <Generator> types;
	    
	    if (str.getRef ().to <semantic::Struct> ().isUnion ()) {
		lexing::Word name;
		try {
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
		} catch (Error::ErrorList list) {		    
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 		
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
	
	generator::Generator CallVisitor::validateFunctionPointer (const lexing::Word & location, const Generator & gen, const std::vector <Generator> & rights_, int & score, std::list <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params = rights_;
	    auto funcType = gen.to <Value> ().getType ().to <FuncPtr> ();
	    if (params.size () != funcType.getParamTypes ().size ()) return Generator::empty ();
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, funcType.getParamTypes ().size ())) {
		{
		    bool succeed = true;
		    try {
			this-> _context.verifyCompatibleType (
			    params [it].getLocation (),
			    funcType.getParamTypes () [it],
			    params [it].to <Value> ().getType ()
			);
		    } catch (Error::ErrorList list) {
			errors = {};
			succeed = false;
		    } 
		    
		    if (!succeed) return Generator::empty ();
		}
		
		{
		    try {		    
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
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), it, funcType.getTypeName ()));
		    } 
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

	generator::Generator CallVisitor::validateDelegate (const lexing::Word & location, const Generator & gen, const std::vector <Generator> & rights_, int & score, std::list <std::string> & errors) {
	    score = 0;
	    std::vector <Generator> params;
	    std::vector <Generator> rights = rights_;
	    std::vector <Generator> paramTypes;
	    Generator retType (Generator::empty ());
	    auto list = std::list <Generator> (rights.begin (), rights.end ());
	    
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
		    
		    if (proto.to <FrameProto> ().getParameters ().size () != rights.size ()) return Generator::empty ();		    
		    for (auto it : Ymir::r (0, proto.to <FrameProto> ().getParameters ().size ())) {
			auto param = findParameter (list, proto.to <FrameProto> ().getParameters () [it].to<ProtoVar> ());
			if (param.isEmpty ()) return Generator::empty ();
			params.push_back (param);
			paramTypes.push_back (proto.to <FrameProto> ().getParameters ()[it].to <ProtoVar> ().getType ());
		    }
		} catch (Error::ErrorList list) {		    
		    errors = list.errors;
		} 
		
		retType = proto.to <FrameProto> ().getReturnType ();
		if (errors.size () != 0) return Generator::empty ();
		if (list.size () != 0) return Generator::empty ();
	    }	    	    
		    
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, paramTypes.size ())) {
		{
		    bool succeed = true;
		    try {			
			this-> _context.verifyCompatibleType (
			    params [it].getLocation (),
			    paramTypes [it],
			    params [it].to <Value> ().getType ()
			);
		    } catch (Error::ErrorList list) {
			errors = {};
			succeed = false;
		    } 
		    if (!succeed) return Generator::empty ();
		}
		
		{
		    try {		    
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

		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PARAMETER_NAME), it, typeName));
		    } 
		}
	    }

	    if (errors.size () != 0)
		return Generator::empty ();
	    
	    return Call::init (location,
			       retType,
			       gen,
			       types,
			       params,
			       {}
	    );
	}
	
	generator::Generator CallVisitor::validateMultSym (const lexing::Word & location, const MultSym & sym, const std::vector <Generator> & rights_, int & score, std::list <std::string> & errors) {
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
		std::list <std::string> local_errors;
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
		    Generator gen (Generator::empty ());
		    if (it.is <TemplateClassCst> ())
			gen = validateTemplateClassCst (location, it, rights_, current, local_errors, _sym, _proto_gen);
		    else
			gen = validateTemplateRef (location, it, rights_, current, local_errors, _sym, _proto_gen);
		    if (!gen.isEmpty ()) templScores [current].push_back (_sym);
		    if (!gen.isEmpty () && current > score && fromTempl) { // Can only take the token over less scored template function
			score = current;
			final_gen = gen;
			used_gen = it;
			proto_gen = _proto_gen;
			fromTempl = true;
			templSym = _sym;
		    } else if (gen.isEmpty ()) {
			auto note = Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.to <TemplateRef> ().getTemplateRef ().getName (), it.prettyString ()) + "\n";
			for (auto & it : local_errors)
			    note = Error::addNote (location, note, it);
			insertCandidate (nbCand, errors, {note});			
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
			auto note = Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), it.to <TemplateRef> ().getTemplateRef ().getName (), it.prettyString ()) + "\n";
			for (auto & it : local_errors)
			    note = Error::addNote (location, note, it);
			insertCandidate (nbCand, errors, {note});
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
		
		std::list <std::string> local_errors;
		try {
		    Visitor::__CALL_NB_RECURS__ += 1;
		    this-> _context.validateTemplateSymbol (templSym, used_gen);		    
		} catch (Error::ErrorList list) {		    
		    auto note = Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), used_gen.to <TemplateRef> ().getTemplateRef ().getName (), used_gen.prettyString ()) + "\n";
		    for (auto & it : list.errors)			
			note = Error::addNote (location, note, it);
		    list.errors = {note};
		    if (Visitor::__CALL_NB_RECURS__ == 3 && !global::State::instance ().isVerboseActive ()) {
			list.errors.push_back (format ("     : %(B)", "..."));
			list.errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (OTHER_CALL)));
		    } else if (Visitor::__CALL_NB_RECURS__ <  3 || global::State::instance ().isVerboseActive ()) {
			list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", proto_gen.getLocation (), proto_gen.prettyString ()));
			list.errors.insert (list.errors.begin (), Ymir::Error::createNote (location, ExternalError::get (IN_TEMPLATE_DEF)));
			Visitor::__LAST__ = true;
		    } else if (Visitor::__LAST__) {			    
			Visitor::__LAST__ = false;
		    } else list.errors = {};
		    
		    errors = list.errors;
		    final_gen = Generator::empty ();
		} 
		Visitor::__CALL_NB_RECURS__ -= 1;
		
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

	generator::Generator CallVisitor::validateTemplateRef (const lexing::Word & location, const Generator & ref, const std::vector <generator::Generator> & rights_, int & score, std::list <std::string> & errors, Symbol & _sym, Generator & proto_gen) {
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
		    if (!var.getValue ().isEmpty ())
			value = this-> _context.validateValue (var.getValue ());
		    this-> _context.quitBlock ();

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
		println (proto_gen.prettyString ());
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
		    ret = validateFrameProto (location, proto_gen.to <FrameProto> (), finalParams, _score, errors);
		}
		score += _score;

		return ret;
	    }
	    
	    return Generator::empty ();	    
	}							   

	generator::Generator CallVisitor::validateTemplateClassCst (const lexing::Word & loc, const Generator & ref, const std::vector <generator::Generator> & rights_, int & score, std::list <std::string> & errors, Symbol & _sym, Generator & proto_gen) {
	    for (auto & it : ref.to <TemplateClassCst> ().getPrototypes ()) {
		int local_score = 0;
		auto gen = validateTemplateClassCst (loc, ref, it, rights_, local_score, errors, _sym);
		if (!gen.isEmpty ()) {
		    if (gen.is <ClassRef> ()) gen = Pointer::init (loc, gen);
		    auto bin = syntax::Binary::init ({loc, Token::DCOLON},
						     TemplateSyntaxWrapper::init (loc, gen),
						     Var::init ({loc, ClassRef::INIT_NAME}),
						     Expression::empty ());
		    
		    std::vector <syntax::Expression> params;
		    for (auto & it : rights_)
			params.push_back (TemplateSyntaxWrapper::init (it.getLocation (), it));
					  
		    auto expr = syntax::MultOperator::init ({loc, Token::LPAR}, {loc, Token::RPAR},
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
	
	generator::Generator CallVisitor::validateTemplateClassCst (const lexing::Word &, const Generator & ref, const syntax::Function::Prototype & prototype, const std::vector <generator::Generator> & rights_, int & score, std::list <std::string> & errors, Symbol & _sym) {
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
		    if (!var.getValue ().isEmpty ())
			value = this-> _context.validateValue (var.getValue ());
		    this-> _context.quitBlock ();

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
	
	Generator CallVisitor::validateDotCall (const syntax::Expression & exp, std::vector <Generator> & params, const std::list <std::string> & errors) {
	    Generator right (Generator::empty ());
	    Generator left (Generator::empty ());
	    if (!exp.is <syntax::Binary> () || exp.to <syntax::Binary> ().getLocation () != Token::DOT) throw Error::ErrorList {errors};
	    auto bin = exp.to <syntax::Binary> ();

	    try {		
		left = this-> _context.validateValue (bin.getLeft ());
	    } catch (Error::ErrorList list) {}       

	    if (left.isEmpty ())  
		throw Error::ErrorList {errors};	    
	    else if (left.to <Value> ().getType ().is <ClassRef> ()) {
		bool succ = false;
		std::list <std::string> localErrors;
		try {
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
		} catch (Error::ErrorList list) {		    
		    if (succ) { // If we failed, we failed after the DotOp, so the failure is due to template call
			localErrors = list.errors;
		    }
		} 
		
		if (succ && localErrors.size () != 0) { // Errors, when processing the template call
		    throw Error::ErrorList {localErrors};
		}
	    }

	    
	    if (right.isEmpty ()) {
		try {
		    right = this-> _context.validateValue (bin.getRight ());		    
		    params.push_back (left);
		} catch (Error::ErrorList list) {}	    		    		

		if (right.isEmpty ())
		    throw Error::ErrorList {errors};
	    }
	    
	    return right;
	}
	
	void CallVisitor::error (const lexing::Word & location, const Generator & left, const std::vector <Generator> & rights, std::list <std::string> & errors) {	    
	    std::list <std::string> names;
	    for (auto & it : rights)
		names.push_back (it.to <Value> ().getType ().to <Type> ().getTypeName ());

	    std::string leftName;
	    match (left) {
		of (FrameProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())
		    else of (MultSym,    sym,   leftName = sym.getLocation ().str)
			else of (TemplateRef, cl, leftName = cl.prettyString ())
			    else of (TemplateClassCst, cl, leftName = cl.prettyString ())
				else of (ModuleAccess, acc, leftName = acc.prettyString ())
				    else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ())
					     }


	    OutBuffer buf;
	    for (auto & it : errors)
		buf.write (it, "\n");
	    
	    Ymir::Error::occurAndNote (
		location,
		buf.str (), 
		ExternalError::get (UNDEFINED_CALL_OP),
		leftName,
		names
	    );
	    
	    
	    throw Error::ErrorList {errors};
	}

	void CallVisitor::error (const lexing::Word & location, const lexing::Word & end, const Generator & left, const std::vector <Generator> & rights, std::list <std::string> & errors) {	    
	    std::list <std::string> names;
	    for (auto & it : rights)
		names.push_back (it.to <Value> ().getType ().to <Type> ().getTypeName ());

	    std::string leftName;
	    match (left) {
		of (FrameProto, proto, leftName = proto.getName ())
		else of (ConstructorProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())			 
		else of (MultSym,    sym,   leftName = sym.prettyString ())
		else of (ModuleAccess, acc, leftName = acc.prettyString ())
		else of (TemplateRef, cl, leftName = cl.prettyString ())
		else of (TemplateClassCst, cl, leftName = cl.prettyString ())
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
	    	    
	    //throw Error::ErrorList {errors};
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
		) else of (MultSym,    sym,   return sym.prettyString ();
		) else of (ModuleAccess, acc, return acc.prettyString ()
		) else of (TemplateRef, cl, return cl.prettyString ()
		) else of (TemplateClassCst, cl, return cl.prettyString ()
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
	
	void CallVisitor::insertCandidate (int & nb, std::list <std::string> & errors, const std::list <std::string> & candErrors) {	    
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
