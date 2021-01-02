#include <ymir/semantic/validator/TemplateVisitor.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <chrono>

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
		auto currentElems = array_view <Generator> (values.begin () + consumed, values.end ());
		
		int current_consumed = 0;
		auto rest = array_view <syntax::Expression> (syntaxTempl.begin () + 1, syntaxTempl.end ());
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
	    std::list <Ymir::Error::ErrorMsg> errors;
	    int consumed = 0;

	    while (consumed < (int) values.size () && syntaxTempl.size () != 0) { 
		auto currentElems = array_view <Generator> (values.begin () + consumed, values.end ());
		
		int current_consumed = 0;
		auto rest = array_view <syntax::Expression> (syntaxTempl.begin () + 1, syntaxTempl.end ());
		Mapper mapper {false, 0, {}, {}};
		bool succeed = true;
		try {
		    mapper = validateParamTemplFromExplicit (rest, syntaxTempl [0], currentElems, current_consumed);
		} catch (Error::ErrorList list) {		    		    
		    errors = list.errors;
		    succeed = false;
		} 
		
		if (!succeed) {
		    auto prevMapper = Mapper {true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to<Template> ().getSpecNameOrder ()};
		    auto merge = mergeMappers (prevMapper, globalMapper);
		    errors.push_back (this-> partialResolutionNote (ref.getLocation (), merge));
		    throw Error::ErrorList {errors};
		}		
		
		if (!mapper.succeed) return Symbol::empty ();
		else {
		    globalMapper = mergeMappers (globalMapper, mapper);		    
		    syntaxTempl = replaceSyntaxTempl (syntaxTempl, globalMapper.mapping);
		    consumed += current_consumed;
		}		
	    }

	    if (consumed < (int) values.size ()) {
		Ymir::OutBuffer buf;
		for (auto it : Ymir::r (consumed, values.size ())) {
		    if (it != consumed) buf.write (", ");
		    buf.write (values [it].prettyString ());
		}
		
		Ymir::Error::occur (values [consumed].getLocation (), ExternalError::get (TEMPLATE_REST),
				    buf.str ());
		return Symbol::empty ();
	    } else if (!globalMapper.succeed) {
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
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
						"",
						syntaxTempl,						
						sym.to <semantic::Template> ().getDeclaration (),
						sym.to <semantic::Template> ().getTest (),
						sym.to <semantic::Template> ().getParams ()
						, true
			);
		    
		    sym2.to <Template> ().setPreviousSpecialization (merge.mapping);
		    sym2.to <Template> ().setSpecNameOrder (merge.nameOrder);		
		
		    ref.getTemplateRef ().getReferent ().insertTemplate (sym2);
		
		    return sym2;
		} else {
		    score = merge.score;
		    finalValidation (ref.getTemplateRef ().getReferent (), sym.to <Template> ().getPreviousParams (), merge, sym.to <semantic::Template> ().getTest ());
		    auto final_syntax = replaceAll (sym.to <semantic::Template> ().getDeclaration (), merge.mapping, ref.getTemplateRef ().getReferent ());

		    auto visit = declarator::Visitor::init ();
		    visit.setWeak ();
		    visit.pushReferent (ref.getTemplateRef ().getReferent ());
		    
		    auto soluce = TemplateSolution::init (sym.getName (), sym.getComments (), sym.to <semantic::Template> ().getParams (), merge.mapping, merge.nameOrder, true);
		    visit.pushReferent (soluce);
		    visit.visit (final_syntax);
		    auto glob = visit.popReferent ();
		    glob.setReferent (visit.getReferent ());
		    
		    auto already = getTemplateSolution (visit.getReferent (), soluce);
		    if (already.isEmpty ()) {			
			visit.getReferent ().insertTemplate (glob);
		    } else glob = already;
		    
		    return glob;
		}
	    }
	}

	TemplateVisitor::Mapper TemplateVisitor::validateParamTemplFromExplicit (const array_view <syntax::Expression> & syntaxTempl, const syntax::Expression & param, const array_view <Generator> & values, int & consumed) const {
	    match (param) {
		of (Var, var, {
			if (values [0].is<Type> ()) {
			    Mapper mapper (true, Scores::SCORE_VAR);
			    this-> _context.verifyNotIsType (var.getName ());
			    
			    mapper.mapping.emplace (var.getName ().getStr (), createSyntaxType (var.getName (), values [0]));
			    mapper.nameOrder.push_back (var.getName ().getStr ());
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
		) else of (ImplVar, var, {
			if (values [0].is <ClassPtr> ()) {
			    consumed += 1;
			    return applyTypeFromExplicitImplVar (syntaxTempl, var, values [0]);
			} else if (values [0].is <Type> ()) {
			    auto note = Ymir::Error::createNote (param.getLocation ());
			    Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (NOT_A_CLASS), values [0].prettyString ());
			} else {
			    auto note = Ymir::Error::createNote (param.getLocation ());
			    Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (USE_AS_TYPE));
			}
		    }
		) else of (StructVar, var, {
			if (values [0].is <Type> ()) {
			    if (values [0].is <StructRef> ()) {
				Mapper mapper (true, Scores::SCORE_VAR);
				mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), values [0]));
				mapper.nameOrder.push_back (var.getLocation ().getStr ());
				consumed += 1;
				return mapper;
			    } else {
				auto note = Ymir::Error::createNote (param.getLocation ());
				Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (NOT_A_STRUCT));
			    }
			} else {
			    auto note = Ymir::Error::createNote (param.getLocation ());
			    Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (USE_AS_TYPE));
			}
		    }
		) else of (ClassVar, var, {
			if (values [0].is <Type> ()) {
			    if (!values [0].isEmpty () && values [0].is <ClassPtr> ()) {
				Mapper mapper (true, Scores::SCORE_VAR);
				mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), values [0]));
				mapper.nameOrder.push_back (var.getLocation ().getStr ());
				consumed += 1;
				return mapper;
			    } else if (!values [0].isEmpty ()) {
				auto note = Ymir::Error::createNote (param.getLocation ());
				Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (NOT_A_CLASS), values [0].prettyString ());
			    } else {
				Ymir::Error::occur (var.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						    var.prettyString (),
						    NoneType::init (var.getLocation ()).prettyString ());
			    }
			    
			    return Mapper (false, 0);			    
			} else {
			    auto note = Ymir::Error::createNote (param.getLocation ());
			    Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (USE_AS_TYPE));
			}
		    }
		) else of (AliasVar, var, {
			if (values [0].is <Type> ()) {
			    if (values [0].to<Type> ().needExplicitAlias ()) {
				Mapper mapper (true, Scores::SCORE_VAR);
				mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), values [0]));
				mapper.nameOrder.push_back (var.getLocation ().getStr ());
				consumed += 1;
				return mapper;
			    } else {
				auto note = Ymir::Error::createNote (param.getLocation ());
				Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (NOT_AN_ALIAS), values [0].prettyString ());
			    }
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
				auto vec = {values [0].to <Value> ().getType ()};
				mapper = validateTypeFromImplicit  (syntaxTempl, decl.getType (), array_view<Generator> (vec), current_consumed);
				Generator type (Generator::empty ());
				
				// The type can be uncomplete, so it is enclosed it in a try catch
				try {
				    type = this-> _context.validateType (replaceAll (decl.getType (), mapper.mapping)); 
				} catch (Error::ErrorList list) {} 
				
				if (!type.isEmpty ())
				    this-> _context.verifySameType (type, values [0].to <Value> ().getType ());
			    }

			    if (mapper.succeed) {				
				mapper.mapping.emplace (decl.getName ().getStr (), createSyntaxValue (decl.getName (), values [0]));
				mapper.nameOrder.push_back (decl.getName ().getStr ());
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
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), values [0]));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			} else {
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), values));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			}
			consumed += values.size ();
			return mapper;
		    }
		) else of (syntax::DecoratedExpression, dc, {
			Ymir::Error::occur (dc.getLocation (),
					    ExternalError::get (DECO_OUT_OF_CONTEXT),
					    dc.prettyDecorators ()
			);			
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
			param.getLocation ().getStr (),
			param.getLocation ().getLine (),
			param.getLocation ().getColumn ()
		), createSyntaxValue (param.getLocation (), value));
	    mapper.nameOrder.push_back (format ("%[%,%]",
						param.getLocation ().getStr (),
						param.getLocation ().getLine (),
						param.getLocation ().getColumn ()));

	    mapper.score += Scores::SCORE_TYPE;
	    return mapper;
	}

	Generator TemplateVisitor::validateFromImplicit (const Generator & ref, const std::vector <Generator> & valueParams, const std::vector <Generator> & types, int & score, Symbol & symbol, std::vector <Generator> & finalParams) const {
	    const Symbol & sym = ref.to <TemplateRef> ().getTemplateRef ();	    
	    if (!sym.to <semantic::Template> ().getDeclaration ().is <syntax::Function> ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    
	    auto & syntaxParams = sym.to <semantic::Template> ().getDeclaration ().to <syntax::Function> ().getPrototype ().getParameters ();
	    return validateFromImplicit (ref, valueParams, types, syntaxParams, score, symbol, finalParams);	    
	}
	
	Generator TemplateVisitor::validateFromImplicit (const Generator & ref, const std::vector <Generator> & valueParams, const std::vector <Generator> & types_, const std::vector <syntax::Expression> & syntaxParams, int & score, Symbol & symbol, std::vector <Generator> & finalParams) const {
	    const Symbol & sym = ref.to <TemplateRef> ().getTemplateRef ();	    
	    auto syntaxTempl = sym.to <semantic::Template> ().getParams ();

	    std::vector <Generator> types = types_;
	    for (auto & it : types) it = Type::init (it.to <Type> (), false, false); // Implicit can not create mutable template
	    
	    /** INFO : Not enough parameters for the function, actually, it
		is probably not mandatory to check that since this
		function is called by CallVisitor, but I don't know
		for the moment if it can change */
	    if (syntaxParams.size () > types.size ()) return Generator::empty ();
	    Mapper globalMapper (true, 0);

	    int consumed = 0;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    for (auto it : Ymir::r (0, syntaxParams.size ())) {
		auto param = replaceAll (syntaxParams [it], globalMapper.mapping);
		
		auto rest = types.size () - consumed;
		auto syntaxRest = (syntaxParams.size () - it) - 1; // we don't count the current one
		auto right = rest - syntaxRest;
		auto current_types = array_view <Generator> (types.begin () + consumed, types.begin () + consumed + right);
		
		int current_consumed = 0;
		Mapper mapper (false, 0);
		bool succeed = true;
		try {		    
		    mapper = validateVarDeclFromImplicit (syntaxTempl, param, current_types, current_consumed);
		} catch (Error::ErrorList list) {		    
		    errors = list.errors;
		    succeed = false;
		} 
	    
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
			auto innerTuple = std::vector <Generator> (types.begin () + consumed, types.begin () + consumed + current_consumed);
			auto tupleType = Tuple::init (syntaxParams [it].getLocation (), innerTuple);
			auto tupleValues = std::vector <Generator> (valueParams.begin () + consumed, valueParams.begin () + consumed + current_consumed);
			finalParams.push_back (NamedGenerator::init (syntaxParams [it].getLocation (), TupleValue::init (syntaxParams [it].getLocation (), tupleType, tupleValues)));
		    }
		    consumed += current_consumed;
		}		
		// The mapper will be applied on the body at the end only, so we need to merge the differents mappers	
	    }
	    
	    
	    for (auto it : Ymir::r (consumed, valueParams.size ())) {
		finalParams.push_back (valueParams [it]);
	    }

	    if (errors.size () != 0) 
		throw Error::ErrorList {errors};	    
	    
	    if (globalMapper.succeed) {
		auto prevMapper = Mapper (true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to <Template> ().getSpecNameOrder ());
		auto merge = mergeMappers (prevMapper, globalMapper);
		
		merge.mapping = validateLambdaProtos (sym.to <Template> ().getPreviousParams (), merge.mapping);
		merge.nameOrder = sortNames (sym.to<Template> ().getPreviousParams (), merge.mapping);
		
		syntaxTempl = replaceSyntaxTempl (syntaxTempl, merge.mapping);
		
		if (syntaxTempl.size () != 0) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    for (auto it : syntaxTempl) {
			errors.push_back (Ymir::Error::makeOccur (
			    it.getLocation (),
			    ExternalError::get (UNRESOLVED_TEMPLATE)
			));					  
		    }
		    throw Error::ErrorList {errors};
		}

		
		score = merge.score;
		finalValidation (ref.to <TemplateRef> ().getTemplateRef ().getReferent (), sym.to <Template> ().getPreviousParams (), merge, sym.to <semantic::Template> ().getTest ());
		auto func = replaceAll (sym.to <semantic::Template> ().getDeclaration (), merge.mapping, ref.to <TemplateRef> ().getTemplateRef ().getReferent ());
				
		auto visit = declarator::Visitor::init ();
		visit.setWeak ();
		visit.pushReferent (ref.to <TemplateRef> ().getTemplateRef ().getReferent ());
		
		auto soluce = TemplateSolution::init (sym.getName (), sym.getComments (), sym.to <semantic::Template> ().getParams (), merge.mapping, merge.nameOrder, true);
		
		visit.pushReferent (soluce);
		auto sym_func = visit.visit (func);
		auto glob = visit.popReferent ();
		
		// Strange but not always the case, it depends on what is glob 
		glob.setReferent (visit.getReferent ());
		
		auto already = getTemplateSolution (visit.getReferent (), soluce);
		
		if (already.isEmpty ()) {
		    visit.getReferent ().insertTemplate (glob);
		} else glob = already;		
		
		symbol = glob;
		{ // In a block, because it declare a var named ref
		    match (func) {
			of (syntax::Function, f, {
				sym_func = symbol.getLocal (f.getLocation ().getStr ()) [0];
			    }
			) else of (syntax::Class, c, {
				sym_func = symbol.getLocal (c.getLocation ().getStr ()) [0];
			    }
			) else {
				Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");   
			    }
		    }
		} // And ref is already defined 
		
		Generator proto (Generator::empty ());
		this-> _context.pushReferent (sym_func.getReferent (),  "TemplateVisitor::validateFromImplicit");
		
		try {
		    if (ref.is <MethodTemplateRef> ()) {
		    	auto & self = ref.to <MethodTemplateRef> ().getSelf ();
			auto classType = self.to <Value> ().getType ().to <ClassPtr> ().getInners ()[0].to<Type> ().toDeeplyMutable ();
			proto = this-> _context.validateMethodProto (sym_func.to <semantic::Function> (), classType);			
		    } else if (ref.is <TemplateClassCst> ()) {
			proto = this-> _context.validateClass (sym_func, false);
		    } else {
			proto = this-> _context.validateFunctionProto (sym_func.to <semantic::Function> ());
		    }
		} catch (Error::ErrorList list) {		    
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 
		
		this-> _context.popReferent ("TemplateVisitor::validateFromImplicit");
		symbol = glob;
		if (errors.size () != 0) 
		    throw Error::ErrorList {errors};
		return proto;		
	    } else {
		auto prevMapper = Mapper (true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to<Template> ().getSpecNameOrder ());
		auto merge = mergeMappers (prevMapper, globalMapper);
		errors.push_back (this-> partialResolutionNote (ref.getLocation (), merge));
		throw Error::ErrorList {errors};
	    }
	    
	    return Generator::empty ();
	}

	TemplateVisitor::Mapper TemplateVisitor::validateVarDeclFromImplicit (const array_view <Expression> & params, const Expression & left, const array_view <generator::Generator> & types, int & consumed) const {
	    auto type_decl = left.to <syntax::VarDecl> ().getType ();
	    if (!type_decl.isEmpty ())
		return validateTypeFromImplicit (params, type_decl, types, consumed);
	    else { // If the vardecl has no type ...
		// 2h, to find that, what a shame
		consumed += 1;
		return Mapper (true, 0);
	    }
	}	

	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromImplicit (const array_view <Expression> & params, const Expression & leftT, const array_view <generator::Generator> & types, int & consumed) const {
	    match (leftT) {
		of (Var, var, {
			Expression expr = findExpression (var.getName ().getStr (), params);
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
				auto vec = {type.to <Type> ().getInners () [it]};
				
				auto mp = validateTypeFromImplicit (params, param, array_view <Generator> (vec), current_consumed);
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
			    auto vec = {type.to <Type> ().getInners () [0]};
			    auto ret = validateTypeFromImplicit (params, arr.getLeft (), array_view<Generator> (vec), current_consumed);
			    if (ret.succeed && arr.getSize ().is <syntax::Var> ()) {
				Expression exp = findExpressionValue (arr.getSize ().to <syntax::Var> ().getName ().getStr (), params);
				if (!exp.isEmpty ()) {
				    auto local_consumed = 0;
				    auto size = ufixed (arr.getSize ().getLocation (), type.to<Array> ().getSize ());
				    auto vec = {size};
				    
				    auto auxMap = validateParamTemplFromExplicit (params, exp, array_view<Generator> (vec), local_consumed);
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
			if (dc.getContent ().is <Var> ()) {
			    consumed += 1;
			    auto var = dc.getContent ();
			    Mapper localMapper (true, Scores::SCORE_VAR);			    
			    localMapper.mapping.emplace (var.to <Var> ().getName ().getStr (), createSyntaxType (var.to <Var> ().getName (), types [0]));
			    localMapper.nameOrder.push_back (var.to <Var> ().getName ().getStr ());
			    
			    auto realType = this-> replaceAll (leftT, localMapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);
			
			    auto fakeType = this-> replaceAll (dc.getContent (), localMapper.mapping);			
			    auto rightType = this-> _context.validateType (fakeType, true);
			    
			    this-> _context.verifySameType (genType, rightType);
			
			    Mapper mapper (true, Scores::SCORE_VAR);
			    mapper.mapping.emplace (var.to <Var> ().getName ().getStr (), createSyntaxType (leftT.getLocation (), types [0]));
			    mapper.nameOrder.push_back (var.to <Var> ().getName ().getStr ());
			    return mapper;
			}
		    }		    
		) else of (syntax::FuncPtr, fPtr, {
			consumed += 1;
			auto type = types [0];
			if (type.is <Delegate> ()) type = type.to <Type> ().getInners ()[0];
			if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () >= fPtr.getParameters ().size () + 1) {
			    Mapper mapper (false, 0);
			    auto syntaxParams = fPtr.getParameters ();
			    int current_consumed = 0;
			    auto & vec_types = type.to <Type> ().getInners ();
			    auto types = array_view <Generator> (vec_types.begin () + 1, vec_types.end ()); // remove the return type
			    for (auto it : Ymir::r (0, syntaxParams.size ())) {
				auto param = replaceAll (syntaxParams [it], mapper.mapping);
				
				auto rest = types.size () - current_consumed;
				auto syntaxRest = (syntaxParams.size () - it) - 1; // we don't count the current one
				auto right = rest - syntaxRest;
		
				auto current_types = array_view <Generator> (types.begin () + current_consumed, types.begin () + current_consumed + right);
				auto mp = validateTypeFromImplicit (params, param, current_types, current_consumed); // On funcPtr type, the first one is the type
				if (!mp.succeed) return mp;
				mapper = mergeMappers (mapper, mp);
			    }
			    
			    if (current_consumed < (int) types.size ()) {
				Ymir::Error::occur (fPtr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						    fPtr.prettyString (),
						    type.to<Type> ().getTypeName ());

				Mapper mapper (false, 0);
				return mapper;
			    }

			    current_consumed = 0;
			    auto param = replaceAll (fPtr.getRetType (), mapper.mapping);
			    auto vec = {type.to <Type> ().getInners ()[0]};
			    auto mp = validateTypeFromImplicit (params, param, array_view<Generator> (vec), current_consumed); // specialize the return type
			    if (!mp.succeed) return mp;
			    return mergeMappers (mapper, mp);
			} else if (type.is <NoneType> ()) { // None type are accepted, it only refers to templateref i think
			    // And in all cases will be validated in finalvalidation 
			    Mapper mapper (true, 0);
			    return mapper;
			} else {
			    Ymir::Error::occur (fPtr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						fPtr.prettyString (),
						type.to<Type> ().getTypeName ());
			    
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
	
	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromTemplCall (const array_view <syntax::Expression> & params, const syntax::TemplateCall  & cl, const generator::Generator & type, int & consumed) const {
	    Generator currentType = type;
	    // We get the first templateCall in the expression
	    match (currentType) {
		of (StructRef, strRef, {
			auto tmplSoluce = getFirstTemplateSolution (strRef.getRef ());
			if (!tmplSoluce.isEmpty ())
			    return validateTypeFromTemplCall (params, cl, tmplSoluce.to <TemplateSolution> (), consumed);
		    }
		) else of (ClassRef, clRef, {
			auto tmplSoluce = getFirstTemplateSolution (clRef.getRef ());
			if (!tmplSoluce.isEmpty ())
			    return validateTypeFromTemplCall (params, cl, tmplSoluce.to <TemplateSolution> (), consumed);
		    }		    
		) else of (TraitRef, trRef, {
			auto tmplSoluce = getFirstTemplateSolution (trRef.getRef ());
			if (!tmplSoluce.isEmpty ())
			    return validateTypeFromTemplCall (params, cl, tmplSoluce.to <TemplateSolution> (), consumed);			
		    }
		) else of (Range, rng, { // Range are created by template Call
			auto left = cl.getContent ();
			if (left.is<syntax::Var> () && left.to <syntax::Var> ().getName () == Range::NAME) {
			    if (cl.getParameters ().size () == 1) {
				auto vec = {rng.getInners ()[0]};
				return validateTypeFromImplicit (params, cl.getParameters ()[0], array_view <Generator> (vec), consumed);
			    }
			}
		    }
		);			
	    }	

	    // We don't validate anything but, the function that called us will with no new information though
	    return Mapper (true, 0); 
	}

	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromTemplCall (const array_view <syntax::Expression> & params, const syntax::TemplateCall & cl, const semantic::TemplateSolution & soluce, int &) const {
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
			    auto vec = {type};
			    auto mp = validateTypeFromImplicit (params, param, array_view<Generator> (vec), current_consumed);
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
	    
	TemplateVisitor::Mapper TemplateVisitor::applyTypeFromExplicit (const array_view <Expression> & params, const Expression & leftT, const array_view <generator::Generator> & types, int & consumed) const {
	    match (leftT) {
		of (Var, var, {
			if (types [0].isEmpty ()) {
			    // It can happen when we use a lambdaProto as a value in a validateExplicit
			    // and it is not a problem
			    consumed += 1;
			    Mapper mapper (true, 0);
			    return mapper; 
			} else if (!types [0].is<Type> ()) {
			    auto note = Ymir::Error::createNote (var.getLocation ());
			    Ymir::Error::occurAndNote (types [0].getLocation (), note, ExternalError::get (USE_AS_TYPE), types [0].prettyString ());
			} else {			    
			    Mapper mapper (true, Scores::SCORE_VAR);
			    mapper.mapping.emplace (var.getName ().getStr (), createSyntaxType (var.getName (), types [0]));
			    mapper.nameOrder.push_back (var.getName ().getStr ());
			    consumed += 1;
			    return mapper;
			}
		    }
		) else of (OfVar, var, {
			consumed += 1;
			return applyTypeFromExplicitOfVar (params, var, types [0]);
		    }
		) else of (ImplVar, var, {
			if (!types [0].is <ClassPtr> ()) {
			    auto note = Ymir::Error::createNote (var.getLocation ());
			    Ymir::Error::occurAndNote (types [0].getLocation (), note, ExternalError::get (NOT_A_CLASS), types [0].prettyString ());
			}
			
			consumed += 1;
			return applyTypeFromExplicitImplVar (params, var, types [0]);			
		    }
		) else of (StructVar, var, {
			if ((!types [0].isEmpty ()) && types [0].is <StructRef> ()) {
			    Mapper mapper (true, Scores::SCORE_TYPE);
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), types [0]));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			    consumed += 1;
			    return mapper;
			} else if (!types [0].isEmpty ()) {
			    Ymir::Error::occur (var.getLocation (), ExternalError::get (NOT_A_STRUCT),
						types [0].to<Type> ().getTypeName ());
			} else
			    Ymir::Error::occur (var.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						var.prettyString (),
						NoneType::init (var.getLocation ()).to <Type> ().getTypeName ());
			return Mapper (false, 0);
		    }
		) else of (ClassVar, var, {
			if (!types [0].isEmpty () && types [0].is <ClassPtr> ()) {
			    Mapper mapper (true, Scores::SCORE_TYPE);
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), types [0]));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			    consumed += 1;
			    return mapper;
			} else if (!types [0].isEmpty ()) {
			    Ymir::Error::occur (var.getLocation (), ExternalError::get (NOT_A_CLASS),
						types [0].to<Type> ().getTypeName ());
			} else
			    Ymir::Error::occur (var.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						var.prettyString (),
						NoneType::init (var.getLocation ()).to <Type> ().getTypeName ());
			return Mapper (false, 0);
		    }
		) else of (AliasVar, var, {
			if ((!types [0].isEmpty ()) && types [0].to<Type> ().needExplicitAlias ()) {
			    Mapper mapper (true, Scores::SCORE_TYPE);
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), types [0]));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			    consumed += 1;
			    return mapper;
			} else if (!types [0].isEmpty ()) {
			    Ymir::Error::occur (var.getLocation (), ExternalError::get (NOT_AN_ALIAS),
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
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), types [0]));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			} else {
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), types));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			}
			consumed += types.size ();
			return mapper;
		    }
		) else of (DecoratedExpression, dc, {
			Ymir::Error::occur (dc.getLocation (),
					    ExternalError::get (DECO_OUT_OF_CONTEXT),
					    dc.prettyDecorators ()
			);
		    }
		);
	    }

	    OutBuffer buf;
	    leftT.treePrint (buf, 0);
	    println (buf.str ());
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Mapper {false, 0, {}, {}};
	}

	TemplateVisitor::Mapper TemplateVisitor::applyTypeFromExplicitOfVar (const array_view <Expression> & params, const OfVar & ofv, const generator::Generator & type) const {
	    if (type.isEmpty ()) return Mapper {true, 0, {}, {}};
	    match (ofv.getType ()) {
		of (Var, var, {
			auto expr = findExpression (var.getName ().getStr (), params);
			if (!expr.isEmpty ()) {						
			    int consumed = 0;
			    auto vec = {type};
			    Mapper mapper = applyTypeFromExplicit (params, expr, array_view<Generator> (vec), consumed);
			    auto realType = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);

			    this-> _context.verifyNotIsType (ofv.getLocation ());
			    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), genType));
			    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
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
				auto vec = {type.to <Type> ().getInners () [it]};
				auto mp = validateTypeFromImplicit (params, param, array_view<Generator> (vec), consumed);				
				mapper = mergeMappers (mapper, mp);
			    }
			    
			    Expression realType  = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);
			    this-> _context.verifySameType (genType, type);
			    this-> _context.verifyNotIsType (ofv.getLocation ());
			    
			    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), genType));
			    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
			    mapper.score += Scores::SCORE_TYPE;
			    return mapper;
			} else {
			    Ymir::Error::occur (lst.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						NoneType::init (ofv.getLocation ()).to <Type> ().getTypeName (),
						type.to<Type> ().getTypeName ());
			}
		    }
		) else of (syntax::FuncPtr, fPtr, {
			auto ftype = type;
			if (type.is <Delegate> ()) ftype = type.to <Type> ().getInners ()[0];
			if (ftype.to <Type> ().isComplex () && ftype.to<Type> ().getInners ().size () >= fPtr.getParameters ().size () + 1) {
			    Mapper mapper (false, 0);
			    int current_consumed = 0;
			    auto vec_types = type.to <Type> ().getInners ();
			    auto types = array_view <Generator> (vec_types.begin () + 1, vec_types.end ()); // remove the return type
			    auto syntaxParams = fPtr.getParameters ();
			    for (auto it : Ymir::r (0, syntaxParams.size ())) {
				auto param = replaceAll (syntaxParams [it], mapper.mapping);

				auto rest = types.size () - current_consumed;
				auto syntaxRest = (syntaxParams.size () - it) - 1; // we don't count the current one
				auto right = rest - syntaxRest;
		
				auto current_types = array_view <Generator> (types.begin () + current_consumed, types.begin () + current_consumed + right);
				auto mp = validateTypeFromImplicit (params, param, current_types, current_consumed); // On funcPtr type, the first one is the type
				if (!mp.succeed) return mp;
				mapper = mergeMappers (mapper, mp);
			    }

			    if (current_consumed < (int) types.size ()) {
				Ymir::Error::occur (fPtr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						    NoneType::init (ofv.getLocation ()).to <Type> ().getTypeName (),
						    type.to<Type> ().getTypeName ());
				
				Mapper mapper (false, 0);
				return mapper;
			    }

			    current_consumed = 0;
			    auto param = replaceAll (fPtr.getRetType (), mapper.mapping);
			    auto vec = {ftype.to <Type> ().getInners ()[0]};
			    auto mp = validateTypeFromImplicit (params, param, array_view<Generator> (vec), current_consumed); // specialize the return type
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
			    auto vec = {type.to <Type> ().getInners () [0]};
			    Mapper mapper = validateTypeFromImplicit (params, arr.getLeft (), array_view<Generator> (vec), consumed);
			    if (mapper.succeed && arr.getSize ().is <syntax::Var> ()) {
				Expression expr = findExpressionValue (arr.getSize ().to <syntax::Var> ().getName ().getStr (), params);				
				if (!expr.isEmpty ()) {
				    int local_consumed = 0;
				    auto size = ufixed (arr.getSize ().getLocation (), type.to <Array> ().getSize ());
				    auto vec = {size};
				    auto auxMap = validateParamTemplFromExplicit (params, expr, array_view<Generator> (vec), local_consumed);
				    mapper = mergeMappers (mapper, auxMap);
				}
			    }
			    
			    Expression realType  = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);
			    this-> _context.verifySameType (genType, type);
			    this-> _context.verifyNotIsType (ofv.getLocation ());
			    
			    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), genType));
			    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
			    mapper.score += Scores::SCORE_TYPE;
			    return mapper;
			} else {
			    Ymir::Error::occur (arr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						NoneType::init (ofv.getLocation ()).to <Type> ().getTypeName (),
						type.to<Type> ().getTypeName ());
			}
		    }
		) else of (DecoratedExpression, dc, {
			Ymir::Error::occur (dc.getLocation (),
					    ExternalError::get (DECO_OUT_OF_CONTEXT),
					    dc.prettyDecorators ()
			);
		    }
		) else of (TemplateCall, cl, {
			int current_consumed = 0;
			auto mapper = validateTypeFromTemplCall (params, cl, type, current_consumed);
			Expression realType = this-> replaceAll (ofv.getType (), mapper.mapping);
			auto genType = this-> _context.validateType (realType, true);
			this-> _context.verifySameType (genType, type);
			this-> _context.verifyNotIsType (ofv.getLocation ());
			
			mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), genType));
			mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
			mapper.score += Scores::SCORE_TYPE;
			return mapper;			
		    }
		);
	    }
	    
	    // Default case, we just validate it and check the type equality
	    auto left = this-> _context.validateType (ofv.getType (), true);
	    auto score = Scores::SCORE_TYPE;
	    if (type.is <ClassPtr> ()) {
		this-> _context.verifyCompatibleType (ofv.getLocation (), left, type);
		if (left.equals (type))
		    score = Scores::SCORE_TYPE;
		else score = Scores::SCORE_VAR;
	    } else 
		this-> _context.verifySameType (left, type);
			    
	    Mapper mapper (true, score);
	    this-> _context.verifyNotIsType (ofv.getLocation ());
			    
	    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
	    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
	    return mapper;
	}

	TemplateVisitor::Mapper TemplateVisitor::applyTypeFromExplicitImplVar (const array_view <Expression> & params, const ImplVar & implv, const generator::Generator & type) const {
	    if (type.isEmpty ()) return Mapper {true, 0, {}, {}};
	    match (implv.getType ()) {
		of (Var, var, {
			auto expr = findExpression (var.getName ().getStr (), params);
			if (!expr.isEmpty ()) {
			    int consumed = 0;
			    auto vec = {type};
			    Mapper mapper = applyTypeFromExplicit (params, expr, array_view <Generator> (vec), consumed);
			    auto realType = this-> replaceAll (implv.getType (), mapper.mapping);			    
			    auto genType = this-> _context.validateType (realType, true);
			    this-> _context.verifyClassImpl (implv.getLocation (), type, genType);
			    
			    mapper.mapping.emplace (implv.getLocation ().getStr (), createSyntaxType (implv.getLocation (), type));
			    mapper.nameOrder.push_back (implv.getLocation ().getStr ());
			    return mapper;			    
			}
		    }
		) else of (TemplateCall, cl, {
			std::list <Ymir::Error::ErrorMsg> errors;
			for (auto & trait : this-> _context.getAllImplClass (type)) {
			    bool succeed = true;
			    Mapper mapper (true, 0);
			    try {
				int current_consumed = 0;
				auto loc_mapper = validateTypeFromTemplCall (params, cl, trait, current_consumed);		
				Expression realType = this-> replaceAll (implv.getType (), loc_mapper.mapping);
				auto genType = this-> _context.validateType (realType, true);
				this-> _context.verifyClassImpl (implv.getLocation (), type, genType);
				mapper = mergeMappers (loc_mapper, mapper);
			    } catch (Error::ErrorList list) {				
				errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
				succeed = false;
			    } 

			    if (succeed) {
				mapper.mapping.emplace (implv.getLocation ().getStr (), createSyntaxType (implv.getLocation (), type));
				mapper.nameOrder.push_back (implv.getLocation ().getStr ());
				mapper.score += Scores::SCORE_TYPE;
				return mapper;
			    }
			}
			
			throw Error::ErrorList {errors};
		    }
		) else of (syntax::List, lst, {
			Ymir::Error::occur (lst.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    NoneType::init (implv.getLocation ()).to <Type> ().getTypeName (),
					    type.to<Type> ().getTypeName ());		    
		    }
		) else of (syntax::FuncPtr, fPtr, {
			Ymir::Error::occur (fPtr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    NoneType::init (implv.getLocation ()).to <Type> ().getTypeName (),
					    type.to<Type> ().getTypeName ());			
		    }		   
		) else of (syntax::ArrayAlloc, arr, {
			Ymir::Error::occur (arr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    NoneType::init (implv.getLocation ()).to <Type> ().getTypeName (),
					    type.to<Type> ().getTypeName ());		    
		    }
		) else of (DecoratedExpression, dc, {
			Ymir::Error::occur (dc.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    NoneType::init (implv.getLocation ()).to <Type> ().getTypeName (),
					    type.to<Type> ().getTypeName ());	
		    }
		);
	    }
	    
	    auto left = this-> _context.validateType (implv.getType (), true);
	    this-> _context.verifyClassImpl (implv.getLocation (), type, left);
	    Mapper mapper (true, Scores::SCORE_TYPE);
	    this-> _context.verifyNotIsType (implv.getLocation ());
			    
	    mapper.mapping.emplace (implv.getLocation ().getStr (), createSyntaxType (implv.getLocation (), type));
	    mapper.nameOrder.push_back (implv.getLocation ().getStr ());
	    return mapper;	
	}

	
	TemplateVisitor::Mapper TemplateVisitor::applyTypeFromDecoratedExpression (const array_view <Expression> & params, const DecoratedExpression & expr, const array_view <generator::Generator> & types, int & consumed) const {
	    if (expr.hasDecorator (syntax::Decorator::CTE) || expr.hasDecorator (syntax::Decorator::REF))
		return Mapper (false, 0);
	    
	    return validateTypeFromImplicit (params, expr.getContent (), types, consumed);
	}
	
	Expression TemplateVisitor::createSyntaxType (const lexing::Word & location, const generator::Generator & gen// , bool isMutable, bool isRef
	) const {
	    // Syntax template types can't be mutable
	    Generator type = Type::init (location, gen.to <Type> (), false, false);
	    return TemplateSyntaxWrapper::init (location, type);
	}

	Expression TemplateVisitor::createSyntaxType (const lexing::Word & location, const array_view <generator::Generator> & gens) const {
	    std::vector <Generator> types (gens.begin (), gens.end ());
	    for (auto & type : types) {
		type = Type::init (location, type.to<Type> (), type.to <Type> ().isMutable (), false);
	    }
	    return TemplateSyntaxList::init (location, types);
	}	

	Expression TemplateVisitor::createSyntaxValue (const lexing::Word & location, const generator::Generator & gen) const {
	    auto val = Generator::init (location, gen);
	    return TemplateSyntaxWrapper::init (location, val);
	}

	Expression TemplateVisitor::findExpression (const std::string & name, const array_view <Expression> & params) const {
	    for (auto & it : params) {
		match (it) {
		    of (Var, var, {
			    if (var.getName ().getStr () == name) return it;
			}
		    ) else of (OfVar, var, {
			    if (var.getLocation ().getStr () == name) return it;
			}
		    ) else of (ImplVar, var, {
			    if (var.getLocation ().getStr () == name) return it;
			}
		    ) else of (VariadicVar, var, {
			    if (var.getLocation ().getStr () == name) return it;
			}
		    ) else of (StructVar, var, {
			    if (var.getLocation ().getStr () == name) return it;
			}
		    ) else of (ClassVar, var, {
			    if (var.getLocation ().getStr () == name) return it;
			}
		    ) else of (AliasVar, var, {
			    if (var.getLocation ().getStr () == name) return it;
			}
		    ) else of (DecoratedExpression, dc, {
			    if (dc.getContent ().is<Var> () && dc.getContent ().to <Var> ().getName ().getStr () == name) return it;
			}
		    ); // We don't do anything for the rest of expression types as they do not refer to types
		}
	    }
	    return Expression::empty ();
	}


	Expression TemplateVisitor::findExpressionValue (const std::string & name, const array_view <Expression> & params) const {
	    for (auto & it : params) {
		match (it) {
		    of (syntax::VarDecl, var, {
			    if (var.getName ().getStr () == name) return it;
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
			Declaration declMod (Declaration::empty ());
			if (!block.getDeclModule ().isEmpty ())
			    // No reference of symbol, here, but no risk to have a CondBlock inside a normal block
			    declMod = replaceAll (block.getDeclModule (), mapping, semantic::Symbol::empty ());
			for (auto & it : block.getContent ())
			    content.push_back (replaceAll (it, mapping));
			Expression catcher (Expression::empty ());
			if (!block.getCatcher ().isEmpty ()) catcher = replaceAll (block.getCatcher (), mapping);
			std::vector <Expression> scopes;
			for (auto & it : block.getScopes ())
			    scopes.push_back (replaceAll (it, mapping));
			
			return syntax::Block::init (element.getLocation (), block.getEnd (), declMod, content, catcher, scopes);
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
				lexing::Word::init (element.getLocation (), Token::LPAR),
				lexing::Word::init (element.getLocation (), Token::RPAR),
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
		) else of (syntax::ImplVar, im,
			   return syntax::ImplVar::init (
			       element.getLocation (),
			       replaceAll (im.getType (), mapping)
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
		) else of (syntax::ClassVar, var ATTRIBUTE_UNUSED, {
			return syntax::ClassVar::init (
			    element.getLocation ()
			);
		    }
		) else of (syntax::AliasVar, var ATTRIBUTE_UNUSED, {
			return syntax::AliasVar::init (
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
			auto inner = mapping.find (var.getName ().getStr ());
			if (inner != mapping.end ()) {
			    if (inner-> second.is <TemplateSyntaxWrapper> ()) {
				auto tmplSynt = inner-> second.to <TemplateSyntaxWrapper> ();
				auto content = Generator::init (var.getLocation (), tmplSynt.getContent ());
				return TemplateSyntaxWrapper::init (var.getLocation (), content);
			    } else if (inner-> second.is <TemplateSyntaxList> ()) { // Variadic
				std::vector <Generator> params;
				auto tmplSynt = inner-> second.to <TemplateSyntaxList> ();
				for (auto it : tmplSynt.getContents ()) {				   
				    params.push_back (Generator::init (var.getLocation (), it));
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
			auto inner = mapping.find (element.getLocation ().getStr ());
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
		) else of (syntax::Match, m, {
			std::vector <Expression> matchers;
			std::vector <Expression> actions;
			for (auto & it : m.getMatchers ())
			    matchers.push_back (replaceAll (it, mapping));
			for (auto & it : m.getActions ())
			    actions.push_back (replaceAll (it, mapping));
			return syntax::Match::init (m.getLocation (), replaceAll (m.getContent (), mapping),
						    matchers, actions, m.isFinal ());

		    }
		) else of (syntax::Catch, c, {
			std::vector <Expression> matchers;
			std::vector <Expression> actions;
			for (auto & it : c.getMatchs ())
			    matchers.push_back (replaceAll (it, mapping));
			for (auto & it : c.getActions ())
			    actions.push_back (replaceAll (it, mapping));
			
			return syntax::Catch::init (c.getLocation (), matchers, actions);
		    }
		) else of (syntax::Scope, s, {
			return syntax::Scope::init (s.getLocation (), replaceAll (s.getContent (), mapping));
		    }
		) else of (syntax::Throw, thr, {
			return syntax::Throw::init (thr.getLocation (), replaceAll (thr.getValue (), mapping));
		    }
		) else of (syntax::Pragma, prg, {
			std::vector <syntax::Expression> exprs;
			for (auto & it : prg.getContent ()) {
			    exprs.push_back (replaceAll (it, mapping));
			}
			return syntax::Pragma::init (prg.getLocation (), exprs);
		    }
		) else of (syntax::MacroMult, m, {
			std::vector <syntax::Expression> exprs;
			for (auto & it : m.getContent ()) {
			    exprs.push_back (replaceAll (it, mapping));
			}
			return syntax::MacroMult::init (m.getLocation (), m.getEnd (), exprs, m.getMult ());
		    }
		) else of (syntax::MacroCall, c, {
			return syntax::MacroCall::init (c.getLocation (), c.getEnd (), replaceAll (c.getLeft (), mapping), c.getContent ());
		    }
		) else of (syntax::MacroEval, e, {
			return syntax::MacroEval::init (e.getLocation (), e.getEnd (), replaceAll (e.getContent (), mapping));
		    }
		) else of (syntax::MacroOr, o, {
			return syntax::MacroOr::init (o.getLocation (), replaceAll (o.getLeft (), mapping), replaceAll (o.getRight (), mapping));
		    }
		) else of (syntax::MacroResult, r, {
			return syntax::MacroResult::init (r.getLocation (), r.getContent ());
		    }
		) else of (syntax::MacroToken, t, {
			return syntax::MacroToken::init (t.getLocation (), t.getContent ());
		    }
		) else of (syntax::MacroVar, v, {
			return syntax::MacroVar::init (v.getLocation (), replaceAll (v.getContent (), mapping));
		    }
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


	Declaration TemplateVisitor::replaceAll (const Declaration & decl, const std::map <std::string, Expression> & mapping, const Symbol & _ref) const {
	    match (decl) {
		of (syntax::Alias, al, return syntax::Alias::init (al.getLocation (), "", replaceAll (al.getValue (), mapping)))
		else of (syntax::CondBlock, cd, {
			return replaceAll (_ref, cd, mapping);
		    }
		) else of (syntax::DeclBlock, dl, {
			std::vector <Declaration> decls;
			for (auto & it : dl.getDeclarations ())
			    decls.push_back (replaceAll (it, mapping, _ref));
			return syntax::DeclBlock::init (dl.getLocation (), "", decls, dl.isPrivate (), dl.isProt ());
		    }
		) else of (syntax::Class, cl, {
			std::vector <Declaration> decls;
			for (auto & it : cl.getDeclarations ()) {
			    decls.push_back (replaceAll (it, mapping, _ref));
			}
			return syntax::Class::init (cl.getLocation (), "", replaceAll (cl.getAncestor (), mapping), decls, cl.getAttributes ());
		    }
		) else of (syntax::Enum, en, {
			std::vector <Expression> values;
			for (auto & it : en.getValues ())
			    values.push_back (replaceAll (it, mapping));
			return syntax::Enum::init (en.getLocation (), "", replaceAll (en.getType (), mapping), values, {});
		    }
		) else of (syntax::ExpressionWrapper, wrp, {
			return syntax::ExpressionWrapper::init (wrp.getLocation (), "", replaceAll (wrp.getContent (), mapping));
		    }
		) else of (syntax::ExternBlock, ext, {
			return syntax::ExternBlock::init (
			    ext.getLocation (),
			    "", 
			    ext.getFrom (),
			    ext.getSpace (),
			    replaceAll (ext.getDeclaration (), mapping, _ref)
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

			std::vector <syntax::Expression> throwers;
			for (auto &it : cst.getThrowers ())
			    throwers.push_back (replaceAll (it, mapping));
			
			return syntax::Constructor::init (cst.getLocation (),
							  "",
							  replaceAll (cst.getPrototype (), mapping),
							  exprs,
							  fields,
							  replaceAll (cst.getBody (), mapping),
							  cst.getExplicitSuperCall (), cst.getExplicitSelfCall (), cst.getCustomAttributes (),
							  throwers
			);
		    }		    
		) else of (syntax::Function, func, {
			std::vector <syntax::Expression> throwers;
			for (auto &it : func.getThrowers ())
			    throwers.push_back (replaceAll (it, mapping));
			
			return syntax::Function::init (func.getLocation (),
						       "", 
						       replaceAll (func.getPrototype (), mapping),
						       replaceAll (func.getBody (), mapping),
						       func.getCustomAttributes (),
						       throwers,
						       func.isOver ()
			);			
		    }
		) else of (syntax::Global, glb, {
			return syntax::Global::init (glb.getLocation (), "", replaceAll (glb.getContent (), mapping));
		    }
		) else of (syntax::Import, imp ATTRIBUTE_UNUSED, return decl; 
		) else of (syntax::Mixin, mx, {
			std::vector <Declaration> decls;
			for (auto& it : mx.getDeclarations ())
			    decls.push_back (replaceAll (it, mapping, _ref));
			return syntax::Mixin::init (mx.getLocation (), "", replaceAll (mx.getMixin (), mapping), decls);
		    }
		) else of (syntax::Module, mod, {
			std::vector <Declaration> decls;
			for (auto & it : mod.getDeclarations ())
			    decls.push_back (replaceAll (it, mapping, _ref));
			return syntax::Module::init (
			    mod.getLocation (),
			    "",
			    decls,
			    mod.isGlobal ()
			);
		    }
		) else of (syntax::Struct, str, {
			std::vector <Expression> vars;
			for (auto &it : str.getDeclarations ())
			    vars.push_back (replaceAll (it, mapping));
			return syntax::Struct::init (str.getLocation (), "", str.getCustomAttributes (), vars, {});
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
			    "",
			    params,
			    replaceAll (tmpl.getContent (), mapping, _ref),
			    replaceAll (tmpl.getTest (), mapping)
			);
		    }
		) else of (syntax::Trait, trai, {
			std::vector <Declaration> inner;
			for (auto & it : trai.getDeclarations ())
			    inner.push_back (replaceAll (it, mapping, _ref));
			return syntax::Trait::init (trai.getLocation (), "", inner);
		    }
		) else of (syntax::Use, use, {
			return syntax::Use::init (use.getLocation (), "", replaceAll (use.getModule (), mapping));
		    }
		) else of (syntax::Macro, m, {
			std::vector <Declaration> inner;
			for (auto & it : m.getContent ())
			    inner.push_back (replaceAll (it, mapping, _ref));
			return syntax::Macro::init (m.getLocation (), "", inner);
		    }
		) else of (syntax::MacroConstructor, c, {
			std::vector <Expression> skips;
			for (auto & it : c.getSkips ())
			    skips.push_back (replaceAll (it, mapping));
			return syntax::MacroConstructor::init (c.getLocation (), c.getContentLoc (), "", replaceAll (c.getRule (), mapping), c.getContent (), skips);
		    }
		) else of (syntax::MacroRule, r, {
			std::vector <Expression> skips;
			for (auto & it : r.getSkips ())
			    skips.push_back (replaceAll (it, mapping));
			return syntax::MacroRule::init (r.getLocation (), r.getContentLoc (), "", replaceAll (r.getRule (), mapping), r.getContent (), skips);	
		    }
		    );			   
	    }

	    if (!decl.isEmpty ()) {
		OutBuffer buf;
		decl.treePrint (buf, 0);
		println (buf.str ());
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		return Declaration::empty ();
	    }
	    
	    return Declaration::empty ();
	}

	syntax::Function::Prototype TemplateVisitor::replaceAll (const syntax::Function::Prototype & proto, const std::map <std::string, Expression> & mapping) const {
	    std::vector <Expression> vars;
	    for (auto & it : proto.getParameters ()) {
		vars.push_back (replaceAll (it, mapping));	    
	    }
	    return syntax::Function::Prototype::init (vars, replaceAll (proto.getType (), mapping), proto.isVariadic ());
	    
	}
		
	std::vector <Expression> TemplateVisitor::replaceSyntaxTempl (const std::vector<Expression> & elements, const std::map <std::string, Expression> & mapping) const {
	    if (mapping.size () == 0) return elements;
	    std::vector <Expression> results;	    
	    for (auto & it : elements) {
		match (it) {
		    of (syntax::Var, var, {
			    if (mapping.find (var.getName ().getStr ()) == mapping.end ())
				results.push_back (it);
			    continue;
			}
		    ) else of (syntax::OfVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) == mapping.end ()) 
				results.push_back (replaceAll (it, mapping));
			    continue;
			}
		    ) else of (syntax::ImplVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) == mapping.end ()) 
				results.push_back (replaceAll (it, mapping));
			    continue;
			}
		    ) else of (syntax::VariadicVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) == mapping.end ())
				results.push_back (it);
			    continue;
			}
		    ) else of (syntax::VarDecl, decl, {
			    if (mapping.find (decl.getLocation ().getStr ()) == mapping.end ()) 
				results.push_back (replaceAll (it, mapping));
			    continue;
			}
		    ) else of (syntax::StructVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) == mapping.end ())
				results.push_back (replaceAll (it, mapping));
			    continue;
			}
		    ) else of (syntax::ClassVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) == mapping.end ())
				results.push_back (replaceAll (it, mapping));
			    continue;
			}
		    ) else of (syntax::AliasVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) == mapping.end ())
				results.push_back (replaceAll (it, mapping));
			    continue;
			}
		    ) else of (syntax::DecoratedExpression, dc, {
			    if (mapping.find (dc.getContent ().to <Var> ().getName ().getStr ()) == mapping.end ())
				results.push_back (replaceAll (it, mapping));
			    continue;
			}
		    );

		} // else {

		if (mapping.find (
		    format ("%[%,%]",
			    it.getLocation ().getStr (),
			    it.getLocation ().getLine (),
			    it.getLocation ().getColumn ())) == mapping.end ()) {
		    
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
			    results.push_back (mapping.find (var.getName ().getStr ())-> second);
			}
		    ) else of (syntax::OfVar, var, {
			    results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
			}
		    ) else of (syntax::ImplVar, var, {
			    results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
			}
		    ) else of (syntax::VariadicVar, var, {
			    results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
			}
		    ) else of (syntax::VarDecl, var, {
			    results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
			}
		    ) else of (syntax::StructVar, var, {
			    results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
			}
		    ) else of (syntax::ClassVar, var, {
			    results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
			}
		    ) else of (syntax::AliasVar, var, {
			    results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
			}
		    ) else of (syntax::DecoratedExpression, dc, {
			    results.push_back (mapping.find (dc.getContent ().to <Var> ().getName ().getStr ())-> second);
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
			    if (mapping.find (var.getName ().getStr ()) != mapping.end ())
				results.push_back (var.getName ().getStr ());
			}
		    ) else of (syntax::OfVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
				results.push_back (var.getLocation ().getStr ());
			}
		    ) else of (syntax::ImplVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
				results.push_back (var.getLocation ().getStr ());
			}
		    ) else of (syntax::VariadicVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
				results.push_back (var.getLocation ().getStr ());
			}
		    ) else of (syntax::VarDecl, var, {
			    if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
				results.push_back (var.getLocation ().getStr ());
			}
		    ) else of (syntax::StructVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
				results.push_back (var.getLocation ().getStr ());
			}
		    ) else of (syntax::ClassVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
				results.push_back (var.getLocation ().getStr ());
			}
		    ) else of (syntax::AliasVar, var, {
			    if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
				results.push_back (var.getLocation ().getStr ());
			}
		    ) else {
						    auto name = format ("%[%,%]", it.getLocation ().getStr (), it.getLocation ().getLine (), it.getLocation ().getColumn ());
						    if (mapping.find (name) != mapping.end ())
							results.push_back (name);		    			    
						}
		}
	    }
	    return results;
	}

	Expression TemplateVisitor::retreiveFuncPtr (const Expression & elem, const std::vector <Expression> & syntaxTempl) const {
	    match (elem) {
		of (syntax::FuncPtr, ptr ATTRIBUTE_UNUSED, return elem;)
		else of (syntax::Var, var, {
			auto F = this-> findExpression (var.getName ().getStr (), syntaxTempl);
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
	    try {
		gen = this-> _context.validateType (param, true);
	    } catch (Error::ErrorList list) {} 
	    
	    return gen;
	}

	
	std::map <std::string, syntax::Expression> TemplateVisitor::validateLambdaProtos (const std::vector<syntax::Expression> & exprs, const std::map <std::string, syntax::Expression> & mapping) const {
	    std::list <Ymir::Error::ErrorMsg> errors;
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
		    auto vec = {it.second};
		    auto mp = validateTypeFromImplicit (exprs, param, array_view<Generator> (vec), current_consumed);
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
				auto auxType = this-> _context.validateValue (mapper.mapping.find (decl.getName ().getStr ())-> second);
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

	Declaration TemplateVisitor::replaceAll (const Symbol & ref, const CondBlock & decl, const std::map <std::string, Expression> & mapping) const {
	    auto test = this-> _context.validateTemplateTest (ref, replaceAll (decl.getTest (), mapping));
	    auto val = this-> _context.retreiveValue (test);
	    if (!val.is<BoolValue> ()) {
		Ymir::Error::occur (test.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
				    val.to <Value> ().getType ().to <Type> ().getTypeName (),
				    Bool::NAME
		);
	    }
	    
	    if (val.to<BoolValue> ().getValue ()) {
		std::vector <Declaration> decls;
		for (auto & it : decl.getDeclarations ())
		    decls.push_back (replaceAll (it, mapping, ref));
		
		return syntax::DeclBlock::init (decl.getLocation (), "", decls, true, false);
	    } else if (!decl.getElse ().isEmpty () && decl.getElse ().is <CondBlock> ()) {
		return replaceAll (ref, decl.getElse ().to <CondBlock> (), mapping);
	    } else if (!decl.getElse ().isEmpty ()) {
		return replaceAll (decl.getElse (), mapping, ref);
	    } else return syntax::DeclBlock::init (decl.getLocation (), "", {}, false, false);
	}
	
	Symbol TemplateVisitor::getTemplateSolution (const Symbol & ref, const Symbol & solution) const {
	    for (auto & it : ref.getTemplates ()) {
		if (it.equals (solution)) {
		    return it;
		}
	    }
	    
	    return Symbol::__empty__;
	}

	Ymir::Error::ErrorMsg TemplateVisitor::partialResolutionNote (const lexing::Word & location, const Mapper & mapper) const {
	    if (mapper.nameOrder.size () != 0) {
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
	    } else return Ymir::Error::ErrorMsg ("");
	}	
	
    }
    
    
}
