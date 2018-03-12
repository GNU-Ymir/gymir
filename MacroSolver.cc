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
	    if (auto var = it-> to<IMacroVar> ()) {
		for (auto it_ : left.elements) {
		    if (auto var_ = it_-> to <IMacroVar> ()) {
			if (var_-> name.getStr () == var-> name.getStr ()) 
			    return {false, {}, NULL};			
		    } else if (auto rep_ = it_-> to <IMacroRepeat> ()) {
			if (rep_-> ident.getStr () == var-> name.getStr ()) 
			    return {false, {}, NULL};
		    }
		}
	    } else if (auto rep = it-> to <IMacroRepeat> ()) {
		for (auto it_ : left.elements) {
		    if (auto var_ = it_-> to <IMacroVar> ()) {
			if (var_-> name.getStr () == rep-> ident.getStr ()) 
			    return {false, {}, NULL};			
		    } else if (auto rep_ = it_-> to <IMacroRepeat> ()) {
			if (rep_-> ident.getStr () == rep-> ident.getStr ()) 
			    return {false, {}, NULL};
		    }
		}
	    }
	    global.elements.push_back (it);
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
	for (auto elem : expr-> getElements ()) {
	    MacroSolution soluce {false, {}, NULL};
	    if (auto tok = elem-> to <IMacroToken> ()) {
		lex.skipEnable (Token::SPACE, false);
		soluce = solve (tok, lex);
		lex.skipEnable (Token::SPACE, true);
	    } else if (auto rep = elem-> to <IMacroRepeat> ()) {
		soluce = solve (rep, lex);
	    } else if (auto var = elem-> to <IMacroVar> ()) {
		soluce = solve (var, lex);		
	    }
	    if (!soluce.valid) return soluce;
	    soluce = merge (soluce, globSoluce);
	    if (!globSoluce.valid) return globSoluce;
	}
	return globSoluce;
    }
    
    MacroSolution MacroSolver::solve (MacroToken tok, FakeLexer & lex) {
	ulong beg = 0, current = 0;
	std::string val = tok-> getValue ();
	if (val.length () == 0) return {true, {}, NULL};
	while (true) {
	    auto word = lex.next ();
	    if (word.isEof ()) return {false, {}, NULL};
	    else if (word == Token::SPACE && current == 0 && val [current] != ' ') continue;
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

    MacroSolution MacroSolver::solve (MacroRepeat rep, FakeLexer & lex) {
	Ymir::Error::assert ("TODO");
	return {false, {}, NULL};
    }

    MacroSolution MacroSolver::solve (MacroVar var, FakeLexer & lex) {
	ulong nbErrorBeg = Ymir::Error::nb_errors;
	Visitor visit (lex);
	Expression content = NULL;
	if (var-> getType () == MacroVarConst::EXPR)
	    content = visit.visitExpression ();
	else if (var-> getType () == MacroVarConst::IDENT)
	    content = new (Z0) IVar (visit.visitIdentifiant ());
	if (Ymir::Error::nb_errors - nbErrorBeg)
	    return {false, {}, NULL};
	auto ret = var-> clone ();
	ret-> setContent (content);
	return {true, {ret}, NULL};
    }
    
}
