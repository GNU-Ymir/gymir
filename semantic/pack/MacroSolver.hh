#pragma once

#include <ymir/ast/Expression.hh>
#include <ymir/syntax/FakeLexer.hh>
#include <ymir/ast/Var.hh>

namespace syntax {
    class IMacro;
    class IMacroExpr;
    class IMacroElement;
    class IMacroToken;
    class IMacroRepeat;
    class IMacroVar;
    class IMacroCall;
    class IBlock;
}

namespace semantic {

    class ISymbol;
    typedef ISymbol* Symbol;

    class IInfoType;
    typedef IInfoType* InfoType;
    
    struct MacroSolution {
	bool valid;
	std::map <std::string, syntax::Expression> elements;
	syntax::IBlock* block;
    };


    class MacroSolver {

	static MacroSolver __instance__;
	
    public:

	static MacroSolver & instance ();

	MacroSolution solve (syntax::IMacro * left, syntax::IMacroCall * right);	

	MacroSolution solve (syntax::IMacroExpr* left, lexical::FakeLexer& lex, bool needAll = true);

	MacroSolution solve (syntax::IMacroToken* left, lexical::FakeLexer& lex);

	MacroSolution solve (syntax::IMacroRepeat* left, lexical::FakeLexer& lex);

	MacroSolution solve (syntax::IMacroVar* left, lexical::FakeLexer& lex);

	MacroSolution merge (MacroSolution &left, MacroSolution &right);

    private:
	
	std::vector <Word> until (syntax::IMacroToken * tok, lexical::FakeLexer& lex, bool&);
	
    };
        
}

