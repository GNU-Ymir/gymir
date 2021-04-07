#include <ymir/semantic/validator/FunctionVisitor.hh>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	
	
	FunctionVisitor::FunctionVisitor (Visitor & context) :
	    _context (context) 
	{}

	FunctionVisitor FunctionVisitor::init (Visitor & context) {
	    return FunctionVisitor (context);
	}
	
	void FunctionVisitor::validate (const semantic::Function & func) {
	    auto & function = func.getContent ();
	    std::vector <Generator> params;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    Generator retType (Generator::empty ());
	    
	    std::vector <Generator> throwers = this-> validateThrowers (func.getThrowers (), errors);

	    // We start by entering a specific context, with the custom attributes of the user
	    this-> _context.enterContext (function.getCustomAttributes ());
	    this-> _context.enterBlock (); // enter a new block for the validation of the prototype
	    this-> validatePrototypeForFrame (func.getName (), function.getPrototype (), params, retType, errors); 
	    if (function.getLocation () == Keys::MAIN) { // special case of main function, that must be validated with a specific prototype		
		this-> verifyMainPrototype (func.getName (), function.getPrototype (), params, retType, errors);
	    }	    
	    
	    this-> _context.setCurrentFuncType (retType); // used for the return statement
	    
	    bool needFinalReturn = false; // if true, the body is not a returner
	    auto body = this-> validateBody (func.getName (), func.getRealName (), function.getBody (), throwers, retType, needFinalReturn, errors); 
	    
	    try { // we enclose that in a try catch, because some vars may be unused
		this-> _context.quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	
		
	    this-> _context.exitContext (); // closing the context (custom user attributes)
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	    
	    if (!body.isEmpty ()) { // the function has a body, then we must insert a generator
		auto frame = Frame::init (function.getLocation (), func.getRealName (), params, retType, body, needFinalReturn);
		auto ln = func.getExternalLanguage ();
		if (ln == Keys::CLANG) 
		frame.to <Frame> ().setManglingStyle (Frame::ManglingStyle::C);
		else if (ln == Keys::CPPLANG)
		frame.to <Frame> ().setManglingStyle (Frame::ManglingStyle::CXX);

		frame.to <Frame> ().isWeak (func.isWeak ());
		frame.to <Frame> ().setMangledName (func.getMangledName ());

		this-> _context.insertNewGenerator (frame);
	    }
	}

	
	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================      BODY      ===============================
	 * ================================================================================
	 * ================================================================================
	 */


	Generator FunctionVisitor::validateBody (const lexing::Word & loc, const std::string & funcName, const syntax::Expression & bodyExpr, const std::vector <generator::Generator> & throwers, const Generator & retType, bool & needFinalReturn, std::list <Ymir::Error::ErrorMsg> & errors) {
	    if (!bodyExpr.isEmpty () && errors.empty ()) {
		try {
		    auto body = this-> _context.validateValue (bodyExpr);
		    needFinalReturn = this-> verifyFinalReturn (bodyExpr.getLocation (), retType, body);
		    this-> verifyThrowing (loc, funcName, body.getThrowers (), throwers, errors);
		    return body;
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    } else {
		// If the function has no body, it is normal that none of the parameters are used
		this-> _context.discardAllLocals ();
	    }
	    return Generator::empty ();
	}

	
	bool FunctionVisitor::verifyFinalReturn (const lexing::Word & loc, const generator::Generator & retType, const Generator & body) {
	    if (!body.to<Value> ().isReturner ()) {
		this-> _context.verifyMemoryOwner (loc, retType, body, true);
		return !retType.is<Void> ();
	    }
	    return false;
	}

	/**
	 * ================================================================================
	 * ================================================================================
	 * ================================    PROTO UTILS   ==============================
	 * ================================================================================
	 * ================================================================================
	 */

	void FunctionVisitor::verifyMainPrototype (const lexing::Word & loc, const syntax::Function::Prototype & proto,  const std::vector <Generator> & params, const generator::Generator & retType, std::list <Ymir::Error::ErrorMsg> & errors) {
	    if (params.size () > 1) {
		errors.push_back (Ymir::Error::makeOccur (params [1].getLocation (),
							  ExternalError::get (MAIN_FUNCTION_ONE_ARG)));		    
	    } else if (params.size () == 1) {
		auto argtype =  Slice::init (loc,
					     Slice::init (
						 loc,
						 Char::init (loc, 8) // The input given by the task launcher is in utf8
						 )
		    );

		try {
		    this-> _context.verifyCompatibleTypeWithValue (params [0].getLocation (), argtype, params [0]);
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    }

	    if (!retType.is<Void> ()) {
		auto realRetType = Integer::init (loc, 32, true); // do better 
		try {
		    this-> _context.verifyCompatibleType (params [0].getLocation (), loc, realRetType, retType);
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    }	    
	}


	void FunctionVisitor::validateParamDecl (const lexing::Word & loc, const syntax::VarDecl & var, bool no_value, Generator & type, Generator & value, bool & isMutable) {		    
	    if (!var.getType ().isEmpty ()) {
		try {
		    type = this-> _context.validateType (var.getType ());
		} catch (Ymir::Error::ErrorList list) {
		    Ymir::Error::occurAndNote (var.getType ().getLocation (), list.errors, "");
		}
	    }
		    		
	    if (!var.getValue ().isEmpty () && !no_value) {
		value = this-> _context.validateValue (var.getValue ());
		if (!type.isEmpty ()) 
		this-> _context.verifyCompatibleTypeWithValue (var.getLocation (), type, value);
		else {
		    type = value.to <Value> ().getType ();
		}
	    }

	    if (var.getType ().isEmpty () && no_value && !var.getValue ().isEmpty ()) {
		Ymir::Error::occur (var.getLocation (), ExternalError::get (FORWARD_REFERENCE_VAR));
	    }
				
	    bool isRef = false;
	    bool dmut = false;
	    type = this-> _context.applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut);
		    
	    this-> _context.verifyMutabilityRefParam (var.getLocation (), type, MUTABLE_CONST_PARAM);
		
	    if (!value.isEmpty ()) {		    
		this-> _context.verifyMemoryOwner (value.getLocation (), type, value, true);
	    }
	}


	Generator FunctionVisitor::validateReturnType (const lexing::Word & loc, const syntax::Expression & ret) {
	    if (!ret.isEmpty ()) {
		Generator retType (Generator::empty ());
		try {
		    retType = this-> _context.validateType (ret, true);
		} catch (Ymir::Error::ErrorList list) {
		    Ymir::Error::noteAndNote (ret.getLocation (), list.errors, "");
		}
		
		if (retType.to <Type> ().isRef ()) {
		    Ymir::Error::occur (retType.getLocation (), ExternalError::get (REF_RETURN_TYPE), retType.prettyString ());
		}
		return retType;
	    } else return Void::init (loc);
	}

	void FunctionVisitor::insertParameters (const std::vector <Generator> & params) {
	    // We insert them after validation to avoid cross referencing value of param
	    for (auto & param : params) {
		if (param.getName () != Keys::UNDER) {
		    this-> _context.verifyShadow (param.getLocation ());		
		    this-> _context.insertLocal (param.getLocation ().getStr (), param);
		}
	    }
	}


	Generator FunctionVisitor::validateFunctionType (const Generator & proto) {
	    auto params = proto.to <FrameProto> ().getParameters ();
	    auto ret = proto.to <FrameProto> ().getReturnType ();
	    std::vector <Generator> paramTypes;
	    for (auto & it : params) {
		paramTypes.push_back (it.to <generator::ProtoVar> ().getType ());
		bool isMut = false;
		bool isRef = paramTypes.back ().to <Type> ().isRef ();
		if (it.to <generator::ProtoVar> ().isMutable ()) isMut = true;
		if (it.to <generator::ProtoVar> ().getType ().to <Type> ().isRef ()) isRef = true;
		
		paramTypes.back () = Type::init (paramTypes.back ().to <Type> (), isMut, isRef);
	    }
	    
	    return FuncPtr::init (proto.getLocation (), ret, paramTypes);
	}

	
	/**
	 * ================================================================================
	 * ================================================================================
	 * ================================      PROTO      ===============================
	 * ================================================================================
	 * ================================================================================
	 */

	Generator FunctionVisitor::validateFunctionProto (const semantic::Symbol & fSym) {	    
	    if (fSym.to <semantic::Function> ().getGenerator ().isEmpty ()) {
		auto & func = fSym.to <semantic::Function> ();
		auto sym = fSym;
		std::vector <Generator> params;
		static std::list <lexing::Word> __validating__; 
		auto & function = func.getContent ();
		std::list <Ymir::Error::ErrorMsg> errors;
		bool no_value = false;
		Generator retType (Generator::empty ());

		for (auto func_loc : __validating__) {
		    // If there is a foward reference, we can't validate the values
		    if (func_loc.isSame (func.getName ())) no_value = true;		    
		}

		__validating__.push_back (func.getName ());
		this-> _context.enterForeign ();
		this-> _context.enterBlock ();
	    
		this-> validatePrototypeForProto (func.getName (), function.getPrototype (), no_value, params, retType, errors);
	    
		try {
		    this-> _context.discardAllLocals ();
		    this-> _context.quitBlock (errors.size () == 0);
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 	

		std::vector <Generator> throwers = this-> validateThrowers (func.getThrowers (), errors);	    
		__validating__.pop_back ();
		this-> _context.exitForeign ();

		if (errors.size () != 0) {
		    throw Error::ErrorList {errors};		
		}

		auto frame = FrameProto::init (function.getLocation (), func.getRealName (), retType, params, func.isVariadic (), func.isSafe (), throwers);
		auto ln = func.getExternalLanguage ();
		auto style = Frame::ManglingStyle::Y;
		if (ln == Keys::CLANG) style = Frame::ManglingStyle::C;
		else if (ln == Keys::CPPLANG) style = Frame::ManglingStyle::CXX;
	    
		frame = FrameProto::init (frame.to <FrameProto> (), func.getMangledName (), style);
		sym.to <semantic::Function> ().setGenerator (frame);
	    }
	    return fSym.to <semantic::Function> ().getGenerator ();
	}


	Generator FunctionVisitor::validateConstructorProto (const semantic::Symbol & cSym) {
	    if (cSym.to <Constructor> ().getGenerator ().isEmpty ()) {
		auto sym = cSym;
		auto & func = sym.to <Constructor> ();
		auto & function = func.getContent ();
		static std::list <lexing::Word> __validating__;
		    
		this-> _context.pushReferent (sym, "validateConstructorProto");
		this-> _context.enterForeign ();
	    
		std::vector <Generator> params;
		std::list <Ymir::Error::ErrorMsg> errors;
		bool no_value = false;
		Generator retType (Generator::empty ());
	    
		for (auto func_loc : __validating__) {
		    if (func_loc.isSame (func.getName ())) no_value = true;
		}

		Generator cl (Generator::empty ());
		try {
		    auto icl = this-> _context.validateClass (func.getClass ());
		    cl = Type::init (func.getClass ().getName (), ClassPtr::init (func.getClass ().getName (), Type::init (func.getClass ().getName (), icl.to <Type> (), true, false)).to <Type> (), true, false);
		} catch (Error::ErrorList list) {
		    errors = list.errors;
		} 
	    
		if (!cl.isEmpty ()) { // Error in the class validation	
		    __validating__.push_back (func.getName ());
		    this-> _context.enterBlock ();
		
		    this-> _context.insertLocal (Keys::SELF, ProtoVar::init (func.getName (), cl, Generator::empty (), true, 1, true));
		    this-> validatePrototypeForProto (func.getName (), function.getPrototype (), no_value, params, retType, errors);
		
		    try {
			this-> _context.discardAllLocals ();
			this-> _context.quitBlock (errors.size () == 0);
		    } catch (Error::ErrorList list) {
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    } 	    

		    __validating__.pop_back ();
		}
	    
		std::vector <Generator> throwers = this-> validateThrowers (func.getThrowers (), errors);
		this-> _context.exitForeign ();
		this-> _context.popReferent ("validateConstructorProto");
	    	    
		if (errors.size () != 0) {
		    throw Error::ErrorList {errors};		
		}
	    
		auto frame = ConstructorProto::init (func.getName (), func.getRealName (), sym, cl, params, throwers);	    
		frame = ConstructorProto::init (frame.to <ConstructorProto> (), func.getMangledName ());
		sym.to <Constructor> ().setGenerator (frame);
	    }
	    
	    return cSym.to <Constructor> ().getGenerator ();
	}

	Generator FunctionVisitor::validateMethodProto (const semantic::Function & func, const Generator & classType_, const Generator & trait) {
	    // we can't store the prototype, because is depends on classType (for inheritence, of impl), but it is stored inside the vtable, so validated only once
	    std::vector <Generator> params; 
	    static std::list <lexing::Word> __validating__;
	    auto & function = func.getContent ();
	    std::list <Ymir::Error::ErrorMsg> errors;
	    bool no_value = false;
	    Generator retType (Generator::empty ());	    
	    for (auto func_loc : __validating__) {
		if (func_loc.isSame (func.getName ())) no_value = true;
	    }
	    
	    auto classType = Type::init (function.getLocation (), ClassPtr::init (function.getLocation (), classType_).to <Type> ().toDeeplyMutable ().to <Type> (), true, false);
	    __validating__.push_back (func.getName ());
	    
	    this-> _context.enterForeign ();	    
	    this-> _context.enterBlock ();
	    this-> _context.insertLocal (Keys::SELF, ProtoVar::init (func.getName (), classType, Generator::empty (), true, 1, true));

	    auto & __params = function.getPrototype ().getParameters ();
	    auto fakeParams = std::vector <syntax::Expression> (__params.begin () + 1, __params.end ());
	    auto proto = syntax::Function::Prototype::init (fakeParams, function.getPrototype ().getType (), false);
	    this-> validatePrototypeForProto (func.getName (), proto, no_value, params, retType, errors);
	    
	    try {
		this-> _context.discardAllLocals ();
		this-> _context.quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	    

	    std::vector <Generator> throwers = this-> validateThrowers (func.getThrowers (), errors);
	    
	    __validating__.pop_back ();
	    this-> _context.exitForeign ();
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};		
	    }

	    if (!function.getPrototype ().getParameters ()[0].to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT))
	    classType = Type::init (classType.getLocation (), classType.to <Type> (), false, false);
	    
	    auto frame = MethodProto::init (function.getLocation (), func.getComments (), func.getRealName (), retType, params, false,
					    classType,
					    function.getPrototype ().getParameters ()[0].to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT), function.getBody ().isEmpty (), func.isFinal (), func.isSafe (), trait, throwers);
	    
	    return FrameProto::init (frame.to <FrameProto> (), func.getMangledName (), Frame::ManglingStyle::Y);	    
	}
		
	
	void FunctionVisitor::validatePrototypeForFrame (const lexing::Word & loc, const syntax::Function::Prototype & proto,  std::vector <Generator> & params, generator::Generator & retType, std::list <Ymir::Error::ErrorMsg> & errors) {
	    std::vector <Generator> addedParams;
	    addedParams.reserve (proto.getParameters ().size ());
	    for (auto & param : proto.getParameters ()) {
		try {
		    auto var = param.to <syntax::VarDecl> ();
		    Generator type (Generator::empty ());
		    Generator value (Generator::empty ());
		    auto isMutable = false;
		    this-> validateParamDecl (param.getLocation (), var, false, type, value, isMutable);

		    if (type.is <NoneType> () || type.is<Void> ()) {
			Ymir::Error::occur (var.getLocation (), ExternalError::get (VOID_VAR));
		    } else if (type.is <generator::LambdaType> ()) {
			Ymir::Error::occur (type.getLocation (), ExternalError::get (INCOMPLETE_TYPE), type.prettyString ());
		    }
		
		    addedParams.push_back (ParamVar::init (var.getName (), type, isMutable, false));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    try {
		this-> insertParameters (addedParams);
		params.insert (params.end (), addedParams.begin (), addedParams.end ());
		retType = this-> validateReturnType (loc, proto.getType ());
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }
	    
	}	
	
	void FunctionVisitor::validatePrototypeForProto (const lexing::Word & loc, const syntax::Function::Prototype & proto, bool no_value, std::vector <Generator> & params, generator::Generator & retType, std::list <Ymir::Error::ErrorMsg> & errors) {
	    std::vector <Generator> addedParams;
	    addedParams.reserve (proto.getParameters ().size ());
	    
	    for (auto & param : proto.getParameters ()) {
		try {
		    auto var = param.to <syntax::VarDecl> ();
		    Generator type (Generator::empty ()), value (Generator::empty ());
		    bool isMutable = false;
		    this-> validateParamDecl (param.getLocation (), var, no_value, type, value, isMutable);
		    
		    if (type.is <NoneType> () || type.is<Void> ()) {
			Ymir::Error::occur (var.getLocation (), ExternalError::get (VOID_VAR));
		    }
		    
		    if (type.is <generator::LambdaType> ()) {
			Ymir::Error::occur (type.getLocation (), ExternalError::get (INCOMPLETE_TYPE), type.prettyString ());
		    }

		    int nb_consumed = 1;
		    if (!var.getType ().isEmpty () && var.getType ().is <TemplateSyntaxList> ()) {
			nb_consumed = var.getType ().to <TemplateSyntaxList> ().getContents ().size ();
		    }
		    
		    addedParams.push_back (ProtoVar::init (var.getName (), type, value, isMutable, nb_consumed, false));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    errors.push_back (Ymir::Error::createNote (param.getLocation ()));
		} 
	    }

	    try {
		this-> insertParameters (addedParams);
		params.insert (params.end (), addedParams.begin (), addedParams.end ());	    	    
		retType = this-> validateReturnType (loc, proto.getType ());
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }
	}

	/**
	 * ================================================================================
	 * ================================================================================
	 * ==============================      THROWING      ==============================
	 * ================================================================================
	 * ================================================================================
	 */	    	

	
	void FunctionVisitor::verifyThrowing (const lexing::Word & loc, const std::string & funcName, const std::vector <Generator> & throwers, const std::vector <Generator> & asked, std::list <Ymir::Error::ErrorMsg> & errors) {
	    std::vector <Generator> unused, notfound;		    
	    this-> computeThrows (throwers, asked, unused, notfound);

	    std::vector <Generator> types;
	    std::vector <Error::ErrorMsg> msg_types;
	    for (auto & it : notfound) {
		bool found = false;
		for (auto jt : Ymir::r (0, types.size ())) {
		    if (it.to <Type> ().isCompatible (types [jt])) {
			found = true;				
			msg_types[jt].addNote (Ymir::Error::createNote (it.getLocation ()));
		    }
		}
		if (!found) {
		    auto note = Ymir::Error::createNote (it.getLocation ());
		    auto err = Error::makeOccurAndNote (loc, note, ExternalError::get (THROWS_NOT_DECLARED), funcName, it.prettyString ());
		    err.setWindable (true);
		    msg_types.push_back (err);
		    types.push_back (it);
		}
	    }
	    
	    errors.insert (errors.end (), msg_types.begin (), msg_types.end ());			
		    
	    for (auto & it : unused) {
		auto note = Ymir::Error::createNote (it.getLocation ());
		errors.push_back (Error::makeOccurAndNote (loc, note, ExternalError::get (THROWS_NOT_USED), funcName, it.prettyString ()));
	    }
	}
	
	void FunctionVisitor::computeThrows (const std::vector <Generator> & types, const std::vector <Generator> & rethrow, std::vector <Generator> & unused, std::vector <Generator> & notfound) {	    
	    std::vector <Generator> used;
	    for (auto &it : types) {
		bool found = false;
		for (auto & j : used) {
		    if (it.to<Type> ().isCompatible (j)) {
			found = true;
			break;
		    }
		}
		
		if (!found) {
		    for (auto &j : rethrow) {
			if (it.to<Type> ().isCompatible (j)) {
			    found = true;
			    used.push_back (j);
			    break;
			}
		    }
		    
		    if (!found) notfound.push_back (it);
		}
	    }

	    for (auto & it : rethrow) {
		bool found = false;
		for (auto & j : used) {
		    if (it.to <Type> ().isCompatible (j)) {
			found = true;
			break;
		    }
		}
		if (!found) unused.push_back (it);
	    }
	}
	
	std::vector <Generator> FunctionVisitor::validateThrowers (const std::vector <syntax::Expression> & throwers, std::list <Error::ErrorMsg> & errors) {
	    std::vector <Generator> rets;
	    for (auto &it : throwers) {
		try {
		    rets.push_back (Generator::init (it.getLocation (), this-> _context.validateType (it)));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    return rets;
	}       
	
	/**
	 * ================================================================================
	 * ================================================================================
	 * ==============================     CONSTRUCTOR    ==============================
	 * ================================================================================
	 * ================================================================================
	 */
	
	void FunctionVisitor::validateConstructor (const semantic::Symbol & sym, const Generator & classType_, const Generator & ancestor, const std::vector <Generator> & ancestorFields) {
	    auto & cs = sym.to <Constructor> ();
	    auto constr = cs.getContent ();
	    std::vector <Generator> params;
	    Generator retType (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto classType = classType_;
	    
	    auto proto = this-> validateConstructorProto (sym);
	    this-> verifyConstructionLoop (proto.getLocation (), proto);

	    auto currentClassDef = classType_.to <ClassRef> ().getRef ();
	    this-> _context.enterClassDef (currentClassDef);
	    this-> _context.enterContext (cs.getCustomAttributes ());
	    
	    classType = Type::init (proto.getLocation (), ClassPtr::init (proto.getLocation (), Type::init (proto.getLocation (), classType.to <Type> (), true, false)).to <Type> (), true, false);
	    this-> _context.enterForeign ();
	    
	    std::vector <Generator> throwers = this-> validateThrowers (constr.getThrowers (), errors);
	    
	    
	    this-> _context.enterBlock ();
	    this-> validatePrototypeForFrame (cs.getName (), constr.getPrototype (), params, retType, errors);
	    retType = classType.to <ClassPtr> ().getInners ()[0].to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <Value> ().getType ();
	    params.insert (params.begin (), ParamVar::init (cs.getName (), classType, true, true));
	    this-> _context.insertLocal (params [0].getName (), params [0]);


	    Generator body (Generator::empty ());	    
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

	    this-> verifyThrowing (sym.getName (), sym.getRealName (), body.getThrowers (), throwers, errors);
	    	    
	    try {
		this-> _context.quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	    
	    
	    this-> _context.exitForeign ();
	    this-> _context.exitContext ();
	    this-> _context.exitClassDef (currentClassDef);
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	    
	    auto frame = Frame::init (constr.getLocation (), cs.getRealName (), params, classType, body, false);
	    frame.to <Frame> ().setMangledName (cs.getMangledName ());
	    frame.to <Frame> ().isWeak (cs.isWeak ());
	    this-> _context.insertNewGenerator (frame);
	}


	generator::Generator FunctionVisitor::validatePreConstructor (const semantic::Constructor & cs, const Generator & classType, const Generator & ancestor, const std::vector<Generator> & ancestorFields) {
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

	void FunctionVisitor::verifyConstructionLoop (const lexing::Word & location, const Generator & call) {
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


	    this-> validatePrototypeForFrame (cs.getName (), cs.getContent ().getPrototype (), params, retType, errors);
	    retType = clRef.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ().to <Value> ().getType ();
	    params.insert (params.begin (), ParamVar::init (cs.getName (), clRef, true, true));
	    this-> _context.insertLocal (params [0].getName (), params [0]);
	    
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
		    this-> _context.quitBlock (errors.size () == 0);
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
	 * ==============================        METHOD      ==============================
	 * ================================================================================
	 * ================================================================================
	 */

	
	void FunctionVisitor::validateMethod (const semantic::Function & func, const Generator & classType_, bool isWeak) {
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

	    std::vector <Generator> throwers = this-> validateThrowers (function.getThrowers (), errors);  
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
		
		this-> validatePrototypeForFrame (cs.getName (), proto, params, retType, errors);
		if (retType.isEmpty ()) retType = Void::init (func.getName ());
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    } 
	    
	    bool needFinalReturn = false;
	    Generator body = this-> validateBody (func.getName (), func.getRealName (), function.getBody (), throwers, retType, needFinalReturn, errors);
	    	    
	    try {
		if (errors.size () != 0) {
		    this-> _context.discardAllLocals ();
		}		    
		this-> _context.quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
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


    	/**
	 * ================================================================================
	 * ================================================================================
	 * ==============================         DTOR       ==============================
	 * ================================================================================
	 * ================================================================================
	 */

	void FunctionVisitor::validateDestructor (const semantic::Function & func, const Generator & classType_, const Generator & ancDtorProto) {
	    auto function = func.getContent ();
	    std::vector <Generator> params;
	    Generator retType (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    auto classType = classType_;
	    auto & cs = classType.to <ClassRef> ().getRef ().to <semantic::Class> ();
	    auto currentClassDef = classType.to <ClassRef> ().getRef ();
	    this-> _context.enterClassDef (currentClassDef);
	    auto attrs = function.getCustomAttributes ();
	    
	    attrs.push_back (lexing::Word::init (func.getName (), Keys::DTOR));
	    this-> _context.enterContext (attrs);
	    
	    classType = Type::init (function.getLocation (), ClassPtr::init (function.getLocation (), classType).to <Type> ().toDeeplyMutable ().to <Type> (), true, false);
	    this-> _context.enterForeign ();
	    
	    std::vector <Generator> throwers = this-> validateThrowers (function.getThrowers (), errors);  
	    this-> _context.enterBlock ();
	    
	    auto & __params = function.getPrototype ().getParameters ();
	    
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
		
		classType = Type::init (__params [0].getLocation (), classType.to <Type> (), isMutable, false);		
		params.insert (params.begin (), ParamVar::init (lexing::Word::init (__params [0].getLocation (), Keys::SELF), classType, isMutable, true));
		this-> _context.insertLocal (params [0].getName (), params [0]);		

		auto fakeParams = std::vector <syntax::Expression> (__params.begin () + 1, __params.end ());
		auto proto = syntax::Function::Prototype::init (fakeParams, function.getPrototype ().getType (), false);
		
		this-> validatePrototypeForFrame (cs.getName (), proto, params, retType, errors);
		if (retType.isEmpty ()) retType = Void::init (func.getName ());
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    } 
	    
	    bool needFinalReturn = false;
	    Generator body = this-> validateBody (func.getName (), func.getRealName (), function.getBody (), throwers, retType, needFinalReturn, errors);
	    
	    if (!ancDtorProto.isEmpty () && !ancDtorProto.is <NullValue> ()) {
		auto self = this-> _context.validateValue (syntax::Var::init (lexing::Word::init (__params [0].getLocation (), Keys::SELF)));
		auto ancCall = Call::init (func.getName (), Void::init (func.getName ()), ancDtorProto, {classType}, {self}, {});
		body = Block::init (func.getName (), Void::init (func.getName ()), {body, ancCall}); 
	    }		    
	    
	    try {
		if (errors.size () != 0) {
		    this-> _context.discardAllLocals ();
		}		    
		this-> _context.quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	
	    
	    this-> _context.exitForeign ();
	    this-> _context.exitContext ();
	    this-> _context.exitClassDef (currentClassDef);

	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};
		
	    auto frame = Frame::init (function.getLocation (), func.getRealName (), params, retType, body, needFinalReturn);
	    frame.to <Frame> ().isWeak (func.isWeak ());
	    frame.to <Frame> ().setMangledName (func.getMangledName ());

	    this-> _context.insertNewGenerator (frame);		

	    
	}
	
    }

    
}
