#include <ymir/semantic/validator/TemplateVisitor.hh>
#include <ymir/semantic/declarator/Visitor.hh>

namespace semantic {

    namespace validator {

	using namespace Ymir;
	using namespace generator;
	using namespace syntax;
	
	TemplateVisitor::TemplateVisitor (Visitor & context) :
	    _context (context)
	{}

	TemplateVisitor TemplateVisitor::init (Visitor & context) {
	    return TemplateVisitor (context);
	}
	
	TemplateVisitor::Mapper TemplateVisitor::validateFromExplicit (const std::vector <Expression> & params, const std::vector <Generator> & values) const {
	    auto syntaxTempl = params;
	    Mapper globalMapper (true, 0);
	    int consumed = 0;
	    while (consumed < (int) values.size () && syntaxTempl.size () != 0) { 
		auto currentElems = std::vector <Generator> (values.begin () + consumed, values.end ());
		
		int current_consumed = 0;
		auto rest = std::vector<syntax::Expression> (syntaxTempl.begin () + 1, syntaxTempl.end ());
		auto mapper = validateParamTemplFromExplicit (rest, syntaxTempl [0], currentElems, current_consumed);	
		if (!mapper.succeed) return mapper;
		else {
		    globalMapper = mergeMappers (globalMapper, mapper);
		    syntaxTempl = replaceSyntaxTempl (syntaxTempl, globalMapper.mapping);
		    consumed += current_consumed;
		}		 
	    }
	    
	    return globalMapper;
	}
	
	Symbol TemplateVisitor::validateFromExplicit (const TemplateRef & ref, const std::vector <Generator> & values, int & score) const {
	    score = -1;
	    const Symbol & sym = ref.getTemplateRef ();
	    auto syntaxTempl = sym.to <semantic::Template> ().getParams ();
	    Mapper globalMapper (true, 0);
	    std::vector<std::string> errors;
	    int consumed = 0;
	    while (consumed < (int) values.size () && syntaxTempl.size () != 0) { 
		auto currentElems = std::vector <Generator> (values.begin () + consumed, values.end ());
		
		int current_consumed = 0;
		auto rest = std::vector<syntax::Expression> (syntaxTempl.begin () + 1, syntaxTempl.end ());
		Mapper mapper {false, 0, {}, {}};
		bool succeed = true;
		TRY (
		    mapper = validateParamTemplFromExplicit (rest, syntaxTempl [0], currentElems, current_consumed);
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);		    
		    errors = msgs;
		    succeed = false;
		} FINALLY;
		
		if (!succeed) {
		    auto prevMapper = Mapper {true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to<Template> ().getSpecNameOrder ()};
		    auto merge = mergeMappers (prevMapper, globalMapper);
		    errors.push_back (this-> partialResolutionNote (ref.getLocation (), merge));
		    THROW (ErrorCode::EXTERNAL, errors);
		}		
		
		if (!mapper.succeed) return Symbol::empty ();
		else {
		    globalMapper = mergeMappers (globalMapper, mapper);
		    syntaxTempl = replaceSyntaxTempl (syntaxTempl, globalMapper.mapping);
		    consumed += current_consumed;
		}		
	    }
	    
	    if (!globalMapper.succeed || consumed < (int) values.size ()) {
		return Symbol::empty ();	    
	    } else {
		auto prevMapper = Mapper {true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to<Template> ().getSpecNameOrder ()};
		auto merge = mergeMappers (prevMapper, globalMapper);
		
		merge.mapping = validateLambdaProtos (sym.to <Template> ().getPreviousParams (), merge.mapping);
		merge.nameOrder = sortNames (sym.to<Template> ().getPreviousParams (), merge.mapping);
		
		syntaxTempl = replaceSyntaxTempl (syntaxTempl, merge.mapping);
		
		if (syntaxTempl.size () != 0) { // Rest some template that are not validated
		    static int __tmpTemplate__ = 0;
		    __tmpTemplate__ += 1;
		    score = merge.score;

		    auto sym2 = Template::init (ref.getLocation (),
						syntaxTempl,
						sym.to <semantic::Template> ().getDeclaration (),
						sym.to <semantic::Template> ().getTest (),
						sym.to <semantic::Template> ().getParams ()
		    );

		    sym2.to <Template> ().setPreviousSpecialization (merge.mapping);
		    sym2.to <Template> ().setSpecNameOrder (merge.nameOrder);		
		
		    ref.getTemplateRef ().getReferent ().insertTemplate (sym2);
		
		    return sym2;
		} else {
		    score = merge.score;
		    finalValidation (ref.getTemplateRef ().getReferent (), sym.to <Template> ().getPreviousParams (), merge, sym.to <semantic::Template> ().getTest ());
		    auto final_syntax = replaceAll (sym.to <semantic::Template> ().getDeclaration (), merge.mapping);
		    auto visit = declarator::Visitor::init ();
		    visit.pushReferent (ref.getTemplateRef ().getReferent ());

		    auto soluce = TemplateSolution::init (sym.getName (), sym.to <semantic::Template> ().getParams (), merge.mapping, merge.nameOrder);
		    auto glob = getTemplateSolution (visit.getReferent (), soluce);
		    if (glob.isEmpty ()) {
			visit.pushReferent (soluce);
			visit.visit (final_syntax);
			glob = visit.popReferent ();
			visit.getReferent ().insertTemplate (glob);
		    }
		    
		    return glob;
		}
	    }
	}

