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

	Generator TemplateVisitor::validateFromImplicit (const TemplateRef & ref, const std::vector <Generator> & types, int & score, Symbol & symbol) const {
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
	    
	    for (auto it : Ymir::r (0, syntaxParams.size ())) {
		auto param = replaceAll (syntaxParams [it], globalMapper.mapping);
		auto mapper = validateVarDeclFromImplicit (syntaxTempl, param, types [it]);

		// We apply the mapper, to gain time
		if (!mapper.succeed) return Generator::empty ();
		else {
		    globalMapper = mergeMappers (globalMapper, mapper);
		    syntaxTempl = replaceSyntaxTempl (syntaxTempl, globalMapper.mapping);
		}		
		// The mapper will be applied on the body at the end only, so we need to merge the differents mappers	
	    }	    

	    if (globalMapper.succeed) {
		score += globalMapper.score;
		auto func = replaceAll (sym.to <semantic::Template> ().getDeclaration (), globalMapper.mapping);
		auto visit = declarator::Visitor::init ();
		visit.pushReferent (ref.getTemplateRef ().getReferent ());
		symbol = visit.visit (func);
		return this-> _context.validateMultSym (ref.getLocation (), {symbol});
	    } 
	    
	    return Generator::empty ();
	}

	TemplateVisitor::Mapper TemplateVisitor::validateVarDeclFromImplicit (const std::vector <Expression> & params, const Expression & left, const generator::Generator & type) const {
	    auto type_decl = left.to <syntax::VarDecl> ().getType ();
	    return validateTypeFromImplicit (params, type_decl, type);
	}	

	TemplateVisitor::Mapper TemplateVisitor::validateTypeFromImplicit (const std::vector <Expression> & params, const Expression & leftT, const generator::Generator & type) const {
	    match (leftT) {
		of (Var, var, {
			Expression expr = findExpression (var.getName ().str, params);
			if (expr.isEmpty ()) {
			    // if expression is empty, the type is a real one, and not a template
			    Mapper mapper;
			    mapper.succeed = true;
			    mapper.score = 0;			    
			    return mapper;
			} else return applyTypeFromExplicit (params, expr, type);
		    }
		) else of (TemplateSyntaxWrapper, wp ATTRIBUTE_UNUSED, {
			Mapper mapper;
			mapper.succeed = true;
			mapper.score = 0;			    
			return mapper;
		    }
		) else of (List, lst, {
			if (type.to <Type> ().isComplex () && type.to <Type> ().getInners ().size () == lst.getParameters ().size ()) {
			    Mapper mapper;
			    for (auto it : Ymir::r (0, lst.getParameters ().size ())) {
				auto mp = validateTypeFromImplicit (params, lst.getParameters () [it], type.to <Type> ().getInners () [it]);
				if (!mp.succeed) return mp;
				mapper = mergeMappers (mapper, mp);
			    }
			    return mapper;
			} else {
			    Mapper mapper;
			    mapper.succeed = false;
			    mapper.score = 0;			    
			    return mapper;
			}
		    }
		);
	    }

	    OutBuffer buf;
	    leftT.treePrint (buf, 0);
	    println (buf.str ());
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");	    
	}

	TemplateVisitor::Mapper TemplateVisitor::applyTypeFromExplicit (const std::vector <Expression> & params, const Expression & leftT, const generator::Generator & type) const {
	    match (leftT) {
		of (Var, var, {
			Mapper mapper;
			mapper.succeed = true;
			mapper.score = Scores::SCORE_VAR;
			mapper.mapping.emplace (var.getName ().str, createSyntaxType (var.getName (), type));
			return mapper;
		    }
		);
	    }

	    OutBuffer buf;
	    leftT.treePrint (buf, 0);
	    println (buf.str ());
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");	    
	}

	Expression TemplateVisitor::createSyntaxType (const lexing::Word & location, const generator::Generator & gen) const {
	    Generator type = gen;
	    type.to <Type> ().isMutable (false);
	    type.to <Type> ().isRef (false);
	    return TemplateSyntaxWrapper::init (location, type);
	}

	Expression TemplateVisitor::findExpression (const std::string & name, const std::vector <Expression> & params) const {
	    for (auto & it : params) {
		match (it) {
		    of (Var, var, {
			    if (var.getName ().str == name) return it;
			}
		    ) else {
			OutBuffer buf;
			it.treePrint (buf, 0);
			println (buf.str ());
			Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");	    
		    }
		}
	    }
	    return Expression::empty ();
	}

	Expression TemplateVisitor::replaceAll (const Expression & element, const std::map <std::string, Expression> & mapping) const {
	    if (mapping.size () == 0) return element;
	    match (element) {
		of (ArrayAlloc, arr,
		    return ArrayAlloc::init (
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
		     for (auto & it : fnp.getParams ())
			 params.push_back (replaceAll (it, mapping));
		     return syntax::FuncPtr::init (element.getLocation (),
				     replaceAll (fnp.getRet (), mapping),
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
			 replaceAll (lmb.getProto (), mapping),
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
		) else of (syntax::Unary, un,
		     return syntax::Unary::init (element.getLocation (), replaceAll (un.getContent (), mapping));
		) else of (syntax::Unit, uni ATTRIBUTE_UNUSED, return element;
		) else of (syntax::Var, var, {
		     auto inner = mapping.find (var.getName ().str);
		     if (inner != mapping.end ()) return inner-> second;
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
	    } else return decl;
	}

	syntax::Function::Prototype TemplateVisitor::replaceAll (const syntax::Function::Prototype & proto, const std::map <std::string, Expression> & mapping) const {
	    std::vector <Expression> vars;
	    for (auto & it : proto.getParameters ()) 
		vars.push_back (replaceAll (it, mapping));
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
			}) else {
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
	    for (auto & it : right.mapping) {
		if (result.find (it.first) != result.end ()) {		    
		    Ymir::Error::halt ("%(r) - reaching impossible point, multiple definition of same template param", "Critical");	    
		} else {
		    result.emplace (it.first, it.second);
		}
	    }
	    return Mapper {true, left.score + right.score, result};
	}

    }    
    
}
