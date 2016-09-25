#include "Visitor.hh"
#include "Error.hh"
#include <map>
#include <vector>


namespace Syntax {

    using namespace Lexical;

    Syntax::AstPtr Visitor::visit () {
	return visit_program (lexer);
    }

    Syntax::AstPtr visit_program (Lexer & lexer) {	
	TokenPtr token = lexer.next ();
	std::vector<Syntax::DeclarationPtr> decls;
	while (token->getId() != Token::EOF_TOKEN()) {
	    if (token->getId() == DEF) decls.push_back(visit_def (lexer));
	    else if (token->getId() == IMPORT) decls.push_back (visit_import (lexer));
	    else Ymir::Error::append (token->getLocus (),
			   "[%s, %s] attendues, mais %s trouvé\n",
			   getTokenDescription(DEF),
			   getTokenDescription(IMPORT),
			   getTokenDescription(token->getId ()));
	    
	    token = lexer.next ();
	}
	return new Program (Token::makeEof (), decls);
    }

    Syntax::DeclarationPtr visit_import (Lexer &) {
	return NULL;
    }

    Syntax::DeclarationPtr visit_def (Lexer & lexer) {
	TokenPtr name = visit_identifiant (lexer);
	std::vector <Syntax::VarPtr> exps;
	TokenPtr word = lexer.next ();	
	if (word->getId() != PAR_G) Ymir::Error::append (word->getLocus (),
					 "%s attendue, mais %s trouvé\n",
					 getTokenDescription(PAR_G),
					 getTokenDescription(word->getId ()));
	word = lexer.next ();	
	if (word->getId () != PAR_D) {
	    lexer.rewind ();
	    while (1) {
		exps.push_back (visit_var_declaration (lexer));
		word = lexer.next ();
		if (word->getId() == PAR_D) break;
		else if (word->getId () != VIRGULE)
		    Ymir::Error::append (word->getLocus (),
			      "[%s, %s] attendue, mais %s trouvé\n",
			      getTokenDescription(PAR_D),
			      getTokenDescription(VIRGULE),
			      getTokenDescription(word->getId ()));
	    }
	}
	
	word = lexer.next ();
	if (word->getId() == DEUX_POINT) {
	    Syntax::VarPtr type = visit_type (lexer);
	    return (new Syntax::Function (name, type, exps, visit_block (lexer)));
	} else lexer.rewind ();
	return (new Syntax::Function (name, exps, visit_block (lexer)));
    }

    VarPtr visit_var (Lexer & lexer) {
	return visit_type (lexer);
    }
    
    Syntax::VarPtr visit_var_declaration (Lexer & lexer) {
	TokenPtr token = visit_identifiant (lexer);
	TokenPtr next = lexer.next ();
	if (next->getId () == DEUX_POINT) {
	    auto type = visit_type (lexer);
	    return (new TypedVar (token, type));
	} else lexer.rewind ();
	return (new Var (token));
    }

    Syntax::VarPtr visit_complex_var (Lexer & lexer) {
	TokenPtr token = visit_identifiant (lexer);
	TokenPtr next = lexer.next ();
	if (next->getId () == DEUX_POINT) {
	    auto type = visit_type (lexer);
	    return (new TypedVar (token, type));
	} else Ymir::Error::append (next->getLocus (),
			 "%s attendus, mais %s trouvé",
			 getTokenDescription (DEUX_POINT),
			 getTokenDescription (next->getId ()));
	return NULL;
    }
    
    Syntax::VarPtr visit_type (Lexer & lexer) {
	TokenPtr token = visit_identifiant (lexer);
	TokenPtr next = lexer.next ();
	if (next->getId () == NOT) {
	    std::vector<ExpressionPtr> params;
	    next = lexer.next ();
	    if (next->getId () == PAR_G) {
		while (1) {
		    params.push_back (visit_expression (lexer));
		    next = lexer.next ();
		    if (next->getId () == PAR_D) break;
		    else if (next->getId () != VIRGULE)
			Ymir::Error::append (next->getLocus (),
				  "[%s, %s] attendues, mais %s trouvé",
				  getTokenDescription(VIRGULE),
				  getTokenDescription (PAR_D),
				  getTokenDescription (next->getId ()));
		}
	    } else {
		lexer.rewind();
		params.push_back (visit_expression (lexer));
	    }
	    return new Var (token, params);
	} else lexer.rewind ();
	return (new Var (token));
    }

