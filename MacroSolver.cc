#include <ymir/semantic/pack/MacroSolver.hh>
#include <ymir/ast/Macro.hh>
#include <ymir/syntax/_.hh>

namespace semantic {
    using namespace syntax;
    using namespace lexical;
    
    MacroSolver MacroSolver::__instance__;

    MacroSolver& MacroSolver::instance () {
	return __instance__;
    }

    MacroSolution MacroSolver::merge (MacroSolution &left, MacroSolution &right) {
	MacroSolution global {true, left.elements, NULL};
	for (auto it : right.elements) {
	    for (auto it_ : left.elements) {
		if (it.first == it_.first) 
		    return {false, {}, NULL};
	    }
	    global.elements [it.first] = (it.second);
	}
	return global;
    }
    
    MacroSolution MacroSolver::solve (Macro expr, MacroCall call) {       
	for (auto it : Ymir::r (0, expr-> getExprs ().size ())) {
	    auto left = expr-> getExprs () [it];
	    FakeLexer lex (call-> getTokens ());
	    auto soluce = this-> solve (left, lex);
	    
	    if (soluce.valid) {
		soluce.block = expr-> getBlocks () [it];
		return soluce;
	    }
	}
	return {false, {}, NULL};
    }

       
    MacroSolution MacroSolver::solve (MacroExpr expr, FakeLexer& lex) {	
	MacroSolution globSoluce {true, {}, NULL};
	std::vector <FakeLexer> lexers;
	std::vector <uint> get;
	uint current = 0;
	for (auto elem : expr-> getElements ()) {
	    if (auto tok = elem-> to <IMacroToken> ()) {
		bool success = true;
		auto toks = this-> until (tok, lex, success);
		if (!success) return {false, {}, NULL};
		else {
		    FakeLexer other (toks);
		    lexers.push_back (other);
		    current ++;
		} 
	    } else {
		get.push_back (current);
	    }
	}
		
	uint i = 0;	
	for (auto elem : expr-> getElements ()) {
	    MacroSolution soluce {true, {}, NULL};
	    if (auto rep = elem-> to <IMacroRepeat> ()) {
		if (get [i] < lexers.size ())
		    soluce = solve (rep, lexers [get [i]]);
		else
		    soluce = solve (rep, lex);
		i++;
	    } else if (auto var = elem-> to <IMacroVar> ()) {
		if (get [i] < lexers.size ())
		    soluce = solve (var, lexers [get [i]]);
		else
		    soluce = solve (var, lex);
		i++;
	    }
	    
	    if (!soluce.valid) return soluce;
	    globSoluce = merge (globSoluce, soluce);
	    if (!globSoluce.valid) return globSoluce;
	}

	for (auto it : lexers)
	    if (!it.next ().isEof ()) return {false, {}, NULL};
	
	if (!lex.next ().isEof ()) return {false, {}, NULL};
	
	return globSoluce;
    }
    
    MacroSolution MacroSolver::solve (MacroToken tok, FakeLexer & lex) {
	ulong beg = 0, current = 0;
	std::string val = tok-> getValue ();
	if (val.length () == 0) return {true, {}, NULL};
	while (true) {
	    auto word = lex.next ();
	    if (word.isEof ()) return {false, {}, NULL};
	    else if (word == Token::SPACE && current == 0 && val [current] != ' ')
		continue;
	    beg = 0;
	    for (auto it : Ymir::r (0, word.getStr ().length ())) {
		if (current >= val.length ()) {
		    lex.cutCurrentWord (beg);
		    return {true, {}, NULL};
		} else if (word.getStr () [it] == val [current]) {
		    current ++;
		    beg++;
		} else {
		    Ymir::Error::syntaxError (word);
		    return {false, {}, NULL};
		}
	    }
	    if (current == val.length ()) return {true, {}, NULL};
	}
    }
    
    std::vector<Word> MacroSolver::until (MacroToken tok, FakeLexer & lex, bool &success) {
	std::vector<Word> words;
	ulong beginWord = lex.tell ();
	int openAcc = 0, openCro = 0, openPar = 0;
	std::vector <Word> read;
	auto val = tok-> getValue ();
	success = true;	
	while (true) {
	    auto word = lex.next ();
	    if (word.isEof ()) break;

	    if (word == val && openPar <= 0 && openCro <= 0 && openAcc <= 0)
		return read;

	    if (word == Token::LACC) openAcc++;
	    else if (word == Token::RACC) openAcc --;
	    else if (word == Token::LCRO) openCro ++;
	    else if (word == Token::RCRO) openCro --;
	    else if (word == Token::LPAR) openPar ++;
	    else if (word == Token::RPAR) openPar --;
	    read.push_back (word);
	}

	lex.seek (beginWord);
	success = false;
	return {};
    }
    
    MacroSolution MacroSolver::solve (MacroRepeat rep, FakeLexer & lex) {
	auto closeToken = rep-> getClose ();
	auto result = new (Z0) IMacroRepeat (rep-> token, rep-> getExpr (), rep-> getClose (), rep-> isOneTime ());
	bool end = false;
	while (!end) {
	    bool fail = false, ignore = true;
	    ulong beginLex = lex.tell ();
	    auto toks = this-> until (closeToken, lex, ignore);
	    FakeLexer other (toks);
	    FakeLexer* doing = &other;
	    if (toks.size () == 0) {
		end = true;
		doing = &lex;
	    }
	    
	    Ymir::Error::activeError (false);
	    auto soluce = this-> solve (rep-> getExpr (), *doing);
	    auto errors = Ymir::Error::caught ();
	    Ymir::Error::activeError (true);
	    if (errors.size () != 0 || !soluce.valid) {
		lex.seek (beginLex);
		fail = true;
	    }
	    
	    if (fail && rep-> isOneTime () && result-> getSolution ().size () == 0) return {false, {}, NULL};
	    else if (fail && end && result-> getSolution ().size () == 0) return {true, {{rep-> token.getStr (), result}}, NULL};
	    else if (fail) return {false, {}, NULL};

	    result-> addSolution (soluce);
	}
	
	return {true, {{rep-> token.getStr (), result}}, NULL};	
    }

    MacroSolution MacroSolver::solve (MacroVar var, FakeLexer & lex) {
	ulong nbErrorBeg = Ymir::Error::nb_errors;
	Visitor visit (lex);
	Expression content = NULL;
	if (var-> getType () == MacroVarConst::EXPR)
	    content = visit.visitExpression ();
	else if (var-> getType () == MacroVarConst::IDENT)
	    content = new (Z0) IVar (visit.visitIdentifiant ());
	else if (var-> getType () == MacroVarConst::BLOCK) 
	    content = visit.visitBlock ();
	
	if (Ymir::Error::nb_errors - nbErrorBeg)
	    return {false, {}, NULL};
	auto ret = var-> clone ();
	ret-> setContent (content);
	
	return MacroSolution {true, {{var-> name.getStr (), ret}}, NULL};
    }
    
    
}
