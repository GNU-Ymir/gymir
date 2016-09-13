#pragma once

#include "Token.hh"
#include <list>
#include <cstdlib>

namespace Syntax {
    
    struct Ast {	
	Lexical::TokenPtr token;
	Ast (Lexical::TokenPtr token);
	virtual void print (int nb = 0) = 0;
	virtual ~Ast() {
	}
    };

    struct AstGC {

    public:
	static AstGC & instance () {
	    if (inst == NULL) inst = new AstGC ();
	    return *inst;
	}

	void addInfo (Ast*);
	void empty ();
	
    private:

	AstGC () {}
	AstGC (const AstGC &);
	AstGC & operator=(const AstGC &);
	
    private:

	std::list <Ast*> table;
	static AstGC * inst;
	
    };
    
    typedef Ast* AstPtr;	           

    struct Instruction : Ast {
	Instruction (Lexical::TokenPtr ptr) : Ast (ptr) {}
	virtual void print (int nb = 0);
    };    

    typedef Instruction* InstructionPtr;


    struct Expression : Instruction {
	Expression (Lexical::TokenPtr ptr) : Instruction (ptr) {}
	virtual void print (int nb = 0);
    };

    typedef Expression* ExpressionPtr;
    

    struct Declaration : Ast {
	Declaration (Lexical::TokenPtr ptr) : Ast(ptr) {}
	virtual void print (int nb = 0);
    };

    typedef Declaration * DeclarationPtr;
    

    struct Program : Ast {
	std::vector <DeclarationPtr> elems;
	Program (Lexical::TokenPtr token, std::vector<DeclarationPtr> elems)
	    : Ast (token),
	      elems (elems) {}
	
	virtual void print (int nb = 0);
    };
    

    struct Var;
    typedef Var * VarPtr;
    
    struct Var : Expression {
	std::vector<ExpressionPtr> templates;

	Var (Lexical::TokenPtr ptr) : Expression (ptr) {}
	Var (Lexical::TokenPtr ptr, std::vector<ExpressionPtr> templates)
	    : Expression (ptr),
	      templates (templates) {}

	static VarPtr empty () {
	    return VarPtr ();
	}
	
	virtual void print (int nb = 0);
	
    };
    
    struct TypedVar : Var {
	VarPtr type;

	TypedVar(Lexical::TokenPtr token, VarPtr type)
	    : Var (token),
	      type (type) {}
	
	virtual void print (int nb = 0);	
    };

    struct Binary : Expression {

	ExpressionPtr left;
	ExpressionPtr right;

	Binary (Lexical::TokenPtr token, ExpressionPtr left, ExpressionPtr right)
	    : Expression (token),
	      left (left),
	      right (right) {}
	
	virtual void print (int nb = 0);	
    };

    struct AfUnary : Expression {
	ExpressionPtr elem;
	AfUnary (Lexical::TokenPtr token, ExpressionPtr elem)
	    : Expression (token),
	      elem (elem) {}
	
	virtual void print (int nb = 0);
    };


    struct BefUnary : Expression {
	ExpressionPtr elem;
	BefUnary (Lexical::TokenPtr token, ExpressionPtr elem)
	    : Expression (token),
	      elem (elem) {}
	virtual void print (int nb = 0);
    };
        
    struct Cast : Expression {
	VarPtr type;
	ExpressionPtr inside;
	Cast (Lexical::TokenPtr token, VarPtr type, ExpressionPtr inside)
	    : Expression (token),
	      type (type),
	      inside (inside) {}
	
	virtual void print (int nb = 0);
    };

    struct New : Expression {
	VarPtr type;
	ExpressionPtr size;
	New (Lexical::TokenPtr token, VarPtr type, ExpressionPtr inside)
	    : Expression (token),
	      type (type),
	      size (inside) {}
	
	virtual void print (int nb = 0);
    };

    struct InsideIf : Expression {
	ExpressionPtr test;
	ExpressionPtr if_bl;
	ExpressionPtr else_bl;

	InsideIf (Lexical::TokenPtr token, ExpressionPtr test, ExpressionPtr if_bl, ExpressionPtr else_bl)
	    : Expression (token),
	      test (test),
	      if_bl (if_bl),
	      else_bl (else_bl) {}
	
	virtual void print (int nb = 0);
    };
    
    struct InsideFor : Expression {
	ExpressionPtr var;
	ExpressionPtr iter;
	ExpressionPtr value;

	InsideFor (Lexical::TokenPtr token, ExpressionPtr var, ExpressionPtr iter, ExpressionPtr value)
	    : Expression (token),
	      var (var),
	      iter (iter),
	      value (value) {}
	
	virtual void print (int nb = 0);
    };

    struct Int : Expression {
	Int (Lexical::TokenPtr token)
	    : Expression (token) {}
	
	virtual void print (int nb = 0);
    };

    struct Float : Expression {
	std::string suite;
	
	Float (Lexical::TokenPtr token)
	    : Expression (token) {}

	Float (Lexical::TokenPtr token, std::string suite)
	    : Expression (token),
	      suite (suite) {}
	
	virtual void print (int nb = 0);
	
    };

    struct Bool : Expression {
	Bool (Lexical::TokenPtr token)
	    : Expression (token) {}
	
	virtual void print (int nb = 0);
    };

    struct Null : Expression {
	Null (Lexical::TokenPtr token)
	    : Expression (token) {}
	
	virtual void print (int nb = 0);
    };