    TokenPtr visit_identifiant (Lexer & lexer) {
	TokenPtr token = lexer.next ();
	if (token->getId() == OTHER) {
	    return token;
	} else Ymir::Error::append (token->getLocus (), "Un identifiant est attendue, mais %s trouvé", getTokenDescription(token->getId()));
	return Token::makeEof ();
    }
    
    BlockPtr visit_block (Lexer & lexer) {
	TokenPtr begin = lexer.next ();
	if (begin -> getId () == ACC_G) {
	    std::vector <DeclarationPtr> decls;
	    std::vector <InstructionPtr> insts;
	    while (1)  {
		TokenPtr next = lexer.next ();
		if (next->getId () == DEF) decls.push_back (visit_def (lexer));
		else if (next->getId () == IMPORT) decls.push_back (visit_import (lexer));
		else if (next->getId () == ACC_G) insts.push_back ((InstructionPtr)visit_block (lexer));
		else if (next->getId () == POINT_VIG) {}
		else if (next->getId () == ACC_D) break;
		else {
		    lexer.rewind ();
		    insts.push_back (visit_instruction (lexer));
		}
	    }
	    return new Block (begin, decls, insts);
	} else lexer.rewind ();
	return new Block (begin, {}, {visit_instruction (lexer)});
    }

    InstructionPtr visit_instruction (Lexer & lexer) {
	TokenPtr token = lexer.next ();
	if (token->getId () == IF) return visit_if (lexer);
	else if (token->getId () == RETURN) return visit_return (lexer);
	else if (token->getId () == FOR) return visit_for (lexer);
	else if (token->getId () == FOREACH) return visit_foreach (lexer);
	else if (token->getId () == WHILE) return visit_while (lexer);
	else if (token->getId () == BREAK) return visit_break (lexer);
	else if (token->getId () == DELETE) return visit_delete (lexer);
	else if (token->getId () == POINT_VIG) {
	    warning_at (token->getLocus (), 0,
			"Utilisez {} pour une instruction vide pas %s",
			token -> getCstr ());
	    return new Instruction (token);
	}
	else {
	    lexer.rewind ();
	    auto retour = (InstructionPtr)visit_expression_ult (lexer);
	    TokenPtr next = lexer.next ();
	    if (next -> getId () != POINT_VIG)
		Ymir::Error::append (next->getLocus (),
			  "%s attendue, mais %s trouvé",
			  getTokenDescription(POINT_VIG),
			  next->getCstr ());
	    return retour;
	}	
    }
    
    bool find (TokenId id, std::vector <TokenId> tab) {
	for(auto & it : tab) {
	    if (it == id) return true;
	}
	return false;
    }

    Syntax::ExpressionPtr visit_expression_ult (Lexer & lexer) {
	ExpressionPtr left = visit_expression (lexer);
	TokenPtr token = lexer.next ();
	if (find (token->getId (),
		  {AFFECT, PLUS_AFF, MUL_AFF, SUB_AFF, DIV_AFF})) {
	    ExpressionPtr right = visit_expression (lexer);
	    return visit_expression_ult (lexer, new Binary (token, left, right));
	} else lexer.rewind();
	return left;
    }
    