	TemplateVisitor::Mapper TemplateVisitor::validateParamTemplFromExplicit (const std::vector <syntax::Expression> & syntaxTempl, const syntax::Expression & param, const std::vector <Generator> & values, int & consumed) const {
	    match (param) {
		of (Var, var, {
			if (values [0].is<Type> ()) {
			    Mapper mapper (true, Scores::SCORE_VAR);
			    this-> _context.verifyNotIsType (var.getName ());
			    
			    mapper.mapping.emplace (var.getName ().str, createSyntaxType (var.getName (), values [0]));
			    mapper.nameOrder.push_back (var.getName ().str);
			    consumed += 1;
			    return mapper;
			} else {
			    auto note = Ymir::Error::createNote (param.getLocation ());
			    Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (USE_AS_TYPE));
			}
		    }
		) else of (OfVar, var, {
			if (values [0].is<Type> ()) {
			    consumed += 1;
			    return applyTypeFromExplicitOfVar (syntaxTempl, var, values [0]);
			} else {
			    auto note = Ymir::Error::createNote (param.getLocation ());
			    Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (USE_AS_TYPE));
			}
		    }
		) else of (StructVar, var, {
			if (values [0].is <Type> ()) {
			    if (values [0].is <StructRef> ()) {
				Mapper mapper (true, Scores::SCORE_VAR);
				mapper.mapping.emplace (var.getLocation ().str, createSyntaxType (var.getLocation (), values [0]));
				mapper.nameOrder.push_back (var.getLocation ().str);
				consumed += 1;
				return mapper;
			    } else return Mapper (false, 0);
			} else {
			    auto note = Ymir::Error::createNote (param.getLocation ());
			    Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (USE_AS_TYPE));
			}
		    }
		) else of (syntax::VarDecl, decl, {
			if (values [0].is <Value> ()) {
			    consumed += 1;
			    Mapper mapper (false, 0);			    
			    if (!decl.getValue ().isEmpty ()) {
				auto value = this-> _context.retreiveValue (
				    this-> _context.validateValue (decl.getValue ())
				);
				
				if (!values [0].equals (value)) {
				    auto note = Ymir::Error::createNote (values [0].getLocation ());
				    Ymir::Error::occurAndNote (
					value.getLocation (),
					note,
					ExternalError::get (INCOMPATIBLE_VALUES)
				    );
				}
				mapper.succeed = true;
			    }
			    
			    if (!decl.getType ().isEmpty ()) {			    
				auto current_consumed = 0;
				mapper = validateTypeFromImplicit  (syntaxTempl, decl.getType (), {values [0].to <Value> ().getType ()}, current_consumed);
				Generator type (Generator::empty ());
				
				// The type can be uncomplete, so it is enclosed it in a try catch
				TRY (
				    type = this-> _context.validateType (replaceAll (decl.getType (), mapper.mapping)); 
				) CATCH (ErrorCode::EXTERNAL) {
				    GET_ERRORS_AND_CLEAR (msgs);
				} FINALLY;
				
				if (!type.isEmpty ())
				    this-> _context.verifySameType (type, values [0].to <Value> ().getType ());
			    }

			    if (mapper.succeed) {				
				mapper.mapping.emplace (decl.getName ().str, createSyntaxValue (decl.getName (), values [0]));
				mapper.nameOrder.push_back (decl.getName ().str);
			    }
			    
			    mapper.score += Scores::SCORE_VAR;
			    return mapper;
			} else {
			    auto note = Ymir::Error::createNote (param.getLocation ());
			    Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (USE_AS_VALUE));
			}
		    }
		) else of (syntax::VariadicVar, var, {
			Mapper mapper (true, Scores::SCORE_TYPE);
			for (auto & x : values) {
			    if (x.is <Value> ()) {
				auto note = Ymir::Error::createNote (param.getLocation ());
				Ymir::Error::occurAndNote (x.getLocation (), note, ExternalError::get (USE_AS_VALUE));
			    }
			}
			
			if (values.size () == 1) {
			    mapper.mapping.emplace (var.getLocation ().str, createSyntaxType (var.getLocation (), values [0]));
			    mapper.nameOrder.push_back (var.getLocation ().str);
			} else {
			    mapper.mapping.emplace (var.getLocation ().str, createSyntaxType (var.getLocation (), values));
			    mapper.nameOrder.push_back (var.getLocation ().str);
			}
			consumed += values.size ();
			return mapper;
		    }
		);			    
	    }
	    // else, we validate the value directly, and consume one

	    consumed += 1;
	    auto value = this-> _context.retreiveValue (
		this-> _context.validateValue (param)
	    );

	    if (!values [0].equals (value)) {
		auto note = Ymir::Error::createNote (values [0].getLocation ());
		Ymir::Error::occurAndNote (
		    value.getLocation (),
		    note,
		    ExternalError::get (INCOMPATIBLE_VALUES)
		);
	    }

	    Mapper mapper (true, 0);
	    mapper.mapping.emplace (
		format ("%[%,%]",
			param.getLocation ().str,
			param.getLocation ().line,
			param.getLocation ().column
		), param);
		
	    return mapper;
	}
	
	Generator TemplateVisitor::validateFromImplicit (const TemplateRef & ref, const std::vector <Generator> & valueParams, const std::vector <Generator> & types, int & score, Symbol & symbol, std::vector <Generator> & finalParams) const {
	    const Symbol & sym = ref.getTemplateRef ();
	    
	    // For the moment I assume that only function can be implicitly specialized
	    if (!sym.to<semantic::Template> ().getDeclaration ().is <syntax::Function> ())
		return Generator::empty ();
	    
	    auto syntaxParams = sym.to <semantic::Template> ().getDeclaration ().to <syntax::Function> ().getPrototype ().getParameters ();
	    auto syntaxTempl = sym.to <semantic::Template> ().getParams ();

	    /** INFO : Not enough parameters for the function, actually, it
		is probably not mandatory to check that since this
		function is called by CallVisitor, but I don't know
		for the moment if it can change */
	    if (syntaxParams.size () > types.size ()) return Generator::empty ();
	    Mapper globalMapper (true, 0);

	    volatile int consumed = 0; // longjmp
	    std::vector <std::string> errors;
	    
	    for (auto it : Ymir::r (0, syntaxParams.size ())) {
		auto param = replaceAll (syntaxParams [it], globalMapper.mapping);
		auto current_types = std::vector <Generator> (types.begin () + consumed, types.begin () + consumed + 1 + (types.size () - syntaxParams.size ()));
		
		int current_consumed = 0;
		Mapper mapper (false, 0);
		bool succeed = true;
		TRY (
		    mapper = validateVarDeclFromImplicit (syntaxTempl, param, current_types, current_consumed);
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors = msgs;
		    succeed = false;
		} FINALLY;
		
		if (!succeed) {
		    auto prevMapper = Mapper (true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to<Template> ().getSpecNameOrder ());
		    auto merge = mergeMappers (prevMapper, globalMapper);
		    errors.push_back (this-> partialResolutionNote (ref.getLocation (), merge));
		    break;
		}

		// We apply the mapper, to gain time
		if (!mapper.succeed) {
		    globalMapper.succeed = false;
		    break;
		} else {
		    globalMapper = mergeMappers (globalMapper, mapper);
		    syntaxTempl = replaceSyntaxTempl (syntaxTempl, globalMapper.mapping);

		    if (current_consumed <= 1) {
			finalParams.push_back (NamedGenerator::init (syntaxParams [it].getLocation (), valueParams [consumed]));
		    } else {
			// Create a tuple containing the number of types consumed
			auto tupleType = Tuple::init (syntaxParams [it].getLocation (), std::vector <Generator> (types.begin () + consumed, types.begin () + consumed + current_consumed));
			finalParams.push_back (NamedGenerator::init (syntaxParams [it].getLocation (), TupleValue::init (syntaxParams [it].getLocation (), tupleType, std::vector <Generator> (valueParams.begin () + consumed, valueParams.begin () + consumed + current_consumed))));
		    }
		    consumed += current_consumed;
		}		
		// The mapper will be applied on the body at the end only, so we need to merge the differents mappers	
	    }	    

	    for (auto it : Ymir::r (consumed, valueParams.size ())) {
		finalParams.push_back (valueParams [it]);
	    }

	    if (errors.size () != 0) 
		THROW (ErrorCode::EXTERNAL, errors);	    
	    
	    if (globalMapper.succeed) {
		auto prevMapper = Mapper (true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to <Template> ().getSpecNameOrder ());
		auto merge = mergeMappers (prevMapper, globalMapper);
		
		merge.mapping = validateLambdaProtos (sym.to <Template> ().getPreviousParams (), merge.mapping);		
		merge.nameOrder = sortNames (sym.to<Template> ().getPreviousParams (), merge.mapping);
		
		syntaxTempl = replaceSyntaxTempl (syntaxTempl, merge.mapping);
		
		if (syntaxTempl.size () != 0) {
		    std::vector <std::string> errors;
		    for (auto it : syntaxTempl) {
			errors.push_back (Ymir::Error::makeOccur (
			    it.getLocation (),
			    ExternalError::get (UNRESOLVED_TEMPLATE)
			));					  
		    }
		    THROW (ErrorCode::EXTERNAL, errors);
		}
		
		score = merge.score;
		finalValidation (ref.getTemplateRef ().getReferent (), sym.to <Template> ().getPreviousParams (), merge, sym.to <semantic::Template> ().getTest ());
		auto func = replaceAll (sym.to <semantic::Template> ().getDeclaration (), merge.mapping);
		
		auto visit = declarator::Visitor::init ();
		visit.pushReferent (ref.getTemplateRef ().getReferent ());

		auto soluce = TemplateSolution::init (sym.getName (), sym.to <semantic::Template> ().getParams (), merge.mapping, merge.nameOrder);
		auto glob = getTemplateSolution (visit.getReferent (), soluce);
		if (glob.isEmpty ()) {
		    visit.pushReferent (soluce);
		    auto sym_func = visit.visitFunction (func.to <syntax::Function> ());
		    symbol = visit.popReferent ();
		    visit.getReferent ().insertTemplate (symbol);
		    this-> _context.pushReferent (sym_func.getReferent ());
		    Generator proto (Generator::empty ());
		    TRY (
			proto = this-> _context.validateFunctionProto (sym_func.to <semantic::Function> ());
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
		    } FINALLY;
		    this-> _context.popReferent ();
		    
		    if (errors.size () != 0) 
			THROW (ErrorCode::EXTERNAL, errors);
		    return proto;
		} else {
		    symbol = glob;
		    auto sym_func = symbol.getLocal (func.to<syntax::Function> ().getName ().str) [0];
		    Generator proto (Generator::empty ());
		    this-> _context.pushReferent (sym_func.getReferent ());
		    TRY (
			proto = this-> _context.validateFunctionProto (sym_func.to <semantic::Function> ());
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
		    } FINALLY;
		    this-> _context.popReferent ();
		    
		    if (errors.size () != 0) 
			THROW (ErrorCode::EXTERNAL, errors);
		    return proto;		    
		}
	    } 
	    
	    return Generator::empty ();
	}

	TemplateVisitor::Mapper TemplateVisitor::validateVarDeclFromImplicit (const std::vector <Expression> & params, const Expression & left, const std::vector <generator::Generator> & types, int & consumed) const {
	    auto type_decl = left.to <syntax::VarDecl> ().getType ();
	    return validateTypeFromImplicit (params, type_decl, types, consumed);
	}	

	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromImplicit (const std::vector <Expression> & params, const Expression & leftT, const std::vector <generator::Generator> & types, int & consumed) const {
	    match (leftT) {
		of (Var, var, {
			Expression expr = findExpression (var.getName ().str, params);
			if (expr.isEmpty ()) {
			    consumed += 1;
			    // if expression is empty, the type is a real one, and not a template
			    return  Mapper (true, 0);		    			    
			} return applyTypeFromExplicit (params, expr, types, consumed);
		    }
		) else of (TemplateSyntaxList, lst, {
			consumed += lst.getContents ().size ();
			for (auto it : Ymir::r (0, lst.getContents ().size ())) {
			    this-> _context.verifySameType (types [it], lst.getContents () [it]);
			}
			
			return Mapper (true, 0);
		    }
		) else of (TemplateSyntaxWrapper, wp ATTRIBUTE_UNUSED, {
			consumed += 1;
			return Mapper (true, 0);
		    }
		) else of (syntax::List, lst, {
			consumed += 1;
			auto type = types [0];
			if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () == lst.getParameters ().size ()) {
			    Mapper mapper (false, 0);
			    auto syntaxParams = lst.getParameters ();
			    for (auto it : Ymir::r (0, syntaxParams.size ())) {
				auto param = replaceAll (syntaxParams [it], mapper.mapping);
				int current_consumed = 0;
				auto mp = validateTypeFromImplicit (params, param, {type.to <Type> ().getInners () [it]}, current_consumed);
				if (!mp.succeed) return mp;
				mapper = mergeMappers (mapper, mp);
			    }
			    
			    return mapper;
			} else {
			    Ymir::Error::occur (leftT.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						NoneType::init (leftT.getLocation ()).to <Type> ().getTypeName (),
						type.to<Type> ().getTypeName ());

			}
		    }
		) else of (syntax::ArrayAlloc, arr, {
			auto type = types [0];
			int current_consumed = 0;
			if (type.is <Array> ()) {
			    consumed += 1;
			    auto ret = validateTypeFromImplicit (params, arr.getLeft (), {type.to <Type> ().getInners () [0]}, current_consumed);
			    if (ret.succeed && arr.getSize ().is <syntax::Var> ()) {
				Expression exp = findExpressionValue (arr.getSize ().to <syntax::Var> ().getName ().str, params);
				if (!exp.isEmpty ()) {
				    auto local_consumed = 0;
				    auto size = ufixed (arr.getSize ().getLocation (), type.to<Array> ().getSize ());
				    
				    auto auxMap = validateParamTemplFromExplicit (params, exp, {size}, local_consumed);
				    ret = mergeMappers (ret, auxMap);
				}
			    }
			    
			    return ret;
			} else return Mapper (false, 0);
		    }
		) else of (TemplateCall, cl, {
			consumed += 1;
			int current_consumed = 0;
			return validateTypeFromTemplCall (params, cl, types [0], current_consumed);
		    }
		) else of (DecoratedExpression, dc, {
			consumed += 1;
			int current_consumed = 0;
			return applyTypeFromDecoratedExpression (params, dc, types, current_consumed);
		    }
		) else of (syntax::FuncPtr, fPtr, {
			consumed += 1;
			auto type = types [0];
			if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () == fPtr.getParameters ().size () + 1) {
			    Mapper mapper (false, 0);
			    auto syntaxParams = fPtr.getParameters ();
			    for (auto it : Ymir::r (0, syntaxParams.size ())) {
				auto param = replaceAll (syntaxParams [it], mapper.mapping);
				int current_consumed = 0;
				auto mp = validateTypeFromImplicit (params, param, {type.to <Type> ().getInners ()[it + 1]}, current_consumed); // On funcPtr type, the first one is the type
				if (!mp.succeed) return mp;
				mapper = mergeMappers (mapper, mp);
			    }

			    int current_consumed = 0;
			    auto param = replaceAll (fPtr.getRetType (), mapper.mapping);
			    auto mp = validateTypeFromImplicit (params, param, {type.to <Type> ().getInners ()[0]}, current_consumed); // specialize the return type
			    if (!mp.succeed) return mp;
			    return mergeMappers (mapper, mp);
			} else if (type.is <NoneType> ()) { // None type are accepted, it only refers to templateref i think
			    // And in all cases will be validated in finalvalidation 
			    Mapper mapper (true, 0);
			    return mapper;
			} else {
			    Mapper mapper (false, 0);
			    return mapper;
			}
		    }
		);	       
	    }

	    this-> _context.validateType (leftT, true);
	    Mapper mapper (false, 0);
	    return mapper;
	    // OutBuffer buf;
	    // leftT.treePrint (buf, 0);
	    // println (buf.str ());
	    // Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    // return Mapper {};
	}
	
	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromTemplCall (const std::vector <syntax::Expression> & params, const syntax::TemplateCall  & cl, const generator::Generator & type, int & consumed) const {
	    Generator currentType = type;
	    // We get the first templateCall in the expression
	    match (currentType) {
		of (StructRef, strRef, {
			auto tmplSoluce = getFirstTemplateSolution (strRef.getRef ());
			if (!tmplSoluce.isEmpty ())
			    return validateTypeFromTemplCall (params, cl, tmplSoluce.to <TemplateSolution> (), consumed);
		    }
		) else of (Range, rng, { // Range are created by template Call
			auto left = cl.getContent ();
			if (left.is<syntax::Var> () && left.to <syntax::Var> ().getName () == Range::NAME) {
			    if (cl.getParameters ().size () == 1) {
				return validateTypeFromImplicit (params, cl.getParameters ()[0], {rng.getInners ()[0]}, consumed);
			    }
			}
		    }
		) return Mapper (false, 0);
	    }	
	    
	    return Mapper (false, 0);
	}

	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromTemplCall (const std::vector <syntax::Expression> & params, const syntax::TemplateCall & cl, const semantic::TemplateSolution & soluce, int &) const {
	    auto tmpls = sort (soluce.getTempls (), soluce.getParams ());
	    Mapper mapper (false, 0);
	    auto syntaxParams = cl.getParameters ();
	    if (tmpls.size () != syntaxParams.size ()) return Mapper (false, 0);
	    for (auto it : Ymir::r (0, syntaxParams.size ())) {
		auto param = replaceAll (syntaxParams [it], mapper.mapping);
		int current_consumed = 0;
		match (tmpls [it]) {
		    of (generator::TemplateSyntaxWrapper, wrp, {
			    auto type = wrp.getContent ();
			    auto mp = validateTypeFromImplicit (params, param, {type}, current_consumed);
			    if (!mp.succeed) return mp;
			    mapper = mergeMappers (mapper, mp);
			}
		    ) else of (generator::TemplateSyntaxList, lst, {
			    auto mp = validateTypeFromImplicit (params, param, lst.getContents (), current_consumed);
			    if (!mp.succeed) return mp;
			    mapper = mergeMappers (mapper, mp);
			}
		    ) else {
			    println (tmpls [it].prettyString ());
			    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
			}		    
		}
		
	    }
	    return mapper;
	}       
	
	semantic::Symbol TemplateVisitor::getFirstTemplateSolution (const semantic::Symbol & symbol) const {
	    if (symbol.isEmpty ()) return symbol;
	    match (symbol) {
		of (TemplateSolution, sl, {
			auto ft = getFirstTemplateSolution (sl.getReferent ());
			if (ft.isEmpty ()) return symbol;
			return ft;
		    }
		);
	    }
	    
	    return getFirstTemplateSolution (symbol.getReferent ());	    
	}
	    
	TemplateVisitor::Mapper TemplateVisitor::applyTypeFromExplicit (const std::vector <Expression> & params, const Expression & leftT, const std::vector <generator::Generator> & types, int & consumed) const {
	    match (leftT) {
		of (Var, var, {
			if (types [0].isEmpty ()) {
			    // It can happen when we use a lambdaProto as a value in a validateExplicit
			    // and it is not a problem
			    consumed += 1;
			    Mapper mapper (true, 0);
			    return mapper; 
			} else {
			    Mapper mapper (true, Scores::SCORE_VAR);
			    mapper.mapping.emplace (var.getName ().str, createSyntaxType (var.getName (), types [0]));
			    mapper.nameOrder.push_back (var.getName ().str);
			    consumed += 1;
			    return mapper;
			}
		    }
		) else of (OfVar, var, {
			consumed += 1;
			return applyTypeFromExplicitOfVar (params, var, types [0]);
		    }
		) else of (StructVar, var, {
			if ((!types [0].isEmpty ()) && types [0].is <StructRef> ()) {
			    Mapper mapper (true, Scores::SCORE_TYPE);
			    mapper.mapping.emplace (var.getLocation ().str, createSyntaxType (var.getLocation (), types [0]));
			    mapper.nameOrder.push_back (var.getLocation ().str);
			    consumed += 1;
			    return mapper;
			} else if (!types [0].isEmpty ()) {
			    Ymir::Error::occur (var.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						var.prettyString (),
						types [0].to<Type> ().getTypeName ());
			} else
			    Ymir::Error::occur (var.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						var.prettyString (),
						NoneType::init (var.getLocation ()).to <Type> ().getTypeName ());
			return Mapper (false, 0);
		    }
		) else of (VariadicVar, var, {
			Mapper mapper (true, Scores::SCORE_TYPE);
			if (types.size () == 1) {
			    mapper.mapping.emplace (var.getLocation ().str, createSyntaxType (var.getLocation (), types [0]));
			    mapper.nameOrder.push_back (var.getLocation ().str);
			} else {
			    mapper.mapping.emplace (var.getLocation ().str, createSyntaxType (var.getLocation (), types));
			    mapper.nameOrder.push_back (var.getLocation ().str);
			}
			consumed += types.size ();
			return mapper;
		    }
		);
	    }

	    OutBuffer buf;
	    leftT.treePrint (buf, 0);
	    println (buf.str ());
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Mapper {false, 0, {}, {}};
	}

	TemplateVisitor::Mapper TemplateVisitor::applyTypeFromExplicitOfVar (const std::vector <Expression> & params, const OfVar & ofv, const generator::Generator & type) const {
	    if (type.isEmpty ()) return Mapper {true, 0, {}, {}};
	    match (ofv.getType ()) {
		of (Var, var, {
			auto expr = findExpression (var.getName ().str, params);
			if (expr.isEmpty ()) {
			    auto left = this-> _context.validateType (ofv.getType (), true);
			    this-> _context.verifySameType (left, type);
			    
			    Mapper mapper (true, Scores::SCORE_TYPE);
			    this-> _context.verifyNotIsType (ofv.getLocation ());
			    
			    mapper.mapping.emplace (ofv.getLocation ().str, createSyntaxType (ofv.getLocation (), type));
			    mapper.nameOrder.push_back (ofv.getLocation ().str);
			    return mapper;			
			} else {
			    int consumed = 0;
			    Mapper mapper = applyTypeFromExplicit (params, expr, {type}, consumed);
			    auto realType = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);

			    this-> _context.verifyNotIsType (ofv.getLocation ());
			    mapper.mapping.emplace (ofv.getLocation ().str, createSyntaxType (ofv.getLocation (), genType));
			    mapper.nameOrder.push_back (ofv.getLocation ().str);
			    return mapper;			    
			}
		    }
		) else of (syntax::List, lst, {
			if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () == lst.getParameters ().size ()) {
			    Mapper mapper (false, 0);
			    auto syntaxParam = lst.getParameters ();
			    for (auto it : Ymir::r (0, syntaxParam.size ())) {
				auto param = replaceAll (syntaxParam [it], mapper.mapping);
				int consumed = 0;
				auto mp = validateTypeFromImplicit (params, param, {type.to <Type> ().getInners () [it]}, consumed);				
				mapper = mergeMappers (mapper, mp);
			    }
			    
			    Expression realType  = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);
			    this-> _context.verifySameType (genType, type);
			    this-> _context.verifyNotIsType (ofv.getLocation ());
			    
			    mapper.mapping.emplace (ofv.getLocation ().str, createSyntaxType (ofv.getLocation (), genType));
			    mapper.nameOrder.push_back (ofv.getLocation ().str);
			    mapper.score += Scores::SCORE_TYPE;
			    return mapper;
			} else {
			    Ymir::Error::occur (lst.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						NoneType::init (ofv.getLocation ()).to <Type> ().getTypeName (),
						type.to<Type> ().getTypeName ());
			}
		    }
		) else of (syntax::FuncPtr, fPtr, {
			if (type.to <Type> ().isComplex () && type.to<Type> ().getInners ().size () == fPtr.getParameters ().size () + 1) {
			    Mapper mapper (false, 0);
			    auto syntaxParams = fPtr.getParameters ();
			    for (auto it : Ymir::r (0, syntaxParams.size ())) {
				auto param = replaceAll (syntaxParams [it], mapper.mapping);
				int current_consumed = 0;
				auto mp = validateTypeFromImplicit (params, param, {type.to <Type> ().getInners ()[it + 1]}, current_consumed); // On funcPtr type, the first one is the type
				if (!mp.succeed) return mp;
				mapper = mergeMappers (mapper, mp);
			    }

			    int current_consumed = 0;
			    auto param = replaceAll (fPtr.getRetType (), mapper.mapping);
			    auto mp = validateTypeFromImplicit (params, param, {type.to <Type> ().getInners ()[0]}, current_consumed); // specialize the return type
			    if (!mp.succeed) return mp;
			    mapper.score += Scores::SCORE_TYPE;
			    return mergeMappers (mapper, mp);
			} else {
			    Ymir::Error::occur (fPtr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						NoneType::init (ofv.getLocation ()).to <Type> ().getTypeName (),
						type.to<Type> ().getTypeName ());
			}
		    }		   
		) else of (syntax::ArrayAlloc, arr, {
			if (type.is<Array> ()) {
			    int consumed = 0;
			    Mapper mapper = validateTypeFromImplicit (params, arr.getLeft (), {type.to <Type> ().getInners () [0]}, consumed);
			    if (mapper.succeed && arr.getSize ().is <syntax::Var> ()) {
				Expression expr = findExpressionValue (arr.getSize ().to <syntax::Var> ().getName ().str, params);				
				if (!expr.isEmpty ()) {
				    int local_consumed = 0;
				    auto size = ufixed (arr.getSize ().getLocation (), type.to <Array> ().getSize ()); 
				    auto auxMap = validateParamTemplFromExplicit (params, expr, {size}, local_consumed);
				    mapper = mergeMappers (mapper, auxMap);
				}
			    }
			    
			    Expression realType  = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);
			    this-> _context.verifySameType (genType, type);
			    this-> _context.verifyNotIsType (ofv.getLocation ());
			    
			    mapper.mapping.emplace (ofv.getLocation ().str, createSyntaxType (ofv.getLocation (), genType));
			    mapper.nameOrder.push_back (ofv.getLocation ().str);
			    mapper.score += Scores::SCORE_TYPE;
			    return mapper;
			} else {
			    Ymir::Error::occur (arr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						NoneType::init (ofv.getLocation ()).to <Type> ().getTypeName (),
						type.to<Type> ().getTypeName ());
			}
		    }
		) else of (DecoratedExpression, dc, {
			int current_consumed = 0;
			Mapper mapper = applyTypeFromDecoratedExpression (params, dc, {type}, current_consumed);
			Expression realType = this-> replaceAll (ofv.getType (), mapper.mapping);
			auto genType = this-> _context.validateType (realType, true);
			this-> _context.verifySameType (genType, type);
			this-> _context.verifyNotIsType (ofv.getLocation ());
			
			mapper.mapping.emplace (ofv.getLocation ().str, createSyntaxType (ofv.getLocation (), genType, dc.hasDecorator (syntax::Decorator::MUT)));
			mapper.nameOrder.push_back (ofv.getLocation ().str);
			mapper.score += Scores::SCORE_TYPE;
			return mapper;
		    }
		);
	    }

	    OutBuffer buf;
	    ofv.getType ().treePrint (buf, 0);
	    println (buf.str ());
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Mapper (false, 0);
	}

	TemplateVisitor::Mapper TemplateVisitor::applyTypeFromDecoratedExpression (const std::vector <Expression> & params, const DecoratedExpression & expr, const std::vector <generator::Generator> & types, int & consumed) const {
	    if (expr.hasDecorator (syntax::Decorator::CTE) || expr.hasDecorator (syntax::Decorator::REF))
		return Mapper (false, 0);	    

	    auto result = validateTypeFromImplicit (params, expr.getContent (), types, consumed);
	    if (result.succeed) {
		std::map<std::string, syntax::Expression> maps;
		for (auto & it : result.mapping) {
		    match (it.second) {
			of (TemplateSyntaxWrapper, syn, {
				auto content = syn.getContent ();
				if (content.is <Type> ()) {
				    if (expr.hasDecorator (syntax::Decorator::MUT))
					content.to <Type> ().isMutable (true);

				    if (expr.hasDecorator (syntax::Decorator::CONST))
					content.to <Type> ().isMutable (false);				    
				}
				maps.emplace (it.first, TemplateSyntaxWrapper::init (syn.getLocation (), content));
			    }
			) else {
			    maps.emplace (it.first, it.second);
			}
		    }
		}
		return Mapper (true, result.score, maps, result.nameOrder);
	    } else return Mapper (false, 0);	    
	}
	
	Expression TemplateVisitor::createSyntaxType (const lexing::Word & location, const generator::Generator & gen, bool isMutable, bool isRef) const {
	    Generator type = gen;
	    type.to <Type> ().isMutable (isMutable);
	    type.to <Type> ().isRef (isRef);
	    type.changeLocation (location);
	    return TemplateSyntaxWrapper::init (location, type);
	}

	Expression TemplateVisitor::createSyntaxType (const lexing::Word & location, const std::vector <generator::Generator> & gens) const {
	    std::vector <Generator> types = gens;
	    for (auto & type : types)
		type.changeLocation (location);
	    return TemplateSyntaxList::init (location, types);
	}	

	Expression TemplateVisitor::createSyntaxValue (const lexing::Word & location, const generator::Generator & gen) const {
	    auto val = gen;
	    val.changeLocation (location);
	    return TemplateSyntaxWrapper::init (location, val);
	}

	Expression TemplateVisitor::findExpression (const std::string & name, const std::vector <Expression> & params) const {
	    for (auto & it : params) {
		match (it) {
		    of (Var, var, {
			    if (var.getName ().str == name) return it;
			}
		    ) else of (OfVar, var, {
			    if (var.getLocation ().str == name) return it;
			}
		    ) else of (VariadicVar, var, {
			    if (var.getLocation ().str == name) return it;
			}
		    ) else of (StructVar, var, {
			    if (var.getLocation ().str == name) return it;
			}
		    ); // We don't do anything for the rest of expression types as they do not refer to types
		}
	    }
	    return Expression::empty ();
	}


	Expression TemplateVisitor::findExpressionValue (const std::string & name, const std::vector <Expression> & params) const {
	    for (auto & it : params) {
		match (it) {
		    of (syntax::VarDecl, var, {
			    if (var.getName ().str == name) return it;
			}		    
		    ); // We don't do anything for the rest of expression types as they do not refer to values
		}
	    }
	    return Expression::empty ();
	}

	
	Expression TemplateVisitor::replaceAll (const Expression & element, const std::map <std::string, Expression> & mapping) const {
	    if (mapping.size () == 0) return element;
	    match (element) {
		of (syntax::ArrayAlloc, arr,
		    return syntax::ArrayAlloc::init (
			element.getLocation (),
			replaceAll (arr.getLeft (), mapping),
			replaceAll (arr.getSize (), mapping),
			arr.isDynamic ()
		    );
		) else of (Assert, asr,
			   return Assert::init (
			       element.getLocation (),
			       replaceAll (asr.getTest (), mapping),
			       replaceAll (asr.getMsg (), mapping)
			   );
		) else of (syntax::Binary, bin,
			   return syntax::Binary::init (
			       element.getLocation (),
			       replaceAll (bin.getLeft (), mapping),
			       replaceAll (bin.getRight (), mapping),
			       replaceAll (bin.getType (), mapping)
			   );
		) else of (syntax::Block, block, {
			std::vector <Expression> content;
			Declaration declMod = replaceAll (block.getDeclModule (), mapping);
			for (auto & it : block.getContent ())
			    content.push_back (replaceAll (it, mapping));
			return syntax::Block::init (element.getLocation (), block.getEnd (), declMod, content);
		    }
		) else of (syntax::Bool, b ATTRIBUTE_UNUSED, return element;
		) else of (syntax::Break, b, {
			return syntax::Break::init (b.getLocation (), replaceAll (b.getValue (), mapping));
		    }		
		) else of (syntax::Cast, c,
			   return syntax::Cast::init (
			       element.getLocation (),
			       replaceAll (c.getType (), mapping),
			       replaceAll (c.getContent (), mapping)
			   );
		) else of (syntax::Char, c ATTRIBUTE_UNUSED, return element;
		) else of (DecoratedExpression, dec, {
			auto j = replaceAll (dec.getContent (), mapping);
			if (j.is <TemplateSyntaxList> ()) { // need to set the decoration on each element
			    std::vector <Expression> params;
			    for (auto & z : j.to <TemplateSyntaxList> ().getContents ()) {
				params.push_back (
				    DecoratedExpression::init (
					element.getLocation (),
					dec.getDecorators (),
					TemplateSyntaxWrapper::init (z.getLocation (), z))
				);				
			    }
			    return syntax::List::init (
				{element.getLocation (), Token::LPAR},
				{element.getLocation (), Token::RPAR},
				params
			    );
			} else {
			    return syntax::DecoratedExpression::init (
				element.getLocation (),
				dec.getDecorators (),
				j
			    );
			}
		    }
		) else of (syntax::Dollar, dol ATTRIBUTE_UNUSED, return element;
		) else of (syntax::Fixed, fix ATTRIBUTE_UNUSED, return element;
		) else of (syntax::Float, fl ATTRIBUTE_UNUSED, return element;
		) else of (syntax::For, fr, {
			std::vector <Expression> vars;
			for (auto & it : fr.getVars ())
			    vars.push_back (replaceAll (it, mapping));
			return syntax::For::init (
			    element.getLocation (),
			    vars,
			    replaceAll (fr.getIter (), mapping),
			    replaceAll (fr.getBlock (), mapping)
			);
		    }
		) else of (syntax::FuncPtr, fnp, {
			std::vector<Expression> params;
			for (auto & it : fnp.getParameters ()) {
			    auto j = replaceAll (it, mapping);
			    if (j.is <TemplateSyntaxList> ()) { // need to add every content one by one
				for (auto & z : j.to <TemplateSyntaxList> ().getContents ())
				    params.push_back (TemplateSyntaxWrapper::init (z.getLocation (), z));
			    }  else // just a syntax wrapper
				params.push_back (j);
			}
			return syntax::FuncPtr::init (element.getLocation (),
						      replaceAll (fnp.getRetType (), mapping),
						      params);
		    }
		) else of (syntax::If, fi,
			   return syntax::If::init (
			       element.getLocation (),
			       replaceAll (fi.getTest (), mapping),
			       replaceAll (fi.getContent (), mapping),
			       replaceAll (fi.getElsePart (), mapping)
			   );
		) else of (syntax::Ignore, ig ATTRIBUTE_UNUSED, return element;
		) else of (syntax::Intrinsics, intr,
			   return syntax::Intrinsics::init (
			       element.getLocation (),
			       replaceAll (intr.getContent (), mapping)
			   );
		) else of (syntax::Lambda, lmb,
			   return syntax::Lambda::init (
			       element.getLocation (),
			       replaceAll (lmb.getPrototype (), mapping),
			       replaceAll (lmb.getContent (), mapping)
			   );
		) else of (syntax::List, lst, {
			std::vector <Expression> params;
			for (auto & it : lst.getParameters ()) {
			    auto j = replaceAll (it, mapping);
			    if (j.is <TemplateSyntaxList> ()) {
				for (auto & z : j.to <TemplateSyntaxList> ().getContents ())
				    params.push_back (TemplateSyntaxWrapper::init (z.getLocation (), z));
			    } else
				params.push_back (j);
			}
			return syntax::List::init (
			    element.getLocation (),
			    lst.getEnd (),
			    params
			);
		    }
		) else of (syntax::MultOperator, mlt, {
			std::vector <Expression> rights;
			for (auto & it : mlt.getRights ())
			    rights.push_back (replaceAll (it, mapping));
			return syntax::MultOperator::init (
			    element.getLocation (),
			    mlt.getEnd (),
			    replaceAll (mlt.getLeft (), mapping),
			    rights
			);
		    }
		) else of (syntax::NamedExpression, nmd,
			   return syntax::NamedExpression::init (
			       element.getLocation (),
			       replaceAll (nmd.getContent (), mapping)
			   );
		) else of (syntax::Null, nl ATTRIBUTE_UNUSED, return element;
		) else of (syntax::OfVar, of,
			   return syntax::OfVar::init (
			       element.getLocation (),
			       replaceAll (of.getType (), mapping)
			   );
		) else of (syntax::Return, ret,
			   return syntax::Return::init (
			       element.getLocation (),
			       replaceAll (ret.getValue (), mapping)
			   );
		) else of (syntax::Set, set, {
			std::vector <Expression> content;
			for (auto & it : set.getContent ())
			    content.push_back (replaceAll (it, mapping));
			return syntax::Set::init (
			    element.getLocation (),
			    content
			);
		    }
		) else of (syntax::StructVar, var ATTRIBUTE_UNUSED, {
			return syntax::StructVar::init (
			    element.getLocation ()
			);
		    }
		) else of (syntax::String, str ATTRIBUTE_UNUSED, return element;
		) else of (syntax::TemplateCall, tmpl, {
			std::vector <Expression> params;
			for (auto & it : tmpl.getParameters ()) {
			    auto j = replaceAll (it, mapping);
			    if (j.is <TemplateSyntaxList> ()) {
				for (auto & z : j.to <TemplateSyntaxList> ().getContents ())
				    params.push_back (TemplateSyntaxWrapper::init (z.getLocation (), z));
			    } else
				params.push_back (j);
			}
			return syntax::TemplateCall::init (
			    element.getLocation (),
			    params,
			    replaceAll (tmpl.getContent (), mapping)
			);
		    }
		) else of (TemplateSyntaxWrapper, wrp,
			   return TemplateSyntaxWrapper::init (wrp.getLocation (), wrp.getContent ());
		) else of (syntax::Unary, un,
			   return syntax::Unary::init (element.getLocation (), replaceAll (un.getContent (), mapping));
		) else of (syntax::Unit, uni ATTRIBUTE_UNUSED, { return element; }
		) else of (syntax::Var, var, {
			auto inner = mapping.find (var.getName ().str);
			if (inner != mapping.end ()) {
			    if (inner-> second.is <TemplateSyntaxWrapper> ()) {
				auto tmplSynt = inner-> second.to <TemplateSyntaxWrapper> ();
				auto content = tmplSynt.getContent ();
				content.changeLocation (var.getLocation ());
				return TemplateSyntaxWrapper::init (var.getLocation (), content);
			    } else if (inner-> second.is <TemplateSyntaxList> ()) { // Variadic
				std::vector <Generator> params;
				auto tmplSynt = inner-> second.to <TemplateSyntaxList> ();
				for (auto it : tmplSynt.getContents ()) {				   
				    it.changeLocation (var.getLocation ());
				    params.push_back (it);
				}
				return TemplateSyntaxList::init (var.getLocation (), params);
			    } else return inner-> second; // ?!
			}
			else return element;			
		    }
		) else of (syntax::VarDecl, vdecl,
			   return syntax::VarDecl::init (
			       element.getLocation (),
			       vdecl.getDecorators (),
			       replaceAll (vdecl.getType (), mapping),
			       replaceAll (vdecl.getValue (), mapping)
			   );

		) else of (syntax::DestructDecl, ddecl,
			   std::vector <Expression> params;
			   for (auto & it : ddecl.getParameters ())
			       params.push_back (replaceAll (it, mapping));
			   return syntax::DestructDecl::init (
			       ddecl.getLocation (),
			       params,
			       replaceAll (ddecl.getValue (), mapping),
			       ddecl.isVariadic ()
			   );			   
		) else of (syntax::VariadicVar, vdvar ATTRIBUTE_UNUSED, {
			auto inner = mapping.find (element.getLocation ().str);
			if (inner != mapping.end ()) return inner-> second;
			else return element;		    
		    }
		) else of (syntax::While, wh, 
			   return syntax::While::init (
			       element.getLocation (),
			       replaceAll (wh.getTest (), mapping),
			       replaceAll (wh.getContent (), mapping),
			       wh.isDo ()
			   );
		) else of (syntax::TemplateChecker, ch,
			   std::vector <syntax::Expression> params;
			   std::vector <syntax::Expression> calls;
			   for (auto & it : ch.getCalls ()) {
			       auto j = replaceAll (it, mapping);
			       if (j.is <TemplateSyntaxList> ()) {
				   for (auto & z : j.to <TemplateSyntaxList> ().getContents ())
				       params.push_back (TemplateSyntaxWrapper::init (z.getLocation (), z));
			       } else
				   calls.push_back (j);
			   }
			   
			   for (auto & it : ch.getParameters ())
			       params.push_back (replaceAll (it, mapping));
			   return syntax::TemplateChecker::init (
			       element.getLocation (),
			       calls, params
			   );
		);
	    }
	    
	    if (!element.isEmpty ()) {
		OutBuffer buf;
		element.treePrint (buf, 0);
		println (buf.str ());
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		return Expression::empty ();
	    } else return element;
	}


	Declaration TemplateVisitor::replaceAll (const Declaration & decl, const std::map <std::string, Expression> & mapping) const {
	    match (decl) {
		of (syntax::Alias, al, return syntax::Alias::init (al.getName (), replaceAll (al.getValue (), mapping)))
		else of (syntax::DeclBlock, dl, {
			std::vector <Declaration> decls;
			for (auto & it : dl.getDeclarations ())
			    decls.push_back (replaceAll (it, mapping));
			return syntax::DeclBlock::init (dl.getLocation (), decls, dl.isPrivate ());
		    }
		) else of (syntax::Class, cl, {
			std::vector <Declaration> decls;
			for (auto & it : cl.getDeclarations ())
			    decls.push_back (replaceAll (it, mapping));
			return syntax::Class::init (cl.getName (), replaceAll (cl.getAncestor (), mapping), decls);
		    }
		) else of (syntax::Enum, en, {
			std::vector <Expression> values;
			for (auto & it : en.getValues ())
			    values.push_back (replaceAll (it, mapping));
			return syntax::Enum::init (en.getName (), replaceAll (en.getType (), mapping), values);
		    }
		) else of (syntax::ExpressionWrapper, wrp, {
			return syntax::ExpressionWrapper::init (replaceAll (wrp.getContent (), mapping));
		    }
		) else of (syntax::ExternBlock, ext, {
			return syntax::ExternBlock::init (
			    ext.getLocation (),
			    ext.getFrom (),
			    ext.getSpace (),
			    replaceAll (ext.getDeclaration (), mapping)
			);
		    }
		) else of (syntax::Constructor, cst, {
			std::vector <syntax::Expression> exprs;
			for (auto & it : cst.getSuperParams ())
			    exprs.push_back (replaceAll (it, mapping));
			
			auto fields = cst.getFieldConstruction ();
			for (auto & it : fields) {
			    it.second = replaceAll (it.second, mapping);
			};
			
			return syntax::Constructor::init (cst.getName (),
							  replaceAll (cst.getPrototype (), mapping),
							  exprs,
							  fields,
							  replaceAll (cst.getBody (), mapping),
							  cst.getExplicitSuperCall (), cst.getExplicitSelfCall ()
			);
		    }		    
		) else of (syntax::Function, func, {
			auto ret = syntax::Function::init (func.getName (), replaceAll (func.getPrototype (), mapping), replaceAll (func.getBody (), mapping));
			ret.to <syntax::Function> ().setCustomAttributes (func.getCustomAttributes ());
			ret.to <syntax::Function> ().setWeak ();
			if (func.isOver ()) ret.to <syntax::Function> ().setOver ();
			return ret;
		    }
		) else of (syntax::Global, glb, {
			return syntax::Global::init (glb.getLocation (), replaceAll (glb.getContent (), mapping));
		    }
		) else of (syntax::Import, imp ATTRIBUTE_UNUSED, return decl; 
		) else of (syntax::Mixin, mx, {
			return syntax::Mixin::init (mx.getLocation (), replaceAll (mx.getMixin (), mapping));
		    }
		) else of (syntax::Module, mod, {
			std::vector <Declaration> decls;
			for (auto & it : mod.getDeclarations ())
			    decls.push_back (replaceAll (it, mapping));
			auto ret = syntax::Module::init (
			    mod.getIdent (),
			    decls
			);
			ret.to <syntax::Module> ().isGlobal (mod.isGlobal ());
			return ret;
		    }
		) else of (syntax::Struct, str, {
			std::vector <Expression> vars;
			for (auto &it : str.getDeclarations ())
			    vars.push_back (replaceAll (it, mapping));
			return syntax::Struct::init (str.getName (), str.getCustomAttributes (), vars);
		    }
		) else of (syntax::Template, tmpl, {
			std::vector <Expression> params;
			for (auto & it : mapping) {
			    auto expr = findExpression (it.first, tmpl.getParams ());
			    if (!expr.isEmpty ())
				Error::occur (expr.getLocation (), ExternalError::get (SHADOWING_DECL), it.first);
			}
			
			for (auto & it : tmpl.getParams ()) {
			    params.push_back (replaceAll (it, mapping));
			}
			
			return syntax::Template::init (
			    tmpl.getLocation (),
			    params,
			    replaceAll (tmpl.getContent (), mapping),
			    replaceAll (tmpl.getTest (), mapping)
			);
		    }
		) else of (syntax::Trait, trai, {
			std::vector <Declaration> inner;
			for (auto & it : trai.getInners ())
			    inner.push_back (replaceAll (it, mapping));
			return syntax::Trait::init (trai.getName (), inner, trai.isMixin ());
		    }
		) else of (syntax::Use, use, {
			return syntax::Use::init (use.getLocation (), replaceAll (use.getModule (), mapping));
		    }
		);			 
	    }

	    if (!decl.isEmpty ()) {
		OutBuffer buf;
		decl.treePrint (buf, 0);
		println (buf.str ());
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		return Declaration::empty ();
	    } else return decl;
	}

	syntax::Function::Prototype TemplateVisitor::replaceAll (const syntax::Function::Prototype & proto, const std::map <std::string, Expression> & mapping) const {
	    std::vector <Expression> vars;
	    for (auto & it : proto.getParameters ()) {
		if (!it.to <syntax::VarDecl> ().getValue ().isEmpty ()) {
		    /** As we are in a template function, it can be possible that the type of the value is not compatible, with the infered type of the var
		     * We want the following exemple to successfully compile :
		     * --------
		     * def foo (T) (a : T = 12) ...
		     * ...
		     * foo (?a=[1, 2, 3])
		     * --------
		     */
		    vars.push_back (replaceAll (syntax::VarDecl::init (
			it.getLocation (),
			it.to <syntax::VarDecl> ().getDecorators (),
			it.to <syntax::VarDecl> ().getType (),
			syntax::Expression::empty ()
		    ), mapping));
		} else {
		    vars.push_back (replaceAll (it, mapping));
		}
	    }
	    return syntax::Function::Prototype::init (vars, replaceAll (proto.getType (), mapping), proto.isVariadic ());
	    
	}
	
	syntax::Function::Body TemplateVisitor::replaceAll (const syntax::Function::Body & body, const std::map <std::string, Expression> & mapping) const {
	    return syntax::Function::Body::init (
		replaceAll (body.getInner (), mapping),
		replaceAll (body.getBody (), mapping),
		replaceAll (body.getOuter (), mapping),
		body.getOuterName ()
	    );
	}
	
	std::vector <Expression> TemplateVisitor::replaceSyntaxTempl (const std::vector<Expression> & elements, const std::map <std::string, Expression> & mapping) const {
	    if (mapping.size () == 0) return elements;
	    std::vector <Expression> results;	    
	    for (auto & it : elements) {
		match (it) {
		    of (syntax::Var, var, {
			    if (mapping.find (var.getName ().str) == mapping.end ())
				results.push_back (it);
			    continue;
			}
		    ) else of (syntax::OfVar, var, {
			    if (mapping.find (var.getLocation ().str) == mapping.end ()) 
				results.push_back (replaceAll (it, mapping));
			    continue;
			}
		    ) else of (syntax::VariadicVar, var, {
			    if (mapping.find (var.getLocation ().str) == mapping.end ())
				results.push_back (it);
			    continue;
			}
		    ) else of (syntax::VarDecl, decl, {
			    if (mapping.find (decl.getLocation ().str) == mapping.end ()) 
				results.push_back (replaceAll (it, mapping));
			    continue;
			}
		    ) else of (syntax::StructVar, var, {
			    if (mapping.find (var.getLocation ().str) == mapping.end ())
				results.push_back (replaceAll (it, mapping));
			    continue;
			}
		    );

		} // else {

		if (mapping.find (
		    format ("%[%,%]",
			    it.getLocation ().str,
			    it.getLocation ().line,
			    it.getLocation ().column)) == mapping.end ()) {
		    
		    results.push_back (replaceAll (it, mapping));
		}
		
	    }	    
	    return results;
	}	    	    
	
	TemplateVisitor::Mapper TemplateVisitor::mergeMappers (const TemplateVisitor::Mapper & left, const TemplateVisitor::Mapper & right) const {
	    std::map <std::string, Expression> result = left.mapping;
	    std::vector <std::string> nameOrder = left.nameOrder;
	    nameOrder.insert (nameOrder.end (), right.nameOrder.begin (), right.nameOrder.end ());
	    
	    for (auto & it : right.mapping) {
		if (result.find (it.first) != result.end ()) {		    
		    Ymir::Error::halt ("%(r) - reaching impossible point, multiple definition of same template param", "Critical");	    
		} else {
		    result.emplace (it.first, it.second);
		}
	    }
	    return Mapper {true, left.score + right.score, result, nameOrder};
	}

	std::vector <syntax::Expression> TemplateVisitor::sort (const std::vector <syntax::Expression> & exps, const std::map <std::string, syntax::Expression> & mapping) const {
	    std::vector <syntax::Expression> results;
	    for (auto & it : exps) {
		match (it) {
		    of (syntax::Var, var, {
			    results.push_back (mapping.find (var.getName ().str)-> second);
			}
		    ) else of (syntax::OfVar, var, {
			    results.push_back (mapping.find (var.getLocation ().str)-> second);
			}
		    ) else of (syntax::VariadicVar, var, {
			    results.push_back (mapping.find (var.getLocation ().str)-> second);
			}
		    ) else of (syntax::VarDecl, var, {
			    results.push_back (mapping.find (var.getLocation ().str)-> second);
			}
		    ) else of (syntax::StructVar, var, {
			    results.push_back (mapping.find (var.getLocation ().str)-> second);
			}
		    ) else {
				    OutBuffer buf;
				    it.treePrint (buf, 0);
				    println (buf.str ());
				    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");	    
				}
		}
	    }
	    return results;
	}
	
	std::vector <std::string> TemplateVisitor::sortNames (const std::vector <syntax::Expression> & exps, const std::map <std::string, syntax::Expression> & mapping) const {
	    std::vector <std::string> results;
	    for (auto & it : exps) {
		match (it) {
		    of (syntax::Var, var, {
			    if (mapping.find (var.getName ().str) != mapping.end ())
				results.push_back (var.getName ().str);
			}
		    ) else of (syntax::OfVar, var, {
			    if (mapping.find (var.getLocation ().str) != mapping.end ())
				results.push_back (var.getLocation ().str);
			}
		    ) else of (syntax::VariadicVar, var, {
			    if (mapping.find (var.getLocation ().str) != mapping.end ())
				results.push_back (var.getLocation ().str);
			}
		    ) else of (syntax::VarDecl, var, {
			    if (mapping.find (var.getLocation ().str) != mapping.end ())
				results.push_back (var.getLocation ().str);
			}
		    ) else of (syntax::StructVar, var, {
			    if (mapping.find (var.getLocation ().str) != mapping.end ())
				results.push_back (var.getLocation ().str);
			}
		    ) else { // just ignore it, it is a value
				    // OutBuffer buf;
				    // it.treePrint (buf, 0);
				    // println (buf.str ());
				    // Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");	    
				}
		}
	    }
	    return results;
	}

	Expression TemplateVisitor::retreiveFuncPtr (const Expression & elem, const std::vector <Expression> & syntaxTempl) const {
	    match (elem) {
		of (syntax::FuncPtr, ptr ATTRIBUTE_UNUSED, return elem;)
		else of (syntax::Var, var, {
			auto F = this-> findExpression (var.getName ().str, syntaxTempl);
			return retreiveFuncPtr (F, syntaxTempl);
		    }
		) else of (syntax::OfVar, var, {
			if (var.getType ().is<syntax::FuncPtr> ()) return var.getType ();
			else retreiveFuncPtr (var.getType (), syntaxTempl);
		    }
		);
	    }
	    return Expression::empty ();
	}

	Generator TemplateVisitor::validateTypeOrEmpty (const Expression & type, const std::map<std::string, Expression> & mapping) const {
	    auto param = replaceAll (type, mapping);	    
	    Generator gen (Generator::empty ());
	    TRY (
		gen = this-> _context.validateType (param, true);
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
	    } FINALLY;
	    
	    return gen;
	}

	
	std::map <std::string, syntax::Expression> TemplateVisitor::validateLambdaProtos (const std::vector<syntax::Expression> & exprs, const std::map <std::string, syntax::Expression> & mapping) const {
	    std::vector <std::string> errors;
	    std::map <std::string, syntax::Expression> maps;
	    std::vector <std::pair <syntax::Expression, Generator> > toValidate;
	    for (auto & it : mapping) {
		if ((it.second.is <TemplateSyntaxWrapper> () && it.second.to<TemplateSyntaxWrapper> ().getContent ().is <LambdaProto> ()) ||
		    (it.second.is <TemplateSyntaxWrapper> () && it.second.to <TemplateSyntaxWrapper> ().getContent ().is <TemplateRef> ()) ||
		    (it.second.is <TemplateSyntaxWrapper> () && it.second.to <TemplateSyntaxWrapper> ().getContent ().is <MultSym> ())
		) {
		    auto expr = this-> findExpressionValue (it.first, exprs);
		    std::vector <Generator> types;
		    bool successful = true;
		    syntax::Expression retType (syntax::Expression::empty ());
		    if (expr.is <syntax::VarDecl> ()) {
			auto F = retreiveFuncPtr (expr.to <syntax::VarDecl> ().getType (), exprs);
			if (F.is <syntax::FuncPtr> ()) {
			    
			    // If it is a funcPtr, it has the same number of var as the lambda,
			    // it is assumed if we are here, the template spec has been successful
			    for (auto & it : F.to <syntax::FuncPtr> ().getParameters ()) {				
				auto type = validateTypeOrEmpty (it, mapping);
				if (type.isEmpty ()) {
				    successful = false;
				    break;
				} else types.push_back (type);
			    }			    
			}
			if (successful) retType = F.to <syntax::FuncPtr> ().getRetType ();
		    }
		    
		    if (!successful) {
			maps.emplace (it.first, it.second);
			continue;
		    }

		    Generator protoGen (Generator::empty ());
		    if (it.second.to <TemplateSyntaxWrapper> ().getContent ().is <LambdaProto> ()) {
			// We assume that lambda closure cannot be known at compile time, and therefore this will always return a Addresser to a frameproto
			auto proto = it.second.to <TemplateSyntaxWrapper> ().getContent ().to <LambdaProto> ();
			protoGen = this-> _context.validateLambdaProto (proto, types).to <Addresser> ().getWho ();
		    } else if (it.second.to <TemplateSyntaxWrapper> ().getContent ().is <LambdaProto> ()) {
			auto proto = it.second.to <TemplateSyntaxWrapper> ().getContent ();
			protoGen = this-> _context.validateMultSymProto (MultSym::init (proto.getLocation (), {proto}), types);
		    } else {
			auto proto = it.second.to <TemplateSyntaxWrapper> ().getContent ();
			protoGen = this-> _context.validateMultSymProto (proto, types); 
		    }
		    
		    auto type = validateTypeOrEmpty (retType, mapping);
		    if (type.isEmpty ()) {
			toValidate.push_back ({retType, protoGen.to <FrameProto> ().getReturnType ()});
		    } else if (!protoGen.to <FrameProto> ().getReturnType ().equals (type)) {
			auto note = Ymir::Error::createNote (protoGen.to <FrameProto> ().getReturnType ().getLocation ());
			Ymir::Error::occurAndNote (type.getLocation (), note, ExternalError::get (INCOMPATIBLE_TYPES),
						   type.prettyString (),
						   protoGen.to <FrameProto> ().getReturnType ().prettyString ()
			);
		    }

		    auto params = protoGen.to <FrameProto> ().getParameters ();
		    auto ret = protoGen.to <FrameProto> ().getReturnType ();
		    std::vector <Generator> paramTypes;
		    for (auto & it : params) {
			paramTypes.push_back (it.to <generator::ProtoVar> ().getType ());
		    }
				
		    auto funcType = FuncPtr::init (expr.getLocation (), ret, paramTypes);
		    maps.emplace (it.first, TemplateSyntaxWrapper::init (expr.getLocation (), Addresser::init (expr.getLocation (), funcType, protoGen)));		    
		} else maps.emplace (it.first, it.second);		    
	    }

	    if (toValidate.size () != 0) {
		Mapper mapper {true, 0, maps, {}};
		for (auto & it : toValidate) {
		    int current_consumed = 0;
		    auto param = replaceAll (it.first, maps);
		    auto mp = validateTypeFromImplicit (exprs, param, {it.second}, current_consumed);
		    if (!mp.succeed) {
			Ymir::Error::occur (it.first.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    it.first.prettyString (),
					    it.second.prettyString ()
			);
		    }
		    mapper = mergeMappers (mapper, mp);
		}
		
		return validateLambdaProtos (exprs, mapper.mapping);
	    }

	    return maps;
	}

	void TemplateVisitor::finalValidation (const Symbol & context, const std::vector<syntax::Expression> & exprs, const Mapper & mapper, const syntax::Expression & test) const {
	    for (auto & it : exprs) {
		match (it) { 
		    of (syntax::VarDecl, decl, { // it is a value, we need to check the type
			    if (!decl.getType ().isEmpty ()) {
				auto type = this-> _context.validateType (replaceAll (decl.getType (), mapper.mapping), true);
				auto auxType = this-> _context.validateValue (mapper.mapping.find (decl.getName ().str)-> second);
				this-> _context.verifySameType (type, auxType.to <Value> ().getType ());
			    }
			}
		    );
		}
	    }

	    if (!test.isEmpty ()) {
		auto value = this-> _context.validateTemplateTest (context, replaceAll (test, mapper.mapping));
		auto val = this-> _context.retreiveValue (value);
		if (!val.is<BoolValue> ()) {
		    Ymir::Error::occur (test.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					val.to <Value> ().getType ().to <Type> ().getTypeName (),
					Bool::NAME
		    );
		}

		if (!val.to <BoolValue> ().getValue ()) {
		    std::vector <std::string> vec;
		    for (auto & it : mapper.nameOrder) {
			vec.push_back (format ("% -> %", it , mapper.mapping.find (it)-> second.prettyString ()));
		    }
		
		    Ymir::Error::occur (test.getLocation (), ExternalError::get (TEMPLATE_TEST_FAILED), vec);
		}
	    }
	}

	Symbol TemplateVisitor::getTemplateSolution (const Symbol & ref, const Symbol & solution) const {
	    for (auto & it : ref.getTemplates ()) {
		if (it.is <TemplateSolution> () && // It can be a Template, from semi explicit validation 
		    it.to <TemplateSolution> ().getSolutionName () == solution.to <TemplateSolution> ().getSolutionName ()
		    && it.getName ().isSame (solution.getName ())) { // To be similar to template solution must have the same name at the same location


		    return it;
		}
	    }
	    
	    return Symbol::__empty__;
	}

	std::string TemplateVisitor::partialResolutionNote (const lexing::Word & location, const Mapper & mapper) const {
	    Ymir::OutBuffer buf;
	    buf.write ("(");
	    int i = 0;
	    for (auto & it : mapper.nameOrder) {
		if (i != 0)
		    buf.write (",");
		buf.writef ("% = %", it, mapper.mapping.find (it)-> second.prettyString ());
		i += 1;
	    }
	    buf.write (")");
	    return Ymir::Error::createNoteOneLine ("for : % with %", location, buf.str ());
	}	
	
    }
    
    
}
