#include <ymir/semantic/validator/MacroVisitor.hh>
#include <ymir/semantic/validator/BinaryVisitor.hh>
#include <ymir/semantic/validator/SubVisitor.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/global/State.hh>
#include <ymir/syntax/declaration/MacroRule.hh>
#include <ymir/global/Core.hh>

using namespace global;

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	using namespace syntax;
	

	const std::string MacroVisitor::__EXPR__ = "expr";

	std::string some (const std::string & text, ulong current) {
	    auto len = text.length () - current;
	    if (len < 10) return text.substr (current, 10);
	    else return text.substr (current, 4) + "[...]" + text.substr (text.length () - 4, 4);
	}
	
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
		this-> _context.enterClassDef (sym.getMacroRef ());
		try {
		    values.push_back (validateConstructor (it, expression));		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}

		this-> _context.exitClassDef ();		
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

	    this-> _call = expression.getLocation ();
	    this-> _content = content;

	    
	    ulong current = 0;
	    Mapper mapper (false);
	    std::list <std::string> errors;
	    
	    this-> _context.pushReferent (constr, "validateMacroExpression");
	    this-> _context.enterForeign ();

	    try {
		match (rule) {
		    of (MacroMult, mult, mapper = validateMacroMult (mult, content, current)
		    ) else of (MacroOr, or_,
			       mapper = validateMacroOr (or_, content, current)
		    ) else of (MacroVar, var,
			       mapper = validateMacroVar (var, content, current)
		    ) else of (MacroToken, tok,
			       mapper = validateMacroToken (tok, content, current)
		    ) else mapper = validateRule (rule, content, current);		
		}
	    }  catch (Error::ErrorList list) {
		errors = list.errors;
	    }

	    this-> _context.exitForeign ();
	    this-> _context.popReferent ("validateMacroExpression");
	    if (errors.size () != 0) throw Error::ErrorList {errors};
	    
	    if (mapper.succeed) {
		if (current != content.length ()) {
		    ulong line = 0, col = 0, seek = 0;
		    computeLine (line, col, seek, current);
		    lexing::Word word (UNKNOWN_LOCATION, content.substr (current, 1));
		    word.setLocus (this-> _call.getFile (), line, col, seek);
		    
		    auto note = Ymir::Error::createNote (word);
		    Ymir::Error::occurAndNote (
			rule.getLocation (),
			note,
			ExternalError::get (MACRO_REST),
			some (content, current)
		    );
			
		}

		return this-> _context.validateMacroExpression (
		    constr,
		    validateMapper (constr.getName (), constr.to <semantic::MacroConstructor> ().getContent ().to <syntax::MacroConstructor> ().getContent (), mapper)
		);
	    }
	    
	    return Generator::empty ();
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroMult (const syntax::MacroMult & mult, const std::string & content, ulong & current) {
	    if (mult.getMult ().isEof () || mult.getMult () == "") {
		return validateMacroList (mult, content, current);
	    } else if (mult.getMult () == Token::INTEG) {
		return validateMacroOneOrNone (mult, content, current);
	    } else {
		if (mult.getMult () == Token::STAR || mult.getMult () == Token::PLUS) {
		    std::vector <Mapper> vecs;
		    if (mult.getMult () == Token::STAR) vecs = validateMacroRepeat (mult, content, current);
		    else vecs = validateMacroOneOrMore (mult, content, current);
					    
		    Ymir::OutBuffer buf;
		    for (auto & it : vecs)
			buf.write (it.consumed);
					    
		    return Mapper (true, buf.str ());
		} else return validateMacroMult (mult, content, current);
	    }

	    Ymir::Error::halt ("", "");
	    return {Mapper (false)};
	}


	MacroVisitor::Mapper MacroVisitor::validateMacroList (const syntax::MacroMult & mult, const std::string & content, ulong & current) {
	    Mapper mapper (true);
	    
	    for (auto & it : mult.getContent ()) {
		Mapper local_mapper (false);
		ulong current_2 = current;

		match (it) {
		    of (MacroMult, mult, local_mapper = validateMacroMult (mult, content, current_2)			
		    ) else of (MacroOr, or_, local_mapper = validateMacroOr (or_, content, current_2)
		    ) else of (MacroVar, var, local_mapper = validateMacroVar (var, content, current_2)
		    ) else of (MacroToken, tok, local_mapper = validateMacroToken (tok, content, current_2)		
		    ) else local_mapper = validateRule (it, content, current_2);		    
		}
		
		if (!local_mapper.succeed) return Mapper (false);
		mapper = mergeMapper (mapper, local_mapper);
		current = current_2;
	    }
	    	    
	    return mapper;
	}
	
	std::vector <MacroVisitor::Mapper> MacroVisitor::validateMacroRepeat (const syntax::MacroMult & mult, const std::string & content, ulong & current) {
	    std::vector <Mapper> values;
	    
	    while (true) {
		auto current_2 = current;
		try {
		    auto local_mapper = validateMacroList (mult, content, current_2);
		    if (local_mapper.succeed) {
			values.push_back (local_mapper);
			current = current_2;
		    } else break;
		} catch (Error::ErrorList list) {		   
		    break; // If a throw has been done, there was a failure however we are in a repeat so that's not a problem
		}
	    }
	    	    	    
	    return values;
	}
	
	std::vector<MacroVisitor::Mapper> MacroVisitor::validateMacroOneOrMore (const syntax::MacroMult & mult, const std::string & content, ulong & current) {
	    std::vector <Mapper> values;
	    
	    while (true) {
		auto current_2 = current;
		try {
		    auto local_mapper = validateMacroList (mult, content, current_2);
		    if (local_mapper.succeed) {
			values.push_back (local_mapper);
			current = current_2;
		    } else break;
		} catch (Error::ErrorList list) {
		    if (values.size () == 0) throw list; // Need at least one valid value
		    break; // If a throw has been done, there was a failure however we are in a repeat so that's not a problem
		}
	    }
	    	    	    
	    return values;
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroOneOrNone (const syntax::MacroMult & mult, const std::string & content, ulong & current) {
	    try {
		auto current_2 = current;
		auto local_mapper = validateMacroList (mult, content, current_2);
		if (local_mapper.succeed) {
		    current = current_2;
		    return local_mapper;
		} else return Mapper (true);
	    } catch (Error::ErrorList list) {
		return Mapper (true);
	    }           
	}		
	
	MacroVisitor::Mapper MacroVisitor::validateMacroOr (const syntax::MacroOr & mult, const std::string & content, ulong & current) {
	    ulong current_left = current;
	    Mapper mapper (false);
	    std::list <std::string> errors;
	    	    
	    match (mult.getLeft ()) {
		try {
		    of (MacroMult, mult, mapper = validateMacroMult (mult, content, current_left)
		    ) else of (MacroOr, or_, mapper = validateMacroOr (or_, content, current_left)
		    ) else of (MacroVar, var, mapper = validateMacroVar (var, content, current_left)
		    ) else of (MacroToken, tok, mapper = validateMacroToken (tok, content, current_left)
		    ) else mapper = validateRule (mult.getLeft (), content, current_left);
		} catch (Error::ErrorList list) {
		    errors = list.errors;
		}
	    }
	    
	    if (mapper.succeed) {
		current = current_left;
		return mapper;
	    } else {
		try {
		    match (mult.getRight ()) {
			of (MacroMult, mult, mapper = validateMacroMult (mult, content, current)
			) else of (MacroOr, or_, mapper = validateMacroOr (or_, content, current)
			) else of (MacroVar, var, mapper = validateMacroVar (var, content, current)
			) else of (MacroToken, tok, mapper = validateMacroToken (tok, content, current)
			) else mapper = validateRule (mult.getRight (), content, current);
		    
		    }
		} catch (Error::ErrorList list) {
		    errors = list.errors;		    
		}
		
		if (!mapper.succeed)
		    throw Error::ErrorList {errors};
		return mapper;		
	    }
	    
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroVar (const syntax::MacroVar & var, const std::string & content, ulong & current) {
	    auto rule = var.getContent ();
	    Mapper mapper (false);
	    
	    match (rule) {
		of (MacroMult, mult, {
			if (mult.getMult () == Token::STAR || mult.getMult () == Token::PLUS) {
			    std::vector <Mapper> vecs;
			    if (mult.getMult () == Token::STAR) vecs = validateMacroRepeat (mult, content, current);
			    else vecs = validateMacroOneOrMore (mult, content, current);
			    
			    Ymir::OutBuffer buf;
			    for (auto & it : vecs)
				buf.write (it.consumed);
			    
			    Mapper result (true, buf.str ());
			    result.mapping.emplace (var.getLocation ().str, vecs);
			    return result;
			} else {
			    auto mapper = validateMacroMult (mult, content, current);
			    Mapper result (true, mapper.consumed);
			    result.mapping.emplace (var.getLocation ().str, std::vector <MacroVisitor::Mapper> {mapper});
			    return result;
			}
		    }		    		    		
		) else of (MacroOr, or_, mapper = validateMacroOr (or_, content, current)
		) else of (MacroVar, var, mapper = validateMacroVar (var, content, current)
		) else of (MacroToken, tok, mapper = validateMacroToken (tok, content, current)
		) else mapper = validateRule (rule, content, current);		
	    }
	    
	    if (mapper.succeed) {
		std::vector <Mapper> vec = {mapper};
		Mapper result (true, mapper.consumed);
		result.mapping.emplace (var.getLocation ().str, vec);
		return result;
	    }
	    
	    // In principle, we can't get here as an error must have been thrown in case of failure
	    return Mapper (false);
	}

	MacroVisitor::Mapper MacroVisitor::validateRule (const syntax::Expression & expr, const std::string & content, ulong & current) {
	    Mapper mapper (false);
	    if (expr.is <syntax::Var> ()) {
		auto left_current = current;
		auto mapper = validateRuleVar (expr.to <syntax::Var> (), content, left_current);
		if (mapper.succeed) {
		    current = left_current;
		    return mapper;
		}
	    }
	    
	    std::list <std::string> errors;
	    Visitor::__CALL_NB_RECURS__ += 1;
	    try {
		auto rules = this-> _context.validateValue (expr);
		match (rules) {
		    of (MacroRuleRef, ruleRef ATTRIBUTE_UNUSED, {
			    auto sem_rule = ruleRef.getMacroRuleRef ().to <semantic::MacroRule> ().getContent ();
			    auto innerRule = sem_rule.to <syntax::MacroRule> ().getRule ();

			    match (innerRule) {
				of (MacroMult, mult, mapper = validateMacroMult (mult, content, current);				    
				) else of (MacroOr, or_,
					   mapper = validateMacroOr (or_, content, current)
				) else of (MacroVar, var,
					   mapper = validateMacroVar (var, content, current)
				) else of (MacroToken, tok,
					   mapper = validateMacroToken (tok, content, current)
				) else Ymir::Error::occur (
				    innerRule.getLocation (),
				    ExternalError::get (INVALID_MACRO_RULE),
				    innerRule.prettyString ()
				);			    
			    }
			}		
		    ) else {
			Ymir::Error::occur (
			    expr.getLocation (),
			    ExternalError::get (INVALID_MACRO_RULE),
			    rules.prettyString ()
			);
		    }
		}
	    } catch (Error::ErrorList list) {		
		if (Visitor::__CALL_NB_RECURS__ == 3 && !global::State::instance ().isVerboseActive ()) {
		    list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine (ExternalError::get (OTHER_CALL)));	    
		    list.errors.insert (list.errors.begin (), format ("     : %(B)", "..."));
		} else if (Visitor::__CALL_NB_RECURS__ <  3 || global::State::instance ().isVerboseActive ()) {   
		    list.errors.insert (list.errors.begin (), Ymir::Error::createNote (expr.getLocation (), ExternalError::get (IN_MACRO_EXPANSION)));
		    Visitor::__LAST__ = true;
		} else if (Visitor::__LAST__) {			    
		    Visitor::__LAST__ = false;
		}
		
		errors = list.errors;
	    }

	    Visitor::__CALL_NB_RECURS__ -= 1;
	    if (errors.size () != 0) throw Error::ErrorList {errors};
	   
	    return mapper;
	}

	MacroVisitor::Mapper MacroVisitor::validateRuleVar (const syntax::Var & var, const std::string & content, ulong & current) {
	    auto name = var.getName ().str;
	    if (name == MacroVisitor::__EXPR__) {
		ulong line = 0, col = 0, seek = 0;
		computeLine (line, col, seek, current);
		    
		auto lex = lexing::Lexer::initFromString (content.substr (current), this-> _call.locFile,
							  {Token::SPACE, Token::TAB, Token::RETURN, Token::RRETURN},
							  {
							      {Token::LCOMM1, {Token::RCOMM1, ""}},
								  {Token::LCOMM2, {Token::RETURN, ""}},
								      {Token::LCOMM3, {Token::RCOMM3, ""}},
									  {Token::LCOMM4, {Token::RCOMM3, Token::STAR}},
									      {Token::LCOMM5, {Token::RCOMM5, Token::PLUS}}
							  }, line);
		
		auto visit = syntax::Visitor::init (lex);
		auto result = visit.visitExpression ();
		lex = visit.getLexer ();
		auto rest = lex.formatRestOfFile ();
		auto expr = content.substr (current, content.length () - rest.length () - current);
		current = current + expr.length ();

		return Mapper (true, expr, {});
	    }

	    return Mapper (false);	    
	}
	
	
	MacroVisitor::Mapper MacroVisitor::validateMacroToken (const syntax::MacroToken & mult, const std::string & content, ulong & current) {
	    auto sem_str = this-> _context.validateString (mult.getContent ().to <syntax::String> (), true);
	    auto ch_str = sem_str.to <Aliaser> ().getWho ().to <StringValue> ().getValue ();
	    Ymir::OutBuffer buf;
	    
	    ulong i = 0;	    
	    for (auto & it : ch_str) {
		i += 1;
		if (it != '\0' && i < ch_str.size ())
		    buf.write (it);
	    }
	    
	    auto str = buf.str ();
	    auto len = str.length ();

	    if (len > (content.length () - current)) {
		std::string wstr;
		if (current < content.length ()) wstr = content.substr (current);
		    
		lexing::Word word (UNKNOWN_LOCATION, wstr);
		ulong line = 0, col = 0, seek = 0;
		computeLine (line, col, seek, current);
		
		word.setLocus (this-> _call.getFile (), line, col, seek);
		auto note = Ymir::Error::createNote (word); 
		Ymir::Error::occurAndNote (
		    mult.getLocation (),
		    note, 
		    ExternalError::get (INCOMPATIBLE_TOKENS),
		    str,
		    wstr
		);
	    }

	    for (auto it : Ymir::r(0, len)) {
		if (str [it] != content [it+current]) {
		    lexing::Word word (UNKNOWN_LOCATION, content.substr (current, len));
		    ulong line = 0, col = 0, seek = 0;
		    computeLine (line, col, seek, current);
		    
		    word.setLocus (this-> _call.getFile (), line, col, seek);
		    auto note = Ymir::Error::createNote (word); 
		    Ymir::Error::occurAndNote (
			mult.getLocation (),
			note,
			ExternalError::get (INCOMPATIBLE_TOKENS),
			str,
			content.substr (current, len)
		    );  
		}
	    }
	    
	    auto ret = content.substr (current, len);
	    current += len;
	    
	    return Mapper (true, ret, {});
	}

	void MacroVisitor::computeLine (ulong & line, ulong & column, ulong & seek, ulong current) {
	    line = this-> _call.line;
	    column = this-> _call.column + this-> _call.length ();
	    seek = this-> _call.seek;
	    for (auto it : Ymir::r (0, current)) {
		if (this-> _content [it] == '\n') {
		    line ++;
		    column = 0;
		} else column += 1;
		seek += 1;
	    }
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
	    lexing::Lexer lex = lexing::Lexer::initFromString (content, loc.locFile, {}, {}, loc.line);
	    try {
		while (true) {
		    auto next = lex.next ();
		    if (next == Token::MACRO_ACC || next == Token::MACRO_CRO || next == Token::MACRO_PAR) {
			auto vecs = validateMacroEval (content, loc, toMacroEval (content, loc, lex, next), mapping);
			for (auto & it : vecs)
			    buf.write (it.consumed);
		    } else if (next == Token::MACRO_FOR) {
			buf.write (validateMacroFor (content, loc, lex, mapping));
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
		
		if (begin == Token::MACRO_FOR) {}
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

	std::string MacroVisitor::validateMacroFor (const std::string & content, const lexing::Word & loc, lexing::Lexer & lex, const Mapper & mapping) {
	    lex.skipEnable (Token::SPACE,   true);
	    lex.skipEnable (Token::TAB,     true);
	    lex.skipEnable (Token::RETURN,  true);
	    lex.skipEnable (Token::RRETURN, true);

	    auto visit = syntax::Visitor::init (lex);
	    auto var = visit.visitIdentifier ();
	    lex = visit.getLexer ();
	    lex.next ({Keys::IN});
	    
	    auto inner = toMacroEval (content, loc, lex, loc);
	    lex.skipEnable (Token::SPACE,   true);
	    lex.skipEnable (Token::TAB,     true);
	    lex.skipEnable (Token::RETURN,  true);
	    lex.skipEnable (Token::RRETURN, true);
	    auto begin = lex.next ({Token::LACC});
	    begin.line += loc.line;
	    
	    lex.skipEnable (Token::SPACE,   false);
	    lex.skipEnable (Token::TAB,     false);
	    lex.skipEnable (Token::RETURN,  false);
	    lex.skipEnable (Token::RRETURN, false);

	    OutBuffer buf_;
	    auto nb = 1;
	    while (nb != 0) {
		auto next = lex.next ();
		if (next == Token::LACC || next == Token::MACRO_ACC) nb++;
		else if (next == Token::RACC) nb --;
		if (nb != 0) buf_.write (next.str);
	    }
	    
	    auto vecs = validateMacroEval (content, loc, inner, mapping);
	    auto text = buf_.str();

	    Ymir::OutBuffer total;
	    for (auto & it : vecs) {
		auto mapper = mapping;
		mapper.mapping.emplace (var.str, std::vector <MacroVisitor::Mapper> {it});
		Ymir::OutBuffer buf;
		std::list <std::string> errors;	    
		try {
		    lexing::Lexer lex = lexing::Lexer::initFromString (text, loc.locFile, {}, {}, loc.line);
		    while (true) {
			auto next = lex.next ();
			if (next == Token::MACRO_ACC || next == Token::MACRO_CRO || next == Token::MACRO_PAR) {
			    auto vecs = validateMacroEval (content, loc, toMacroEval (content, loc, lex, next), mapper);
			    for (auto & it : vecs)
				buf.write (it.consumed);
			} else if (next == Token::MACRO_FOR) {
			    buf.write (validateMacroFor (content, loc, lex, mapper));
			} else if (!next.isEof ()) {
			    buf.write (next.str);
			} else break;		
		    }
		} catch (Error::ErrorList list) {
		    errors = list.errors;
		}
	    	    
		if (errors.size () != 0) {
		    throw Error::ErrorList {errors};
		} else total.write (buf.str ());
	    }
	    return total.str ();
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
