#include <ymir/semantic/validator/TemplateVisitor.hh>
#include <ymir/semantic/validator/FunctionVisitor.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <chrono>
#include <time.h>
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

	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================     INFERENCE    =============================
	 * ================================================================================
	 * ================================================================================
	 */

	TemplateVisitor::Mapper TemplateVisitor::inferArguments (const std::vector <Expression> & params) const {
	    auto syntaxTempl = params;
	    Mapper globalMapper (true, 0);
	    try {
		while (syntaxTempl.size () != 0) {
		    auto rest = array_view <syntax::Expression> (syntaxTempl.begin () + 1, syntaxTempl.end ());
		    auto mapper = std::move (this-> inferArgumentParamTempl (rest, syntaxTempl[0]));
		    if (!mapper.succeed) return mapper;
		    else {
			globalMapper = std::move (mergeMappers (globalMapper, mapper));
			// Update the template parameters, some of them might have been removed, (for instance the first one, and sub templates like U in (T of [U], U))
			syntaxTempl = std::move (replaceSyntaxTempl (syntaxTempl, globalMapper.mapping));
		    }
		}
	    } catch (Error::ErrorList list) {
		return Mapper (false, 0);
	    }

	    return globalMapper;
	}

	TemplateVisitor::Mapper TemplateVisitor::inferArgumentParamTempl (const array_view <Expression> & rest, const syntax::Expression & param) const {
	    match (param) {
		of_u (Var) {
		    return Mapper (false, 0);
		} elof (OfVar, var) {
		    return this-> inferTypeFromOfVar (rest, var);
		} elof_u (ImplVar) {
		    return Mapper (false, 0);
		} elof_u (StructVar) {
		    return Mapper (false, 0);
		} elof_u (ClassVar) {
		    return Mapper (false, 0);
		} elof_u (AliasVar) {
		    return Mapper (false, 0);
		} elof (syntax::VarDecl, decl) {
		    return this-> inferValueFromVarDecl (rest, decl);
		} elof_u (VariadicVar) {
		    return Mapper (false, 0);
		} elfo {
		    auto value = this-> _context.retreiveValue (
			this-> _context.validateValue (param)
			);
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

		    mapper.score += Scores::SCORE_TYPE; // Score is useless in this context, but maybe will have some use in the future
		    return mapper;
		}
	    }
	}

	TemplateVisitor::Mapper TemplateVisitor::inferTypeFromOfVar (const array_view <Expression> & rest, const syntax::OfVar & ofv) const {
	    if (ofv.isOver ()) return Mapper (false, 0);
	    match (ofv.getType ()) {
		of_u (Var) {
		    auto mapper = std::move (inferArgumentInner (rest, ofv.getType ()));
		    if (mapper.succeed) {
			auto type = this-> _context.validateType (replaceAll (ofv.getType (), mapper.mapping));
			mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
			mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
		    }
		    return mapper;
		} elof (syntax::Unary, un) {
		    auto mapper = std::move (inferArgumentInner (rest, un.getContent ()));
		    if (mapper.succeed) {
			auto type = this-> _context.validateType (replaceAll (ofv.getType (), mapper.mapping));
			mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
			mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
		    }
		    mapper.score += Scores::SCORE_TYPE;
		    return mapper;
		} elof (Try, tr) {
		    auto mapper = std::move (inferArgumentInner (rest, tr.getContent ()));
		    if (mapper.succeed) {
			auto type = this-> _context.validateType (replaceAll (ofv.getType (), mapper.mapping));
			mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
			mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
		    }
		    mapper.score += Scores::SCORE_TYPE;
		    return mapper;
		} elof (syntax::List, lst) {
		    Mapper mapper (true, 0);
		    for (auto & it : lst.getParameters ()) {
			auto local = std::move (inferArgumentInner (rest, replaceAll (it, mapper.mapping)));
			if (local.succeed) {
			    mapper = std::move (mergeMappers (mapper, local));
			} else return Mapper (false, 0);
		    }

		    auto type = this-> _context.validateType (replaceAll (ofv.getType (), mapper.mapping));
		    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
		    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
		    mapper.score += Scores::SCORE_TYPE;
		    return mapper;
		} elof (syntax::FuncPtr, fptr) {
		    Mapper mapper (true, 0);
		    for (auto & it : fptr.getParameters ()) {
			auto local = std::move (inferArgumentInner (rest, replaceAll (it, mapper.mapping)));
			if (local.succeed) {
			    mapper = std::move (mergeMappers (mapper, local));
			} else return Mapper (false, 0);
		    }

		    auto ret = std::move (inferArgumentInner (rest, replaceAll (fptr.getRetType (), mapper.mapping)));
		    if (ret.succeed) {
			mapper = std::move (mergeMappers (mapper, ret));
		    } else return Mapper (false, 0);

		    auto type = this-> _context.validateType (replaceAll (ofv.getType (), mapper.mapping));
		    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
		    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
		    mapper.score += Scores::SCORE_TYPE;
		    return mapper;
		} // TODO decorated, and array alloc
		elof (TemplateCall, cl) {
		    Mapper mapper (true, 0);
		    for (auto & it : cl.getParameters ()) {
			auto local = std::move (inferArgumentInner (rest, replaceAll (it, mapper.mapping)));
			if (local.succeed) {
			    mapper = std::move (mergeMappers (mapper, local));
			} else return Mapper (false, 0);
		    }

		    auto type = this-> _context.validateType (replaceAll (ofv.getType (), mapper.mapping));
		    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
		    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
		    mapper.score += Scores::SCORE_TYPE;
		    return mapper;
		} fo;
	    }

	    auto type = this-> _context.validateType (ofv.getType ());
	    Mapper mapper (true, Scores::SCORE_TYPE);
	    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
	    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
	    return mapper;
	}


	TemplateVisitor::Mapper TemplateVisitor::inferArgumentInner (const array_view<Expression>& params, const Expression & elem) const {
	    match (elem) {
		of (Var, var) {
		    auto expr = this-> findExpression (var.getName ().getStr (), params);
		    if (expr.isEmpty ()) {
			return Mapper (true, 0);
		    } else return this-> inferArgumentParamTempl (params, expr);
		} elof (syntax::Unary, un) {
		    return this-> inferArgumentInner (params, un.getContent ());
		} elof (syntax::Try, tr) {
		    return this-> inferArgumentInner (params, tr.getContent ());
		} elof (syntax::List, lst) {
		    Mapper mapper (true, 0);
		    for (auto & it : lst.getParameters ()) {
			auto param = this-> replaceAll (it, mapper.mapping);
			auto loc_mapper = this-> inferArgumentInner (params, param);
			if (loc_mapper.succeed) {
			    mapper = std::move (mergeMappers (loc_mapper, mapper));
			}
			if (!mapper.succeed || !loc_mapper.succeed) return Mapper (false, 0);
		    }
		    return mapper;
		} elof (syntax::ArrayAlloc, arr) {
		    auto l_mapper = this-> inferArgumentInner (params, arr.getLeft ());
		    auto r_mapper = this-> inferArgumentInner (params, arr.getSize ());
		    return mergeMappers (l_mapper, r_mapper);
		} elof (syntax::TemplateCall, tm) {
		    Mapper mapper (true, 0);
		    for (auto & it : tm.getParameters ()) {
			auto param = this-> replaceAll (it, mapper.mapping);
			auto loc_mapper = this-> inferArgumentInner (params, param);
			if (loc_mapper.succeed) {
			    mapper = std::move (mergeMappers (loc_mapper, mapper));
			}
			if (!mapper.succeed || !loc_mapper.succeed) return Mapper (false, 0);
		    }
		    return mapper;		
		} elof (syntax::DecoratedExpression, dc) {
		    return this-> inferArgumentInner (params, dc.getContent ());
		} elof (syntax::FuncPtr, fPtr) {
		    Mapper mapper (true, 0);
		    for (auto & it : fPtr.getParameters ()) {
			auto param = this-> replaceAll (it, mapper.mapping);
			auto loc_mapper = this-> inferArgumentInner (params, param);
			if (loc_mapper.succeed) {
			    mapper = std::move (mergeMappers (loc_mapper, mapper));
			}
			if (!mapper.succeed || !loc_mapper.succeed) return Mapper (false, 0);
		    }
		    auto param = replaceAll (fPtr.getRetType (), mapper.mapping);
		    auto loc_mapper = this-> inferArgumentInner (params, param);
		    if (loc_mapper.succeed) {
			mapper = std::move (mergeMappers (loc_mapper, mapper));
		    }
		    if (!mapper.succeed || !loc_mapper.succeed) return Mapper (false, 0);
		    return mapper;		
		} fo;
	    }
	    return Mapper (true, 0);
	}


	TemplateVisitor::Mapper TemplateVisitor::inferValueFromVarDecl (const array_view<Expression> & rest, const syntax::VarDecl & param) const {
	    if (!param.getValue ().isEmpty ()) {
		auto value = this-> _context.retreiveValue (this-> _context.validateValue (param.getValue ()));
		Mapper mapper (true, 0);
		if (!param.getType ().isEmpty ()) {
		    auto loc_mapper = this-> inferArgumentInner (rest, param.getType ());
		    if (!loc_mapper.succeed) return Mapper (false, 0);
		    auto type = this-> _context.validateType (this-> replaceAll (param.getType (), mapper.mapping));
		    this-> _context.verifySameType (type, value.to <Value> ().getType ());
		    mapper = std::move (mergeMappers (mapper, loc_mapper));
		}
		
		mapper.mapping.emplace (param.getName ().getStr (), createSyntaxValue (param.getName (), value));
		mapper.nameOrder.push_back (param.getName ().getStr ());
		mapper.score += Scores::SCORE_VAR;
		return mapper;
	    }
	    
	    return Mapper (false, 0);
	}

	
	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================     EXPLICIT    ==============================
	 * ================================================================================
	 * ================================================================================
	 */

	TemplateVisitor::Mapper TemplateVisitor::validateFromExplicit (const std::vector <Expression> & params, const std::vector <Generator> & values, int & consumed) const {
	    auto syntaxTempl = params;
	    Mapper globalMapper (true, 0);
	    consumed = 0;
	    while (consumed < (int) values.size () && syntaxTempl.size () != 0) {
		// create an array views, on template arguments that have not been consumed yet
		auto currentElems = array_view <Generator> (values.begin () + consumed, values.end ());
		
		// create an array view on the template parameters that have not been used yet
		auto rest = array_view <syntax::Expression> (syntaxTempl.begin () + 1, syntaxTempl.end ());
		
		int current_consumed = 0;
		
		// Perform an explicit template validation, with the given templates and argument
		auto mapper = std::move (validateParamTemplFromExplicit (rest, syntaxTempl [0], currentElems, current_consumed));
		if (!mapper.succeed) return mapper;
		else {
		    globalMapper = std::move (mergeMappers (globalMapper, mapper));
		    // Update the template parameters, some of them might have been removed, (for instance the first one, and sub templates like U in (T of [U], U))
		    syntaxTempl = std::move (replaceSyntaxTempl (syntaxTempl, globalMapper.mapping));
		    consumed += current_consumed; // Move the cursor forward
		}
	    }
	    
	    return globalMapper; // return the mapper that can be used to make a template replacement (if valid which is not necessarily the case)
	}
	
	Symbol TemplateVisitor::validateFromExplicit (const TemplateRef & ref, const std::vector <Generator> & values, int & score) const {
	    score = -1;
	    const Symbol & sym = ref.getTemplateRef ();
	    int consumed = 0;
	    Mapper globalMapper (true, 0);
	    try {
		globalMapper = std::move (this-> validateFromExplicit (sym.to <semantic::Template> ().getParams (), values, consumed));
	    } catch (Error::ErrorList  list) {
		auto prevMapper = Mapper {true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to<Template> ().getSpecNameOrder ()};
		auto merge = std::move (mergeMappers (prevMapper, globalMapper));
		list.errors.push_back (this-> partialResolutionNote (ref.getLocation (), merge));
		throw list;
	    }
	    
	    if (!globalMapper.succeed) return Symbol::empty ();
	    
	    if (consumed < (int) values.size ()) {
		Ymir::OutBuffer buf;
		for (auto it : Ymir::r (consumed, values.size ())) {
		    if (it != consumed) buf.write (", ");
		    buf.write (values [it].prettyString ());
		}
		
		Ymir::Error::occur (values [consumed].getLocation (), ExternalError::get (TEMPLATE_REST),
				    buf.str ());
		return Symbol::empty ();
	    } else {
		return this-> applyMapperOnTemplate (ref.getLocation (), sym, globalMapper, score);
	    }
	}

	Symbol TemplateVisitor::applyMapperOnTemplate (const lexing::Word & loc, const Symbol & sym, const TemplateVisitor::Mapper & mapper, int & score) const {
	    auto prevMapper = Mapper {true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to<Template> ().getSpecNameOrder ()};
	    auto merge = std::move (mergeMappers (prevMapper, mapper));
	    try {
		merge.mapping = validateLambdaProtos (sym.to <Template> ().getPreviousParams (), merge.mapping);
		merge.nameOrder = sortNames (sym.to<Template> ().getPreviousParams (), merge.mapping);
	    } catch (Error::ErrorList list) {
		list.errors.push_back (this-> partialResolutionNote (loc, merge));
		throw list;
	    }
		
	    auto syntaxTempl = std::move (replaceSyntaxTempl (sym.to <semantic::Template> ().getParams (), merge.mapping));
		
	    if (syntaxTempl.size () != 0) { // Rest some template that are not validated
		static int __tmpTemplate__ = 0;
		__tmpTemplate__ += 1;
		score = merge.score;
		auto & tmpl = sym.to <semantic::Template> ();
		auto sym2 = Template::init (loc, "", syntaxTempl, tmpl.getDeclaration (), tmpl.getTest (), tmpl.getParams (), true);
		sym2.to <Template> ().setPreviousSpecialization (merge.mapping);
		sym2.to <Template> ().setSpecNameOrder (merge.nameOrder);
		sym.getReferent ().insertTemplate (sym2);
		return sym2;
	    } else {
		// Well i did a complex modification here, to gain .. nothing
		// So it is worthless to do it in implicit (TemplatePreSolution)
		score = merge.score;
		finalValidation (sym.getReferent (), sym.to <Template> ().getPreviousParams (), merge, sym.to <semantic::Template> ().getTest ());
		auto & tmpls = sym.to <semantic::Template> ();
		auto sym2 = TemplatePreSolution::init (sym.getName (), sym.getComments (), tmpls.getParams (), tmpls.getDeclaration (), merge.mapping, merge.nameOrder, sym.getReferent ());
		return sym2;
	    }	    
	}
	
	TemplateVisitor::Mapper TemplateVisitor::validateParamTemplFromExplicit (const array_view <syntax::Expression> & syntaxTempl, const syntax::Expression & param, const array_view <Generator> & values, int & consumed) const {
	    match (param) {
		of (Var, var) {
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
		elof (OfVar, var) {
		    if (values [0].is<Type> ()) {
			consumed += 1;
			return validateTypeFromExplicitOfVar (syntaxTempl, var, values [0]);
		    } else {
			auto note = Ymir::Error::createNote (param.getLocation ());
			Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (USE_AS_TYPE));
		    }
		}
		elof (ImplVar, var) {
		    if (values [0].is <ClassPtr> () || values [0].is <ClassRef> ()) {
			consumed += 1;
			return validateTypeFromExplicitImplVar (syntaxTempl, var, values [0]);
		    } else if (values [0].is <Type> ()) {
			auto note = Ymir::Error::createNote (param.getLocation ());
			Ymir::Error::occurAndNote (var.getLocation (), note, ExternalError::get (NOT_A_CLASS), values [0].prettyString ());
		    } else {
			auto note = Ymir::Error::createNote (param.getLocation ());
			Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (USE_AS_TYPE));
		    }
		}
		elof (StructVar, var) {
		    if (values [0].is <Type> ()) {
			if (values [0].is <StructRef> ()) {
			    Mapper mapper (true, Scores::SCORE_TYPE);
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), values [0]));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			    consumed += 1;
			    return mapper;
			} else {
			    auto note = Ymir::Error::createNote (param.getLocation ());
			    Ymir::Error::occurAndNote (var.getLocation (), note, ExternalError::get (NOT_A_STRUCT), values [0].prettyString ());
			}
		    } else {
			auto note = Ymir::Error::createNote (param.getLocation ());
			Ymir::Error::occurAndNote (values[0].getLocation (), note, ExternalError::get (USE_AS_TYPE));
		    }
		}
		elof (ClassVar, var) {
		    if (values [0].is <Type> ()) {
			if (!values [0].isEmpty () && (values [0].is <ClassPtr> () || values [0].is <ClassRef> ())) {
			    Mapper mapper (true, Scores::SCORE_TYPE);
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), values [0]));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			    consumed += 1;
			    return mapper;
			} else if (!values [0].isEmpty ()) {
			    auto note = Ymir::Error::createNote (param.getLocation ());
			    Ymir::Error::occurAndNote (var.getLocation (), note, ExternalError::get (NOT_A_CLASS), values [0].prettyString ());
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
		elof (AliasVar, var) {
		    if (values [0].is <Type> ()) {
			if (values [0].to<Type> ().containPointers ()) {
			    Mapper mapper (true, Scores::SCORE_TYPE);
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
		elof (syntax::VarDecl, decl) {
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
		elof (syntax::VariadicVar, var) {
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
		elof (syntax::DecoratedExpression, dc) {
		    Ymir::Error::occur (dc.getDecorators ()[0].getLocation (),
					ExternalError::get (DECO_OUT_OF_CONTEXT),
					dc.prettyDecorators ()
			);
		} fo;
		
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

             	
	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================     IMPLICIT    ==============================
	 * ================================================================================
	 * ================================================================================
	 */

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
		    mapper = std::move (validateVarDeclFromImplicit (syntaxTempl, param, current_types, current_consumed));
		} catch (Error::ErrorList list) {
		    errors = list.errors;
		    succeed = false;
		}

		
		if (!succeed) {
		    auto prevMapper = Mapper (true, 0, sym.to<Template> ().getPreviousSpecialization (), sym.to<Template> ().getSpecNameOrder ());
		    auto merge = std::move (mergeMappers (prevMapper, globalMapper));
		    errors.push_back (this-> partialResolutionNote (ref.getLocation (), merge));
		    break;
		}
		
		// We validate the mapper, to gain time
		if (!mapper.succeed) {
		    globalMapper.succeed = false;
		    break;
		} else {
		    globalMapper = std::move (mergeMappers (globalMapper, mapper));
		    syntaxTempl = std::move (replaceSyntaxTempl (syntaxTempl, globalMapper.mapping));
		    
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
		auto merge = std::move (mergeMappers (prevMapper, globalMapper));
		try {
		    merge.mapping = validateLambdaProtos (sym.to <Template> ().getPreviousParams (), merge.mapping);
		    merge.nameOrder = sortNames (sym.to<Template> ().getPreviousParams (), merge.mapping);
		} catch (Error::ErrorList list) {
		    list.errors.push_back (this-> partialResolutionNote (ref.getLocation (), merge));
		    throw list;
		}
		
		syntaxTempl = std::move (replaceSyntaxTempl (syntaxTempl, merge.mapping));
		
		if (syntaxTempl.size () != 0) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    for (auto it : syntaxTempl) {
			errors.push_back (Ymir::Error::makeOccur (
					      it.getLocation (),
					      ExternalError::get (UNRESOLVED_TEMPLATE)
					      ));
		    }
		    errors.push_back (this-> partialResolutionNote (ref.getLocation (), merge));
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
			of (syntax::Function, f) {
			    sym_func = symbol.getLocal (f.getLocation ().getStr ()) [0];
			}
			elof (syntax::Class, c) {
			    sym_func = symbol.getLocal (c.getLocation ().getStr ()) [0];
			}
			elfo {
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
			proto = FunctionVisitor::init (this-> _context).validateMethodProto (sym_func.to <semantic::Function> (), classType, Generator::empty (), false);
		    } else if (ref.is <TemplateClassCst> ()) {
			proto = this-> _context.validateClass (sym_func, false);
		    } else {
			proto = FunctionVisitor::init (this-> _context).validateFunctionProto (sym_func);
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
		auto merge = std::move (mergeMappers (prevMapper, globalMapper));
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
		of (Var, var) {
		    Expression expr = findExpression (var.getName ().getStr (), params);
		    if (expr.isEmpty ()) {
			consumed += 1;
			// if expression is empty, the type is a real one, and not a template
			return  Mapper (true, 0);
		    } return validateTypeFromExplicit (params, expr, types, consumed);
		}
		elof (TemplateSyntaxList, lst) {
		    consumed += lst.getContents ().size ();
		    for (auto it : Ymir::r (0, lst.getContents ().size ())) {
			this-> _context.verifySameType (types [it], lst.getContents () [it]);
		    }
			
		    return Mapper (true, 0);
		}
		elof_u (TemplateSyntaxWrapper) {
		    consumed += 1;
		    return Mapper (true, 0);
		}
		elof (syntax::Unary, un) {
		    consumed += 1;
		    auto type = types [0];
		    if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () == 1) {
			auto vec = type.to <Type> ().getInners ();
			int consumed = 0;
			auto mapper = std::move (this-> validateTypeFromImplicit (params, un.getContent (), array_view<Generator> (vec), consumed));
			    
			if (mapper.succeed) {
			    Expression realType = this-> replaceAll (leftT, mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);
			    this-> _context.verifySameType (genType, type);
			    this-> _context.verifyNotIsType (leftT.getLocation ());
			  				
			    mapper.score += Scores::SCORE_TYPE;
			    return mapper;
			}
		    } else {
			Ymir::Error::occur (un.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    un.prettyString (),
					    type.to<Type> ().getTypeName ());
		    }
						    			
		}
		elof (syntax::Try, tr) {
		    consumed += 1;
		    auto type = types [0];
		    if (type.is <Option> ()) {
			std::vector <Generator> vec = {type.to <Type> ().getInners ()[0]};
			int loc_consumed = 0;
			auto mapper = std::move (this-> validateTypeFromImplicit (params, tr.getContent (), array_view<Generator> (vec), loc_consumed));

			if (mapper.succeed) {
			    Expression realType = this-> replaceAll (leftT, mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);
			    this-> _context.verifySameType (genType, type);
			    this-> _context.verifyNotIsType (leftT.getLocation ());
			  				
			    mapper.score += Scores::SCORE_TYPE;
			    return mapper;
			}
		    } else {
			Ymir::Error::occur (tr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    tr.prettyString (),
					    type.to<Type> ().getTypeName ());
		    }
						    			
		}
		elof (syntax::List, lst) {
		    consumed += 1;
		    auto type = types [0];
			
		    if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () >= lst.getParameters ().size () && !type.is <ClassPtr> ()) {
			Mapper mapper (false, 0);
			auto syntaxParam = lst.getParameters ();
			int current_consumed = 0;
			auto & types = type.to <Type> ().getInners ();
			    
			for (auto it : Ymir::r (0, syntaxParam.size ())) {
			    auto param = replaceAll (syntaxParam [it], mapper.mapping);
			    auto rest = types.size () - current_consumed;
			    auto syntaxRest = (syntaxParam.size () - it) - 1;
			    int right = rest - syntaxRest;
			    auto current_types = array_view <Generator> (types.begin () + current_consumed, types.begin () + current_consumed + right);

				
			    auto mp = validateTypeFromImplicit (params, param, current_types, current_consumed);
			    if (!mp.succeed) return mp;
				
			    mapper = std::move (mergeMappers (mapper, mp));
			}

			if (current_consumed < (int) types.size ()) {
			    Ymir::Error::occur (lst.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						lst.prettyString (),
						type.to<Type> ().getTypeName ());

			    Mapper mapper (false, 0);
			    return mapper;
			}
			    
			return mapper;
		    } else if (type.is <Void> ()) {
			return Mapper (true, 0);
		    } else {
			Ymir::Error::occur (leftT.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    lst.prettyString (),
					    type.to<Type> ().getTypeName ());
		    }
		}
		elof (syntax::ArrayAlloc, arr) {
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
		elof (TemplateCall, cl) {
		    consumed += 1;
		    int current_consumed = 0;
		    return validateTypeFromTemplCall (params, cl, types [0], current_consumed);
		}
		elof (DecoratedExpression, dc) {
		    int current_consumed = 0;
		    consumed += 1;
			
		    auto mapper = std::move (this-> validateTypeFromImplicit (params, dc.getContent (), array_view<Generator> ({types [0]}), current_consumed));
		    if (mapper.succeed) {
			
			mapper.mapping.emplace (
			    format ("%[%,%]",
				    dc.getLocation ().getStr (),
				    dc.getLocation ().getLine (),
				    dc.getLocation ().getColumn ()
				), createSyntaxValue (dc.getLocation (), types [0]));
			mapper.nameOrder.push_back (format ("%[%,%]",
							    dc.getLocation ().getStr (),
							    dc.getLocation ().getLine (),
							    dc.getLocation ().getColumn ()));
		    }
		    return mapper;
		}
		elof (syntax::FuncPtr, fPtr) {
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
			    mapper = std::move (mergeMappers (mapper, mp));
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
		} fo;
	      
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

	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================       TYPES      =============================
	 * ================================================================================
	 * ================================================================================
	 */

	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromTemplCall (const array_view <syntax::Expression> & params, const syntax::TemplateCall  & cl, const generator::Generator & type, int & consumed) const {
	    Generator currentType = type;
	    // We get the first templateCall in the expression
	    match (currentType) {
		of (StructRef, strRef) {
		    auto tmplSoluce = getFirstTemplateSolution (strRef.getRef ());
		    if (!tmplSoluce.isEmpty ())
		    return validateTypeFromTemplCall (params, cl, tmplSoluce.to <TemplateSolution> (), consumed);
		}
		elof (ClassRef, clRef) {
		    auto tmplSoluce = getFirstTemplateSolution (clRef.getRef ());
		    if (!tmplSoluce.isEmpty ())
		    return validateTypeFromTemplCall (params, cl, tmplSoluce.to <TemplateSolution> (), consumed);
		}
		elof (TraitRef, trRef) {
		    auto tmplSoluce = getFirstTemplateSolution (trRef.getRef ());
		    if (!tmplSoluce.isEmpty ())
		    return validateTypeFromTemplCall (params, cl, tmplSoluce.to <TemplateSolution> (), consumed);
		}
		elof (Range, rng) { // Range are created by template Call
		    auto left = cl.getContent ();
		    if (left.is<syntax::Var> () && left.to <syntax::Var> ().getName () == Range::NAME) {
			if (cl.getParameters ().size () == 1) {
			    auto vec = {rng.getInners ()[0]};
			    return validateTypeFromImplicit (params, cl.getParameters ()[0], array_view <Generator> (vec), consumed);
			}
		    }
		} fo;
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
		    of (generator::TemplateSyntaxWrapper, wrp) {
			auto type = wrp.getContent ();
			auto vec = {type};
			auto mp = validateTypeFromImplicit (params, param, array_view<Generator> (vec), current_consumed);
			if (!mp.succeed) return mp;
			mapper = std::move (mergeMappers (mapper, mp));
		    }
		    elof (generator::TemplateSyntaxList, lst) {
			auto mp = validateTypeFromImplicit (params, param, lst.getContents (), current_consumed);
			if (!mp.succeed) return mp;
			mapper = std::move (mergeMappers (mapper, mp));
		    }
		    elfo {
			println (tmpls [it].prettyString ());
			Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		    }
		}
		
	    }
	    return mapper;
	}
	
	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromExplicit (const array_view <Expression> & params, const Expression & leftT, const array_view <generator::Generator> & types, int & consumed) const {
	    match (leftT) {
		of (Var, var) {
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
		elof (OfVar, var) {
		    consumed += 1;
		    return validateTypeFromExplicitOfVar (params, var, types [0]);
		}
		elof (ImplVar, var) {
		    if (!types [0].is <ClassPtr> () && !types [0].is <ClassRef> ()) {
			auto note = Ymir::Error::createNote (types [0].getLocation ());
			Ymir::Error::occurAndNote (var.getLocation (), note, ExternalError::get (NOT_A_CLASS), types [0].prettyString ());
		    }

		    consumed += 1;
		    return validateTypeFromExplicitImplVar (params, var, types [0]);
		}
		elof (StructVar, var) {
		    if ((!types [0].isEmpty ()) && types [0].is <StructRef> ()) {
			Mapper mapper (true, Scores::SCORE_TYPE);
			mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), types [0]));
			mapper.nameOrder.push_back (var.getLocation ().getStr ());
			consumed += 1;
			return mapper;
		    } else if (!types [0].isEmpty ()) {
			auto note = Ymir::Error::createNote (types [0].getLocation ());
			Ymir::Error::occurAndNote (var.getLocation (), note, ExternalError::get (NOT_A_STRUCT),
						   types [0].to<Type> ().getTypeName ());
		    } else
		    Ymir::Error::occur (var.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					var.prettyString (),
					NoneType::init (var.getLocation ()).to <Type> ().getTypeName ());
		    return Mapper (false, 0);
		}
		elof (ClassVar, var) {
		    if (!types [0].isEmpty () && (types [0].is <ClassPtr> () || types [0].is <ClassRef> ())) {
			Mapper mapper (true, Scores::SCORE_TYPE);
			mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), types [0]));
			mapper.nameOrder.push_back (var.getLocation ().getStr ());
			consumed += 1;
			return mapper;
		    } else if (!types [0].isEmpty ()) {
			auto note = Ymir::Error::createNote (types [0].getLocation ());
			Ymir::Error::occurAndNote (var.getLocation (), note, ExternalError::get (NOT_A_CLASS),
						   types [0].to<Type> ().getTypeName ());
		    } else
		    Ymir::Error::occur (var.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					var.prettyString (),
					NoneType::init (var.getLocation ()).to <Type> ().getTypeName ());
		    return Mapper (false, 0);
		}
		elof (AliasVar, var) {
		    if ((!types [0].isEmpty ()) && types [0].to<Type> ().containPointers ()) {
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
		elof (VariadicVar, var) {
		    Mapper mapper (true, Scores::SCORE_TYPE);
		    if (types.size () == 1) {
			if (!types [0].isEmpty ()) {
			    // It can happen when we use a lambdaProto as a value in a validateExplicit
			    // and it is not a problem
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), types [0]));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			}
		    } else {
			bool add = true;
			for (auto & it : types) {
			    if (it.isEmpty ()) add = false;
			    else if (!it.is<Type> ()) {
				auto note = Ymir::Error::createNote (var.getLocation ());
				Ymir::Error::occurAndNote (types [0].getLocation (), note, ExternalError::get (USE_AS_TYPE), types [0].prettyString ());
			    }
			}
			if (add) {
			    mapper.mapping.emplace (var.getLocation ().getStr (), createSyntaxType (var.getLocation (), types));
			    mapper.nameOrder.push_back (var.getLocation ().getStr ());
			}
		    }
		    consumed += types.size ();
		    return mapper;
		}
		elof (DecoratedExpression, dc) {
		    Ymir::Error::occur (dc.getDecorators ()[0].getLocation (),
					ExternalError::get (DECO_OUT_OF_CONTEXT),
					dc.prettyDecorators ()
			);
		} fo;

	    }

	    OutBuffer buf;
	    leftT.treePrint (buf, 0);
	    println (buf.str ());
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Mapper {false, 0, {}, {}};
	}

	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromExplicitOfVar (const array_view <Expression> & params, const OfVar & ofv, const generator::Generator & type) const {
	    if (type.isEmpty ()) return Mapper {true, 0, {}, {}};
	    match (ofv.getType ()) {
		of (Var, var) {
		    auto expr = findExpression (var.getName ().getStr (), params);
		    if (!expr.isEmpty ()) {
			int consumed = 0;
			auto vec = {type};
			Mapper mapper = std::move (validateTypeFromExplicit (params, expr, array_view<Generator> (vec), consumed));
			if (!type.is <ClassRef> ()) { // we skip the validation ?
			    auto realType = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);
			    if (ofv.isOver () && !this-> _context.isAncestor (genType, type)) {
				Ymir::Error::occur (var.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						    genType.prettyString (),
						    type.prettyString ());
			    }
			    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), genType));
			} else {
			    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
			}
			    
			this-> _context.verifyNotIsType (ofv.getLocation ());
			mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
			    
			return mapper;
		    }
		}
		elof (syntax::Unary, un) {
		    if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () == 1) {
			auto vec = type.to <Type> ().getInners ();
			int consumed = 0;
			auto mapper = std::move (this-> validateTypeFromImplicit (params, un.getContent (), array_view<Generator> (vec), consumed));
			    
			if (mapper.succeed) {
			    Expression realType = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);
			    if (ofv.isOver ()) {
				if (!this-> _context.isAncestor (genType, type)) {
				    Ymir::Error::occur (un.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
							genType.prettyString (),
							type.prettyString ());
				}
			    } else {
				this-> _context.verifySameType (genType, type);
			    }
				
			    this-> _context.verifyNotIsType (ofv.getLocation ());
			       				
			    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxValue (ofv.getLocation (), type));
			    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
				
			    mapper.score += Scores::SCORE_TYPE;
			    return mapper;
			}
		    } else {
			Ymir::Error::occur (un.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    un.prettyString (),
					    type.to<Type> ().getTypeName ());
		    }
		}
		elof (syntax::Try, tr) {
		    if (type.is <Option> ()) {
			std::vector <Generator> vec = {type.to <Type> ().getInners ()[0]};
			int consumed = 0;
			auto mapper = std::move (this-> validateTypeFromImplicit (params, tr.getContent (), array_view<Generator> (vec), consumed));
			    
			if (mapper.succeed) {
			    Expression realType = this-> replaceAll (ofv.getType (), mapper.mapping);
			    auto genType = this-> _context.validateType (realType, true);
			    if (ofv.isOver ()) {
				if (!this-> _context.isAncestor (genType, type)) {
				    Ymir::Error::occur (tr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
							genType.prettyString (),
							type.prettyString ());
				}
			    } else {
				this-> _context.verifySameType (genType, type);
			    }

			    this-> _context.verifyNotIsType (ofv.getLocation ());
			    
			    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxValue (ofv.getLocation (), type));
			    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
				
			    mapper.score += Scores::SCORE_TYPE;
			    return mapper;
			}
		    } else {
			Ymir::Error::occur (tr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    tr.prettyString (),
					    type.to<Type> ().getTypeName ());
		    }
		}
		elof (syntax::List, lst) {
		    if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () >= lst.getParameters ().size () && !type.is <ClassPtr> ()) {
			Mapper mapper (false, 0);
			auto syntaxParam = lst.getParameters ();
			int current_consumed = 0;
			auto & types = type.to <Type> ().getInners ();
			    
			for (auto it : Ymir::r (0, syntaxParam.size ())) {
			    auto param = replaceAll (syntaxParam [it], mapper.mapping);
			    auto rest = types.size () - current_consumed;
			    auto syntaxRest = (syntaxParam.size () - it) - 1;
			    int right = rest - syntaxRest;
			    auto current_types = array_view <Generator> (types.begin () + current_consumed, types.begin () + current_consumed + right);

				
			    auto mp = validateTypeFromImplicit (params, param, current_types, current_consumed);
			    if (!mp.succeed) return mp;
				
			    mapper = std::move (mergeMappers (mapper, mp));
			}
			    
			if (current_consumed < (int) types.size ()) {
			    Ymir::Error::occur (lst.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						lst.prettyString (),
						type.to<Type> ().getTypeName ());

			    Mapper mapper (false, 0);
			    return mapper;
			}

			Expression realType  = this-> replaceAll (ofv.getType (), mapper.mapping);
			auto genType = this-> _context.validateType (realType, true);
			if (ofv.isOver ()) {
			    if (!this-> _context.isAncestor (genType, type)) {
				Ymir::Error::occur (lst.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						    genType.prettyString (),
						    type.prettyString ());
			    }
			} else {
			    this-> _context.verifySameType (genType, type);
			}

			this-> _context.verifyNotIsType (ofv.getLocation ());
			    
			mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), genType));
			mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
			mapper.score += Scores::SCORE_TYPE;
			return mapper;
		    } else if (type.is<Void> ()) {
			return Mapper (true, 0);
		    } else {
			Ymir::Error::occur (lst.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    lst.prettyString (),
					    type.to<Type> ().getTypeName ());
		    }
		}
		elof (syntax::FuncPtr, fPtr) {
		    if (ofv.isOver ()) {
			Ymir::Error::occur (fPtr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    fPtr.prettyString (),
					    type.prettyString ());
		    }
			 
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
			    mapper = std::move (mergeMappers (mapper, mp));
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
					    fPtr.prettyString (),
					    type.to<Type> ().getTypeName ());
		    }
		}
		elof (syntax::ArrayAlloc, arr) {
		    if (type.is<Array> ()) {
			int consumed = 0;
			auto vec = {type.to <Type> ().getInners () [0]};
			Mapper mapper = std::move (validateTypeFromImplicit (params, arr.getLeft (), array_view<Generator> (vec), consumed));
			if (mapper.succeed && arr.getSize ().is <syntax::Var> ()) {
			    Expression expr = findExpressionValue (arr.getSize ().to <syntax::Var> ().getName ().getStr (), params);
			    if (!expr.isEmpty ()) {
				int local_consumed = 0;
				auto size = ufixed (arr.getSize ().getLocation (), type.to <Array> ().getSize ());
				auto vec = {size};
				auto auxMap = validateParamTemplFromExplicit (params, expr, array_view<Generator> (vec), local_consumed);
				mapper = std::move (mergeMappers (mapper, auxMap));
			    }
			}
			    
			Expression realType  = this-> replaceAll (ofv.getType (), mapper.mapping);
			auto genType = this-> _context.validateType (realType, true);
			if (ofv.isOver ()) {
			    if (!this-> _context.isAncestor (genType, type)) {
				Ymir::Error::occur (arr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						    genType.prettyString (),
						    type.prettyString ());
			    }
			} else {
			    this-> _context.verifySameType (genType, type);
			}

			this-> _context.verifyNotIsType (ofv.getLocation ());
			    
			mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), genType));
			mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
			mapper.score += Scores::SCORE_TYPE;
			return mapper;
		    } else {
			Ymir::Error::occur (arr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    arr.prettyString (),
					    type.to<Type> ().getTypeName ());
		    }
		}
		elof (DecoratedExpression, dc) {
		    for (auto & it : dc.getDecorators ()) {
			if (it.getValue () != Decorator::MUT && it.getValue () != Decorator::DMUT)
			Ymir::Error::occur (it.getLocation (),
					    ExternalError::get (DECO_OUT_OF_CONTEXT),
					    it.getLocation ().getStr ()
			    );
		    }

		    int local_consumed = 0;
		    auto vec = {type};
		    auto mapper = std::move (this-> validateTypeFromImplicit (params, dc.getContent (), array_view<Generator> (vec), local_consumed));
		    if (mapper.succeed) {
			if (dc.hasDecorator (Decorator::MUT) && !type.to <Type> ().isMutable ()) {
			    auto note = this-> partialResolutionNote (dc.getLocation (), mapper);
			    Ymir::Error::occurAndNote (dc.getDecorators ()[0].getLocation (), note, ExternalError::get (DISCARD_CONST));
			} else if (dc.hasDecorator (Decorator::DMUT) && !type.to <Type> ().isDeeplyMutable ()) {
			    auto note = this-> partialResolutionNote (dc.getLocation (), mapper);
			    Ymir::Error::occurAndNote (dc.getDecorators ()[0].getLocation (), note, ExternalError::get (DISCARD_CONST));
			}
			    
			mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
		    }
			    
		    return mapper;
		}
		elof (TemplateCall, cl) {
		    int current_consumed = 0;
		    auto mapper = std::move (validateTypeFromTemplCall (params, cl, type, current_consumed));
		    Expression realType = this-> replaceAll (ofv.getType (), mapper.mapping);
		    auto genType = this-> _context.validateType (realType, true);
		    if (ofv.isOver ()) {
			if (!this-> _context.isAncestor (genType, type)) {
			    Ymir::Error::occur (cl.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
						genType.prettyString (),
						type.prettyString ());
			}
		    } else {
			this-> _context.verifySameType (genType, type);
		    }

		    this-> _context.verifyNotIsType (ofv.getLocation ());
			
		    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), genType));
		    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
		    mapper.score += Scores::SCORE_TYPE;
		    return mapper;
		} fo;
	    }
	    
	    // Default case, we just validate it and check the type equality
	    auto left = this-> _context.validateType (ofv.getType (), true);
	    auto score = Scores::SCORE_TYPE;
	    if (type.is <ClassPtr> ()) {
		if (ofv.isOver ()) {
		    if (!this-> _context.isAncestor (left, type)) {
			Ymir::Error::occur (ofv.getType ().getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					    left.prettyString (),
					    type.prettyString ());
		    }
		} else {
		    this-> _context.verifyCompatibleType (ofv.getLocation (), type.getLocation (), left, type);
		}

		if (left.equals (type))
		score = Scores::SCORE_TYPE;
		else score = Scores::SCORE_VAR;
	    } else  {
		if (!left.to <Type> ().getProxy ().isEmpty ()) {
		    this-> _context.verifyCompleteSameType (left, type);
		} else {
		    this-> _context.verifySameType (left, type);
		}
	    }
	    
	    Mapper mapper (true, score);
	    this-> _context.verifyNotIsType (ofv.getLocation ());
			    
	    mapper.mapping.emplace (ofv.getLocation ().getStr (), createSyntaxType (ofv.getLocation (), type));
	    mapper.nameOrder.push_back (ofv.getLocation ().getStr ());
	    return mapper;
	}

	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromExplicitImplVar (const array_view <Expression> & params, const ImplVar & implv, const generator::Generator & type) const {
	    if (type.isEmpty ()) return Mapper {true, 0, {}, {}};
	    match (implv.getType ()) {
		of (Var, var) {
		    auto expr = findExpression (var.getName ().getStr (), params);
		    if (!expr.isEmpty ()) {
			int consumed = 0;
			auto vec = {type};
			Mapper mapper = std::move (validateTypeFromExplicit (params, expr, array_view <Generator> (vec), consumed));
			auto realType = this-> replaceAll (implv.getType (), mapper.mapping);
			this-> _context.verifyClassImpl (implv.getType ().getLocation (), type, realType);
			    
			mapper.mapping.emplace (implv.getLocation ().getStr (), createSyntaxType (implv.getLocation (), type));
			mapper.nameOrder.push_back (implv.getLocation ().getStr ());
			return mapper;
		    }
		}
		elof (TemplateCall, cl) {
		    std::list <Ymir::Error::ErrorMsg> errors;
		    for (auto & trait : this-> _context.getAllImplClass (type)) {
			bool succeed = true;
			Mapper mapper (true, 0);
			try {
			    int current_consumed = 0;
			    auto loc_mapper = std::move (validateTypeFromTemplCall (params, cl, trait, current_consumed));
			    Expression realType = this-> replaceAll (implv.getType (), loc_mapper.mapping);
			    this-> _context.verifyClassImpl (implv.getType ().getLocation (), type, realType);
			    mapper = std::move (mergeMappers (loc_mapper, mapper));
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
			
		    if (errors.size () == 0) {
			auto note = Ymir::Error::createNote (implv.getType ().getLocation ());
			Ymir::Error::occurAndNote (type.getLocation (), note, ExternalError::get (NOT_IMPL_TRAIT), type.prettyString (), implv.getType ().prettyString ());
		    }
			
		    throw Error::ErrorList {errors};
		}
		elof (syntax::List, lst) {
		    Ymir::Error::occur (lst.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					lst.prettyString (),
					type.to<Type> ().getTypeName ());
		}
		elof (syntax::FuncPtr, fPtr) {
		    Ymir::Error::occur (fPtr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					fPtr.prettyString (),
					type.to<Type> ().getTypeName ());
		}
		elof (syntax::ArrayAlloc, arr) {
		    Ymir::Error::occur (arr.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					arr.prettyString (),
					type.to<Type> ().getTypeName ());
		}
		elof (DecoratedExpression, dc) {
		    Ymir::Error::occur (dc.getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
					dc.prettyString (),
					type.to<Type> ().getTypeName ());
		} fo;
	    }
	    
	    this-> _context.verifyClassImpl (implv.getType ().getLocation (), type, implv.getType ());
	    Mapper mapper (true, Scores::SCORE_TYPE);
	    this-> _context.verifyNotIsType (implv.getLocation ());
			    
	    mapper.mapping.emplace (implv.getLocation ().getStr (), createSyntaxType (implv.getLocation (), type));
	    mapper.nameOrder.push_back (implv.getLocation ().getStr ());
	    return mapper;
	}

	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromDecoratedExpression (const array_view <Expression> & params, const DecoratedExpression & expr, const array_view <generator::Generator> & types, int & consumed) const {
	    if (expr.hasDecorator (syntax::Decorator::CTE) || expr.hasDecorator (syntax::Decorator::REF))
	    return Mapper (false, 0);
	    
	    return validateTypeFromImplicit (params, expr.getContent (), types, consumed);
	}

	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================       FINDS      =============================
	 * ================================================================================
	 * ================================================================================
	 */

	Expression TemplateVisitor::findExpression (const std::string & name, const array_view <Expression> & params) const {
	    for (auto & it : params) {
		match (it) {
		    of (Var, var) {
			if (var.getName ().getStr () == name) return it;
		    }
		    elof (OfVar, var) {
			if (var.getLocation ().getStr () == name) return it;
		    }
		    elof (ImplVar, var) {
			if (var.getLocation ().getStr () == name) return it;
		    }
		    elof (VariadicVar, var) {
			if (var.getLocation ().getStr () == name) return it;
		    }
		    elof (StructVar, var) {
			if (var.getLocation ().getStr () == name) return it;
		    }
		    elof (ClassVar, var) {
			if (var.getLocation ().getStr () == name) return it;
		    }
		    elof (AliasVar, var) {
			if (var.getLocation ().getStr () == name) return it;
		    }
		    elof (DecoratedExpression, dc) {
			if (dc.getContent ().is<Var> () && dc.getContent ().to <Var> ().getName ().getStr () == name) return it;
		    } fo;
		    // We don't do anything for the rest of expression types as they do not refer to types
		}
	    }
	    return Expression::empty ();
	}

	Expression TemplateVisitor::findExpressionValue (const std::string & name, const array_view <Expression> & params) const {
	    for (auto & it : params) {
		match (it) {
		    of (syntax::VarDecl, var) {
			if (var.getName ().getStr () == name) return it;
		    } fo;
		    // We don't do anything for the rest of expression types as they do not refer to values
		}
	    }
	    return Expression::empty ();
	}

	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================      CREATES     =============================
	 * ================================================================================
	 * ================================================================================
	 */


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

	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================      MAPPERS     =============================
	 * ================================================================================
	 * ================================================================================
	 */


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



	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================      REPLACE    ==============================
	 * ================================================================================
	 * ================================================================================
	 */

	Expression TemplateVisitor::replaceAll (const Expression & element, const std::map <std::string, Expression> & mapping) const {
	    if (mapping.size () == 0) return element;
	    match (element) {
		of (syntax::ArrayAlloc, arr) {
		    return syntax::ArrayAlloc::init (
			element.getLocation (),
			replaceAll (arr.getLeft (), mapping),
			replaceAll (arr.getSize (), mapping),
			arr.isDynamic ()
			);
		}
		elof (Assert, asr) {
		    return Assert::init (
			element.getLocation (),
			replaceAll (asr.getTest (), mapping),
			replaceAll (asr.getMsg (), mapping)
			);
		}
		elof (syntax::Binary, bin) {
		    return syntax::Binary::init (
			element.getLocation (),
			replaceAll (bin.getLeft (), mapping),
			replaceAll (bin.getRight (), mapping),
			replaceAll (bin.getType (), mapping)
			);
		}
		elof (syntax::Block, block) {
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
		elof_u (syntax::Bool) return element;
		elof (syntax::Break, b) {
		    return syntax::Break::init (b.getLocation (), replaceAll (b.getValue (), mapping));
		}
		elof (syntax::Cast, c) {
		    return syntax::Cast::init (
			element.getLocation (),
			replaceAll (c.getType (), mapping),
			replaceAll (c.getContent (), mapping)
			);
		}
		elof_u (syntax::Char) return element;
		elof (DecoratedExpression, dec) {
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
		elof_u (syntax::Dollar) return element;
		elof_u (syntax::Fixed) return element;
		elof_u (syntax::Float) return element;
		elof (syntax::For, fr) {
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
		elof (syntax::FuncPtr, fnp) {
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
		elof (syntax::If, fi) {
		    return syntax::If::init (
			element.getLocation (),
			replaceAll (fi.getTest (), mapping),
			replaceAll (fi.getContent (), mapping),
			replaceAll (fi.getElsePart (), mapping)
			);
		}
		elof_u (syntax::Ignore) return element;
		elof (syntax::Intrinsics, intr) {
		    return syntax::Intrinsics::init (
			element.getLocation (),
			replaceAll (intr.getContent (), mapping)
			);
		}
		elof (syntax::Lambda, lmb) {
		    return syntax::Lambda::init (
			element.getLocation (),
			replaceAll (lmb.getPrototype (), mapping),
			replaceAll (lmb.getContent (), mapping)
			);
		}
		elof (syntax::List, lst) {
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
		elof (syntax::MultOperator, mlt) {
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
		elof (syntax::NamedExpression, nmd) {
		    return syntax::NamedExpression::init (
			element.getLocation (),
			replaceAll (nmd.getContent (), mapping)
			);
		}
		elof_u (syntax::Null) return element;
		elof (syntax::OfVar, of) {
		    return syntax::OfVar::init (
			element.getLocation (),
			replaceAll (of.getType (), mapping),
			of.isOver ()
			);
		}
		elof (syntax::ImplVar, im) {
		    return syntax::ImplVar::init (
			element.getLocation (),
			replaceAll (im.getType (), mapping)
			);
		}
		elof (syntax::Return, ret) {
		    return syntax::Return::init (
			element.getLocation (),
			replaceAll (ret.getValue (), mapping)
			);
		}
		elof (syntax::Set, set) {
		    std::vector <Expression> content;
		    for (auto & it : set.getContent ())
		    content.push_back (replaceAll (it, mapping));
		    return syntax::Set::init (
			element.getLocation (),
			content
			);
		}
		elof_u (syntax::StructVar) {
		    return syntax::StructVar::init (
			element.getLocation ()
			);
		}
		elof_u (syntax::ClassVar) {
		    return syntax::ClassVar::init (
			element.getLocation ()
			);
		}
		elof_u (syntax::AliasVar) {
		    return syntax::AliasVar::init (
			element.getLocation ()
			);
		}
		elof_u (syntax::String) return element;
		elof (syntax::TemplateCall, tmpl) {
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
		elof (TemplateSyntaxWrapper, wrp) {
		    return TemplateSyntaxWrapper::init (wrp.getLocation (), wrp.getContent ());
		}
		elof (syntax::Unary, un) {
		    return syntax::Unary::init (element.getLocation (), replaceAll (un.getContent (), mapping));
		}
		elof_u (syntax::Unit) { return element; }
		elof (syntax::Var, var) {
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
		elof (syntax::VarDecl, vdecl) {
		    return syntax::VarDecl::init (
			element.getLocation (),
			vdecl.getDecorators (),
			replaceAll (vdecl.getType (), mapping),
			replaceAll (vdecl.getValue (), mapping)
			);
		}
		elof (syntax::DestructDecl, ddecl) {
		    std::vector <Expression> params;
		    for (auto & it : ddecl.getParameters ())
		    params.push_back (replaceAll (it, mapping));
		    return syntax::DestructDecl::init (
			ddecl.getLocation (),
			params,
			replaceAll (ddecl.getValue (), mapping),
			ddecl.isVariadic ()
			);
		}
		elof_u (syntax::VariadicVar) {
		    auto inner = mapping.find (element.getLocation ().getStr ());
		    if (inner != mapping.end ()) return inner-> second;
		    else return element;
		}
		elof (syntax::While, wh) {
		    return syntax::While::init (
			element.getLocation (),
			replaceAll (wh.getTest (), mapping),
			replaceAll (wh.getContent (), mapping),
			wh.isDo ()
			);
		}
		elof (syntax::TemplateChecker, ch) {
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
		}
		elof (syntax::Match, m) {
		    std::vector <Expression> matchers;
		    std::vector <Expression> actions;
		    for (auto & it : m.getMatchers ())
		    matchers.push_back (replaceAll (it, mapping));
		    for (auto & it : m.getActions ())
		    actions.push_back (replaceAll (it, mapping));
		    return syntax::Match::init (m.getLocation (), replaceAll (m.getContent (), mapping),
						matchers, actions, m.isFinal ());

		}
		elof (syntax::Catch, c) {
		    std::vector <Expression> matchers;
		    std::vector <Expression> actions;
		    for (auto & it : c.getMatchs ())
		    matchers.push_back (replaceAll (it, mapping));
		    for (auto & it : c.getActions ())
		    actions.push_back (replaceAll (it, mapping));
			
		    return syntax::Catch::init (c.getLocation (), matchers, actions);
		}
		elof (syntax::Scope, s) {
		    return syntax::Scope::init (s.getLocation (), replaceAll (s.getContent (), mapping));
		}
		elof (syntax::Throw, thr) {
		    return syntax::Throw::init (thr.getLocation (), replaceAll (thr.getValue (), mapping));
		}
		elof (syntax::Pragma, prg) {
		    std::vector <syntax::Expression> exprs;
		    for (auto & it : prg.getContent ()) {
			exprs.push_back (replaceAll (it, mapping));
		    }
		    return syntax::Pragma::init (prg.getLocation (), exprs);
		}
		elof (syntax::MacroMult, m) {
		    std::vector <syntax::Expression> exprs;
		    for (auto & it : m.getContent ()) {
			exprs.push_back (replaceAll (it, mapping));
		    }
		    return syntax::MacroMult::init (m.getLocation (), m.getEnd (), exprs, m.getMult ());
		}
		elof (syntax::MacroCall, c) {
		    return syntax::MacroCall::init (c.getLocation (), c.getEnd (), replaceAll (c.getLeft (), mapping), c.getContent ());
		}
		elof (syntax::MacroEval, e) {
		    return syntax::MacroEval::init (e.getLocation (), e.getEnd (), replaceAll (e.getContent (), mapping));
		}
		elof (syntax::MacroOr, o) {
		    return syntax::MacroOr::init (o.getLocation (), replaceAll (o.getLeft (), mapping), replaceAll (o.getRight (), mapping));
		}
		elof (syntax::MacroResult, r) {
		    return syntax::MacroResult::init (r.getLocation (), r.getContent ());
		}
		elof (syntax::MacroToken, t) {
		    return syntax::MacroToken::init (t.getLocation (), t.getContent ());
		}
		elof (syntax::MacroVar, v) {
		    return syntax::MacroVar::init (v.getLocation (), replaceAll (v.getContent (), mapping));
		}
		elof (syntax::Try, tr) {
		    return syntax::Try::init (tr.getLocation (), replaceAll (tr.getContent (), mapping));
		}
		elof (syntax::With, wh) {
		    std::vector <syntax::Expression> exprs;
		    for (auto & it : wh.getDecls ())
		    exprs.push_back (replaceAll (it, mapping));
		    return syntax::With::init (wh.getLocation (), exprs, replaceAll (wh.getContent (), mapping));
		}
		elof (syntax::Atomic, at) {
		    return syntax::Atomic::init (at.getLocation (), replaceAll (at.getWho (), mapping), replaceAll (at.getContent (), mapping));
		} fo;
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
		of (syntax::Aka, al) {
		    return syntax::Aka::init (al.getLocation (), "", replaceAll (al.getValue (), mapping));
		}
		elof (syntax::CondBlock, cd) {
		    return replaceAll (_ref, cd, mapping);
		}
		elof (syntax::DeclBlock, dl) {
		    std::vector <Declaration> decls;
		    for (auto & it : dl.getDeclarations ())
		    decls.push_back (replaceAll (it, mapping, _ref));
		    return syntax::DeclBlock::init (dl.getLocation (), "", decls, dl.isPrivate (), dl.isProt ());
		}
		elof (syntax::Class, cl) {
		    std::vector <Declaration> decls;
		    for (auto & it : cl.getDeclarations ()) {
			decls.push_back (replaceAll (it, mapping, _ref));
		    }
		    return syntax::Class::init (cl.getLocation (), "", replaceAll (cl.getAncestor (), mapping), decls, cl.getAttributes ());
		}
		elof (syntax::Enum, en) {
		    std::vector <Expression> values;
		    for (auto & it : en.getValues ())
		    values.push_back (replaceAll (it, mapping));
		    return syntax::Enum::init (en.getLocation (), "", replaceAll (en.getType (), mapping), values, {});
		}
		elof (syntax::ExpressionWrapper, wrp) {
		    return syntax::ExpressionWrapper::init (wrp.getLocation (), "", replaceAll (wrp.getContent (), mapping));
		}
		elof (syntax::ExternBlock, ext) {
		    return syntax::ExternBlock::init (
			ext.getLocation (),
			"",
			ext.getFrom (),
			ext.getSpace (),
			replaceAll (ext.getDeclaration (), mapping, _ref)
			);
		}
		elof (syntax::Constructor, cst) {
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
						      cst.getRename (),
						      replaceAll (cst.getPrototype (), mapping),
						      exprs,
						      fields,
						      replaceAll (cst.getBody (), mapping),
						      cst.getExplicitSuperCall (), cst.getExplicitSelfCall (), cst.getCustomAttributes (),
						      throwers
			);
		}
		elof (syntax::Function, func) {
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
		elof (syntax::Global, glb) {
		    return syntax::Global::init (glb.getLocation (), "", replaceAll (glb.getContent (), mapping));
		}
		elof_u (syntax::Import) return decl;
		elof (syntax::Mixin, mx) {
		    std::vector <Declaration> decls;
		    for (auto& it : mx.getDeclarations ())
		    decls.push_back (replaceAll (it, mapping, _ref));
		    return syntax::Mixin::init (mx.getLocation (), "", replaceAll (mx.getMixin (), mapping), decls);
		}
		elof (syntax::Module, mod) {
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
		elof (syntax::Struct, str) {
		    std::vector <Expression> vars;
		    for (auto &it : str.getDeclarations ())
		    vars.push_back (replaceAll (it, mapping));
		    return syntax::Struct::init (str.getLocation (), "", str.getCustomAttributes (), vars, {});
		}
		elof (syntax::Template, tmpl) {
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
		elof (syntax::Trait, trai) {
		    std::vector <Declaration> inner;
		    for (auto & it : trai.getDeclarations ())
		    inner.push_back (replaceAll (it, mapping, _ref));
		    return syntax::Trait::init (trai.getLocation (), "", inner);
		}
		elof (syntax::Use, use) {
		    return syntax::Use::init (use.getLocation (), "", replaceAll (use.getModule (), mapping));
		}
		elof (syntax::Macro, m) {
		    std::vector <Declaration> inner;
		    for (auto & it : m.getContent ())
		    inner.push_back (replaceAll (it, mapping, _ref));
		    return syntax::Macro::init (m.getLocation (), "", inner);
		}
		elof (syntax::MacroConstructor, c) {
		    std::vector <Expression> skips;
		    for (auto & it : c.getSkips ())
		    skips.push_back (replaceAll (it, mapping));
		    return syntax::MacroConstructor::init (c.getLocation (), c.getContentLoc (), "", replaceAll (c.getRule (), mapping), c.getContent (), skips);
		}
		elof (syntax::MacroRule, r) {
		    std::vector <Expression> skips;
		    for (auto & it : r.getSkips ())
		    skips.push_back (replaceAll (it, mapping));
		    return syntax::MacroRule::init (r.getLocation (), r.getContentLoc (), "", replaceAll (r.getRule (), mapping), r.getContent (), skips);
		} fo;

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
		    of (syntax::Var, var) {
			if (mapping.find (var.getName ().getStr ()) == mapping.end ())
			results.push_back (it);
			continue;
		    }
		    elof (syntax::OfVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) == mapping.end ())
			results.push_back (replaceAll (it, mapping));
			continue;
		    }
		    elof (syntax::ImplVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) == mapping.end ())
			results.push_back (replaceAll (it, mapping));
			continue;
		    }
		    elof (syntax::VariadicVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) == mapping.end ())
			results.push_back (it);
			continue;
		    }
		    elof (syntax::VarDecl, decl) {
			if (mapping.find (decl.getLocation ().getStr ()) == mapping.end ())
			results.push_back (replaceAll (it, mapping));
			continue;
		    }
		    elof (syntax::StructVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) == mapping.end ())
			results.push_back (replaceAll (it, mapping));
			continue;
		    }
		    elof (syntax::ClassVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) == mapping.end ())
			results.push_back (replaceAll (it, mapping));
			continue;
		    }
		    elof (syntax::AliasVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) == mapping.end ())
			results.push_back (replaceAll (it, mapping));
			continue;
		    }
		    elof (syntax::DecoratedExpression, dc) {
			auto f_name = format ("%[%,%]",
					      dc.getLocation ().getStr (),
					      dc.getLocation ().getLine (),
					      dc.getLocation ().getColumn ());
			
			if (mapping.find (f_name) == mapping.end ())
			results.push_back (replaceAll (it, mapping));
			continue;
		    } fo;
		    
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

		return syntax::DeclBlock::init (decl.getLocation (), "", decls, false, true);
	    } else if (!decl.getElse ().isEmpty () && decl.getElse ().is <CondBlock> ()) {
		return replaceAll (ref, decl.getElse ().to <CondBlock> (), mapping);
	    } else if (!decl.getElse ().isEmpty ()) {
		return replaceAll (decl.getElse (), mapping, ref);
	    } else return syntax::DeclBlock::init (decl.getLocation (), "", {}, false, false);
	}


	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================      SOLUTIONS    ============================
	 * ================================================================================
	 * ================================================================================
	 */

	semantic::Symbol TemplateVisitor::getFirstTemplateSolution (const semantic::Symbol & symbol) const {
	    if (symbol.isEmpty ()) return symbol;
	    match (symbol) {
		of (TemplateSolution, sl) {
		    auto ft = getFirstTemplateSolution (sl.getReferent ());
		    if (ft.isEmpty ()) return symbol;
		    return ft;
		} fo;
	    }

	    return getFirstTemplateSolution (symbol.getReferent ());
	}

	Symbol TemplateVisitor::getTemplateSolution (const Symbol & ref, const Symbol & solution) const {
	    for (auto & it : ref.getTemplates ()) {
		if (it.equals (solution)) {
		    return it;
		}
	    }

	    return Symbol::__empty__;
	}

	/**
	 * ================================================================================
	 * ================================================================================
	 * =================================      SORTING    ==============================
	 * ================================================================================
	 * ================================================================================
	 */


	std::vector <syntax::Expression> TemplateVisitor::sort (const std::vector <syntax::Expression> & exps, const std::map <std::string, syntax::Expression> & mapping) const {
	    std::vector <syntax::Expression> results;
	    for (auto & it : exps) {
		match (it) {
		    of (syntax::Var, var) {
			results.push_back (mapping.find (var.getName ().getStr ())-> second);
		    }
		    elof (syntax::OfVar, var) {
			results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
		    }
		    elof (syntax::ImplVar, var) {
			results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
		    }
		    elof (syntax::VariadicVar, var) {
			results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
		    }
		    elof (syntax::VarDecl, var) {
			results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
		    }
		    elof (syntax::StructVar, var) {
			results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
		    }
		    elof (syntax::ClassVar, var) {
			results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
		    }
		    elof (syntax::AliasVar, var) {
			results.push_back (mapping.find (var.getLocation ().getStr ())-> second);
		    }
		    elof (syntax::DecoratedExpression, dc) {
			results.push_back (mapping.find (dc.getContent ().to <Var> ().getName ().getStr ())-> second);
		    }
		    elfo {
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
		    of (syntax::Var, var) {
			if (mapping.find (var.getName ().getStr ()) != mapping.end ())
			results.push_back (var.getName ().getStr ());
		    }
		    elof (syntax::OfVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
			results.push_back (var.getLocation ().getStr ());
		    }
		    elof (syntax::ImplVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
			results.push_back (var.getLocation ().getStr ());
		    }
		    elof (syntax::VariadicVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
			results.push_back (var.getLocation ().getStr ());
		    }
		    elof (syntax::VarDecl, var) {
			if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
			results.push_back (var.getLocation ().getStr ());
		    }
		    elof (syntax::StructVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
			results.push_back (var.getLocation ().getStr ());
		    }
		    elof (syntax::ClassVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
			results.push_back (var.getLocation ().getStr ());
		    }
		    elof (syntax::AliasVar, var) {
			if (mapping.find (var.getLocation ().getStr ()) != mapping.end ())
			results.push_back (var.getLocation ().getStr ());
		    }
		    elfo {
			auto name = format ("%[%,%]", it.getLocation ().getStr (), it.getLocation ().getLine (), it.getLocation ().getColumn ());
			if (mapping.find (name) != mapping.end ())
			results.push_back (name);
		    }
		}
	    }
	    return results;
	}

	/**
	 * ================================================================================
	 * ================================================================================
	 * ==============================    FINAL_VALIDATION    ==========================
	 * ================================================================================
	 * ================================================================================
	 */

	Expression TemplateVisitor::retreiveFuncPtr (const Expression & elem, const std::vector <Expression> & syntaxTempl) const {
	    match (elem) {
		of_u (syntax::FuncPtr) return elem;
		elof (syntax::Var, var) {
		    auto F = this-> findExpression (var.getName ().getStr (), syntaxTempl);
		    if (F.is <syntax::Var> ()) return F;
		    return retreiveFuncPtr (F, syntaxTempl);
		}
		elof (syntax::OfVar, var) {
		    if (var.getType ().is<syntax::FuncPtr> ()) return var.getType ();
		    else retreiveFuncPtr (var.getType (), syntaxTempl);
		} fo;
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

	std::vector <Generator> TemplateVisitor::validateTypeOrEmptyMultiple (const Expression & type, const std::map<std::string, Expression> & mapping) const {
	    auto param = replaceAll (type, mapping);
	    std::vector <Generator> gen;
	    try {
		if (param.is <TemplateSyntaxList> ()) {
		    for (auto & it : param.to <TemplateSyntaxList> ().getContents ()) {
			if (it.is <Type> ())
			gen.push_back (it);
			else return {};
		    }
		} else {
		    gen.push_back (this-> _context.validateType (param, true));
		}
	    } catch (Error::ErrorList list) {
		return {};
	    }
	    
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
				auto type = validateTypeOrEmptyMultiple (it, mapping);
				if (type.empty ()) {
				    successful = false;
				    break;
				} else {
				    types.insert (types.end (), type.begin (), type.end ());
				}
			    }
			    if (successful) retType = F.to <syntax::FuncPtr> ().getRetType ();
			}
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
		    mapper = std::move (mergeMappers (mapper, mp));
		}
		
		return validateLambdaProtos (exprs, mapper.mapping);
	    }

	    return maps;
	}

	void TemplateVisitor::finalValidation (const Symbol & context, const std::vector<syntax::Expression> & exprs, const Mapper & mapper, const syntax::Expression & test) const {
	    for (auto & it : exprs) {
		match (it) {
		    of (syntax::VarDecl, decl) { // it is a value, we need to check the type
			if (!decl.getType ().isEmpty ()) {
			    auto type = this-> _context.validateType (replaceAll (decl.getType (), mapper.mapping), true);
			    auto auxType = this-> _context.validateValue (mapper.mapping.find (decl.getName ().getStr ())-> second);
			    this-> _context.verifySameType (type, auxType.to <Value> ().getType ());
			}
		    } fo;
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


	/**
	 * ================================================================================
	 * ================================================================================
	 * ==============================    FINAL_VALIDATION    ==========================
	 * ================================================================================
	 * ================================================================================
	 */

	Ymir::Error::ErrorMsg TemplateVisitor::partialResolutionNote (const lexing::Word & location, const Mapper & mapper) const {
	    if (mapper.nameOrder.size () != 0) {
		Ymir::OutBuffer buf;
		buf.write ("(");
		int i = 0;
		for (auto & it : mapper.nameOrder) {
		    if (i != 0)
		    buf.write (", ");
		    buf.writef ("% = %", it, mapper.mapping.find (it)-> second.prettyString ());
		    i += 1;
		}
		buf.write (")");
		return Ymir::Error::createNoteOneLine ("for : % with %", location, buf.str ());
	    } else return Ymir::Error::ErrorMsg ("");
	}
	
    }
        
}
