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

	Symbol TemplateVisitor::validateFromExplicit (const TemplateRef & ref, const std::vector <Generator> & values, int & score) {
	    score = -1;
	    const Symbol & sym = ref.getTemplateRef ();
	    auto syntaxTempl = sym.to <semantic::Template> ().getParams ();
	    Mapper globalMapper;

	    int consumed = 0;
	    while (consumed < (int) values.size () && syntaxTempl.size () != 0) { 
		auto currentElems = std::vector <Generator> (values.begin () + consumed, values.end ());

		int current_consumed = 0;
		auto rest = std::vector<syntax::Expression> (syntaxTempl.begin () + 1, syntaxTempl.end ());
		auto mapper = validateParamTemplFromExplicit (rest, syntaxTempl [0], currentElems, current_consumed);
		if (!mapper.succeed) return Symbol::empty ();
		else {
		    globalMapper = mergeMappers (globalMapper, mapper);
		    syntaxTempl = replaceSyntaxTempl (syntaxTempl, globalMapper.mapping);
		    consumed += current_consumed;
		}
		
	    }

	    if (!globalMapper.succeed || consumed < (int) values.size ()) {
		return Symbol::empty ();	    
	    } else if (syntaxTempl.size () != 0) { // Rest some template that are not validated
		static int __tmpTemplate__ = 0;
		__tmpTemplate__ += 1;
		score = globalMapper.score;
		auto final_syntax = replaceAll (sym.to <semantic::Template> ().getDeclaration (), globalMapper.mapping);
		auto final_test = replaceAll (sym.to <semantic::Template> ().getTest (), globalMapper.mapping);
		auto prevMapper = Mapper {true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to<Template> ().getSpecNameOrder ()};
		auto sym = Template::init (ref.getLocation (), syntaxTempl, final_syntax, final_test);

		auto merge = mergeMappers (prevMapper, globalMapper);
		sym.to <Template> ().setPreviousSpecialization (merge.mapping);
		sym.to <Template> ().setSpecNameOrder (merge.nameOrder);		
		
		ref.getTemplateRef ().getReferent ().insertTemplate (sym);
		
		return sym;
	    } else {
		score = globalMapper.score;
		auto final_syntax = replaceAll (sym.to <semantic::Template> ().getDeclaration (), globalMapper.mapping);
		auto final_test = replaceAll (sym.to <semantic::Template> ().getTest (), globalMapper.mapping);
		auto visit = declarator::Visitor::init ();
		visit.pushReferent (ref.getTemplateRef ().getReferent ());
		auto prevMapper = Mapper {true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to<Template> ().getSpecNameOrder ()};

		auto merge = mergeMappers (prevMapper, globalMapper);
		visit.pushReferent (TemplateSolution::init (sym.getName (), sym.to <semantic::Template> ().getParams (), merge.mapping, merge.nameOrder));
		
		auto sym = visit.visit (final_syntax);
		auto glob = visit.popReferent ();
		visit.getReferent ().insertTemplate (glob);
		return glob;
	    }
	}

	TemplateVisitor::Mapper TemplateVisitor::validateParamTemplFromExplicit (const std::vector <syntax::Expression> & syntaxTempl, const syntax::Expression & param, const std::vector <Generator> & values, int & consumed) {
	    match (param) {
		of (Var, var, {
			if (values [0].is<Type> ()) {
			    Mapper mapper;
			    mapper.succeed = true;
			    mapper.score = Scores::SCORE_VAR;
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
		) else of (syntax::VarDecl, decl, {
			if (values [0].is <Value> ()) {
			    consumed += 1;
			    auto mapper = validateTypeFromImplicit  (syntaxTempl, decl.getType (), {values [0].to <Value> ().getType ()}, consumed);
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
			Mapper mapper;
			mapper.succeed = true;
			mapper.score += Scores::SCORE_TYPE;
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
			    mapper.mapping.emplace (var.getLocation ().str, createSyntaxType (var.getLocation (), Tuple::init (var.getLocation (), values)));
			    mapper.nameOrder.push_back (var.getLocation ().str);
			}
			consumed += values.size ();
			return mapper;
		    }
		);
	    }
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Mapper ();
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
	    Mapper globalMapper;

	    int consumed = 0;
	    for (auto it : Ymir::r (0, syntaxParams.size ())) {
		auto param = replaceAll (syntaxParams [it], globalMapper.mapping);
		auto current_types = std::vector <Generator> (types.begin () + consumed, types.begin () + consumed + 1 + (types.size () - syntaxParams.size ()));
		
		int current_consumed = 0;
		auto mapper = validateVarDeclFromImplicit (syntaxTempl, param, current_types, current_consumed);		
		// We apply the mapper, to gain time
		if (!mapper.succeed) return Generator::empty ();
		else {
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
	    
	    if (globalMapper.succeed) {
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
		
		score = globalMapper.score;
		auto func = replaceAll (sym.to <semantic::Template> ().getDeclaration (), globalMapper.mapping);
		auto visit = declarator::Visitor::init ();
		visit.pushReferent (ref.getTemplateRef ().getReferent ());
		auto prevMapper = Mapper {true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to <Template> ().getSpecNameOrder ()};
		auto merge = mergeMappers (prevMapper, globalMapper);
		
		visit.pushReferent (TemplateSolution::init (sym.getName (), sym.to <semantic::Template> ().getParams (), merge.mapping, merge.nameOrder));
		auto sym_func = visit.visitFunction (func.to <syntax::Function> ());
		symbol = visit.popReferent ();
		visit.getReferent ().insertTemplate (symbol);
		return this-> _context.validateFunctionProto (sym_func.to <semantic::Function> ());
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
			    Mapper mapper;
			    mapper.succeed = true;
			    mapper.score = 0;			    
			    return mapper;
			} else return applyTypeFromExplicit (params, expr, types, consumed);
		    }
		) else of (TemplateSyntaxWrapper, wp ATTRIBUTE_UNUSED, {
			consumed += 1;
			Mapper mapper;
			mapper.succeed = true;
			mapper.score = 0;			    
			return mapper;
		    }
		) else of (syntax::List, lst, {
			consumed += 1;
			auto type = types [0];
			if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () == lst.getParameters ().size ()) {
			    Mapper mapper;
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
			if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () == 1) {
			    consumed += 1;
			    return validateTypeFromImplicit (params, arr.getLeft (), {type.to <Type> ().getInners () [0]}, current_consumed);			    
			} else return Mapper ();
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
			    Mapper mapper;
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
			}
		    }
		);
	    }

	    OutBuffer buf;
	    leftT.treePrint (buf, 0);
	    println (buf.str ());
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Mapper {};
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
		) return Mapper ();
	    }	
	    
	    return Mapper ();
	}

	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromTemplCall (const std::vector <syntax::Expression> & params, const syntax::TemplateCall & cl, const semantic::TemplateSolution & soluce, int &) const {
	    auto tmpls = sort (soluce.getTempls (), soluce.getParams ());
	    Mapper mapper;
	    auto syntaxParams = cl.getParameters ();
	    if (tmpls.size () != syntaxParams.size ()) return Mapper ();
	    for (auto it : Ymir::r (0, syntaxParams.size ())) {
		auto param = replaceAll (syntaxParams [it], mapper.mapping);
		int current_consumed = 0;
		Generator type (Generator::empty ());
		match (tmpls [it]) {
		    of (generator::TemplateSyntaxWrapper, wrp, {
			    type = wrp.getContent ();
			}
		    ) else {
			println (tmpls [it].prettyString ());
			Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		    }
		}
		
		auto mp = validateTypeFromImplicit (params, param, {type}, current_consumed);
		if (!mp.succeed) return mp;
		mapper = mergeMappers (mapper, mp);
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
			Mapper mapper;
			mapper.succeed = true;
			mapper.score = Scores::SCORE_VAR;
			mapper.mapping.emplace (var.getName ().str, createSyntaxType (var.getName (), types [0]));
			mapper.nameOrder.push_back (var.getName ().str);
			consumed += 1;
			return mapper;
		    }
		) else of (OfVar, var, {
			consumed += 1;
			return applyTypeFromExplicitOfVar (params, var, types [0]);
		    }
		) else of (VariadicVar, var, {
			Mapper mapper;
			mapper.succeed = true;
			mapper.score = Scores::SCORE_TYPE;
			if (types.size () == 1) {
			    mapper.mapping.emplace (var.getLocation ().str, createSyntaxType (var.getLocation (), types [0]));
			    mapper.nameOrder.push_back (var.getLocation ().str);
			} else {
			    mapper.mapping.emplace (var.getLocation ().str, createSyntaxType (var.getLocation (), Tuple::init (var.getLocation (), types)));
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
	    return Mapper ();
	}

	TemplateVisitor::Mapper TemplateVisitor::applyTypeFromExplicitOfVar (const std::vector <Expression> & params, const OfVar & ofv, const generator::Generator & type) const {
	    match (ofv.getType ()) {
		of (Var, var, {
			auto expr = findExpression (var.getName ().str, params);
			if (expr.isEmpty ()) {
			    auto left = this-> _context.validateType (ofv.getType ());
			    this-> _context.verifySameType (left, type);
			    
			    Mapper mapper;
			    mapper.succeed = true;
			    mapper.score = Scores::SCORE_TYPE;
			    mapper.mapping.emplace (ofv.getLocation ().str, createSyntaxType (ofv.getLocation (), type));
			    mapper.nameOrder.push_back (ofv.getLocation ().str);
			    return mapper;
			} else {
			    int consumed = 0;
			    Mapper mapper = applyTypeFromExplicit (params, expr, {type}, consumed);
			    auto realType = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType);
			    
			    mapper.mapping.emplace (ofv.getLocation ().str, createSyntaxType (ofv.getLocation (), genType));
			    mapper.nameOrder.push_back (ofv.getLocation ().str);
			    return mapper;
			}
		    }
		) else of (syntax::List, lst, {
			if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () == lst.getParameters ().size ()) {
			    Mapper mapper;
			    auto syntaxParam = lst.getParameters ();
			    for (auto it : Ymir::r (0, syntaxParam.size ())) {
				auto param = replaceAll (syntaxParam [it], mapper.mapping);
				int consumed = 0;
				auto mp = validateTypeFromImplicit (params, param, {type.to <Type> ().getInners () [it]}, consumed);				
				mapper = mergeMappers (mapper, mp);
			    }
			    
			    Expression realType  = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType);
			    this-> _context.verifySameType (genType, type);
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
		) else of (syntax::ArrayAlloc, arr, {
			if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () == 1) {
			    int consumed = 0;
			    Mapper mapper = validateTypeFromImplicit (params, arr.getLeft (), {type.to <Type> ().getInners () [0]}, consumed);
			    
			    Expression realType  = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType);
			    this-> _context.verifySameType (genType, type);
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
			auto genType = this-> _context.validateType (realType);
			this-> _context.verifySameType (genType, type);
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
	    return Mapper ();
	}

	TemplateVisitor::Mapper TemplateVisitor::applyTypeFromDecoratedExpression (const std::vector <Expression> & params, const DecoratedExpression & expr, const std::vector <generator::Generator> & types, int & consumed) const {
	    if (expr.hasDecorator (syntax::Decorator::CTE) || expr.hasDecorator (syntax::Decorator::REF))
		return Mapper ();	    

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
		return Mapper {true, result.score, maps, result.nameOrder};
	    } else return Mapper ();	    
	}
	
	Expression TemplateVisitor::createSyntaxType (const lexing::Word & location, const generator::Generator & gen, bool isMutable, bool isRef) const {
	    Generator type = gen;
	    type.to <Type> ().isMutable (isMutable);
	    type.to <Type> ().isRef (isRef);
	    type.changeLocation (location);
	    return TemplateSyntaxWrapper::init (location, type);
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
		    ); // We don't do anything for the rest of expression types as they do not refer to types
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
		     return syntax::DecoratedExpression::init (
			element.getLocation (),
			dec.getDecorators (),
			replaceAll (dec.getContent (), mapping)
		     );
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
		     for (auto & it : fnp.getParameters ())
			 params.push_back (replaceAll (it, mapping));
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
		     for (auto & it : lst.getParameters ())
			 params.push_back (replaceAll (it, mapping));
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
		) else of (syntax::String, str ATTRIBUTE_UNUSED, return element;
		) else of (syntax::TemplateCall, tmpl, {
		     std::vector <Expression> params;
		     for (auto & it : tmpl.getParameters ())
			 params.push_back (replaceAll (it, mapping));
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
		) else of (syntax::Unit, uni ATTRIBUTE_UNUSED, return element;
		) else of (syntax::Var, var, {
		     auto inner = mapping.find (var.getName ().str);
		     if (inner != mapping.end ()) {
			 if (inner-> second.is <TemplateSyntaxWrapper> ()) {
			     auto tmplSynt = inner-> second.to <TemplateSyntaxWrapper> ();
			     auto content = tmplSynt.getContent ();
			     content.changeLocation (var.getLocation ());
			     return TemplateSyntaxWrapper::init (var.getLocation (), content);
			 }
			 return inner-> second;
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
		) else of (syntax::Function, func, {
			auto ret = syntax::Function::init (func.getName (), replaceAll (func.getPrototype (), mapping), replaceAll (func.getBody (), mapping));
			ret.to <syntax::Function> ().setCustomAttributes (func.getCustomAttributes ());
			ret.to <syntax::Function> ().setWeak ();
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
			}
		    ) else of (syntax::OfVar, var, {
			    if (mapping.find (var.getLocation ().str) == mapping.end ())
				results.push_back (it);
			}
		    ) else of (syntax::VariadicVar, var, {
			    if (mapping.find (var.getLocation ().str) == mapping.end ())
				results.push_back (it);
			}
		    ) else of (syntax::VarDecl, decl, {
			    if (mapping.find (decl.getLocation ().str) == mapping.end ())
				results.push_back (it);
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
	
    }    
    
}