    ExpressionPtr visit_expression_ult (Lexer & lexer, ExpressionPtr left) {
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {AFFECT, PLUS_AFF, MUL_AFF, SUB_AFF, DIV_AFF})) {
	    ExpressionPtr right = visit_expression (lexer);
	    return visit_expression_ult (lexer, new Binary (token, left, right));
	} else lexer.rewind();
	return left;
    }

    
    Syntax::ExpressionPtr visit_expression (Lexer & lexer) {
	ExpressionPtr left = visit_ulow (lexer);
	TokenPtr token = lexer.next ();
	if (find (token->getId (), {OR, AND})) {
	    ExpressionPtr right = visit_ulow (lexer);
	    return visit_expression (lexer, new Binary (token, left, right));
	} else lexer.rewind();
	return left;
    }
    
    ExpressionPtr visit_expression (Lexer & lexer, ExpressionPtr left) {
	TokenPtr token = lexer.next ();       
	if (find (token->getId () , {OR, AND})) {
	    ExpressionPtr right = visit_ulow (lexer);
	    return visit_expression (lexer, new Binary (token, left, right));
	} else lexer.rewind();
	return left;
    }

    ExpressionPtr visit_ulow (Lexer & lexer) {
	ExpressionPtr left = visit_low (lexer);
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {INF, SUP, EQUALS, SUP_EQUALS, INF_EQUALS, NOT_EQUALS, IS, NOT})) {
	    ExpressionPtr right = visit_low (lexer);
	    return visit_ulow (lexer, new Binary (token, left, right));
	} else lexer.rewind();
	return left;	
    }

    ExpressionPtr visit_ulow (Lexer & lexer, ExpressionPtr left) {
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {INF, SUP, EQUALS, SUP_EQUALS, INF_EQUALS, NOT_EQUALS, IS, NOT})) {
	    ExpressionPtr right = visit_low (lexer);
	    return visit_ulow (lexer, new Binary (token, left, right));
	} else lexer.rewind();
	return left;	
    }

    ExpressionPtr visit_low (Lexer & lexer) {
	ExpressionPtr left = visit_high (lexer);
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {PLUS, SUB, OR_BIT, LEFT_BIT, RIGHT_BIT, XOR})) {
	    ExpressionPtr right = visit_high (lexer);
	    return visit_low (lexer, new Binary (token, left, right));
	} else lexer.rewind();
	return left;	
    }
    
    ExpressionPtr visit_low (Lexer & lexer, ExpressionPtr left) {
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {MUL, DIV, AND_BIT, MODULO, POINT_POINT, IN})) {
	    ExpressionPtr right = visit_high (lexer);
	    return visit_low (lexer, new Binary (token, left, right));
	} else lexer.rewind();
	return left;	
    }

    ExpressionPtr visit_high (Lexer & lexer) {
	ExpressionPtr left = visit_pth (lexer);
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {STAR, DIV, AND_BIT, MODULO, POINT_POINT, IN})) {
	    ExpressionPtr right = visit_pth (lexer);
	    return visit_high (lexer, new Binary (token, left, right));
	} else lexer.rewind();
	return left;	
    }

    ExpressionPtr visit_high (Lexer & lexer, ExpressionPtr left) {
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {STAR, DIV, AND_BIT, MODULO, POINT_POINT, IN})) {
	    ExpressionPtr right = visit_pth (lexer);
	    return visit_high (lexer, new Binary (token, left, right));
	} else lexer.rewind();
	return left;	
    }
    
    ExpressionPtr visit_pth (Lexer & lexer) {
	TokenPtr token = lexer.next ();
	if (find (token->getId (),
		  {STAR, S_AND, MINUS, NOT})) {
	    return visit_before_pth (lexer, token);
	} else {
	    if (token->getId() == PAR_G)
		return visit_pth_par (lexer);
	    else
		return visit_pth_wpar (lexer, token);
	}
    }
    
    ExpressionPtr visit_pth_par (Lexer & lexer) {	
	auto exp = visit_expression_ult (lexer);
	TokenPtr token = lexer.next ();
	if (token->getId () != PAR_D)
	    Ymir::Error::append (token->getLocus(),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (PAR_D),
		      token->getCstr());
	token = lexer.next ();
	if (find (token->getId (),
		  {CROCHET_G, PAR_G, DOT})) {
	    return visit_suite (lexer, token, exp);
	} else lexer.rewind ();
	return exp;
    }
        
    ExpressionPtr visit_constante (Lexer & lexer) {
	TokenPtr token = lexer.next ();
	if (token->getId() == OTHER
	    && token->getStr ().length() > 0
	    && token->getStr()[0] >= getTokenDescription (ZERO)[0]
	    && token->getStr()[0] <= getTokenDescription (NEUF)[0])
	    return visit_numeric (lexer, token);
	else if (token->getId() == DOT)
	    return visit_flottant (lexer);
	else if (token->getId() == APOS || token->getId () == GUILL)
	    return visit_chaine (lexer, token);
	else if (token->getId() == FALSE_T || token->getId() == TRUE_T)
	    return new Bool (token);
	else if (token->getId () == NULL_T)
	    return new Null (token);
	else lexer.rewind ();
	return NULL;	    
    }

    ExpressionPtr visit_numeric (Lexer & lexer, TokenPtr token) {
	auto zero = getTokenDescription(ZERO)[0];
	auto neuf = getTokenDescription(NEUF)[0];
	for (int i = 0; i < (int)token->getStr ().length(); i++) {
	    if (token->getStr ()[i] < zero || token->getStr ()[i] > neuf)
		Ymir::Error::append (token->getLocus (),
			  "erreur de syntaxe '%s'",
			  token->getCstr ());	   
	}
	TokenPtr next = lexer.next ();
	if (next->getId () == DOT) {
	    next = lexer.next ();
	    std::string suite = next->getStr ();
	    if (next->getId() != OTHER) {
		suite = "0";
		lexer.rewind ();
	    } else {
		for (int i = 0; i < (int)next->getStr().length (); i++) {
		    if (next->getStr () [i] < zero || next->getStr ()[i] > neuf) {
			suite = "0";
			lexer.rewind();
			break;
		    }
		}
	    }
	    return new Float (token, suite);
	} else lexer.rewind ();
	return new Int (token);	    
    }

    ExpressionPtr visit_flottant (Lexer & lexer) {
	TokenPtr next = lexer.next ();
	auto zero = getTokenDescription(ZERO)[0];
	auto neuf = getTokenDescription(NEUF)[0];
	if (next->getId () != OTHER)
	    Ymir::Error::append (next->getLocus (),
		      "erreur de syntaxe %s",
		      next->getCstr());
	
	for (int i = 0; i < (int)next->getStr().length() ; i++) {
	    if (next->getStr () [i] < zero || next->getStr () [i] > neuf)
		Ymir::Error::append (next->getLocus (),
			  "erreur de syntaxe %s",
			  next->getCstr());
	}
	return new Float (next);
    }

    short isChar (std::string value) {
	std::map<std::string, char> escape = {{"\\a", '\a'}, {"\\b", '\b'}, {"\\f", '\f'}, {"\\n", '\n'}, {"\\r", '\r'}, {"\\t", '\t'}, {"\\v", '\v'}, {"\\", '\\'}, {"\'", '\''}, {"\"", '\"'}, {"\?", '\?'}};
	if (value.length() == 1) return (short)(value[0]);
	auto val = escape.find (value);
	if (val != escape.end ()) return (short)val->second;
	if (value[0] == getTokenDescription(ANTI)[0] && value.length() == 4) {
	    if (value[1] == getTokenDescription(LX)[0]) {
		std::string val = value.substr (2, value.length());
		for (int i = 0; i < (int)val.length () ; i++) {
		    if ((val[i] < 'a' || val[i] > 'f') &&
			(val[i] < '0' || val[i] > '9')) return -1;
		}
		//TODO
	    } else if (value[0] == getTokenDescription(ANTI)[0] && value.length() > 1 && value.length() < 5) {
		std::string val = value.substr (1, value.length());
		for (int i = 0; i < (int)val.length(); i++)
		    if (val[i] < '0' || val[i] > '7') return -1;
		//TODO
	    }
	}
	return -1;
    }
    
    ExpressionPtr visit_chaine (Lexer & lexer, TokenPtr token) {
	lexer.removeSkip (" ");
	lexer.setComments (false);
	TokenPtr next, bef;
	std::string val = "";
	while (1) {
	    next = lexer.next ();
	    if (next->getId () == EOF) Ymir::Error::append (next->getLocus (),
						 "Fin de fichier inattendue");
	    else if (next->getId () == token -> getId ()) break;
	    else val += next->getStr ();		
	}
	lexer.addSkip (" ");
	lexer.setComments (true);
	if (token->getId () == APOS) {
	    short c = isChar (val);
	    if (c >= 0) return new Char (token, c);
	}
	return new String (token, val);
    }    
    
    ExpressionPtr visit_pth_wpar (Lexer & lexer, TokenPtr token) {
	if (token->getId() == IF) return visit_inside_if (lexer);
	else if (token->getId () == FOR) return visit_inside_for (lexer);
	else if (token->getId () == NEW) return visit_new (lexer);
	else if (token->getId () == CAST) return visit_cast (lexer);
	else {
	    lexer.rewind ();
	    ExpressionPtr constante = visit_constante (lexer);
	    if (constante != NULL) return constante;
	    ExpressionPtr left = visit_left_op (lexer);
	    token = lexer.next ();
	    if (find (token->getId(),
		      {PPLUS, SSUB})) {
		return visit_after (lexer, token, left);
	    } else lexer.rewind ();
	    return left;
	}
    }    

    ExpressionPtr visit_left_op (Lexer & lexer) {
	ExpressionPtr var = visit_var (lexer);
	TokenPtr next = lexer.next ();
	if (find (next->getId (), {PAR_G, CROCHET_G, DOT})) return visit_suite (lexer, next, var);
	lexer.rewind ();
	return var;
    }
    
    ExpressionPtr visit_inside_if (Lexer & lexer) {
	lexer.rewind();
	TokenPtr begin = lexer.next ();
	TokenPtr next = lexer.next ();
	if (next->getId () == PAR_G)
	    Ymir::Error::append (next->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (PAR_G),
		      next -> getCstr ());
	ExpressionPtr test = visit_expression (lexer);
	next = lexer.next ();
	if (next->getId () == PAR_D)
	    Ymir::Error::append (next->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (PAR_D),
		      next -> getCstr ());
	ExpressionPtr if_bl = visit_expression (lexer);
	next = lexer.next ();
	if (next->getId () == ELSE)
	    Ymir::Error::append (next->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (ELSE),
		      next -> getCstr ());
	ExpressionPtr else_bl = visit_expression (lexer);
	return new InsideIf (begin, test, if_bl, else_bl);
    }


    ExpressionPtr visit_suite (Lexer & lexer, TokenPtr token, ExpressionPtr left) {
	if (token -> getId () == PAR_G) return visit_par (lexer, left);
	else if (token->getId () == CROCHET_G) return visit_access (lexer, left);
	else if (token->getId () == DOT) return visit_dot (lexer, left);
	else
	    Ymir::Error::append (token->getLocus (),
		      "Erreur de syntaxe %s",
		      token->getCstr());
	return NULL;
    }

    ExpressionPtr visit_par (Lexer & lexer, ExpressionPtr left) {
	lexer.rewind ();
	TokenPtr begin = lexer.next (), next = lexer.next ();
	TokenPtr suite = next;
	std::vector <ExpressionPtr> params;
	if (next->getId () != PAR_D) {
	    lexer.rewind ();
	    while (1) {
		params.push_back (visit_expression (lexer));
		next = lexer.next ();
		if (next -> getId () == PAR_D) break;
		else if (next -> getId () != VIRGULE)
		    Ymir::Error::append (next->getLocus (),
			      "[%s, %s] attendues, mais %s trouvé",
			      getTokenDescription (PAR_D),
			      getTokenDescription (VIRGULE),
			      next->getCstr ());
	    }
	}
	ExpressionPtr retour = new Par (begin, left, new ParamList (suite, params));
	next = lexer.next ();
	if (find (next->getId (), {PAR_G, CROCHET_G, DOT})) return visit_suite (lexer, next, retour);
	lexer.rewind ();
	return retour;			
    }

    ExpressionPtr visit_access (Lexer & lexer, ExpressionPtr left) {
	lexer.rewind();
	TokenPtr begin = lexer.next ();
	TokenPtr next = lexer.next ();
	TokenPtr suite = next;
	std::vector <ExpressionPtr> params;
	if (next->getId () != CROCHET_D) {
	    lexer.rewind ();
	    while (1) {
		params.push_back (visit_expression (lexer));
		next = lexer.next ();
		if (next->getId () == CROCHET_D) break;
		else if (next->getId () != VIRGULE)
		    Ymir::Error::append (next->getLocus (),
			      "[%s, %s] attendues, mais %s trouvé",
			      getTokenDescription (CROCHET_D),
			      getTokenDescription (VIRGULE),
			      next->getCstr ());
	    }
	}
	ExpressionPtr retour = new Access (begin, left, new ParamList (next, params));
	next = lexer.next ();
	if (find (next->getId (), {PAR_G, CROCHET_G, DOT})) return visit_suite (lexer, next, retour);
	lexer.rewind ();
	return retour;
    }

    ExpressionPtr visit_dot (Lexer & lexer, ExpressionPtr left) {
	lexer.rewind();
	TokenPtr begin = lexer.next();
	VarPtr right = visit_var (lexer);
	ExpressionPtr retour = new Dot (begin, left, right);
	TokenPtr next = lexer.next ();
	if(find (next->getId(), {PAR_G, CROCHET_G, DOT})) return visit_suite (lexer, next, retour);
	lexer.rewind ();
	return retour;
    }

    ExpressionPtr visit_after (Lexer &, TokenPtr token, ExpressionPtr left) {
	return new AfUnary (token, left);
    }
    
    ExpressionPtr visit_before_pth (Lexer & lexer, TokenPtr token) {
	ExpressionPtr elem = visit_pth (lexer);
	return new BefUnary (token, elem);
    }
    
    ExpressionPtr visit_inside_for (Lexer & lexer) {
	lexer.rewind ();
	TokenPtr begin = lexer.next(), suite = lexer.next ();
	if (suite->getId () != PAR_G)
	    Ymir::Error::append (suite->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (PAR_G),
		      suite->getCstr ());
	ExpressionPtr var = visit_var (lexer);
	suite = lexer.next ();
	if (suite->getId () != IN)
	    Ymir::Error::append (suite->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (IN),
		      suite->getCstr ());
	ExpressionPtr iter = visit_expression (lexer);
	suite = lexer.next ();
	if (suite->getId () != PAR_D)
	    Ymir::Error::append (suite->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (PAR_D),
		      suite->getCstr ());
	ExpressionPtr what = visit_expression (lexer);
	return new InsideFor (begin, var, iter, what);
    }

    ExpressionPtr visit_new (Lexer & lexer) {
	lexer.rewind();
	TokenPtr begin = lexer.next (), next;
	VarPtr type = visit_type (lexer);
	next = lexer.next ();
	ExpressionPtr size = NULL;
	if (next->getId() == CROCHET_G) {
	    size = visit_expression (lexer);
	    next = lexer.next ();
	    if (next->getId () != CROCHET_D)
		Ymir::Error::append (next->getLocus (),
			  "%s attendue, mais %s trouvé",
			  getTokenDescription (CROCHET_D),
			  next->getCstr ());	    
	} else lexer.rewind ();
	return new New (begin, type, size);
    }

    ExpressionPtr visit_cast (Lexer & lexer) {
	lexer.rewind ();
	TokenPtr begin = lexer.next (), next = lexer.next ();
	if (next->getId () != DEUX_POINT)    Ymir::Error::append (next->getLocus (),
						       "%s attendue, mais %s trouvé",
						       getTokenDescription (DEUX_POINT),
						       next->getCstr ());
	VarPtr type = NULL;
	next = lexer.next ();
	if(next->getId () == PAR_G) {
	    type = visit_type (lexer);
	    next = lexer.next ();
	    if (next->getId () != PAR_D)	Ymir::Error::append (next->getLocus (),
							  "%s attendue, mais %s trouvé",
							  getTokenDescription (PAR_D),
							  next->getCstr ());
	} else {
	    lexer.rewind ();
	    auto name = visit_identifiant(lexer);
	    type = new Var (name);
	}
	next = lexer.next ();
	if (next->getId () != PAR_G)	    Ymir::Error::append (next->getLocus (),
						      "%s attendue, mais %s trouvé",
						      getTokenDescription (PAR_G),
						      next->getCstr ());
	ExpressionPtr inside = visit_expression (lexer);
	next = lexer.next ();
	if (next->getId () != PAR_D) 	    Ymir::Error::append (next->getLocus (),
						      "%s attendue, mais %s trouvé",
						      getTokenDescription (PAR_D),
						      next->getCstr ());
	return new Cast (begin, type, inside);
	    
    }

    InstructionPtr visit_if (Lexer & lexer) {
	lexer.rewind ();
	TokenPtr begin = lexer.next ();
	ExpressionPtr test = visit_expression (lexer);
	BlockPtr block = visit_block (lexer);
	TokenPtr next = lexer.next ();
	if (next->getId () == ELSE) {
	    return new If (begin, test, block, visit_else (lexer));
	} else lexer.rewind ();
	return new If (begin, test, block);
    }

    InstructionPtr visit_else (Lexer & lexer) {
	lexer.rewind ();
	TokenPtr begin = lexer.next (), next = lexer.next ();
	if (next->getId () == IF) {
	    return new ElseIf (begin, visit_if (lexer));
	} else lexer.rewind ();
	return new Else (begin, visit_block (lexer));
    }

    InstructionPtr visit_return (Lexer & lexer) {
	lexer.rewind ();
	TokenPtr begin = lexer.next (), next = lexer.next ();
	if (next->getId () == POINT_VIG)
	    return new Return (begin);
	else lexer.rewind ();
	ExpressionPtr exp = visit_expression (lexer);
	next = lexer.next ();
	if (next->getId () != POINT_VIG)
	    Ymir::Error::append (next->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (POINT_VIG),
		      next -> getCstr ());
	return new Return (begin, exp);
    }

    InstructionPtr visit_for (Lexer & lexer) {
	lexer.rewind ();
	TokenPtr begin = lexer.next(), next = lexer.next ();
	if (next->getId () != PAR_G) Ymir::Error::append (next->getLocus (),
					       "%s attendue, mais %s trouvé",
					       getTokenDescription (PAR_G),
					       next->getCstr());
	std::vector <ExpressionPtr> inits;
	next = lexer.next ();
	if (next->getId () != POINT_VIG) {
	    lexer.rewind ();
	    while (1) {
		inits.push_back (visit_expression_ult (lexer));
		next = lexer.next ();
		if (next->getId () == POINT_VIG) break;
		else if (next->getId () != VIRGULE)
		    Ymir::Error::append (next->getLocus (),
			      "[%s, %s] attendues, mais %s trouvé",
			      getTokenDescription(VIRGULE),
			      getTokenDescription(POINT_VIG),
			      next->getCstr ());
	    }
	}

	ExpressionPtr test = visit_expression (lexer);
	next = lexer.next ();
	if (next->getId () != POINT_VIG)
	    Ymir::Error::append (next->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (POINT_VIG),
		      next->getCstr ());

	std::vector<ExpressionPtr> iter;
	next = lexer.next ();
	if (next->getId () != PAR_D) {
	    lexer.rewind ();
	    while (1) {
		iter.push_back (visit_expression_ult (lexer));
		next = lexer.next ();
		if (next->getId () == PAR_D) break;
		else if (next->getId () != VIRGULE)
		    Ymir::Error::append (next->getLocus (),
			      "[%s, %s] attendue, mais %s trouvé",
			      getTokenDescription(VIRGULE),
			      getTokenDescription(PAR_D),
			      next->getCstr ());
	    }
	}
	
	return new For (begin, inits, test, iter, visit_block (lexer));
    }

    
    InstructionPtr visit_foreach (Lexer & lexer) {
	lexer.rewind ();
	bool has_par = false;
	TokenPtr begin = lexer.next (), next = lexer.next ();
	if (next -> getId() == PAR_G) has_par = true;
	else lexer.rewind ();
	ExpressionPtr var = visit_var (lexer);
	next = lexer.next ();
	if (next->getId () != IN)
	    Ymir::Error::append (next->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription(IN),
		      next->getCstr ());
	ExpressionPtr iter = visit_expression (lexer);
	if (has_par) {
	    next = lexer.next ();
	    if (next ->getId() != PAR_D)
		Ymir::Error::append (next->getLocus (),
			  "%s attendue, mais %s trouvé",
			  getTokenDescription (PAR_D),
			  next->getCstr ());
	}
	return new Foreach (begin, var, iter, visit_block (lexer));
    }
    
    InstructionPtr visit_while (Lexer & lexer) {
	lexer.rewind ();
	TokenPtr begin = lexer.next ();
	ExpressionPtr test = visit_expression (lexer);
	return new While (begin, test, visit_block (lexer));
    }
    
    InstructionPtr visit_break (Lexer & lexer) {
	lexer.rewind ();
	TokenPtr begin = lexer.next (), next = lexer.next ();
	if (next->getId () != POINT_VIG)
	    Ymir::Error::append (next->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (POINT_VIG),
		      next->getCstr ());
	return new Break (begin);
    }


    InstructionPtr visit_delete (Lexer & lexer) {
	lexer.rewind ();
	TokenPtr begin = lexer.next ();
	ExpressionPtr left = visit_expression (lexer);
	TokenPtr next = lexer.next ();
	if (next->getId () != POINT_VIG)
	    Ymir::Error::append (next->getLocus (),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (POINT_VIG),
		      next->getCstr ());
	return new Delete (begin, left);
    }
    
    
};
