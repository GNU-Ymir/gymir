#pragma once

#include "Lexer.hh"
#include "ast/_.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    struct Visitor {

	static bool failure;
	
	Visitor (lexical::Lexer & lexer);

	Program visit ();
	
	Program visitProgram ();
	std::vector<Declaration> visitPublicBlock (std::string&);
	std::vector<Declaration> visitPrivateBlock (std::string&);
	std::vector<Declaration> visitVersionGlob (bool globalBlock = false);
	std::vector<Declaration> visitDeclBlock (bool globalBlock = false);
	Declaration visitDeclaration (std::string, bool);

	TypeCreator visitTypeCreator (std::string & docs);
	TypeConstructor visitTypeConstructor ();
	TypeDestructor visitTypeDestructor ();
	Block visitTypeStaticConstruct ();
	TypeMethod visitTypeMethod (std::string & docs);
	TypeAlias visitTypeAlias (std::string & docs);
	void visitTypePrivate (std::string & docs, TypeCreator type);
	void visitTypeProtected (std::string & docs, TypeCreator type);
	
	Self visitSelf (std::string&);
	DestSelf visitDestSelf (std::string&);
	Global visitGlobal (std::string&);
	Global visitGlobalImut (std::string&);
	std::string visitSpace ();
	Import visitImport ();
	Use visitUse ();
	std::vector <Expression> visitTemplateStruct ();
	Struct visitStruct (std::string & docs, bool isUnion = false);
	Enum visitEnum (std::string & docs);
	Expression visitIfFunction ();
	Function visitFunction (std::string & docs);
	Function visitContract (Word ident, std::string & docs, std::vector<Word> &, Expression type, Word retDeco, std::vector<Var> &, std::vector <Expression> &, Expression);
	
	std::vector<Declaration> visitExtern (std::string & docs);
	Declaration visitExternBl ();
	Var visitVar ();
	Expression visitOf (bool&, bool&);
	Var visitVarDeclaration ();
	TypedVar visitStructVarDeclaration ();
	TypedVar visitTypedVarDeclaration ();
	bool canVisitVarDeclaration ();
	bool canVisitIgnore ();
	bool canVisitNamedExpr ();
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
	Expression visitMutable ();
	Expression visitExpand ();
	Expression visitTypeOf ();
	Expression visitStringOf ();
	Expression visitIs ();
	Expression visitNumericOrVar ();
	Expression visitNumeric (const Word& begin, bool abrev = true);
	Expression visitNumericHexa (const Word& begin);
	Expression visitFloat (const Word& begin);
	Expression visitString (Word& word);
	Expression visitPthWPar (Word& tok);
	Expression visitLeftOp ();
	Expression visitLeftOpSimple ();
	Expression visitConstArray ();
	Expression visitCast ();
	Expression visitFuncPtrSimple (const Word &);
	Expression visitFuncPtr (const Word &);
	Expression visitLambdaEmpty ();
	Expression visitLambda ();
	Expression visitLambdaMove ();
	Expression visitSuite (const Word& token, Expression left);
	Expression visitPar (Expression left);
	Expression visitStructCst (Expression left);
	Expression visitAccess (Expression left);
	Expression visitDot (Expression left);
	Expression visitDColon (Expression left);
	Expression visitMixin ();
	Expression visitMatch ();
	Expression visitMatchExpression ();
	Expression visitAfter (const Word& word, Expression left);
	Expression visitBeforePth (const Word& word);
	Instruction visitIf ();
	If visitElse ();
	Assert visitAssert ();
	Throw visitThrow ();
	Scope visitScope ();
	Pragma visitPragma ();
	Instruction visitReturn ();
	Instruction visitBreak ();
	Instruction visitWhile ();
	Instruction visitFor ();
	Instruction visitVersion ();	
	ModDecl visitModule (std::string&);
	Macro visitMacro (std::string&);
	MacroExpr visitMacroExpression (bool in_repeat = false);
	MacroRepeat visitMacroRepeat (bool);
	MacroVar visitMacroVar ();
	MacroToken visitMacroToken ();
	Expression visitMacroCall (Expression left);	
	std::vector <Word> visitAttributes ();	
	Declaration visitProto (Word, std::string &, std::string);
	std::vector<Declaration> visitExternBlock (Word, std::string, bool);
	Alias visitAlias (std::string & docs);
	Expression visitAutoCaster ();
	Trait visitTrait (std::string & docs);
	TraitProto visitTraitProto ();

	
	Word visitFromExtern ();
	Expression visitConstTuple ();
	Expression visitConstTuple (Expression begin);
	
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
	bool isInMatch;
	
	lexical::Lexer & lex;
	
    };

};
