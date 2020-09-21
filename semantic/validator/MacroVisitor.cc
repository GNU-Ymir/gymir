#include <ymir/semantic/validator/MacroVisitor.hh>
#include <ymir/semantic/validator/BinaryVisitor.hh>
#include <ymir/semantic/validator/SubVisitor.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/global/State.hh>
#include <ymir/global/Core.hh>
using namespace global;

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	using namespace syntax;

	MacroVisitor::MacroVisitor (Visitor & context) :
	    _context (context)
	{}

	MacroVisitor MacroVisitor::init (Visitor & context) {
	    return MacroVisitor {context};
	}

	generator::Generator MacroVisitor::validate (const syntax::MacroCall & expression) {
	    auto left = this-> _context.validateValue (expression.getLeft ());
	    std::vector <std::string> errors;
	    Generator ret (Generator::empty ());
	    try {
		match (left) {
		    of (MultSym, sym,
			ret = validateMultSym (sym, expression, errors);
		    ) else of (MacroRef, mref,
			       ret = validateMacroRef (mref, expression, errors);
		    );		    
		}
	    } catch (Error::ErrorList list) {
		ret = Generator::empty ();
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    } 

	    if (ret.isEmpty ()) {
		this-> error (expression.getLocation (), expression.getEnd (), left, errors);
	    }
	    
	    return ret;
	}

	generator::Generator MacroVisitor::validateMultSym (const MultSym & sym, const syntax::MacroCall & expression, std::vector <std::string> & errors) {
	    return Generator::empty ();
	}
	
	generator::Generator MacroVisitor::validateMacroRef (const MacroRef & sym, const syntax::MacroCall & expression, std::vector <std::string> & errors) {
	    auto constructors = this-> _context.getMacroConstructor (sym.getLocation (), sym);
	    std::vector <Generator> values;
	    for (auto & it : constructors) {
		try {
		    values.push_back (validateConstructor (it, expression));		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		} 		
	    }

	    if (values.size () > 1) {
		return MultSym::init (sym.getLocation (), values);
	    } else if (values.size () == 0) 		
		return Generator::empty ();
	    else return values [0];
	}

	generator::Generator MacroVisitor::validateConstructor (const semantic::MacroConstructor & constr, const syntax::MacroCall & expression) {
	    auto rule = constr.getContent ().to <syntax::MacroConstructor> ().getRule ();
	    std::string content = expression.getContent ();
	    std::string rest;
	    Mapper mapper (false);
	    
	    match (rule) {
		of (MacroMult, mult, mapper = validateMacroMult (mult, content, rest))
		else of (MacroOr, or_, mapper = validateMacroOr (or_, content, rest))
		else of (MacroVar, var, mapper = validateMacroVar (var, content, rest))
		else of (MacroToken, tok, mapper = validateMacroToken (tok, content, rest))
	    }

	    if (mapper.succeed) {
		return validateExpression (
		    validateMapper (constr.getContent ().to <syntax::MacroConstructor> ().getContent (), mapper)
		);
	    }
	    
	    return Generator::empty ();
	}

	MacroVisitor::Mapper MacroVisitor::validateMacroMult (const syntax::MacroMult & mult, const std::string & content, std::string & rest) {
	    if (mult.getMult ().isEof () || mult.getMult () == "") {
		return validateMacroList (mult, content, rest);
	    } else if (mult.getMult () == Token::STAR) {
		return validateMacroRepeat (mult, content, rest);
	    } else if (mult.getMult () == Token::PLUS) {
		return validateMacroOneOrMore (mult, content, rest);
	    } else if (mult.getMult () == Token::INTEG) {
		return validateMacroOneOrNone (mult, content, rest);
	    }
	    
	    println (mult.getMult ());
	    Ymir::Error::halt ("", "");	    	    
	    return Mapper (false);
	}


	MacroVisitor::Mapper MacroVisitor::validateMacroList (const syntax::MacroMult & mult, const std::string & content, std::string & rest) {
	    Mapper mapper (true);
	    rest = content;
	    for (auto & it : mult.getContent ()) {
		Mapper local_mapper (false);
		std::string content_2 = rest;
		
		match (it) {
		    of (MacroMult, mult, local_mapper = validateMacroMult (mult, content_2, rest))
		    else of (MacroOr, or_, local_mapper = validateMacroOr (or_, content_2, rest))
		    else of (MacroVar, var, local_mapper = validateMacroVar (var, content_2, rest))
		    else of (MacroToken, tok, local_mapper = validateMacroToken (tok, content_2, rest))   
		}
		if (!local_mapper.succeed) return Mapper (false);
		mapper = mergeMapper (mapper, local_mapper);
	    }
	    
	    return mapper;
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroRepeat (const syntax::MacroMult & mult, const std::string & content, std::string & rest) {
	    Mapper mapper (true);
	    rest = content;
	    std::vector <Mapper> values;
	    while (true) {
		auto content_2 = rest;
		auto local_mapper = validateMacroList (mult, content_2, rest);
		if (local_mapper.succeed) {
		    values.push_back (local_mapper);
		} else break;
	    }
	    	    	    
	    return Mapper (false);
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroOneOrMore (const syntax::MacroMult & mult, const std::string & content, std::string & rest) {
	    return Mapper (false);
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroOneOrNone (const syntax::MacroMult & mult, const std::string & content, std::string & rest) {
	    return Mapper (false);
	}		
	
	MacroVisitor::Mapper MacroVisitor::validateMacroOr (const syntax::MacroOr & mult, const std::string & content, std::string & rest) {
	    return Mapper (false);
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroVar (const syntax::MacroVar & var, const std::string & content, std::string & rest) {
	    auto rule = var.getContent ();
	    Mapper mapper (false);
	    match (rule) {
		of (MacroMult, mult, mapper = validateMacroMult (mult, content, rest))
		else of (MacroOr, or_, mapper = validateMacroOr (or_, content, rest))
		else of (MacroVar, var, mapper = validateMacroVar (var, content, rest))
		else of (MacroToken, tok, mapper = validateMacroToken (tok, content, rest))
	    }

	    if (mapper.succeed) {
		std::vector <Mapper> vec = {Mapper (true, mapper.consumed)};
		mapper.mapping.emplace (var.getLocation ().str, vec);
		return mapper;
	    }
	    
	    return Mapper (false);
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroToken (const syntax::MacroToken & mult, const std::string & content, std::string & rest) {
	    auto len = mult.getContent ().length ();
	    if (len > content.length ()) return Mapper (false);
	    for (auto it : Ymir::r(0, len)) {
		if (mult.getContent () [it] != content [it]) return Mapper (false);
	    }

	    rest = content.substr (len);
	    return Mapper (true, content.substr (0, len), {});
	}

	MacroVisitor::Mapper MacroVisitor::mergeMapper (const MacroVisitor::Mapper & left, const MacroVisitor::Mapper & right) const {
	    Mapper result (left.succeed && right.succeed);
	    for (auto & it : left.mapping) {
		result.mapping.emplace (it.first, it.second);	       
	    }

	    for (auto & it : right.mapping) {
		result.mapping.emplace (it.first, it.second);	       
	    }

	    result.consumed = left.consumed + right.consumed;
	    return result;
	}

	syntax::Expression MacroVisitor::validateMapper (const syntax::Expression & element, const MacroVisitor::Mapper & mapping) {
	    if (mapping.mapping.size () == 0) return element;
	    match (element) {
		of (syntax::ArrayAlloc, arr,
		    return syntax::ArrayAlloc::init (
			element.getLocation (),
			validateMapper (arr.getLeft (), mapping),
			validateMapper (arr.getSize (), mapping),
			arr.isDynamic ()
		    );
		) else of (Assert, asr,
			   return Assert::init (
			       element.getLocation (),
			       validateMapper (asr.getTest (), mapping),
			       validateMapper (asr.getMsg (), mapping)
			   );
		) else of (syntax::Binary, bin,
			   return syntax::Binary::init (
			       element.getLocation (),
			       validateMapper (bin.getLeft (), mapping),
			       validateMapper (bin.getRight (), mapping),
			       validateMapper (bin.getType (), mapping)
			   );
		) else of (syntax::Block, block, {
			std::vector <Expression> content;			
			Declaration declMod (Declaration::empty ());
			if (!block.getDeclModule ().isEmpty ())
			    Ymir::Error::halt ("", ""); // TODO, handle that
			// // No reference of symbol, here, but no risk to have a CondBlock inside a normal block
			// declMod = validateMapper (mapping, block.getDeclModule (), semantic::Symbol::empty ());

			
			for (auto & it : block.getContent ())
			    content.push_back (validateMapper (it, mapping));
			
			Expression catcher (Expression::empty ());
			if (!block.getCatcher ().isEmpty ()) catcher = validateMapper (block.getCatcher (), mapping);
			std::vector <Expression> scopes;
			for (auto & it : block.getScopes ())
			    scopes.push_back (validateMapper (it, mapping));
			
			return syntax::Block::init (element.getLocation (), block.getEnd (), declMod, content, catcher, scopes);
		    }
		) else of (syntax::Bool, b ATTRIBUTE_UNUSED, return element;
		) else of (syntax::Break, b, {
			return syntax::Break::init (b.getLocation (), validateMapper (b.getValue (), mapping));
		    }
		) else of (syntax::Cast, c,
			   return syntax::Cast::init (
			       element.getLocation (),
			       validateMapper (c.getType (), mapping),
			       validateMapper (c.getContent (), mapping)
			   );
		) else of (syntax::Char, c ATTRIBUTE_UNUSED, return element;
		) else of (DecoratedExpression, dec, {
			return syntax::DecoratedExpression::init (
			    element.getLocation (),
			    dec.getDecorators (),
			    validateMapper (dec.getContent (), mapping)
			);
		    }
		) else of (syntax::Dollar, dol ATTRIBUTE_UNUSED, return element;
		) else of (syntax::Fixed, fix ATTRIBUTE_UNUSED, return element;
		) else of (syntax::Float, fl ATTRIBUTE_UNUSED, return element;
		) else of (syntax::For, fr, {
			std::vector <Expression> vars;
			for (auto & it : fr.getVars ())
			    vars.push_back (validateMapper (it, mapping));
			return syntax::For::init (
			    element.getLocation (),
			    vars,
			    validateMapper (fr.getIter (), mapping),
			    validateMapper (fr.getBlock (), mapping)
			);
		    }
		) else of (syntax::FuncPtr, fnp, {
			std::vector<Expression> params;
			for (auto & it : fnp.getParameters ()) 
			    params.push_back (validateMapper (it, mapping));
			
			return syntax::FuncPtr::init (element.getLocation (),
						      validateMapper (fnp.getRetType (), mapping),
						      params);
		    }
		) else of (syntax::If, fi,
			   return syntax::If::init (
			       element.getLocation (),
			       validateMapper (fi.getTest (), mapping),
			       validateMapper (fi.getContent (), mapping),
			       validateMapper (fi.getElsePart (), mapping)
			   );
		) else of (syntax::Ignore, ig ATTRIBUTE_UNUSED, return element;
		) else of (syntax::Intrinsics, intr,
			   return syntax::Intrinsics::init (
			       element.getLocation (),
			       validateMapper (intr.getContent (), mapping)
			   );
		) else of (syntax::Lambda, lmb,
			   std::vector <syntax::Expression> params;
			   for (auto & it : lmb.getPrototype ().getParameters ())
			       params.push_back (validateMapper (it, mapping));
			   auto retType = validateMapper (lmb.getPrototype ().getType (), mapping);
			   
			   return syntax::Lambda::init (
			       element.getLocation (),
			       syntax::Function::Prototype::init (params, retType, lmb.getPrototype ().isVariadic ()),
			       validateMapper (lmb.getContent (), mapping)
			   );
		) else of (syntax::List, lst, {
			std::vector <Expression> params;
			for (auto & it : lst.getParameters ()) 
			    params.push_back (validateMapper (it, mapping));
			
			return syntax::List::init (
			    element.getLocation (),
			    lst.getEnd (),
			    params
			);
		    }
		) else of (syntax::MultOperator, mlt, {
			std::vector <Expression> rights;
			for (auto & it : mlt.getRights ())
			    rights.push_back (validateMapper (it, mapping));
			return syntax::MultOperator::init (
			    element.getLocation (),
			    mlt.getEnd (),
			    validateMapper (mlt.getLeft (), mapping),
			    rights
			);
		    }
		) else of (syntax::NamedExpression, nmd,
			   return syntax::NamedExpression::init (
			       element.getLocation (),
			       validateMapper (nmd.getContent (), mapping)
			   );
		) else of (syntax::Null, nl ATTRIBUTE_UNUSED, return element;
		) else of (syntax::OfVar, of,
			   return syntax::OfVar::init (
			       element.getLocation (),
			       validateMapper (of.getType (), mapping)
			   );
		) else of (syntax::ImplVar, im,
			   return syntax::ImplVar::init (
			       element.getLocation (),
			       validateMapper (im.getType (), mapping)
			   );
		) else of (syntax::Return, ret,
			   return syntax::Return::init (
			       element.getLocation (),
			       validateMapper (ret.getValue (), mapping)
			   );
		) else of (syntax::Set, set, {
			std::vector <Expression> content;
			for (auto & it : set.getContent ())
			    content.push_back (validateMapper (it, mapping));
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
			for (auto & it : tmpl.getParameters ()) 
			    params.push_back (validateMapper (it, mapping));
			
			return syntax::TemplateCall::init (
			    element.getLocation (),
			    params,
			    validateMapper (tmpl.getContent (), mapping)
			);
		    }
		) else of (syntax::Unary, un,
			   return syntax::Unary::init (element.getLocation (), validateMapper (un.getContent (), mapping));
		) else of (syntax::Unit, uni ATTRIBUTE_UNUSED, { return element; }
		) else of (syntax::Var, var ATTRIBUTE_UNUSED, {	return element;  }
		) else of (syntax::VarDecl, vdecl,
			   return syntax::VarDecl::init (
			       element.getLocation (),
			       vdecl.getDecorators (),
			       validateMapper (vdecl.getType (), mapping),
			       validateMapper (vdecl.getValue (), mapping)
			   );

		) else of (syntax::DestructDecl, ddecl,
			   std::vector <Expression> params;
			   for (auto & it : ddecl.getParameters ()) params.push_back (validateMapper (it, mapping));
			   return syntax::DestructDecl::init (
			       ddecl.getLocation (),
			       params,
			       validateMapper (ddecl.getValue (), mapping),
			       ddecl.isVariadic ()
			   );			   
		) else of (syntax::VariadicVar, vdvar ATTRIBUTE_UNUSED, { return element; }
		) else of (syntax::While, wh, 
			   return syntax::While::init (
			       element.getLocation (),
			       validateMapper (wh.getTest (), mapping),
			       validateMapper (wh.getContent (), mapping),
			       wh.isDo ()
			   );
		) else of (syntax::TemplateChecker, ch,
			   std::vector <syntax::Expression> params;
			   std::vector <syntax::Expression> calls;
			   for (auto & it : ch.getCalls ()) calls.push_back (validateMapper (it, mapping));			  			   
			   for (auto & it : ch.getParameters ()) params.push_back (validateMapper (it, mapping));
			   
			   return syntax::TemplateChecker::init (
			       element.getLocation (),
			       calls, params
			   );
		) else of (syntax::Match, m, {
			std::vector <Expression> matchers;
			std::vector <Expression> actions;
			for (auto & it : m.getMatchers ()) matchers.push_back (validateMapper (it, mapping));
			for (auto & it : m.getActions ())  actions.push_back (validateMapper (it, mapping));
			return syntax::Match::init (m.getLocation (), validateMapper (m.getContent (), mapping),
						    matchers, actions, m.isFinal ());

		    }
		) else of (syntax::Catch, c, {
			std::vector <Expression> matchers;
			std::vector <Expression> actions;
			for (auto & it : c.getMatchs ())
			    matchers.push_back (validateMapper (it, mapping));
			for (auto & it : c.getActions ())
			    actions.push_back (validateMapper (it, mapping));
			
			return syntax::Catch::init (c.getLocation (), matchers, actions);
		    }
		) else of (syntax::Scope, s, {
			return syntax::Scope::init (s.getLocation (), validateMapper (s.getContent (), mapping));
		    }
		) else of (syntax::Throw, thr, {
			return syntax::Throw::init (thr.getLocation (), validateMapper (thr.getValue (), mapping));
		    }
		) else of (syntax::Pragma, prg, {
			std::vector <syntax::Expression> exprs;
			for (auto & it : prg.getContent ()) {
			    exprs.push_back (validateMapper (it, mapping));
			}
			return syntax::Pragma::init (prg.getLocation (), exprs);
		    }
		) else of (syntax::MacroEval, eval, {
			return validateMacroEval (eval, mapping);
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

	syntax::Expression MacroVisitor::validateMacroEval (const syntax::MacroEval & eval, const Mapper & mapper) {
	    return Expression::empty ();
	}
	

	generator::Generator MacroVisitor::validateExpression (const syntax::Expression & expr) {
	    println (expr.prettyString ());
	    return Generator::empty ();
	}
	    
	void MacroVisitor::error (const lexing::Word & location, const lexing::Word & end, const Generator & left, const std::vector <std::string> & errors) {
	    std::string leftName;
	    match (left) {
		of (FrameProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())
		else of (MultSym,    sym,   leftName = sym.getLocation ().str)
		else of (TemplateRef, cl, leftName = cl.prettyString ())
		else of (TemplateClassCst, cl, leftName = cl.prettyString ())
		else of (ModuleAccess, acc, leftName = acc.prettyString ())
		else of (MacroRef, mref, leftName = mref.prettyString ())
		else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ())		       
	    }
	    
	    OutBuffer buf;
	    for (auto & it : errors)
		buf.write (it, "\n");
	    
	    Ymir::Error::occurAndNote (
		location,
		end,
		buf.str (), 
		ExternalError::get (UNDEFINED_MACRO_OP),
		leftName
	    );
	    	   
	}	
	
    }
    
}
