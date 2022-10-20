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

	Generator Visitor::validateDollar (const syntax::Dollar & dl) {
	    auto loc = dl.getLocation ();
	    if (this-> _dollars.size () != 0) {
	    	auto left = this-> _dollars.back ();
	    	if (left.to <Value> ().getType ().is <Slice> ()) {
		    auto strLit = this-> isStringLiteral (left);
		    if (!strLit.isEmpty ()) return ufixed (strLit.to <StringValue> ().getLen () - 1);
	    	    return StructAccess::init (loc,
	    				       Integer::init (dl.getLocation (), 0, false),
	    				       left, Slice::LEN_NAME
	    		);
	    	} else if (left.to <Value> ().getType ().is <Array> ()) {
	    	    return ufixed (left.to <Value> ().getType ().to <Array> ().getSize ());
	    	} else if (left.to <Value> ().getType ().is <ClassPtr> ()) {
	    	    auto leftSynt = TemplateSyntaxWrapper::init (loc, left);
	    	    auto bin = syntax::Binary::init (
	    		lexing::Word::init (loc, Token::DOT),
	    		leftSynt,
	    		syntax::Var::init (lexing::Word::init (loc, CoreNames::get (DOLLAR_OP_OVERRIDE))),
	    		syntax::Expression::empty ()
	    		);

	    	    auto call = syntax::MultOperator::init (
	    		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
	    		bin,
	    		{}, false
			);
		    
	    	    return validateValue (call);
	    	} 		  
	    }
	    
	    Ymir::Error::occur (dl.getLocation (), ExternalError::DOLLAR_OUSIDE_CONTEXT);
	    return Generator::empty ();
	}

	Generator Visitor::validateFixed (const syntax::Fixed & fixed, int base) {
	    struct Anonymous {

		static std::string removeUnder (const std::string & value) {
		    auto aux = value;
		    aux.erase (std::remove (aux.begin (), aux.end (), '_'), aux.end ());
		    return aux;
		}
		
		static uint64_t convertU (const lexing::Word & loc, const Integer & type, int base) { 
		    char * temp = nullptr; errno = 0; // errno !!
		    auto val = removeUnder (loc.getStr ());
		    if (val.length () > 2 && val [0] == '0' && val [1] == Keys::LX[0]) {
			val = val.substr (2, val.length () - 1);
			base = 16;
		    } else if (val.length () > 2 && val [0] == '0' && val [1] == 'o') {
			val = val.substr (2, val.length () - 1);
			base = 8;
		    }
		    
		    uint64_t value = std::strtoull (val.c_str (), &temp, base);
		    bool overflow = false;
		    uint64_t maxU = CompileTime::maxULimit (type.getSize ());
		    uint64_t maxAll = CompileTime::maxULimit (64);
		    
		    if (temp == val.c_str () || *temp != '\0' ||
			((value == 0 || value == maxAll) && errno == ERANGE)) {
			overflow = true;
		    }
		    
		    if (overflow || (value > maxU && maxU != 0)) {
			Error::occur (loc, ExternalError::OVERFLOW_, type.getTypeName (), val);
		    }
		    
		    return value;
		}
		
		static int64_t convertS (const lexing::Word & loc, const Integer & type, int base) {
		    char * temp = nullptr; errno = 0; // errno !!
		    auto val = removeUnder (loc.getStr ());
		    if (val.length () > 2 && val [0] == '0' && val [1] == Keys::LX[0]) {			
			val = val.substr (2, val.length () - 1);
			base = 16;
		    } else if (val.length () > 2 && val [0] == '0' && val [1] == 'o') {
			val = val.substr (2, val.length () - 1);
			base = 8;
		    }

		    uint64_t value = std::strtoll (val.c_str (), &temp, base);
		    bool overflow = false;
		    uint64_t maxI = CompileTime::maxULimit (type.getSize ());
		    uint64_t maxAll = CompileTime::maxULimit (64);
		    
		    if (temp == val.c_str () || *temp != '\0' ||
			((value == 0 || value == maxAll) && errno == ERANGE)) {
			overflow = true;
		    }
		    
		    if (overflow || (value > maxI && maxI != 0)) {
			Error::occur (loc, ExternalError::OVERFLOW_, type.getTypeName (), val);
		    }
		    
		    return value;
		}		
	    };

	    Generator type (Generator::empty ());
	    if (fixed.getSuffix () == Keys::U8) type = Integer::init (fixed.getLocation (), 8, false);
	    if (fixed.getSuffix () == Keys::U16) type = Integer::init (fixed.getLocation (), 16, false);
	    if (fixed.getSuffix () == Keys::U32) type = Integer::init (fixed.getLocation (), 32, false);
	    if (fixed.getSuffix () == Keys::U64) type = Integer::init (fixed.getLocation (), 64, false);
	    if (fixed.getSuffix () == Keys::USIZE) type = Integer::init (fixed.getLocation (), 0, false);
	    
	    if (fixed.getSuffix () == Keys::I8) type = Integer::init (fixed.getLocation (), 8, true);
	    if (fixed.getSuffix () == Keys::I16) type = Integer::init (fixed.getLocation (), 16, true);
	    if (fixed.getSuffix () == Keys::I32 || fixed.getSuffix () == "") type = Integer::init (fixed.getLocation (), 32, true);
	    if (fixed.getSuffix () == Keys::I64) type = Integer::init (fixed.getLocation (), 64, true);
	    if (fixed.getSuffix () == Keys::ISIZE) type = Integer::init (fixed.getLocation (), 0, true);
		
	    auto integer = type.to<Integer> ();
	    //type.to <Type> ().isMutable (true);
	    Fixed::UI value;
	    
	    if (integer.isSigned ()) value.i = Anonymous::convertS (fixed.getLocation (), integer, base);
	    else value.u = Anonymous::convertU (fixed.getLocation (), integer, base);	    
	    
	    return Fixed::init (fixed.getLocation (), type, value);
	}       
	
	Generator Visitor::validateBool (const syntax::Bool & b) {
	    return BoolValue::init (b.getLocation (), Bool::init (b.getLocation ()), b.getLocation () == Keys::TRUE_);
	}

	Generator Visitor::validateFloat (const syntax::Float & f) {
	    Generator type (Generator::empty ());
	    struct Anonymous {
		
		static std::string removeUnder (const std::string & value) {
		    auto aux = value;
		    aux.erase (std::remove (aux.begin (), aux.end (), '_'), aux.end ());
		    return aux;
		}
		
	    };
	    
	    if (f.getSuffix () == Keys::FLOAT_S) type = Float::init (f.getLocation (), 32);
	    else {
		type = Float::init (f.getLocation (), 64);
	    }
	    
	    return FloatValue::init (f.getLocation (), type, Anonymous::removeUnder (f.getValue ()));
	}

	Generator Visitor::validateChar (const syntax::Char & c) {	  	    
	    Generator type (Generator::empty ());	    
	    if (c.getSuffix () == Keys::C8) type = Char::init (c.getLocation (), 8);
	    else type = Char::init (c.getLocation (), 32);

	    auto visitor = UtfVisitor::init (*this);
	    
	    uint32_t value = visitor.convertChar (c.getLocation (), c.getSequence (), type.to<Char> ().getSize ());	   
	    return CharValue::init (c.getLocation (), type, value);
	}

	Generator Visitor::validateString (const syntax::String & str, bool forceUtf8) {
	    Generator inner (Generator::empty ());
	    if (str.getSuffix () == Keys::S8 || forceUtf8) inner = Char::init (str.getLocation (), 8);
	    else inner = Char::init (str.getLocation (), 32);
	    inner = Type::init (inner.to <Type> (), false);
	    
	    auto visitor = UtfVisitor::init (*this);
	    int len = 0;
	    auto value = visitor.convertString (str.getLocation (), str.getSequence (), inner.to <Char> ().getSize (), len);

	    auto type = Array::init (str.getLocation (), inner, len);
	    type = Type::init (type.to <Type> (), true);

	    auto sliceType = Slice::init (str.getLocation (), inner);
	    sliceType = Type::init (sliceType.to <Type> (), true);
	    
	    return Aliaser::init (
		str.getLocation (),
		sliceType, 
		StringValue::init (str.getLocation (), type, value, len)
		);
	}
	
	Generator Visitor::validateArray (const syntax::List & list) {
	    std::vector <Generator> params;
	    Generator innerType (Generator::empty ());
	    for (auto it : list.getParameters ()) {		
		auto val = validateValueNonVoid (it);
		if (val.is<List> ()) {
		    for (auto & g_it : val.to <List> ().getParameters ()) {
			params.push_back (g_it);
			if (innerType.isEmpty ()) {
			    innerType = params [0].to <Value> ().getType ();
			    try {
				verifyImplicitAlias (params [0].getLocation (), innerType, params [0]);
			    } catch (Error::ErrorList list) {
				innerType = Type::init (innerType.to<Type> (), false);
			    }
			} else {
			    innerType = this-> inferTypeBranchingWithValue (params.back ().getLocation (), params [0].getLocation (), params.back (), innerType);
			}
			//verifyMemoryOwner (params.back ().getLocation (), params [0].to <Value> ().getType (), params.back (), false);
		    }
		} else {
		    params.push_back (val);
		    if (innerType.isEmpty ()) {			
			innerType = params [0].to <Value> ().getType ();
			try {
			    verifyImplicitAlias (params [0].getLocation (), innerType, params [0]);
			} catch (Error::ErrorList list) {
			    innerType = Type::init (innerType.to <Type> (), false);
			}
		    } else {
			innerType = this-> inferTypeBranchingWithValue (params.back ().getLocation (), params [0].getLocation (), params.back (), innerType);
		    }
		    //verifyMemoryOwner (params.back ().getLocation (), params [0].to <Value> ().getType (), params.back (), false);
		}
	    }

	    if (innerType.isEmpty ()) {
		innerType = Void::init (list.getLocation ());
	    }
	    	    
	    innerType = Type::init (innerType.to <Type> (), innerType.to <Type> ().isMutable (), false);
	    
	    // An array literal is always static
	    auto type = Array::init (list.getLocation (), innerType, params.size ());
	    type = Type::init (type.to <Type> (), true); // Array constant are mutable by default (not lvalue), to ease simple affectation
	    innerType = Type::init (innerType.to <Type> (), true);
	    
	    auto slc = Slice::init (list.getLocation (), innerType);
	    slc = Type::init (slc.to <Type> (), true);
	    
	    if (!innerType.is <Void> ()) {
		return Copier::init (list.getLocation (),
				     slc,
				     Aliaser::init (list.getLocation (), slc,
						    ArrayValue::init (list.getLocation (), type.to <Type> ().toDeeplyMutable (), params)
					 )
		    );
	    } else
	    return
		Aliaser::init (list.getLocation (), slc,
			       ArrayValue::init (list.getLocation (), type.to <Type> ().toDeeplyMutable (), params)
		    );
	}	
	
	Generator Visitor::validateTuple (const syntax::List & list) {
	    std::vector <Generator> params;
	    std::vector <Generator> types;
	    for (auto it : list.getParameters ()) {
		auto val = validateValue (it, false, false);
		if (val.is <List> ()) {
		    for (auto & g_it : val.to<List> ().getParameters ()) {
			if (g_it.to <Value> ().getType ().is<NoneType> () || g_it.to <Value> ().getType ().is <Void> ()) {
			    Ymir::Error::occur (g_it.getLocation (), ExternalError::VOID_VALUE);
			}
			
			params.push_back (g_it);
			auto type = params.back ().to <Value> ().getType ();
			try {
			    verifyMemoryOwner (params.back ().getLocation (), type, params.back (), false);
			    types.push_back (Type::init (type.to <Type> (), type.to <Type> ().isMutable (), false));
			} catch (Error::ErrorList ATTRIBUTE_UNUSED lst) { // maybe there was an implicit alias, that is not a problem
			    type = Type::init (type.to <Type> (), false); // we just put it as a const one
			    verifyMemoryOwner (params.back ().getLocation (), type, params.back (), false);
			    types.push_back (Type::init (type.to <Type> (), type.to <Type> ().isMutable (), false));
			}
		    }
		} else {
		    if (val.to <Value> ().getType ().is<NoneType> () || val.to <Value> ().getType ().is <Void> ()) {
			Ymir::Error::occur (val.getLocation (), ExternalError::VOID_VALUE);
		    }
		    params.push_back (val);
		    auto type = params.back ().to <Value> ().getType ();
		    try {
			verifyMemoryOwner (params.back ().getLocation (), type, params.back (), false);
			types.push_back (Type::init (type.to <Type> (), type.to <Type> ().isMutable (), false));
		    } catch (Error::ErrorList ATTRIBUTE_UNUSED lst) { // maybe there was an implicit alias, that is not a problem
			type = Type::init (type.to <Type> (), false); // we just put it as a const one
			verifyMemoryOwner (params.back ().getLocation (), type, params.back (), false);
			types.push_back (Type::init (type.to <Type> (), type.to <Type> ().isMutable (), false));
		    }
		}
	    }
	    
	    auto type = Tuple::init (list.getLocation (), types);
	    type = Type::init (type.to <Type> (), true); // Tuple are mutable by default (not lvalue)
	    
	    return TupleValue::init (list.getLocation (), type, params);	    
	}


	Generator Visitor::validateList (const syntax::List & list) {
	    if (list.isArray ()) return validateArray (list);
	    if (list.isTuple ()) return validateTuple (list);
	    
	    println (list.prettyString ());
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}
       
	Generator Visitor::validateArrayAlloc (const syntax::ArrayAlloc & alloc) {
	    if (alloc.isDynamic ()) {
		auto value = validateValueNonVoid (alloc.getLeft ());
		verifyMemoryOwner (alloc.getLocation (), value.to <Value> ().getType (), value, false);
		auto size = SizeOf::init (
		    alloc.getLocation (),
		    Integer::init (alloc.getLocation (), 0, false),
		    value.to <Value> ().getType ()
		    );
		
		auto len = validateValueNonVoid (alloc.getSize ());
		auto type = Slice::init (alloc.getLocation (), value.to<Value> ().getType ());
		type = Type::init (type.to <Type> (), true);
		
		return ArrayAlloc::init (alloc.getLocation (), type.to<Type> ().toDeeplyMutable (), value, size, len);
	    } else {
		auto value = validateValueNonVoid (alloc.getLeft ());
		verifyMemoryOwner (alloc.getLocation (), value.to <Value> ().getType (), value, false);
		
		auto size = SizeOf::init (
		    alloc.getLocation (),
		    Integer::init (alloc.getLocation (), 0, false),
		    value.to <Value> ().getType ()
		    );
		
		auto len = retreiveValue (validateValueNonVoid (alloc.getSize ()));
		if (!len.is <Fixed> () || (len.to<Fixed> ().getType ().to <Integer> ().isSigned () && len.to <Fixed> ().getUI ().i < 0)) {
		    Ymir::Error::occur (alloc.getSize ().getLocation (), ExternalError::INCOMPATIBLE_TYPES,
					value.to <Value> ().getType ().to <Type> ().getTypeName (),
					(Integer::init (lexing::Word::eof (), 64, false)).to<Type> ().getTypeName ()
			);
		}
		
		auto type = Array::init (alloc.getLocation (), value.to<Value> ().getType (), len.to <Fixed> ().getUI ().u);
		type = Type::init (type.to <Type> (), true);
		
		return ArrayAlloc::init (alloc.getLocation (), type.to <Type> ().toDeeplyMutable (), value, size, len.to <Fixed> ().getUI ().u);
	    }
	}


	Generator Visitor::validateLambda (const syntax::Lambda & function) {
	    auto name = format ("_%", function.getUniqId ());
	    auto frameName = this-> _referent.back ().getRealName () + "::" + name;
	    auto lambdaStored = this-> _lambdas.find (frameName);
	    if (lambdaStored != this-> _lambdas.end ()) { // We want to avoid multiple time validation
		return lambdaStored-> second;
	    }
	    
	    std::vector <Generator> params;
	    std::vector <Generator> paramsProto;
	    std::vector <Generator> paramTypes;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    Generator body (Generator::empty ());
	    Generator retType (Generator::empty ());

	    bool uncomplete = false; // idem
	    auto syms = this-> _symbols.size () - 1; // index of the last symbol is the current enclosure of the frame
	    
	    enterForeign ();
	    enterBlock ();
	    {
		try {
		    for (auto & param : function.getPrototype ().getParameters ()) {
			auto var = param.to <syntax::VarDecl> ();
			Generator type (Generator::empty ());
			if (!var.getType ().isEmpty ()) {
			    type = validateType (var.getType ());
			} else {
			    uncomplete = true;
			}

			bool isMutable = false;
			bool isRef = false;
			bool dmut = false;
			bool isPure = false;
			if (!type.isEmpty ()) {
			    type = applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut, isPure);
			    
			    verifyMutabilityRefParam (var.getLocation (), type, ExternalError::MUTABLE_CONST_PARAM);
			    if (type.is <NoneType> () || type.is<Void> ()) {
				Ymir::Error::occur (var.getLocation (), ExternalError::VOID_VAR);
			    } else if (type.is <TraitRef> ()) {
				Ymir::Error::occur (type.getLocation (), ExternalError::USE_AS_TYPE);
			    }
			}
		
			params.push_back (ParamVar::init (var.getName (), type, isMutable, false));
			paramsProto.push_back (ProtoVar::init (var.getName (), type, Generator::empty (), isMutable, 1, false));
			paramTypes.push_back (type);
			if (var.getName () != Keys::UNDER) {
			    verifyShadow (var.getName ());		
			    insertLocal (var.getName ().getStr (), params.back ());
			}		
		    }
	   
		    if (!function.getPrototype ().getType ().isEmpty ()) {
			retType = validateType (function.getPrototype ().getType (), true);		
		    }

		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }

	    this-> discardAllLocals ();

	    {
		try {
		    quitBlock (errors.size () == 0);
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    exitForeign ();
		
	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};
	    
	    auto proto = LambdaProto::init (function.getLocation (), frameName, retType, paramsProto, function.getContent (), function.isRefClosure (), function.isMoveClosure (), syms);
	    proto = LambdaProto::init (proto.to<LambdaProto>(), format ("%%%", this-> _referent.back ().getMangledName (), (uint32_t) name.length (), name), Frame::ManglingStyle::Y);	    

	    if (!uncomplete) {
		return validateLambdaProto (proto.to <LambdaProto> (), paramTypes);
	    } else {
		return proto;
	    }
	}

	Generator Visitor::validateLambdaProto (const LambdaProto & proto, const std::vector <Generator> & types) {
	    std::vector <Generator> params;
	    std::vector <Generator> paramsProto;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    Generator body (Generator::empty ());
	    Generator retType (proto.getReturnType ());

	    bool needFinalReturn = false;// mmmh, not understanding why, but gcc doesn't like it otherwise
	    enterForeign ();
	    enterContext ({});
	    enterBlock ();
	    {
		try {
		    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
			auto var = proto.getParameters ()[it].to <ProtoVar> ();
			if (it >= (int64_t) types.size () || types [it].isEmpty ()) {
			    Ymir::Error::occur (var.getLocation (), ExternalError::UNKNOWN_LAMBDA_TYPE, var.prettyString ());
			} else {
			    bool isMutable = types [it].to <Type> ().isMutable ();
			    if (!var.getType ().isEmpty ()) {
				this-> verifyCompatibleType (var.getType ().getLocation (), types [it].getLocation (), var.getType (), types [it]);
			    }
			    params.push_back (ParamVar::init (var.getLocation (), types [it], isMutable, false));
			    paramsProto.push_back (ProtoVar::init (var.getLocation (), types [it], Generator::empty (), isMutable, 1, false));
			    if (var.getName () != Keys::UNDER) {
				insertLocal (var.getName (), params.back ());
			    }
			}
		    }
		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 		    
	    }
	    
	    uint32_t refId = 0;
	    {
		try {		    
		    this-> setCurrentFuncType (retType);
		    refId = generator::VarDecl::__lastId__;
		    generator::VarDecl::__lastId__ += 1;
		    if (proto.isRefClosure () || proto.isMoveClosure ())
		    this-> enterClosure (proto.isRefClosure (), refId, proto.getClosureIndex ());
			    
		    body = validateValue (proto.getContent ());		    
		    retType = this-> getCurrentFuncType ();
	    
		    if (!body.to<Value> ().isReturner ()) {
			if (!retType.isEmpty ()) {
			    verifyMemoryOwner (body.getLocation (), retType, body, true);		    
			    needFinalReturn = !retType.is<Void> ();
			} else {
			    needFinalReturn = !body.to <Value> ().getType ().is<Void> ();
			    retType = body.to <Value> ().getType ();
			}
		    }
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 		    
	    }

	    Generator closure (Generator::empty ());
	    if (proto.isRefClosure () || proto.isMoveClosure ()) {
		closure = this-> exitClosure ();
		if (closure.to <Closure> ().getNames ().size () != 0) {
		    params.insert (params.begin (), ParamVar::init (lexing::Word::init (lexing::Word::eof (), "#_closure"), closure, false, false));
		    params [0].setUniqId (refId);
		} else closure = Generator::empty ();
	    }

	    if (!body.isEmpty ()) {
		if (body.getThrowers ().size () != 0) {
		    std::list <Ymir::Error::ErrorMsg> notes;
		    for (auto &it : body.getThrowers ()) {
			notes.push_back (Ymir::Error::createNote (it.getLocation (), it.prettyString ()));		
		    }
		    errors.push_back (Error::makeOccurAndNote (proto.getLocation (), notes, ExternalError::THROWS_IN_LAMBDA));
		}
	    }
	    
	    {
		try { // We want to guarantee that we exit the foreign at the end of this function 
		    quitBlock (errors.size () == 0);
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
	    }
	    
	    exitForeign ();
	    exitContext ();
	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};

	    auto frame = Frame::init (proto.getLocation (), proto.getName (), params, retType, body, needFinalReturn);
	    frame.to <Frame> ().isWeak (true);
	    frame.to <Frame> ().setMangledName (proto.getMangledName ());

	    insertNewGenerator (frame);
	    
	    auto frameProto = FrameProto::init (proto.getLocation (), proto.getName (), retType, paramsProto, false, false, {});
	    frameProto = FrameProto::init (frameProto.to<FrameProto>(), proto.getMangledName (), Frame::ManglingStyle::Y);
		
	    auto funcType = Type::init (FuncPtr::init (proto.getLocation (), frameProto.to <FrameProto> ().getReturnType (), types).to <Type> (), true);
	    auto addr = Addresser::init (proto.getLocation (), funcType, frameProto);
	    insert_or_assign (this-> _lambdas, proto.getName (), addr);
	    
	    if (!closure.isEmpty ()) {
		auto closureValue = validateClosureValue (closure, proto.isRefClosure (), proto.getClosureIndex ());
		auto tupleType = Delegate::init (proto.getLocation (), funcType);		
		auto tuple = DelegateValue::init (proto.getLocation (), tupleType, closureValue.to <Value> ().getType (), closureValue, addr);
						       
		return tuple;
	    } else {		
		return addr;
	    }
	}

	Generator Visitor::validateClosureValue (const Generator & closureType, bool isRefClosure, uint32_t closureIndex) {
	    std::vector <Generator> innerTypes;
	    std::vector <Generator> innerValues;
	    auto loc = closureType.getLocation ();
	    for (auto & name : closureType.to <Closure> ().getNames ()) {
		auto & syms = this-> _symbols [closureIndex];
		for (auto _it : Ymir::r (0, syms.size ())) {
		    auto ptr = syms [_it].find (name);
		    if (ptr != syms [_it].end ()) {
			if (ptr-> second.is <ParamVar> ()) {
			    auto type = ptr-> second.to <Value> ().getType ();
			    auto varRef = VarRef::init (loc, name, type, ptr-> second.getUniqId (), false, Generator::empty (), ptr-> second.to <ParamVar> ().isSelf ());
			    if (isRefClosure) {
				innerValues.push_back (Referencer::init (loc, Type::init (type.to <Type> (), type.to <Type> ().isMutable (), true), varRef));
			    } else
			    innerValues.push_back (varRef);

			    innerTypes.push_back (type);
			} else if (ptr-> second.is <generator::VarDecl> ()) {
			    auto type = ptr-> second.to <generator::VarDecl> ().getVarType ();
			    auto varRef = VarRef::init (loc, name, type, ptr-> second.getUniqId (), false, Generator::empty ());
			    if (isRefClosure) {
				innerValues.push_back (Referencer::init (loc, Type::init (type.to <Type> (), type.to <Type> ().isMutable (), true), varRef));
			    } else {
				Generator value (Generator::empty ());
				if (!ptr-> second.to <generator::VarDecl> ().isMutable ())
				value = ptr-> second.to <generator::VarDecl> ().getVarValue ();
				varRef = VarRef::init (loc, name, type, ptr-> second.getUniqId (), false, value);
				innerValues.push_back (varRef);
			    }
			    
			    innerTypes.push_back (type);
			} else if (ptr-> second.is <StructAccess> ()) {
			    innerValues.push_back (ptr-> second);
			    
			    auto type = ptr-> second.to <Value> ().getType ();
			    innerTypes.push_back (type);
			}
			break; // We found it, go to the the next enclosure
		    }
		}
	    }
	    
	    auto tupleType = TupleClosure::init (loc, innerTypes);
	    auto tupleValue = TupleValue::init (loc, tupleType, innerValues);
	    return Copier::init (loc, Pointer::init (loc, Void::init (loc)), tupleValue);
	}	
	

	Generator Visitor::validateNullValue (const syntax::Null & nl) {
	    auto type = Pointer::init (nl.getLocation (), Void::init (nl.getLocation ()));
	    return NullValue::init (nl.getLocation (), type);
	}

	
    }

}
