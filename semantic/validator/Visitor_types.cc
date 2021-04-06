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

	Generator Visitor::validateType (const syntax::Expression & expr, bool lock) {
	    auto type = validateType (expr);
	    if (lock && !type.to<Type> ().isMutable ())
	    return Type::init (type.to <Type> (), false);
	    return type;		
	}

	Generator Visitor::validateTypeClassRef (const syntax::Expression & expr, bool lock) {
	    auto type = validateTypeClassRef (expr);
	    if (lock && !type.to<Type> ().isMutable ())
	    return Type::init (type.to <Type> (), false);
	    return type;		
	}

	Generator Visitor::validateTypeClassRef (const syntax::Expression & type) {
	    Generator val (Generator::empty ());
	    match (type) {
		of (syntax::Var, var) 
		    val = validateTypeVar (var);
		
		elof (syntax::DecoratedExpression, dec_expr)
		    val = validateTypeDecorated (dec_expr, true);
				
		elof (TemplateSyntaxWrapper, tmplSynt)
		    val =  tmplSynt.getContent ();
		
		fo;		
	    }

	    if (val.isEmpty ()) {		
		val = validateValue (type, true, true); // Can't make a implicit call validation if we are looking for a type
	    }
		
	    if (val.is<ClassRef> ()) return val;	   
	    if (val.is <generator::Class> ()) {
		return val.to <generator::Class> ().getClassRef ();
	    }
	    
	    Ymir::Error::occur (type.getLocation (), ExternalError::get (USE_AS_TYPE));
	    return Generator::empty ();	  
	}
	
	Generator Visitor::validateType (const syntax::Expression & type) {
	    Generator val (Generator::empty ());
	    match (type) {
		of (syntax::ArrayAlloc, array)
		    val = validateTypeArrayAlloc (array);
		
		elof (syntax::Var, var)
		    val = validateTypeVar (var);		

		elof (syntax::DecoratedExpression, dec_expr)
		    val = validateTypeDecorated (dec_expr);		

		elof (syntax::Unary, un)
		    val = validateTypeUnary (un);		

		elof (syntax::Try, tr)
		    val = validateTypeTry (tr);		
		
		elof (syntax::List, list) {
		    if (list.isArray ())
		    val = validateTypeSlice (list);
		    if (list.isTuple ())
		    val = validateTypeTuple (list);
		}

		elof (TemplateSyntaxList, tmplSynt)
		    val = validateTypeTupleTemplate (tmplSynt);		
		
		elof (TemplateSyntaxWrapper, tmplSynt)
		    val =  tmplSynt.getContent ();
		
		elof (syntax::TemplateCall, tmpCall) 
		    val = validateTypeTemplateCall (tmpCall);
		
		fo;		
	    }

	    if (val.isEmpty ()) {		
		val = validateValue (type, true, true, false, true); // Can't make a implicit call validation if we are looking for a type
	    }

	    
	    if (val.is <ClassRef> () || val.is <generator::Class> ()) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (FORGET_TOKEN), Token::AND);
		Ymir::Error::occurAndNote (type.getLocation (), note, ExternalError::get (USE_AS_TYPE));
	    }
		
	    if (val.is<Type> ()) return val;
	    if (val.is<generator::Struct> ())
	    return StructRef::init (type.getLocation (), val.to <generator::Struct> ().getRef ());
	    
	    // if (val.is <StructCst> ()) return val.to <StructCst> ().getStr ();, Why?
	    Ymir::Error::occur (type.getLocation (), ExternalError::get (USE_AS_TYPE));
	    return Generator::empty ();	    	   
	}

	Generator Visitor::validateTypeVar (const syntax::Var & var) {
	    if (std::find (Integer::NAMES.begin (), Integer::NAMES.end (), var.getName ().getStr ()) != Integer::NAMES.end ()) {
		auto size = var.getName ().getStr ().substr (1);
		
		// According to c++ documentation atoi return 0, if the conversion failed
		return Integer::init (var.getName (), std::atoi (size.c_str ()), var.getName ().getStr ()[0] == 'i');
	    } else if (var.getName ().getStr () == Void::NAME) {
		return Void::init (var.getName ());
	    } else if (var.getName ().getStr () == Bool::NAME) {
		return Bool::init (var.getName ());
	    } else if (std::find (Float::NAMES.begin (), Float::NAMES.end (), var.getName ().getStr ()) != Float::NAMES.end ()) {
		auto size = var.getName ().getStr ().substr (1);
		return Float::init (var.getName (), std::atoi (size.c_str ())); 
	    } else if (std::find (Char::NAMES.begin (), Char::NAMES.end (), var.getName ().getStr ()) != Char::NAMES.end ()) {
		return Char::init (var.getName (), std::atoi (var.getName ().getStr ().substr (1).c_str ()));
	    } else {		
		auto syms = getGlobal (var.getName ().getStr ());
		// println( var.getName ().getStr (), " ", this-> _referent.back ().formatTree ());
		// for (auto & it : syms)
		//     println (it.getRealName ());
		if (!syms.empty ()) {
		    auto ret = validateMultSymType (var.getLocation (), syms);		    
		    if (!ret.isEmpty ()) return ret;		    
		} else {
		    syms = getGlobalPrivate (var.getName ().getStr ());
		    std::list <Ymir::Error::ErrorMsg> notes;
		    for (auto it : Ymir::r (0, syms.size ())) {
			notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (PRIVATE_IN_THIS_CONTEXT), syms [it].getName (), syms [it].getRealName ()));
		    }
		    
		    Error::occurAndNote (var.getName (), notes, ExternalError::get (UNDEF_TYPE), var.getName ().getStr ());
		}
	    }

	    Error::occur (var.getName (), ExternalError::get (UNDEF_TYPE), var.getName ().getStr ());
	    return Generator::empty ();
	}

	Generator Visitor::validateTypeUnary (const syntax::Unary & un) {
	    auto op = un.getOperator ();
	    if (op == Token::AND) { // Pointer
		try {
		    auto inner = validateType (un.getContent (), true);		
		    if (!inner.isEmpty ()) return Pointer::init (un.getLocation (), inner);
		} catch (Error::ErrorList &list) {
		    try {
			auto inner = validateTypeClassRef (un.getContent (), true);
			if (inner.is <ClassRef> ()) {
			    auto ret =  ClassPtr::init (un.getLocation (), inner);
			    return ret;
			}
		    } catch (Error::ErrorList &ignore) {
			throw list;
		    }
		}
	    }

	    return Generator::empty ();
	}

	Generator Visitor::validateTypeTry (const syntax::Try & tr) {
	    auto inner = validateType (tr.getContent (), true);
	    auto syntaxType = createClassTypeFromPath (tr.getLocation (), {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (EXCEPTION_TYPE)});
	    auto errType = Type::init (validateType (syntaxType).to <Type> (), false, false);
	    
	    if (!inner.isEmpty ()) return Option::init (tr.getLocation (), inner, errType);
	    return Generator::empty ();
	}

	Generator Visitor::validateTypeDecorated (const syntax::DecoratedExpression & expr, bool canBeClassRef) {
	    Generator type (Generator::empty ());
	    if (canBeClassRef) 
	    type = validateTypeClassRef (expr.getContent ());
	    else
	    type = validateType (expr.getContent ());
	    
	    lexing::Word gotConstOrMut (lexing::Word::eof ());
	    for (auto & deco : expr.getDecorators ()) {
		switch (deco.getValue ()) {
		case syntax::Decorator::REF : type = Type::init (type.to<Type> (), type.to <Type> ().isMutable (), true); break;
		case syntax::Decorator::CONST : {
		    if (!gotConstOrMut.isEof ()) {
			auto note = Ymir::Error::createNote (gotConstOrMut);
			Ymir::Error::occurAndNote (expr.getDecorator (syntax::Decorator::CONST).getLocation (), note, ExternalError::get (CONFLICT_DECORATOR));
		    }
		    gotConstOrMut = expr.getDecorator (syntax::Decorator::CONST).getLocation ();
		    type = Type::init (type.to<Type> (), false); break;
		}
		case syntax::Decorator::MUT : {
		    if (!gotConstOrMut.isEof ()) {
			auto note = Ymir::Error::createNote (gotConstOrMut);
			Ymir::Error::occur (expr.getDecorator (syntax::Decorator::MUT).getLocation (), ExternalError::get (CONFLICT_DECORATOR));
		    }
		    gotConstOrMut = expr.getDecorator (syntax::Decorator::MUT).getLocation ();
		    type = Type::init (type.to<Type> (), true); break;
		}
		case syntax::Decorator::DMUT : {
		    if (!gotConstOrMut.isEof ()) {
			auto note = Ymir::Error::createNote (gotConstOrMut);
			Ymir::Error::occur (expr.getDecorator (syntax::Decorator::DMUT).getLocation (), ExternalError::get (CONFLICT_DECORATOR));		    
		    }
		    gotConstOrMut = expr.getDecorator (syntax::Decorator::DMUT).getLocation ();
		    type = type.to<Type> ().toDeeplyMutable (); break;
		}
		default :
		    Ymir::Error::occur (deco.getLocation (),
					ExternalError::get (DECO_OUT_OF_CONTEXT),
					deco.getLocation ().getStr ()
			);		
		}
	    }
	    	   	    
	    return type;
	}

	Generator Visitor::validateTypeArrayAlloc (const syntax::ArrayAlloc & alloc) {
	    if (alloc.isDynamic ())
	    Ymir::Error::occur (alloc.getLocation (), ExternalError::get (USE_AS_TYPE));

	    auto type = validateType (alloc.getLeft (), true);
	    auto size = validateValue (alloc.getSize ());

	    Generator value = retreiveValue (size);
	    if (!value.is <Fixed> () || (value.to<Fixed> ().getType ().to <Integer> ().isSigned () && value.to <Fixed> ().getUI ().i < 0)) {
		Ymir::Error::occur (alloc.getSize ().getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    value.to <Value> ().getType ().to <Type> ().getTypeName (),
				    (Integer::init (lexing::Word::eof (), 64, false)).to<Type> ().getTypeName ()
		    );
	    }
	    
	    return Array::init (alloc.getLocation (), type, value.to <Fixed> ().getUI ().u);	    
	}

	Generator Visitor::validateTypeSlice (const syntax::List & list) {
	    if (list.getParameters ().size () != 1)
	    Ymir::Error::occur (list.getLocation (), ExternalError::get (USE_AS_TYPE));

	    auto type = validateType (list.getParameters () [0], true);
	    return Slice::init (list.getLocation (), type);	    
	}
	
	Generator Visitor::validateTypeTuple (const syntax::List & list) {
	    std::vector <Generator> params;
	    for (auto & it : list.getParameters ()) {
		params.push_back (validateType (it, true));
	    }
	    
	    return Tuple::init (list.getLocation (), params);
	}	

	Generator Visitor::validateTypeTupleTemplate (const TemplateSyntaxList & lst) {
	    std::vector<Generator> params;
	    for (auto & it : lst.getContents ()) {
		params.push_back (validateType (TemplateSyntaxWrapper::init (it.getLocation (), it), true));
	    }

	    return Tuple::init (lst.getLocation (), params);
	}

	Generator Visitor::validateTypeClassContext (const lexing::Word & loc, const Generator & cl, const syntax::Expression & type) {
	    auto sym = cl.to <ClassPtr> ().getInners ()[0].to <ClassRef> ().getRef ();
	    Generator gen (Generator::empty ());
	    std::list <Error::ErrorMsg> errors;
	    
	    pushReferent (sym, "validateTypeClContext");	    
	    try {
		gen = this-> validateType (type, true);
	    } catch (Error::ErrorList &list) {
		errors = list.errors;
	    }
	    popReferent ("validateTypeClContext");
	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};
	    
	    return gen;	    
	}

		Generator Visitor::validateTypeInfo (const lexing::Word & loc, const Generator & type_) {
	    auto type = Type::init (type_.to <Type> (), false, false);
	    if (type.is <ClassPtr> ())
		type = type.to <ClassPtr> ().getInners ()[0];
	    
	    auto typeInfo = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (TYPE_INFO_MODULE), CoreNames::get (TYPE_INFO)});
		
	    auto str = validateType (typeInfo);

	    auto typeIDs = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (TYPE_INFO_MODULE), CoreNames::get (TYPE_IDS)});
	    auto en_m = validateValue (typeIDs);
	       
	    std::vector <Generator> types = {
		Integer::init (loc, 32, false),
		Integer::init (loc, 0, false),
		Slice::init (loc, str),
		Slice::init (loc, Char::init (loc, 32))
	    };

	    std::vector <Generator> innerTypes;
	    if (!type.is <ClassRef> ()) {
		if (type.to <Type> ().isComplex ()) {
		    for (auto & it : type.to <Type> ().getInners ())
			innerTypes.push_back (validateTypeInfo (loc, it));
		}
	    } else {		
		if (!type.to <ClassRef> ().getAncestor ().isEmpty ()) {
		    innerTypes.push_back (validateTypeInfo (loc, type.to <ClassRef> ().getAncestor ()));
		}
	    }
	    
	    auto arrayType = Array::init (loc, str, innerTypes.size ());
	    auto stringLit = syntax::String::init (loc, loc, lexing::Word::init (loc, type.prettyString ()), lexing::Word::eof ());
	    auto name = validateValue (stringLit);
	    auto constName = Mangler::init ().mangle (type) + "_" + "name";
	    auto constNameInner = Mangler::init ().mangle (type) + "_" + "nameInner";
	    auto sliceType = Slice::init (loc, str);
	    auto inner = name.to <Aliaser> ().getWho ();

	    std::vector <Generator> values = {
		en_m.to <generator::Enum> ().getFieldValue (typeInfoName (type)),
		SizeOf::init (loc, Integer::init (loc, 0, false), type),
		Copier::init (loc, sliceType, Aliaser::init (loc, sliceType, ArrayValue::init (loc, arrayType, innerTypes))),	       
		GlobalConstant::init (loc, constName, name.to <Value> ().getType (),
				      Aliaser::init (loc, name.to <Value> ().getType (),
						     GlobalConstant::init (
							 loc, constNameInner, inner.to<Value> ().getType (), inner
						     )
				      )
		)
	    };
	    
	    return StructCst::init (
		loc,
		str,
		str.to <StructRef> ().getRef ().to <semantic::Struct> ().getGenerator (),
		types,
		values
	    );
	}

	std::string Visitor::typeInfoName (const Generator & type) {
	    // Maybe that's enhanceable
	    match (type) {
		of_u (Void) return "VOID";
		elof_u (Array) return "ARRAY";
		elof_u (Bool) return "BOOL";
		elof_u (Char) return "CHAR";
		elof_u (Closure) return "CLOSURE";
		elof_u (Float) return "FLOAT";
		elof_u (FuncPtr) return "FUNC_PTR";
		elof (Integer,  i) {
		    if (i.isSigned ()) return "SIGNED_INT";
		    return "UNSIGNED_INT";
		}
		elof_u (Pointer) return "POINTER";
		elof_u (Slice) return "SLICE";
		elof_u (StructRef) return "STRUCT";
		elof_u (Tuple) return "TUPLE";
		elof_u (ClassRef) return "OBJECT";
		fo;
	    }
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return "";
	}


		Generator Visitor::validateFuncPtr (const syntax::FuncPtr & ptr) {
	    std::vector <Generator> params;
	    if (ptr.getLocation () == Keys::FUNCTION) {
		for (auto & it : ptr.getParameters ()) {
		    params.push_back (validateType (it, true));
		}
		
		return FuncPtr::init (ptr.getLocation (), validateType (ptr.getRetType (), true), params);
	    } else if (ptr.getLocation () == Keys::DELEGATE) {
		for (auto & it : ptr.getParameters ()) {
		    params.push_back (validateType (it, true));
		}
		
		return Delegate::init (ptr.getLocation (), FuncPtr::init (ptr.getLocation (), validateType (ptr.getRetType (), true), params));
	    } else {
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		return Generator::empty ();
	    }
	}

    }

}
