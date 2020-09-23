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

	std::string MacroVisitor::Mapper::toString (int i) const {
	    Ymir::OutBuffer buf;
	    buf.writef ("%*# : \"%\" {", i, "\t", this-> consumed);
	    int y = 0;
	    if (this-> mapping.size () != 0) buf.writeln ("");	    
	    for (auto & it : this-> mapping) {
		if (y != 0) buf.writeln (",");
		buf.writefln ("%*%=> [", i+1, "\t", it.first);
		if (it.second.size () > 1) buf.writeln ("");
		int z = 0;
		for (auto &j : it.second) {
		    if (z != 0) buf.writeln (",");
		    buf.write (j.toString (i + 2));
		    z += 1;
		}
		if (it.second.size () != 0)
		    buf.writef ("\n%*]", i+1 , "\t");
		else buf.write ("]");
		y += 1;
	    }
	    if (this-> mapping.size () != 0)
		buf.writef ("\n%*}", i, "\t");
	    else buf.writef ("}");
	    return buf.str ();
	}
	
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

	generator::Generator MacroVisitor::validateConstructor (const semantic::Symbol & constr, const syntax::MacroCall & expression) {
	    auto rule = constr.to <semantic::MacroConstructor> ().getContent ().to <syntax::MacroConstructor> ().getRule ();
	    std::string content = expression.getContent ();
	    std::string rest;
	    Mapper mapper (false);
	    
	    match (rule) {
		of (MacroMult, mult, {
		    if (mult.getMult ().isEof () || mult.getMult () == "")
			mapper = validateMacroList (mult, content, rest);
		    }
		) else of (MacroOr, or_, mapper = validateMacroOr (or_, content, rest))
		else of (MacroVar, var, mapper = validateMacroVar (var, content, rest))
		else of (MacroToken, tok, mapper = validateMacroToken (tok, content, rest))
	    }

	    if (mapper.succeed && rest.length () == 0) {
		return this-> _context.validateMacroExpression (
		    constr,
		    validateMapper (constr.getName (), constr.to <semantic::MacroConstructor> ().getContent ().to <syntax::MacroConstructor> ().getContent (), mapper)
		);
	    }
	    
	    return Generator::empty ();
	}

	std::vector <MacroVisitor::Mapper> MacroVisitor::validateMacroMult (const syntax::MacroMult & mult, const std::string & content, std::string & rest) {
	    if (mult.getMult ().isEof () || mult.getMult () == "") {
		return {validateMacroList (mult, content, rest)};
	    } else if (mult.getMult () == Token::STAR) {
		return validateMacroRepeat (mult, content, rest);
	    } else if (mult.getMult () == Token::PLUS) {
		return validateMacroOneOrMore (mult, content, rest);
	    } else if (mult.getMult () == Token::INTEG) {
		return validateMacroOneOrNone (mult, content, rest);
	    }
	    
	    println (mult.getMult ());
	    Ymir::Error::halt ("", "");	    	    
	    return {Mapper (false)};
	}


	MacroVisitor::Mapper MacroVisitor::validateMacroList (const syntax::MacroMult & mult, const std::string & content, std::string & rest) {
	    Mapper mapper (true);
	    rest = content;
	    for (auto & it : mult.getContent ()) {
		Mapper local_mapper (false);
		std::string content_2 = rest;
		
		match (it) {
		    of (MacroMult, mult, {
			if (mult.getMult ().isEof () || mult.getMult () == "")
			    local_mapper = validateMacroList (mult, content_2, rest);
			}
		    ) else of (MacroOr, or_, local_mapper = validateMacroOr (or_, content_2, rest))
		    else of (MacroVar, var, local_mapper = validateMacroVar (var, content_2, rest))
		    else of (MacroToken, tok, local_mapper = validateMacroToken (tok, content_2, rest))   
		}
		if (!local_mapper.succeed) return Mapper (false);
		mapper = mergeMapper (mapper, local_mapper);
	    }
	    
	    return mapper;
	}
	
	std::vector <MacroVisitor::Mapper> MacroVisitor::validateMacroRepeat (const syntax::MacroMult & mult, const std::string & content, std::string & rest) {
	    rest = content;
	    std::vector <Mapper> values;
	    while (true) {
		auto content_2 = rest;
		auto local_mapper = validateMacroList (mult, content_2, rest);
		if (local_mapper.succeed) {
		    values.push_back (local_mapper);
		} else break;
	    }
	    	    	    
	    return values;
	}
	
	std::vector<MacroVisitor::Mapper> MacroVisitor::validateMacroOneOrMore (const syntax::MacroMult & mult, const std::string & content, std::string & rest) {
	    return {};
	}
	
	std::vector<MacroVisitor::Mapper> MacroVisitor::validateMacroOneOrNone (const syntax::MacroMult & mult, const std::string & content, std::string & rest) {
	    return {};
	}		
	
	MacroVisitor::Mapper MacroVisitor::validateMacroOr (const syntax::MacroOr & mult, const std::string & content, std::string & rest) {
	    std::string rest_left;
	    Mapper mapper (true);
	    match (mult.getLeft ()) {
		of (MacroMult, mult, {
			if (mult.getMult ().isEof () || mult.getMult () == "")
			    mapper = validateMacroList (mult, content, rest_left);
		    }		    
		) else of (MacroOr, or_, mapper = validateMacroOr (or_, content, rest_left)
		) else of (MacroVar, var, mapper = validateMacroVar (var, content, rest_left)
		) else of (MacroToken, tok, mapper = validateMacroToken (tok, content, rest_left));		
	    }
	    if (mapper.succeed) {
		rest = rest_left;
		return mapper;
	    } else {
		match (mult.getRight ()) {
		    of (MacroMult, mult, {
			    if (mult.getMult ().isEof () || mult.getMult () == "")
				mapper = validateMacroList (mult, content, rest);
			}		    
		    ) else of (MacroOr, or_, mapper = validateMacroOr (or_, content, rest)
		    ) else of (MacroVar, var, mapper = validateMacroVar (var, content, rest)
		    ) else of (MacroToken, tok, mapper = validateMacroToken (tok, content, rest));		    
		}
		return mapper;
	    }
	    
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroVar (const syntax::MacroVar & var, const std::string & content, std::string & rest) {
	    auto rule = var.getContent ();
	    Mapper mapper (false);
	    match (rule) {
		of (MacroMult, mult, {
			auto mappers = validateMacroMult (mult, content, rest);
			if (mappers.size () == 1 && !mappers [0].succeed) return Mapper (false);
			else {
			    Ymir::OutBuffer buf;
			    for (auto & it : mappers)
				buf.write (it.consumed);
			    Mapper result (true, buf.str ());
			    result.mapping.emplace (var.getLocation ().str, mappers);
			    return result;
			}
		    }
		)
		else of (MacroOr, or_, mapper = validateMacroOr (or_, content, rest))
		else of (MacroVar, var, mapper = validateMacroVar (var, content, rest))
		else of (MacroToken, tok, mapper = validateMacroToken (tok, content, rest))
	    }

	    if (mapper.succeed) {
		std::vector <Mapper> vec = {mapper};
		Mapper result (true, mapper.consumed);
		result.mapping.emplace (var.getLocation ().str, vec);
		return result;
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

	syntax::Expression MacroVisitor::validateMapper (const lexing::Word & loc, const std::string & content, const MacroVisitor::Mapper & mapping) {	    
	    Ymir::OutBuffer buf;
	    std::list <std::string> errors;	    
	    try {		
		lexing::Lexer lex = lexing::Lexer::initFromString (content, loc.locFile, {}, {}, loc.line);
		while (true) {
		    auto next = lex.next ();
		    if (next == Token::MACRO_ACC || next == Token::MACRO_CRO || next == Token::MACRO_PAR) {
			auto vecs = validateMacroEval (content, loc, toMacroEval (content, loc, lex, next), mapping);
			for (auto & it : vecs)
			    buf.write (it.consumed);
		    } else if (!next.isEof ()) {
			buf.write (next.str);
		    } else break;		
		}
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    }
	    	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }

	    auto expr = "{" + buf.str () + "}";

	    syntax::Expression result (syntax::Expression::empty ());	    
	    try {
		auto lex = lexing::Lexer::initFromString (expr, loc.locFile,
							  {Token::SPACE, Token::TAB, Token::RETURN, Token::RRETURN},
							  {
							      {Token::LCOMM1, {Token::RCOMM1, ""}},
								  {Token::LCOMM2, {Token::RETURN, ""}},
								      {Token::LCOMM3, {Token::RCOMM3, ""}},
									  {Token::LCOMM4, {Token::RCOMM3, Token::STAR}},
									      {Token::LCOMM5, {Token::RCOMM5, Token::PLUS}}
							  }, loc.line);
		
		auto visit = syntax::Visitor::init (lex);
		result = visit.visitExpression ();
	    }  catch (Error::ErrorList list) {
		errors = list.errors;
	    }
	    	    
	    if (result.isEmpty ()) throw Error::ErrorList {errors};
	    
	    return result;
	} 

	syntax::Expression MacroVisitor::toMacroEval (const std::string & content, const lexing::Word & loc, lexing::Lexer & lex, const lexing::Word & begin) {
	    std::list <std::string> errors;
	    syntax::Expression inner (syntax::Expression::empty ());
	    
	    try {

		lex.skipEnable (Token::SPACE,   true);
		lex.skipEnable (Token::TAB,     true);
		lex.skipEnable (Token::RETURN,  true);
		lex.skipEnable (Token::RRETURN, true);

		auto visit = syntax::Visitor::init (lex);
		inner = visit.visitExpression ();
		lex = visit.getLexer ();
		
		if (begin == Token::MACRO_ACC) lex.next ({Token::RACC});
		if (begin == Token::MACRO_PAR) lex.next ({Token::RPAR});
		if (begin == Token::MACRO_CRO) lex.next ({Token::RCRO});

		lex.skipEnable (Token::SPACE,   false);
		lex.skipEnable (Token::TAB,     false);
		lex.skipEnable (Token::RETURN,  false);
		lex.skipEnable (Token::RRETURN, false);
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    }
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};		
	    }
	    
	    return inner;
	} 
	
	std::vector <MacroVisitor::Mapper> MacroVisitor::validateMacroEval (const std::string & content, const lexing::Word & loc, const syntax::Expression & eval, const Mapper & mapper) {
	    std::list <std::string> errors;	    
	    try {
		match (eval) {
		    of (syntax::Var, var, {
			    auto inner = mapper.mapping.find (var.getName ().str);
			    if (inner != mapper.mapping.end ()) {
				return inner-> second;
			    } else {
				Ymir::OutBuffer buf;
				int i = 0;
				for (auto & it : mapper.mapping) {
				    if (i != 0) buf.write (", ");
				    buf.write (it.first);
				    i += 1;
				}
				
				Ymir::Error::occur (
				    var.getLocation (),
				    ExternalError::get (UNDEF_MACRO_EVAL),
				    var.getName ().str
				);
			    }
			}
		    ) else of (syntax::Binary, bin, {
			    if (bin.getLocation () == Token::DCOLON) {
				auto left = validateMacroEval (content, loc, bin.getLeft (), mapper);
				auto right = bin.getRight ().to <syntax::Var> ().getName ().str;
				if (left.size () == 1) {
				    auto inner = left[0].mapping.find (right);
				    if (inner != left[0].mapping.end ()) {
					return inner-> second;
				    } else {
					Ymir::OutBuffer buf;
					int i = 0;
					for (auto & it : left [0].mapping) {
					    if (i != 0) buf.write (", ");
					    buf.write (it.first);
					    i += 1;
					}

					Ymir::Error::occur (
					    bin.getRight ().getLocation (),
					    ExternalError::get (UNDEFINED_SUB_PART_FOR),
					    right,
					    bin.getLeft ().prettyString ()
					);
				    }
				} else {
				    Ymir::Error::occur (
					eval.getLocation (),
					ExternalError::get (UNDEFINED_SUB_PART_FOR),
					right					,
					bin.getLeft ().prettyString ()
				    );				    
				}
			    }
			}
		    ) else of (syntax::MultOperator, op, {
			    if (op.getLocation () == Token::LCRO && op.getRights ().size () == 1) {
				auto left = validateMacroEval (content, loc, op.getLeft (), mapper);				
				auto value = this-> _context.retreiveValue (this-> _context.validateValue (op.getRights () [0]));
				if (!value.is<Fixed> () || (value.to<Fixed> ().getType ().to <Integer> ().isSigned () && value.to <Fixed> ().getUI ().i < 0)) {
				    Ymir::Error::occur (op.getRights () [0].getLocation (), ExternalError::get (INCOMPATIBLE_TYPES),
							      value.to <Value> ().getType ().to <Type> ().getTypeName (),
							      (Integer::init (lexing::Word::eof (), 64, false)).to<Type> ().getTypeName ()
				    );				    
				}
				auto size = value.to <Fixed> ().getUI ().u;
				if (size >= left.size ()) {
				    Ymir::Error::occur (value.getLocation (), ExternalError::get (OVERFLOW_ARITY), size, left.size ());
				}
				return {left [size]};
			    } else {
				std::vector <std::string> names;
				for (auto & it : op.getRights ())
				    names.push_back (this-> _context.validateValue (it).to <Value> ().getType ().to <Type> ().getTypeName ());
			    }
			}
		    );
		}
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    }
	    
	    Ymir::OutBuffer buf;
	    for (auto & it : errors) {
		buf.write (it);
	    }
	    
	    Ymir::Error::occurAndNote (
		eval.getLocation (),
		buf.str (),
		ExternalError::get (UNDEF_MACRO_EVAL),
		eval.prettyString ()
	    );

	    return {};
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
