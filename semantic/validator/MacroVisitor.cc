#include <ymir/semantic/validator/MacroVisitor.hh>
#include <ymir/semantic/validator/BinaryVisitor.hh>
#include <ymir/semantic/validator/SubVisitor.hh>
#include <ymir/semantic/validator/CallVisitor.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/global/State.hh>
#include <ymir/syntax/declaration/MacroRule.hh>
#include <ymir/global/Core.hh>
#include <algorithm>

using namespace global;

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	using namespace syntax;
	

	const std::string MacroVisitor::__EXPR__ = "__expr";
	const std::string MacroVisitor::__IDENT__ = "__ident";
	const std::string MacroVisitor::__TOKEN__ = "__token";
	const std::string MacroVisitor::__ANY__ = "__any";
	const std::string MacroVisitor::__WORD__ = "__word";
	const std::string MacroVisitor::__INT__ = "__int";
	const std::string MacroVisitor::__FLOAT__ = "__float";
	const std::string MacroVisitor::__STRING__ = "__str";
	const std::string MacroVisitor::__CHAR__ = "__char";
	const std::string MacroVisitor::__RULE_INDEX__ = "__index";

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
	    _context (context),
	    _content (""),
	    _call (lexing::Word::eof ()),
	    _known_rules (MacroVisitor::getKnwonRules ())
	{}

	MacroVisitor MacroVisitor::init (Visitor & context) {
	    return MacroVisitor {context};
	}

	generator::Generator MacroVisitor::validate (const syntax::MacroCall & expression) {
	    auto left = this-> _context.validateValue (expression.getLeft ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    Generator ret (Generator::empty ());

	    try {
		match (left) {
		    of (MultSym, sym) {
			ret = validateMultSym (sym, expression, errors);
		    } elof (MacroRef, mref) {
			ret = validateMacroRef (mref, expression, errors);
		    } fo;
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

	generator::Generator MacroVisitor::validateMultSym (const MultSym & sym, const syntax::MacroCall & expression, std::list <Ymir::Error::ErrorMsg> & errors) {
	    std::vector <Generator> success;
	    for (auto & it : sym.getGenerators ()) {
		try {
		    Generator ret (Generator::empty ());
		    match (it) {			
			of (MultSym, sym) {
			    ret = validateMultSym (sym, expression, errors);
			} elof (MacroRef, mref) {
			    ret = validateMacroRef (mref, expression, errors);
			} fo;
			
			if (!ret.isEmpty ()) {
			    success.push_back (ret);
			} else {
			    this-> error (expression.getLocation (), expression.getEnd (), it, {});
			}
		    }
		} catch (Error::ErrorList & list) {
		    auto note = Ymir::Error::createNoteOneLine (ExternalError::CANDIDATE_ARE, it.getLocation (), it.prettyString ());
		    for (auto & n : list.errors) {
			note.addNote (n);
		    }
		    errors.push_back (note);
		}
	    }

	    if (success.size () == 0) {
		return Generator::empty ();
	    } else if (success.size () != 1) {		
		std::list <Ymir::Error::ErrorMsg> notes;
		for (auto &gen : success) {
		    notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::CANDIDATE_ARE, CallVisitor::realLocation (gen), CallVisitor::prettyName (gen)));		    
		}
		
		Ymir::Error::occurAndNote (expression.getLocation (), expression.getEnd (),
					   notes,
					   ExternalError::SPECIALISATION_WORK_WITH_BOTH_PURE);
		
		return Generator::empty ();
	    } else {
		return success [0];
	    }
	}
	
	generator::Generator MacroVisitor::validateMacroRef (const MacroRef & sym, const syntax::MacroCall & expression, std::list <Ymir::Error::ErrorMsg> & errors) {
	    auto constructors = this-> _context.getMacroConstructor (sym.getLocation (), sym);
	    std::vector <Generator> values;
	    for (auto & it : constructors) {
		this-> _context.enterClassDef (sym.getMacroRef ());
		try {
		    values.push_back (validateConstructor (it, expression));		    
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}

		this-> _context.exitClassDef (sym.getMacroRef ());		
	    }

	    if (values.size () > 1) {
		return MultSym::init (sym.getLocation (), values);
	    } else if (values.size () == 0) 		
		return Generator::empty ();
	    else return values [0];
	}

	generator::Generator MacroVisitor::validateConstructor (const semantic::Symbol & constr, const syntax::MacroCall & expression) {
	    auto rule = constr.to <semantic::MacroConstructor> ().getContent ().to <syntax::MacroConstructor> ().getRule ();
	    auto skips = this-> validateSkips (constr.to <semantic::MacroConstructor> ().getContent ().to <syntax::MacroConstructor> ().getSkips ());
	    std::string content = expression.getContent ();

	    this-> _call = expression.getLocation ();
	    this-> _content = content;
	    
	    ulong current = 0;
	    Mapper mapper (false);
	    std::list <Ymir::Error::ErrorMsg> errors;
	    
	    this-> _context.pushReferent (constr, "validateMacroExpression");
	    this-> _context.enterForeign ();
	    
	    try {
		match (rule) {
		    of (MacroMult, mult) {
			mapper = validateMacroMult (mult, content, current, skips);
		    } elfo {
			Ymir::Error::halt ("%(r) reaching impossible point", "Critical");
		    }
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
		    computeLine (line, col, seek, current, this-> _call);
		    lexing::Word word = lexing::Word::init (content.substr (current, 1), this-> _call.getFile (), line, col, seek);
		    
		    auto note = Ymir::Error::createNote (word);
		    Ymir::Error::occurAndNote (
			rule.getLocation (),
			note,
			ExternalError::MACRO_REST,
			some (content, current)
		    );
			
		}

		auto ret = validateMapper (constr.to <semantic::MacroConstructor> ().getContent ().to <syntax::MacroConstructor> ().getContentLoc (), constr.to <semantic::MacroConstructor> ().getContent ().to <syntax::MacroConstructor> ().getContent (), mapper);

		return this-> _context.validateMacroExpression (
		    constr,
		    ret
		);
	    }
	    
	    return Generator::empty ();
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroMult (const syntax::MacroMult & mult, const std::string & content, ulong & current, const Expression & skips) {
	    if (mult.getMult ().isEof () || mult.getMult () == "") {
		return validateMacroList (mult, content, current, skips);
	    } else if (mult.getMult () == Token::INTEG) {
		return validateMacroOneOrNone (mult, content, current, skips);
	    } else {
		if (mult.getMult () == Token::STAR || mult.getMult () == Token::PLUS) {
		    std::vector <Mapper> vecs;
		    if (mult.getMult () == Token::STAR) vecs = validateMacroRepeat (mult, content, current, skips);
		    else vecs = validateMacroOneOrMore (mult, content, current, skips);
					    
		    Ymir::OutBuffer buf;
		    for (auto & it : vecs)
		    	buf.write (it.consumed);
					    
		    return Mapper (true, buf.str ());
		} else return validateMacroMult (mult, content, current, skips);
	    }

	    Ymir::Error::halt ("", "");
	    return {Mapper (false)};
	}


	MacroVisitor::Mapper MacroVisitor::validateMacroList (const syntax::MacroMult & mult, const std::string & content, ulong & current, const Expression & skips) {
	    Mapper mapper (true);
	    
	    for (auto & it : mult.getContent ()) {
		if (!skips.isEmpty ()) 
		    validateMacroRepeat (skips.to <MacroMult> (), content, current, Expression::empty ());
		
		Mapper local_mapper (false);
		ulong current_2 = current;
		
		match (it) {
		    of (MacroMult, mult) local_mapper = validateMacroMult (mult, content, current_2, skips);		
		    elof (MacroOr, or_) local_mapper = validateMacroOr (or_, content, current_2, skips);
		    elof (MacroVar, var) local_mapper = validateMacroVar (var, content, current_2, skips);
		    elof (MacroToken, tok) local_mapper = validateMacroToken (tok, content, current_2, skips);
		    elfo {
			local_mapper = validateRule (it, content, current_2, skips);
		    }
		}
		
		if (!local_mapper.succeed) 
		    return Mapper (false);
				
		mapper = mergeMapper (mapper, local_mapper);
		current = current_2;		
	    }
	    	    
	    return mapper;
	}
	
	std::vector <MacroVisitor::Mapper> MacroVisitor::validateMacroRepeat (const syntax::MacroMult & mult, const std::string & content, ulong & current, const Expression & skips) {
	    std::vector <Mapper> values;
	    
	    while (true) {
		auto current_2 = current;
		try {
		    auto local_mapper = validateMacroList (mult, content, current_2, skips);
		    if (local_mapper.succeed) {
			values.push_back (local_mapper);
			current = current_2;
		    } else break;
		} catch (Error::ErrorList list) {		   
		    break; // If there is a throw, there was a failure however we are in a repeat so that's not a problem
		}
	    }
	    	    	    
	    return values;
	}
	
	std::vector<MacroVisitor::Mapper> MacroVisitor::validateMacroOneOrMore (const syntax::MacroMult & mult, const std::string & content, ulong & current, const Expression & skips) {
	    std::vector <Mapper> values;
	    
	    while (true) {
		auto current_2 = current;
		try {
		    auto local_mapper = validateMacroList (mult, content, current_2, skips);
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
	
	MacroVisitor::Mapper MacroVisitor::validateMacroOneOrNone (const syntax::MacroMult & mult, const std::string & content, ulong & current, const Expression & skips) {
	    try {
		auto current_2 = current;
		auto local_mapper = validateMacroList (mult, content, current_2, skips);
		if (local_mapper.succeed) {
		    current = current_2;
		    return local_mapper;
		} else return Mapper (true);
	    } catch (Error::ErrorList list) {
		return Mapper (true);
	    }           
	}		
	
	MacroVisitor::Mapper MacroVisitor::validateMacroOr (const syntax::MacroOr & mult, const std::string & content, ulong & current, const Expression & skips) {	    
	    ulong current_left = current;
	    Mapper mapper (false);
	    std::list <Ymir::Error::ErrorMsg> errors;
	    try {
		match (mult.getLeft ()) {
		    of (MacroMult, mult) mapper = validateMacroMult (mult, content, current_left, skips);
		    elof (MacroOr, or_) mapper = validateMacroOr (or_, content, current_left, skips);
		    elof (MacroVar, var) mapper = validateMacroVar (var, content, current_left, skips);
		    elof (MacroToken, tok) mapper = validateMacroToken (tok, content, current_left, skips);
		    elfo {
			mapper = validateRule (mult.getLeft (), content, current_left, skips);
		    }
		}
	    } catch (Error::ErrorList list) {
		errors = list.errors;
	    }

	    if (mapper.succeed) {
		current = current_left;
		return mapper;
	    } else {		
		try {
		    match (mult.getRight ()) {
			of (MacroMult, mult) mapper = validateMacroMult (mult, content, current, skips);
			elof (MacroOr, or_) mapper = validateMacroOr (or_, content, current, skips);
			elof (MacroVar, var) mapper = validateMacroVar (var, content, current, skips);
			elof (MacroToken, tok) mapper = validateMacroToken (tok, content, current, skips);
			elfo {
			    mapper = validateRule (mult.getRight (), content, current, skips);
			}
		    }
		} catch (Error::ErrorList list) {
		    errors = list.errors;		    
		}
		
		if (!mapper.succeed)
		    throw Error::ErrorList {errors};
		return mapper;		
	    }
	    
	}
	
	MacroVisitor::Mapper MacroVisitor::validateMacroVar (const syntax::MacroVar & var, const std::string & content, ulong & current, const Expression & skips) {
	    auto rule = var.getContent ();
	    Mapper mapper (false);
	    
	    match (rule) {
		of (MacroMult, mult) {
		    if (mult.getMult () == Token::STAR || mult.getMult () == Token::PLUS) {
			std::vector <Mapper> vecs;
			if (mult.getMult () == Token::STAR) vecs = validateMacroRepeat (mult, content, current, skips);
			else vecs = validateMacroOneOrMore (mult, content, current, skips);
			    
			Ymir::OutBuffer buf;
			for (auto & it : vecs)
			buf.write (it.consumed);
			    
			Mapper result (true, buf.str ());
			result.mapping.emplace (var.getLocation ().getStr (), vecs);
			    
			return result;
		    } else {
			auto mapper = validateMacroMult (mult, content, current, skips);
			Mapper result (true, mapper.consumed);
			result.mapping.emplace (var.getLocation ().getStr (), std::vector <MacroVisitor::Mapper> {mapper});
			return result;
		    }
		}		    		    		
		elof (MacroOr, or_) mapper = validateMacroOr (or_, content, current, skips);
		elof (MacroVar, var) mapper = validateMacroVar (var, content, current, skips);
		elof (MacroToken, tok) mapper = validateMacroToken (tok, content, current, skips);
		elfo {
		    mapper = validateRule (rule, content, current, skips);
		}
	    }
	    	    
	    if (mapper.succeed) {
		std::vector <Mapper> vec = {mapper};
		Mapper result (true, mapper.consumed);
		result.mapping.emplace (var.getLocation ().getStr (), vec);
		return result;
	    }
	    
	    // In principle, we can't get here as an error must have been thrown in case of failure
	    return Mapper (false);
	}

	MacroVisitor::Mapper MacroVisitor::validateRule (const syntax::Expression & expr, const std::string & content, ulong & current, const Expression &) {
	    Mapper mapper (false);
	    if (expr.is <syntax::Var> ()) {	       
		if (std::find (this-> _known_rules.begin (), this-> _known_rules.end (), expr.to <syntax::Var> ().getName ().getStr ()) != this-> _known_rules.end ()) {
		    return validateKnownRules (expr, content, current);
		}
	    }
	    
	    std::list <Ymir::Error::ErrorMsg> errors;
	    Visitor::__CALL_NB_RECURS__ += 1;

	    this-> _ruleIndex += 1;
	    try {
		generator::Generator rules (Generator::empty ());
		if (expr.is <syntax::Var> ()) {
		    auto sym = this-> _context.getCurrentMacroRules (expr.getLocation (), expr.to <syntax::Var> ().getName ().getStr ());
		    if (!sym.isEmpty ())
			rules = MacroRuleRef::init (sym.getName (), sym);
		}
		
		if (rules.isEmpty ()) {
		    rules = this-> _context.validateValue (expr);
		}
		
	       		
		match (rules) {
		    of (MacroRuleRef, ruleRef) {
			this-> _context.enterClassDef (ruleRef.getMacroRuleRef ().getReferent ());			    
			    
			try {
			    auto sem_rule = ruleRef.getMacroRuleRef ().to <semantic::MacroRule> ().getContent ();
			    auto innerRule = sem_rule.to <syntax::MacroRule> ().getRule ();
			    auto skips = this-> validateSkips (sem_rule.to <syntax::MacroRule> ().getSkips ());
			    
			    match (innerRule) {
				of (MacroMult, mult)
				    mapper = validateMacroMult (mult, content, current, skips);
				elfo {
				    Ymir::Error::halt ("%(r) reaching impossible point", "Critical");
				}
			    }
			       
			    if (mapper.succeed) {
				auto ret = validateMapperString (ruleRef.getMacroRuleRef ().to <semantic::MacroRule> ().getContent ().to <syntax::MacroRule> ().getContentLoc (),
								 sem_rule.to <syntax::MacroRule> ().getContent (),
								 mapper);



				Mapper result (true, ret);
				result.mapping.emplace ("rule", std::vector <Mapper> ({mapper}));				
				mapper = result;
			    }
			} catch (Error::ErrorList err) {
			    errors = err.errors;
			}

			this-> _context.exitClassDef (ruleRef.getMacroRuleRef ().getReferent ());
			if (errors.size () != 0) throw Error::ErrorList {errors};
		    }
		    elfo {
			errors.push_back (Ymir::Error::makeOccur (
					      expr.getLocation (),
					      ExternalError::INVALID_MACRO_RULE,
					      rules.prettyString ()
					      ));
		    }
		}
	    } catch (Error::ErrorList list) {
		list.errors.insert (list.errors.begin (), Ymir::Error::createNote (expr.getLocation (), ExternalError::IN_MACRO_EXPANSION));		    		
		errors = std::move (list.errors);
	    }

	    this-> _ruleIndex -= 1;
	    Visitor::__CALL_NB_RECURS__ -= 1;
	    if (errors.size () != 0) throw Error::ErrorList {errors};
	   
	    return mapper;
	}

	MacroVisitor::Mapper MacroVisitor::validateKnownRules (const syntax::Expression & expr, const std::string & content, ulong & current) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto name = expr.to <syntax::Var> ().getName ().getStr ();
	    if (name == MacroVisitor::__ANY__ || name == MacroVisitor::__CHAR__) {
		if (current < content.length () && name == MacroVisitor::__ANY__) {
		    current += 1;
		    return Mapper (true, content.substr (current-1, 1));
		} else if (current < content.length () && name == MacroVisitor::__CHAR__) {
		    current += 1;
		    if (content [current - 1] == '\"')
			return Mapper (true, "\\\"");
		    else if (content [current - 1] == '\'')
			return Mapper (true, "\\\'");
		    return Mapper (true, content.substr (current-1, 1));
		} else {
		    ulong line = 0, col = 0, seek = 0;
		    computeLine (line, col, seek, current, this-> _call);
		    
		    lexing::Word word = lexing::Word::init ("", this-> _call.getFile (), line, col, seek);
		    
		    auto note = Ymir::Error::createNote (word); 
		    Ymir::Error::occurAndNote (
			expr.getLocation (),
			note, 
			ExternalError::INCOMPATIBLE_TOKENS,
			MacroVisitor::__ANY__,
			""
			);
		}
	    }
	    
	    auto file = lexing::StringFile::init (content.substr (current));
	    auto lex = lexing::Lexer::initFromString (file, this-> _call.getFilename (),
						      {Token::SPACE, Token::TAB, Token::RETURN, Token::RRETURN},
						      {
							  {Token::LCOMM1, {Token::RCOMM1, ""}},
							  {Token::LCOMM2, {Token::RETURN, ""}},
							  {Token::LCOMM3, {Token::RCOMM3, ""}},
							  {Token::LCOMM4, {Token::RCOMM3, Token::STAR}},
							  {Token::LCOMM5, {Token::RCOMM5, Token::PLUS}}
						      }, this-> _call.getLine () - 1);
	    
	    try {	    
		
		if (name == MacroVisitor::__IDENT__) {
		    auto visit = syntax::Visitor::init (lex);
		    auto ignore = visit.visitIdentifier ();
		    lex = visit.getLexer ();
		} else if (name == MacroVisitor::__EXPR__) {
		    auto visit = syntax::Visitor::init (lex);
		    auto ignore = visit.visitExpression ();
		    lex = visit.getLexer ();
		} else if (name == MacroVisitor::__TOKEN__) {
		    auto ignore = lex.next (Token::members ());
		} else if (name == MacroVisitor::__STRING__) {
		    auto visit = syntax::Visitor::init (lex);
		    auto ignore = visit.visitString ();
		    lex = visit.getLexer ();
		} else if (name == MacroVisitor::__WORD__) {
		    auto m = Token::members ();
		    auto ignore = lex.next ();
		    if (std::find(m.begin (), m.end (), ignore.getStr ()) != m.end ())
			Ymir::Error::occur (ignore, ExternalError::SYNTAX_ERROR_AT_SIMPLE, ignore.getStr ());
		} else {
		    Ymir::Error::halt ("%(r) Unknwon known rule %(y)", "Critical", name);
		}

		auto x = lex.next ();
		int end = content.length () - current;
		if (!x.isEof ()) end = x.getSelfSeek ();
		
		auto result = content.substr (current, end);
		current += end;
	    
		return Mapper (true, result);
	    } catch (Ymir::Error::ErrorList list) {		
		list.errors.insert (list.errors.begin (), Ymir::Error::createNote (expr.getLocation (), ExternalError::IN_MACRO_EXPANSION));		
		errors = std::move (list.errors);
	    }
	    
	    if (errors.size () != 0) throw Error::ErrorList {errors};	    
	    return Mapper (false);
	}
	

	Expression MacroVisitor::validateSkips (const std::vector <Expression> & skips) const {
	    if (skips.size () == 0) return Expression::empty ();
	    Expression ret (Expression::empty ());
	    int i = 0;
	    for (auto & it : skips) {
		if (i == 0)
		    ret = it;
		else
		    ret = MacroOr::init (it.getLocation (), ret, it);

		// We validate the string that might be never used
		this-> _context.validateString (it.to <MacroToken> ().getContent ().to <syntax::String> (), true);
		
		i += 1;
	    }
	    
	    return MacroMult::init (ret.getLocation (), ret.getLocation (), {ret}, lexing::Word::init (ret.getLocation (), Token::STAR));
	}
		
	MacroVisitor::Mapper MacroVisitor::validateMacroToken (const syntax::MacroToken & mult, const std::string & content, ulong & current, const Expression &) {
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
		ulong line = 0, col = 0, seek = 0;
		computeLine (line, col, seek, current, this-> _call);
		lexing::Word word = lexing::Word::init (wstr, this-> _call.getFile (), line, col, seek);
		
		auto note = Ymir::Error::createNote (word); 
		Ymir::Error::occurAndNote (
		    mult.getLocation (),
		    note, 
		    ExternalError::INCOMPATIBLE_TOKENS,
		    str,
		    wstr
		);
	    }

	    for (auto it : Ymir::r(0, len)) {
		if (str [it] != content [it+current]) {
		    ulong line = 0, col = 0, seek = 0;
		    computeLine (line, col, seek, current, this-> _call);
		    
		    lexing::Word word = lexing::Word::init (content.substr (current, len), this-> _call.getFile (), line, col, seek);
		    
		    auto note = Ymir::Error::createNote (word); 
		    Ymir::Error::occurAndNote (
			mult.getLocation (),
			note,
			ExternalError::INCOMPATIBLE_TOKENS,
			str,
			content.substr (current, len)
		    );  
		}
	    }
	    
	    auto ret = content.substr (current, len);
	    current += len;
	    	    
	    return Mapper (true, ret, {});
	}

	void MacroVisitor::computeLinePure (ulong & line, ulong & column, ulong & seek, const lexing::Word & start, const lexing::Word & relative) {
	    line = start.getLine () + relative.getLine ();
	    if (relative.getLine () == 0)
		column = start.getColumn () + relative.getColumn ();
	    else column = relative.getColumn ();
	    seek = start.getSeek () + relative.getSeek ();
	}
	
	void MacroVisitor::computeLine (ulong & line, ulong & column, ulong & seek, ulong current, const lexing::Word & relative) {
	    line = relative.getLine ();
	    column = relative.getColumn () + relative.length ();
	    seek = relative.getSeek ();
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

	std::string MacroVisitor::validateMapperString (const lexing::Word & loc, const std::string & content, const MacroVisitor::Mapper & mapping) {	    
	    Ymir::OutBuffer buf;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    auto file = lexing::StringFile::init (content);
	    
	    lexing::Lexer lex = lexing::Lexer::initFromString (lexing::StringFile::init (content), loc.getFilename (), {}, {}, loc.getLine () - 1);
	    
	    while (true) {
		auto next = lex.next ();
		if (next == Token::MACRO_ACC || next == Token::MACRO_CRO || next == Token::MACRO_PAR) {
		    auto loop = lex.consumeIf ({Keys::FOR});
		    if (loop == Keys::FOR) {			
			buf.write (validateMacroFor (content, next, loc, lex, mapping));
		    } else {
			auto vecs = validateMacroEval (content, loc, toMacroEval (content, loc, lex, next), mapping);
			for (auto & it : vecs) {			
			    buf.write (it.consumed);
			}
		    }
		} else if (!next.isEof ()) {
		    buf.write (next.getStr ());
		} else break;		
	    }

	    return buf.str ();
	}
	
	syntax::Expression MacroVisitor::validateMapper (const lexing::Word & loc, const std::string & content, const MacroVisitor::Mapper & mapping) {
	    std::list <Ymir::Error::ErrorMsg> errors;
	    try {
		auto expr = "{" + validateMapperString (loc, content, mapping) + "}";
		auto lex = lexing::Lexer::initFromString (lexing::StringFile::init (expr), loc.getFilename (),
							  {Token::SPACE, Token::TAB, Token::RETURN, Token::RRETURN},
							  {
							      {Token::LCOMM1, {Token::RCOMM1, ""}},
							      {Token::LCOMM2, {Token::RETURN, ""}},
							      {Token::LCOMM3, {Token::RCOMM3, ""}},
							      {Token::LCOMM4, {Token::RCOMM3, Token::STAR}},
							      {Token::LCOMM5, {Token::RCOMM5, Token::PLUS}}
							  }, loc.getLine () - 1);

		// the return line in string literals must be considered
		auto visit = syntax::Visitor::init (lex, false);		
		auto result = visit.visitExpression ();
		return result;
	    } catch (Error::ErrorList list) {		
		list.errors.insert (list.errors.begin (), Ymir::Error::createNote (loc, ExternalError::IN_MACRO_EXPANSION));
		
		throw Error::ErrorList {list.errors};
	    }
	} 

	syntax::Expression MacroVisitor::toMacroEval (const std::string & content, const lexing::Word & loc, lexing::Lexer & lex, const lexing::Word & begin) {
	    std::list <Ymir::Error::ErrorMsg> errors;
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

	std::string MacroVisitor::validateMacroFor (const std::string & content, const lexing::Word & open, const lexing::Word & loc, lexing::Lexer & lex, const Mapper & mapping) {
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
	    
	    if (open == Token::MACRO_PAR) lex.next ({Token::RPAR});
	    else if (open == Token::MACRO_ACC) lex.next ({Token::RACC});
	    else lex.next ({Token::RCRO});
	    
	    auto begin = lex.next ({Token::LACC});
	    
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
		if (nb != 0) buf_.write (next.getStr ());
	    }
	    
	    auto vecs = validateMacroEval (content, loc, inner, mapping);
	    auto text = buf_.str();

	    Ymir::OutBuffer total;
	    for (auto & it : vecs) {
		auto mapper = mapping;
		mapper.mapping.emplace (var.getStr (), std::vector <MacroVisitor::Mapper> {it});
		Ymir::OutBuffer buf;
		std::list <Ymir::Error::ErrorMsg> errors;	    
		try {
		    lexing::Lexer lex = lexing::Lexer::initFromString (lexing::StringFile::init (text), loc.getFilename (), {}, {}, loc.getLine () - 1);
		    while (true) {
			auto next = lex.next ();
			if (next == Token::MACRO_ACC || next == Token::MACRO_CRO || next == Token::MACRO_PAR) {
			    auto loop = lex.consumeIf ({Keys::FOR});
			    if (loop == Keys::FOR) {
				buf.write (validateMacroFor (content, next, loc, lex, mapper));
			    } else {
				auto vecs = validateMacroEval (content, loc, toMacroEval (content, loc, lex, next), mapper);
				for (auto & it : vecs)
				    buf.write (it.consumed);
			    }
			} else if (!next.isEof ()) {
			    buf.write (next.getStr ());
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
	    std::list <Ymir::Error::ErrorMsg> errors;	    
	    try {
		match (eval) {
		    of (syntax::Var, var) {
			auto inner = mapper.mapping.find (var.getName ().getStr ());
			if (inner != mapper.mapping.end ()) {
			    return inner-> second;
			} else if (var.getName ().getStr () == MacroVisitor::__RULE_INDEX__) {
			    std::vector<Mapper> ret;
			    Ymir::OutBuffer buf;
			    buf.write (this-> _ruleIndex);
			    ret.push_back (Mapper (true, buf.str ()));
			    return ret;
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
				ExternalError::UNDEF_MACRO_EVAL,
				var.getName ().getStr ()
				);
			}
		    }
		    elof (syntax::Binary, bin) {
			if (bin.getLocation () == Token::DCOLON) {
			    auto left = validateMacroEval (content, loc, bin.getLeft (), mapper);
			    auto right = bin.getRight ().to <syntax::Var> ().getName ().getStr ();
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
					ExternalError::UNDEFINED_SUB_PART_FOR,
					right,
					bin.getLeft ().prettyString ()
					);
				}
			    } else if (right == Keys::LEN) {
				std::vector <Mapper> ret;
				Ymir::OutBuffer buf;
				buf.write (left.size (), Keys::USIZE);
				ret.push_back (Mapper (true, buf.str ()));
				return ret;
			    } else {
				Ymir::Error::occur (
				    eval.getLocation (),
				    ExternalError::UNDEFINED_SUB_PART_FOR,
				    right					,
				    bin.getLeft ().prettyString ()
				    );				    
			    }
			}
		    }
		    elof (syntax::MultOperator, op) {
			if (op.getLocation () == Token::LCRO && op.getRights ().size () == 1) {
			    auto left = validateMacroEval (content, loc, op.getLeft (), mapper);				
			    auto value = this-> _context.retreiveValue (this-> _context.validateValue (op.getRights () [0]));
			    if (!value.is<Fixed> () || (value.to<Fixed> ().getType ().to <Integer> ().isSigned () && value.to <Fixed> ().getUI ().i < 0)) {
				Ymir::Error::occur (op.getRights () [0].getLocation (), ExternalError::INCOMPATIBLE_TYPES,
						    value.to <Value> ().getType ().to <Type> ().getTypeName (),
						    (Integer::init (lexing::Word::eof (), 64, false)).to<Type> ().getTypeName ()
				    );				    
			    }
			    auto size = value.to <Fixed> ().getUI ().u;
			    if (size >= left.size ()) {
				Ymir::Error::occur (value.getLocation (), ExternalError::OVERFLOW_ARITY, size, left.size ());
			    }
			    return {left [size]};
			} else {
			    std::vector <std::string> names;
			    for (auto & it : op.getRights ())
			    names.push_back (this-> _context.validateValue (it).to <Value> ().getType ().to <Type> ().getTypeName ());
			}
		    } fo;
		    
		}
	    } catch (Error::ErrorList list) {	
		errors = list.errors;		
	    }
	    	    
	    Ymir::Error::occurAndNote (
		eval.getLocation (),
		errors,
		ExternalError::UNDEF_MACRO_EVAL,
		eval.prettyString ()
	    );

	    return {};
	}
	
	    
	void MacroVisitor::error (const lexing::Word & location, const lexing::Word & end, const Generator & left, const std::list <Ymir::Error::ErrorMsg> & errors) {
	    std::string leftName;
	    match (left) {
		of (FrameProto, proto) leftName = proto.getName ();
		elof (generator::Struct, str) leftName = str.getName ();
		elof (MultSym,    sym)  leftName = sym.getLocation ().getStr ();
		elof (TemplateRef, cl) leftName = cl.prettyString ();
		elof (TemplateClassCst, cl) leftName = cl.prettyString ();
		elof (ModuleAccess, acc) leftName = acc.prettyString ();
		elof (MacroRef, mref) leftName = mref.prettyString ();
		elof (Value,      val)  leftName = val.getType ().to <Type> ().getTypeName ();
		fo;
	    }

	    Ymir::Error::occurAndNote (
		location,
		end,
		errors, 
		ExternalError::UNDEFINED_MACRO_OP,
		leftName
		);
	    	   
	}	

	std::vector <std::string> MacroVisitor::getKnwonRules () {
	    return {__EXPR__, __IDENT__, __TOKEN__, __ANY__, __WORD__, __CHAR__, __STRING__};
	}
	
    }
    
}