    struct Char : Expression {
	short value;
	Char (Lexical::TokenPtr token, short value)
	    : Expression (token),
	      value (value)
	{}
	virtual void print (int nb = 0);
    };

    struct String : Expression {
	std::string content;
	String (Lexical::TokenPtr token, std::string content)
	    : Expression (token),
	      content (content) {}

	virtual void print (int nb = 0);
    };

    
    struct ParamList : Expression {	
	std::vector <ExpressionPtr> params;
	ParamList (Lexical::TokenPtr token, std::vector<ExpressionPtr> params)
	    : Expression (token),
	      params (params) {}
	virtual void print (int nb = 0);
    };

    struct Par : Expression {
	ExpressionPtr left;
	ExpressionPtr right;
	Par (Lexical::TokenPtr token, ExpressionPtr left, ExpressionPtr right)
	    : Expression (token),
	      left (left),
	      right (right)
	{}
	virtual void print (int nb = 0);
    };

    struct Access : Expression {
	ExpressionPtr left;
	ExpressionPtr right;
	Access (Lexical::TokenPtr token, ExpressionPtr left, ExpressionPtr right)
	    : Expression (token),
	      left (left),
	      right (right)
	{}
	virtual void print (int nb = 0);
    };    
    
    struct Dot : Expression {	
	ExpressionPtr left;
	VarPtr right;
	Dot (Lexical::TokenPtr token, ExpressionPtr left, VarPtr right)
	    : Expression (token),
	      left (left),
	      right (right) {}
	virtual void print (int nb = 0);
    };
    

    struct Block : Instruction {
	std::vector<DeclarationPtr> decls;
	std::vector <InstructionPtr> instructions;	
	Block (Lexical::TokenPtr ptr, std::vector<DeclarationPtr> decls, std::vector <InstructionPtr> insts)
	    : Instruction (ptr),
	      decls (decls),
	      instructions (insts) {}
	virtual void print (int nb = 0);
    };
    
    typedef Block* BlockPtr;

    struct If : Instruction {
	ExpressionPtr test;
	BlockPtr block;
	InstructionPtr else_bl;
	If (Lexical::TokenPtr token, ExpressionPtr test, BlockPtr block)
	    : Instruction (token),
	      test (test),
	      block (block),
	      else_bl (NULL) {}

	If (Lexical::TokenPtr token, ExpressionPtr test, BlockPtr block, InstructionPtr else_bl)
	    : Instruction (token),
	      test (test),
	      block (block),
	      else_bl (else_bl) {}
	virtual void print (int nb = 0);
    };        

    struct ElseIf : Instruction {
	InstructionPtr if_bl;
	ElseIf (Lexical::TokenPtr token, InstructionPtr if_bl)
	    : Instruction (token),
	      if_bl (if_bl) {}
	virtual void print (int nb = 0);
    };
    
    struct Else : Instruction {
	BlockPtr block;
	Else (Lexical::TokenPtr token, BlockPtr block)
	    : Instruction (token),
	      block (block)
	{}
	virtual void print (int nb = 0);
    };


    struct Return : Instruction {
	ExpressionPtr ret;
	Return(Lexical::TokenPtr token)
	    : Instruction (token) {}

	Return (Lexical::TokenPtr token, ExpressionPtr ret)
	    : Instruction (token),
	      ret (ret) {}
	virtual void print (int nb = 0);
    };

    struct For : Instruction {
	std::vector<ExpressionPtr> inits;
	ExpressionPtr test;
	std::vector<ExpressionPtr> iter;
	BlockPtr block;
	For (Lexical::TokenPtr token, std::vector<ExpressionPtr> inits, ExpressionPtr test, std::vector<ExpressionPtr> iter, BlockPtr block) 
	    : Instruction (token),
	      inits (inits),
	      test (test),
	      iter (iter),
	      block (block)
	{}
	virtual void print (int nb = 0);

    };

    struct Foreach : Instruction {
	ExpressionPtr var;
	ExpressionPtr iter;
	BlockPtr block ;
	Foreach (Lexical::TokenPtr token, ExpressionPtr var, ExpressionPtr iter, BlockPtr block)
	    : Instruction (token),
	      var (var),
	      iter (iter),
	      block (block) {}
	virtual void print (int nb = 0);
    };

    struct While : Instruction {
	ExpressionPtr test;
	BlockPtr block;
	While (Lexical::TokenPtr token, ExpressionPtr test, BlockPtr block)
	    : Instruction (token),
	      test (test),
	      block (block) {}
	virtual void print (int nb = 0);
    };

    struct Break : Instruction {
	Break (Lexical::TokenPtr token)
	    : Instruction (token) {}
	virtual void print (int nb = 0);
    };

    struct Delete : Instruction {
	ExpressionPtr elem;
	Delete (Lexical::TokenPtr token, ExpressionPtr elem)
	    : Instruction (token),
	      elem (elem) {}
	virtual void print (int nb = 0);
    };
    
    
    struct Function : Declaration {
	
	VarPtr type = Var::empty ();
	std::vector <VarPtr> params;
	BlockPtr block;
	
	Function (Lexical::TokenPtr info, VarPtr type, std::vector<VarPtr> params, BlockPtr block)
	    : Declaration (info),
	      type (type),
	      params (params),
	      block (block) {}

	    Function (Lexical::TokenPtr info, std::vector<VarPtr> params, BlockPtr block)
	    : Declaration (info),
	      params (params),
	      block (block) {}
	
	virtual void print (int nb = 0);
	
    };
    
};

