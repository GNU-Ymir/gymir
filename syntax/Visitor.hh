#pragma once

#include "Lexer.hh"
#include "ast/_.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    struct Visitor {

	Visitor (lexical::Lexer & lexer);

	Program visit ();
	
    private:


	Program visitProgram ();
	std::vector<Declaration> visitPublicBlock ();
	std::vector<Declaration> visitPrivateBlock ();
	Declaration visitDeclaration (bool);

	Impl visitImpl ();
	Constructor visitConstructor ();
	Declaration visitFunctionImpl (bool &);
	Self visitSelf ();
	Global visitGlobal ();
	std::string visitSpace ();
	Import visitImport ();
	std::vector <Expression> visitTemplateStruct ();
	Struct visitStruct ();
	Enum visitEnum ();
	Expression visitIfFunction ();
	Function visitFunction ();
	Proto visitExtern ();
	Var visitVar ();
	Expression visitOf ();
	Var visitVarDeclaration ();
	TypedVar visitStructVarDeclaration ();
	TypedVar visitTypedVarDeclaration ();
	bool canVisitVarDeclaration ();
	Var visitDecoType (const Word&);
	Var visitType ();
	Word visitIdentifiant ();
	bool canVisitIdentifiant ();
	Block visitBlockOutSide ();
	Block visitBlock ();
	Instruction visitInstruction ();
	Instruction visitLet ();
	Instruction visitLetDestruct (const Word& begin);
	Expression visitExpressionOutSide ();
	Expression visitExpressionUlt ();
	Expression visitExpressionUlt (Expression left);
	Expression visitExpression ();
	Expression visitExpression (Expression left);
	Expression visitUlow ();
	Expression visitUlow (Expression left);
	Expression visitLow ();
	Expression visitLow (Expression left);
	Expression visitHigh ();
	Expression visitHigh (Expression left);
	Expression visitPth ();
	Expression visitPthPar (const Word& token);
	Expression visitConstante ();
	Expression visitConstanteSimple ();
	Expression visitExpand ();
	Expression visitTypeOf ();
	Expression visitIs ();
	Expression visitNumeric (const Word& begin, bool abrev = true);
	Expression visitFloat (const Word& begin);
	Expression visitString (Word& word);
	Expression visitPthWPar (Word& tok);
	Expression visitLeftOp ();
	Expression visitConstArray ();
	Expression visitCast ();
	Expression visitFuncPtrSimple ();
	Expression visitFuncPtr ();
	Expression visitLambdaEmpty ();
	Expression visitLambda ();
	Expression visitSuite (const Word& token, Expression left);
	Expression visitPar (Expression left);
	Expression visitAccess (Expression left);
	Expression visitDot (Expression left);
	Expression visitDColon (Expression left);
	Expression visitMixin ();
	Expression visitMatch ();
	Expression visitAfter (const Word& word, Expression left);
	Expression visitBeforePth (const Word& word);
	Instruction visitIf ();
	If visitElse ();
	Assert visitAssert ();
	Instruction visitReturn ();
	Instruction visitBreak ();
	Instruction visitWhile ();
	Instruction visitFor ();
	
    private:
	
	std::vector <std::string> ultimeOp;
	std::vector <std::string> expOp;
	std::vector <std::string> ulowOp;
	std::vector <std::string> lowOp;
	std::vector <std::string> highOp;
	std::vector <std::string> befUnary;
	std::vector <std::string> afUnary;
	std::vector <std::string> suiteElem;
	std::vector <std::string> forbiddenIds;
	std::vector <std::string> decoKeys;
	bool lambdaPossible;
	
	lexical::Lexer & lex;

    };

};
