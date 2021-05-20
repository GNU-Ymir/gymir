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


	Generator Visitor::validateThrow (const syntax::Throw & thr) {
	    auto inner = this-> validateValue (thr.getValue ());
	    auto type = inner.to <Value> ().getType ();
	    
	    auto uniq = UniqValue::init (thr.getLocation (), type, inner);	    
	    auto ancType = this-> _cache.exceptionType.getValue ();

	    verifyCompatibleType (thr.getLocation (), type.getLocation (), ancType, type);
	    
	    auto loc = thr.getLocation ();
	    auto bin = syntax::Binary::init (lexing::Word::init (loc, Token::DCOLON),
					     TemplateSyntaxWrapper::init (loc, uniq),
					     syntax::Var::init (lexing::Word::init (loc, SubVisitor::__TYPEINFO__)),
					     syntax::Expression::empty ()
		);
	    auto info = validateValue (bin);
	    
	    return Throw::init (thr.getLocation (), info, uniq);
	}

	Generator Visitor::validateMatch (const syntax::Match & matcher) {
	    auto visitor = MatchVisitor::init (*this);
	    return visitor.validate (matcher);
	}

	
	Generator Visitor::validateAssert (const syntax::Assert & assert) {
	    auto test = validateValue (assert.getTest ());
	    if (!test.to <Value> ().getType ().is <Bool> ()) {
		Ymir::Error::occur (test.getLocation (), ExternalError::INCOMPATIBLE_TYPES,
				    test.to <Value> ().getType ().to <Type> ().getTypeName (),
				    Bool::NAME
		    );
	    }

	    std::vector <syntax::Expression> params;
	    params.push_back (TemplateSyntaxWrapper::init (test.getLocation (), test));
	    if (!assert.getMsg ().isEmpty ()) {
		params.push_back (assert.getMsg ());
	    }

	    auto loc = assert.getLocation ();
	    auto func = createVarFromPath (assert.getLocation (), {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (ASSERT_FUNC)});
	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		func,
		params
		);
	    
	    auto ret = validateValue (call);
	    try {
	    	auto val = retreiveValue (test);
	    	ret = Value::initBrRet (ret.to <Value> (), ret.to <Value> ().isBreaker (), !val.to <BoolValue> ().getValue (), assert.getLocation (), assert.getLocation ());
	    } catch (Error::ErrorList list) {
	    } 
	    
	    return ret;
	}
		
	Generator Visitor::validateCteAssert (const syntax::Assert & assert) {
	    auto test = validateValue (assert.getTest ());
	    if (!test.to <Value> ().getType ().is <Bool> ()) {
		Ymir::Error::occur (test.getLocation (), ExternalError::INCOMPATIBLE_TYPES,
				    test.to <Value> ().getType ().to <Type> ().getTypeName (),
				    Bool::NAME
		    );
	    }

	    auto val = retreiveValue (test);
	    if (!val.to <BoolValue> ().getValue ()) {
		std::string msg;
		if (!assert.getMsg ().isEmpty ()) {
		    try {
			auto msgVal = retreiveValue (validateValue (assert.getMsg ()));
			std::vector <char> text;
			if (msgVal.is <StringValue> ()) {
			    text = msgVal.to <StringValue> ().getValue ();
			} else if (msgVal.is <Aliaser> () && msgVal.to <Aliaser> ().getWho ().is <StringValue> ()) {
			    text = msgVal.to <Aliaser> ().getWho ().to <StringValue> ().getValue ();
			}
			if (text.size () != 0) {
			    Ymir::OutBuffer buf;
			    text = UtfVisitor::utf32_to_utf8 (text);
			    for (int i = 0 ; i < (int) text.size () - 1; i++) buf.write (text [i]);
			    msg = buf.str ();
			} else {
			    msg = msgVal.prettyString ();
			}
		    } catch (Ymir::Error::ErrorList) {
			msg = assert.getMsg ().prettyString ();
		    }
		}
		Ymir::Error::occur (assert.getLocation (), ExternalError::ASSERT_FAILED, msg);
	    }
	    
	    return None::init (assert.getLocation ());
	}


	
	Generator Visitor::validatePragma (const syntax::Pragma & prg) {
	    auto visitor = PragmaVisitor::init (*this);
	    return visitor.validate (prg);	    
	}

	Generator Visitor::validateWith (const syntax::With & wh) {
	    enterBlock ();
	    std::vector <Generator> varDecls;
	    std::vector <Generator> exits;
	    
	    std::list <Error::ErrorMsg> errors;
	    for (auto & it : wh.getDecls ()) {
		try {
		    varDecls.push_back (this-> validateVarDeclValue (it.to <syntax::VarDecl> (), true));
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    }

	    for (auto & it : varDecls) {
		try {
		    auto vdecl = it.to <generator::VarDecl> ();
		    auto loc = vdecl.getLocation ();
		    auto trait = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (DISPOSING_MODULE), CoreNames::get (DISPOSABLE_TRAITS)});		
		    auto impl = validateType (trait);
		    
		    if (!vdecl.getVarType ().is <ClassPtr> ()) {
			Ymir::Error::occur (vdecl.getLocation (), ExternalError::NOT_IMPL_TRAIT, vdecl.getVarType ().prettyString (), impl.prettyString ());	
		    } else {
			verifyClassImpl (vdecl.getLocation (), vdecl.getVarType (), impl);
		    }

		    auto vRef = VarRef::init (it.getLocation (), vdecl.getName (), vdecl.getVarType (), vdecl.getUniqId (), vdecl.isMutable (), Generator::empty ());
		    auto intr = syntax::Intrinsics::init (lexing::Word::init (loc, Keys::ALIAS), TemplateSyntaxWrapper::init (loc, vRef));
		    auto call = syntax::MultOperator::init (
			lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
			syntax::Binary::init (lexing::Word::init (loc, Token::DOT),
					      intr,
					      syntax::Var::init (lexing::Word::init (loc, global::CoreNames::get (DISPOSE_OP_OVERRIDE))),
					      syntax::Expression::empty ()),
			{}, false
			);
		    exits.push_back (this-> validateValue (call));
		} catch (Error::ErrorList list) {
		    list.errors.back ().addNote (Error::createNote (wh.getLocation ()));
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    }		

	    std::vector <syntax::Expression> scopes;
	    syntax::Expression catcher (syntax::Expression::empty ());
	    
	    Generator ret (Generator::empty ());
	    try {
		ret = this-> validateValue (wh.getContent ());
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }

	    try {		
		quitBlock (errors.size () == 0);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }
	    
	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};
	    
	    auto jmp_buf_type = validateType (syntax::Var::init (lexing::Word::init (wh.getLocation (), global::CoreNames::get (JMP_BUF_TYPE))));
	    for (auto it : Ymir::r (exits.size (), 0)) {
		ret = ExitScope::init (wh.getLocation (), ret.to <Value> ().getType (), jmp_buf_type, ret, {exits [it - 1]}, {}, Generator::empty (), Generator::empty (), Generator::empty ());
		ret = Block::init (wh.getLocation (), ret.to<Value> ().getType (), {varDecls [it - 1], ret});		
	    }
	    
	    return ret;
	}

	Generator Visitor::validateAtomic (const syntax::Atomic & atom) {
	    if (atom.getWho ().isEmpty ()) {
		auto loc = atom.getLocation ();
		auto mutexType = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (ATOMIC_MODULE), CoreNames::get (MUTEX_TYPE)});
		auto mutexInitValue = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (ATOMIC_MODULE), CoreNames::get (MUTEX_INIT)});
				
		auto type = validateType (mutexType);
		auto initValue = this-> validateValue (mutexInitValue);
		auto ptrType = Pointer::init (atom.getLocation (), type);		
		
		
		auto name = Ymir::format ("__atom%", atom.getLocation ().getLine ());
		auto glbVar = GlobalVar::init (lexing::Word::init (atom.getLocation (), name), name, "", false, type, initValue, false);
		auto vRef = VarRef::init (lexing::Word::init (atom.getLocation (), name), name, type, glbVar.getUniqId (), false, Generator::empty ());
		
		insertNewGenerator (glbVar);
		enterBlock ();
		
		auto inner = this-> validateValue (atom.getContent ());
		inner = Block::init (atom.getLocation (), inner.to <Value> ().getType (),
				     {
					 AtomicLocker::init (atom.getLocation (), Addresser::init (atom.getLocation (), ptrType, vRef), false),
					 inner
				     }
		    );
		auto jmp_buf_type = validateType (syntax::Var::init (lexing::Word::init (atom.getLocation (), global::CoreNames::get (JMP_BUF_TYPE))));
		auto exit = ExitScope::init (atom.getLocation (), inner.to <Value> ().getType (), jmp_buf_type, inner, {
			AtomicUnlocker::init (atom.getLocation (), Addresser::init (atom.getLocation (), ptrType, vRef), false)
			    }, {}, Generator::empty (), Generator::empty (), Generator::empty ()
		    );
		return exit;
	    } else {
		auto value = this-> validateValue (atom.getWho ());
		if (!value.to <Value> ().getType ().is <ClassPtr> ()) {
		    Ymir::Error::occur (atom.getLocation (), ExternalError::MONITOR_NON_CLASS, value.to <Value> ().getType ().prettyString ());
		}

		auto inner = this-> validateValue (atom.getContent ());
		inner = Block::init (atom.getLocation (), inner.to <Value> ().getType (),
				     {
					 AtomicLocker::init (atom.getLocation (), value, true),
					 inner
				     }
		    );

		auto jmp_buf_type = validateType (syntax::Var::init (lexing::Word::init (atom.getLocation (), global::CoreNames::get (JMP_BUF_TYPE))));
		auto exit = ExitScope::init (atom.getLocation (), inner.to <Value> ().getType (), jmp_buf_type, inner, {
			AtomicUnlocker::init (atom.getLocation (), value, true)
			    }, {}, Generator::empty (), Generator::empty (), Generator::empty ()
		    );
		return exit;
	    }
	}	
	
	

	Generator Visitor::validateCast (const syntax::Cast & cast) {
	    auto visitor = CastVisitor::init (*this);
	    return visitor.validate (cast);
	}

	Generator Visitor::validateIfExpression (const syntax::If & _if) {
	    Generator test (Generator::empty ());
	    if (!_if.getTest ().isEmpty ()) { // Has a test if it is not an else
		test = validateValue (_if.getTest ());
		
		if (!test.to<Value> ().getType ().is <Bool> ()) {
		    Ymir::Error::occur (test.getLocation (), ExternalError::INCOMPATIBLE_TYPES,
					test.to <Value> ().getType ().to <Type> ().getTypeName (),
					Bool::NAME
			);
		}
	    }

	    auto content = validateValue (_if.getContent (), false, false, false);
	    auto type = content.to <Value> ().getType ();

	    if (!_if.getElsePart ().isEmpty ()) {
		auto _else = validateValue (_if.getElsePart (), false, false, false);
		if (!_else.to <Value> ().isReturner () && !_else.to <Value> ().isBreaker ()) {
		    if (!_else.to <Value> ().getType ().to <Type> ().isCompatible (type)) {
			auto anc = getCommonAncestor (_else.to <Value> ().getType (), type);
			if (!anc.isEmpty ())
			type = anc;
		    }

		    try {
			type = this-> inferTypeBranching (content.getLocation (), _else.getLocation (), type, _else.to<Value> ().getType ());
		    } catch (Error::ErrorList list) {
			Ymir::Error::occurAndNote (_if.getLocation (), list.errors, ExternalError::BRANCHING_VALUE);
		    }
		}

		if (content.to <Value> ().isReturner () || content.to <Value> ().isBreaker ()) type = _else.to <Value> ().getType ();
		return Conditional::init (_if.getLocation (), type, test, content, _else);	    
	    } else {
		verifyCompatibleType (_if.getLocation (), content.getLocation (), Void::init (_if.getLocation ()), type);
		// We check if the type is void, it impose to add a ; at the end of if expression
		
		return Conditional::init (_if.getLocation (), type, test, content, Generator::empty ());
	    }
	}

	
	Generator Visitor::validateCteIfExpression (const syntax::If & _if) {
	    Generator test (Generator::empty ());
	    if (!_if.getTest ().isEmpty ()) {
		auto test = validateValue (_if.getTest ());
		auto value = retreiveValue (test);
		if (!value.is<BoolValue> ()) {
		    Ymir::Error::occur (test.getLocation (), ExternalError::INCOMPATIBLE_TYPES,
					test.to <Value> ().getType ().to <Type> ().getTypeName (),
					Bool::NAME
			);
		    return Generator::empty ();
		}
		if (value.to <BoolValue> ().getValue ()) {
		    return validateValue (_if.getContent (), false, false, false);
		} else if (!_if.getElsePart ().isEmpty ()) {
		    return validateValue (_if.getElsePart ());
		} else return Block::init (_if.getLocation (), Void::init (_if.getLocation ()), {});
	    } else return validateValue (_if.getContent (), false, false, false);
	}	
	
	Generator Visitor::validateWhileExpression (const syntax::While & _wh) {
	    Generator test (Generator::empty ());
	    if (!_wh.getTest ().isEmpty ()) {
		test = validateValue (_wh.getTest ());
		if (!test.to <Value> ().getType ().is <Bool> ()) {
		    Ymir::Error::occur (test.getLocation (), ExternalError::INCOMPATIBLE_TYPES,
					test.to <Value> ().getType ().to <Type> ().getTypeName (),
					Bool::NAME
			);
		}
	    }

	    std::list <Ymir::Error::ErrorMsg> errors;
	    enterLoop ();
	    auto voidType = Void::init (_wh.getLocation ());
	    if (!test.isEmpty ()) this-> setCurrentLoopType (voidType); // we cannot garantee that we will enter the loop	    
	    Generator content (Generator::empty ());
	    try {
		content = validateValue (_wh.getContent (), false, false, false);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	    
	    
	    auto breakType = quitLoop ();
	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};

	    this-> verifyCompatibleType (content.getLocation (), _wh.getLocation (), content.to <Value> ().getType (), Void::init (_wh.getLocation ()));
	    if (test.isEmpty ()) {
		if (!breakType.isEmpty ()) voidType = breakType;
	    }

	    return Loop::init (_wh.getLocation (), voidType, test, content, _wh.isDo ());	    
	}	

	Generator Visitor::validateForExpression (const syntax::For & _for, bool isCte) {
	    auto forVisitor = ForVisitor::init (*this);
	    Generator content (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    try {
		if (isCte)
		content = forVisitor.validateCte (_for);
		else
		content = forVisitor.validate (_for);
	    } catch (Error::ErrorList list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 	    
	    
	    if (errors.size () != 0) throw Error::ErrorList {errors};
	    return content;
	}
	
	Generator Visitor::validateBreak (const syntax::Break & _break) {
	    if (!this-> isInLoop ())
	    Ymir::Error::occur (_break.getLocation (), ExternalError::BREAK_NO_LOOP);
				    
	    Generator value = Generator::empty ();
	    Generator type = Generator::empty ();
	    if (!_break.getValue ().isEmpty ()) {
		value = validateValue (_break.getValue ());
		type = Type::init (_break.getLocation (), value.to <Value> ().getType ().to <Type> ());
	    } else type = Void::init (_break.getLocation ());

	    bool set = false;
	    auto loop_type = getCurrentLoopType ();
	    if (loop_type.isEmpty ()) {
		setCurrentLoopType (type);
		loop_type = type;
		set = true;
	    } else if (!loop_type.equals (type)) {
		auto anc = getCommonAncestor (loop_type, type);
		if (!anc.isEmpty ()) {
		    loop_type = anc;
		    setCurrentLoopType (anc);
		} else {		
		    auto note = Ymir::Error::createNote (loop_type.getLocation ());
		    Ymir::Error::occurAndNote (value.getLocation (), note, ExternalError::INCOMPATIBLE_TYPES,
					       type.to <Type> ().getTypeName (),
					       loop_type.to <Type> ().getTypeName ()
			);
		}
	    }

	    if (!loop_type.is<Void> ()) {
		if (set) {
		    try {
		    	verifyMemoryOwner (_break.getLocation (), loop_type, value, false);
		    } catch (Error::ErrorList err) { // Maybe implicit alias problem, we set the type to non mutable, to check
		    	loop_type = Type::init (loop_type.to <Type> (), false, loop_type.to <Type> ().isRef ());
		    	setCurrentLoopType (loop_type);
		    	verifyMemoryOwner (_break.getLocation (), loop_type, value, false); // if this pass, the loop type is const, and it is ok
		    }		    
		} else {
		    loop_type = this-> inferTypeBranching (_break.getLocation (), loop_type.getLocation (), value.to <Value> ().getType (), loop_type);
		    setCurrentLoopType (loop_type);
		}
	    }
	    
	    return Break::init (_break.getLocation (), Void::init (_break.getLocation ()), value);
	}

	Generator Visitor::validateReturn (const syntax::Return & rt) {
	    Generator value = Generator::empty ();
	    Generator type = Generator::empty ();
	    if (!rt.getValue ().isEmpty ()) {
	    	value = validateValue (rt.getValue ());
	    	type = value.to <Value> ().getType ();
	    } else type = Void::init (rt.getLocation ());

	    auto fn_type = getCurrentFuncType ();

	    /** In case of lambda proto, we are able to validate it with the function return type */
	    if (type.is <LambdaType> () && (fn_type.is <FuncPtr> () || fn_type.is <Delegate> ())) {
		std::vector <Generator> paramTypes;
		if (fn_type.is <FuncPtr> ()) paramTypes = fn_type.to <FuncPtr> ().getParamTypes ();
		else paramTypes = fn_type.to <Delegate> ().getInners ()[0].to <FuncPtr> ().getParamTypes ();

		if (value.is <VarRef> ()) {
		    value = validateLambdaProto (value.to <VarRef> ().getValue ().to <LambdaProto> (), paramTypes);
		    type = value.to <Value> ().getType ();
		} else if (value.is <LambdaProto> ()) {
		    value = validateLambdaProto (value.to <LambdaProto> (), paramTypes);
		    type = value.to <Value> ().getType ();
		} // This is working exactly like findParameter for function, maybe we can merge those two blocks
	    }

	    try {
		if (fn_type.isEmpty ()) {
		    this-> setCurrentFuncType (type);
		    fn_type = type;
		} else if (!value.isEmpty ()) { // If value, it can be a mut [mut void]
		    verifyCompatibleTypeWithValue (fn_type.getLocation (), fn_type, value);
		} else {		    
		    verifyCompatibleType (fn_type.getLocation (), type.getLocation (), fn_type, type);
		}
	    } catch (Error::ErrorList list) {
		list.errors.back ().addNote (Ymir::Error::createNote (rt.getLocation()));
		throw list;
	    }
	    
	    // 	if (!fn_type.equals (type)) {
	    // 	auto note = Ymir::Error::createNote (fn_type.getLocation ());
	    // 	Ymir::Error::occurAndNote (value.getLocation (), note, ExternalError::INCOMPATIBLE_TYPES,
	    // 				   type.to <Type> ().getTypeName (),
	    // 				   fn_type.to <Type> ().getTypeName ()
	    // 	);				    
	    // }

	    if (!fn_type.is<Void> ()) {
		verifyMemoryOwner (rt.getLocation (), fn_type, value, true);
	    }
	    
	    return Return::init (rt.getLocation (), Void::init (rt.getLocation ()), fn_type, value);
	}
	Generator Visitor::validateDestructDecl (const syntax::DestructDecl & decl) {
	    auto value = validateValue (syntax::Intrinsics::init (lexing::Word::init (decl.getLocation (), Keys::EXPAND), decl.getValue ()));
	    match (value) {
		of (List, lst) {
		    if ((decl.isVariadic () && lst.getParameters ().size () < decl.getParameters ().size ())
			|| (!decl.isVariadic () && lst.getParameters ().size () != decl.getParameters ().size ())) {
			Ymir::Error::occur (decl.getLocation (),
					    ExternalError::MISMATCH_ARITY,
					    decl.getParameters ().size (),
					    lst.getParameters ().size ());			    
		    }

		    std::vector <Generator> values;
		    Generator type (Void::init (decl.getLocation ()));
		    for (int i = 0 ; i < (int) decl.getParameters ().size () ; i++) {
			if (i != (int) decl.getParameters ().size () - 1 || lst.getParameters ().size () == decl.getParameters ().size ()) {
			    auto varDecl = decl.getParameters ()[i].to <syntax::VarDecl> ();
			    auto auxDecl = syntax::VarDecl::init (varDecl.getName (), varDecl.getDecorators (), varDecl.getType (), TemplateSyntaxWrapper::init (lst.getLocation (), lst.getParameters ()[i]));
			    values.push_back (validateValue (auxDecl));
				
			} else {
			    std::vector <Generator> rest (lst.getParameters ().begin () + i, lst.getParameters ().end ());
			    std::vector <Generator> types;
			    for (auto & it : rest) {
				types.push_back (it.to<Value> ().getType ());
			    }
			    auto tupleType = Tuple::init (lst.getLocation (), types);
			    tupleType = Type::init (tupleType.to <Type> (), true);
				
			    auto varDecl = decl.getParameters ()[i].to <syntax::VarDecl> ();
			    auto auxDecl = syntax::VarDecl::init (varDecl.getName (), varDecl.getDecorators (), varDecl.getType (), 
								  TemplateSyntaxWrapper::init (
								      lst.getLocation (),
								      TupleValue::init (
									  lst.getLocation (),
									  tupleType,
									  rest
									  ))
				);
				
			    values.push_back (validateValue (auxDecl));				
			}			    
		    }
		    return Set::init (decl.getLocation (), type, values);
		}
		elfo {
		    if (decl.getParameters ().size () != 1) {
			Ymir::Error::occur (decl.getLocation (),
					    ExternalError::OVERFLOW_ARITY,
					    decl.getParameters ().size (),
					    1);
		    }
		    auto varDecl = decl.getParameters () [0].to<syntax::VarDecl> ();
		    auto auxDecl = syntax::VarDecl::init (varDecl.getName (), varDecl.getDecorators (), varDecl.getType (), 
							  TemplateSyntaxWrapper::init (value.getLocation (), value)
			);
		    return validateValue (auxDecl);
		}
	    }
	    
	    return Generator::empty ();
	}

	Generator Visitor::validateVarDeclValue (const syntax::VarDecl & var, bool needInitValue) {
	    if (var.getName () != Keys::UNDER)
	    verifyShadow (var.getName ());

	    Generator value (Generator::empty ());
	    if (!var.getValue ().isEmpty ()) {
		value = validateValue (var.getValue ());		
	    }

	    if (var.getValue ().isEmpty () && var.getType ().isEmpty ()) {
		Error::occur (var.getLocation (), ExternalError::VAR_DECL_WITH_NOTHING);
	    }

	    Generator type (Generator::empty ());
	    if (!var.getType ().isEmpty ()) {
		try {
		    type = validateType (var.getType ());
		} catch (Ymir::Error::ErrorList list) {
		    Ymir::Error::noteAndNote (var.getType ().getLocation (), list.errors, "");
		}
	    } else {
		type = Type::init (value.to <Value> ().getType ().to <Type> (), false, false);
	    }

	    
	    if (var.getValue ().isEmpty () && needInitValue) {
		Error::occur (var.getLocation (), ExternalError::VAR_DECL_WITHOUT_VALUE);
	    } 
		    
	    bool isMutable = false, isRef = false, dmut = false;
	    type = applyDecoratorOnVarDeclType (var.getDecorators (), type, isRef, isMutable, dmut);
	    
	    if (!value.isEmpty ()) {
		// We do not check the lambdatype complete type if the var is not mutable
		if (isMutable || !type.is <LambdaType> ())
		verifyMemoryOwner (var.getLocation (), type, value, true);
	    }

	    if (type.is<NoneType> () || type.is<Void> ()) {
		if (!value.isEmpty ()) {
		    Ymir::Error::occur (var.getLocation (), ExternalError::VOID_VAR_VALUE, value.prettyString ());
		} else {
		    Ymir::Error::occur (var.getLocation (), ExternalError::VOID_VAR);
		}
	    }
	    
	    
	    auto ret = generator::VarDecl::init (var.getLocation (), var.getName ().getStr (), type, value, isMutable);
	    if (var.getName () != Keys::UNDER)
	    insertLocal (var.getName ().getStr (), ret);
	    return ret;
	}

	Generator Visitor::validateTry (const syntax::Try & tr) {
	    try {
		return this-> validateTypeTry (tr);
	    } catch (Error::ErrorList&) {}
	    
	    auto inner = this-> validateValue (tr.getContent ());
	    auto innerType = inner.to <Value> ().getType ();
	    try {		
		verifyMemoryOwner (inner.getLocation (), innerType, inner, false);
		innerType = Type::init (innerType.to <Type> (), innerType.to <Type> ().isMutable (), false);
	    } catch (Error::ErrorList ATTRIBUTE_UNUSED lst) {
		innerType = Type::init (innerType.to <Type> (), false);
		verifyMemoryOwner (inner.getLocation (), innerType, inner, false);
	    }
	    
	    auto errType = Type::init (this-> _cache.exceptionType.getValue ().to <Type> (), false, false);	    
	    auto optionType = Option::init (tr.getLocation (), innerType, errType);
	    
	    optionType = Type::init (optionType.to <Type> (), true);		    
	    auto throwsType = inner.getThrowers ();
	    inner = OptionValue::init (tr.getLocation (), optionType, inner, true);
	    
	    if (throwsType.size () != 0) {
		auto jmp_buf_type = validateType (syntax::Var::init (lexing::Word::init (tr.getLocation (), global::CoreNames::get (JMP_BUF_TYPE))));

		auto loc = tr.getLocation ();
		auto varDecl = generator::VarDecl::init (lexing::Word::init (loc, "#catch"), "#catch", errType, Generator::empty (), false);
		auto typeInfo = validateTypeInfo (loc, errType);
		auto vref = VarRef::init (loc, "#catch", errType, varDecl.getUniqId (),  false, Generator::empty ());

		auto outer = OptionValue::init (tr.getLocation (), optionType, vref, false);

		auto ret = ExitScope::init (tr.getLocation (), optionType, jmp_buf_type, inner, {}, {}, varDecl, typeInfo, outer);		
		if (optionType.to <Type> ().needExplicitAlias ()) {
		    ret = Aliaser::init (tr.getLocation (), optionType, ret);
		}
		return ret;
	    }
	    
	    return inner;
	    
	}

	
    }

}
