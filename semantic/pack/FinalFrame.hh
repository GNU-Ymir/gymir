#pragma once

#include <ymir/utils/memory.hh>
#include <ymir/semantic/pack/Symbol.hh>
#include <ymir/semantic/pack/Namespace.hh>

#include <map>
#include <vector>
#include <string>

namespace syntax {
    class IVar;
    typedef IVar* Var;

    class IExpression;
    typedef IExpression* Expression;

    class IBlock;
    typedef IBlock* Block;
}

namespace semantic {

    class IFinalFrame  {
    private :

	Symbol _type;
	std::string _file;
	Namespace _space;
	std::string _name;
	std::vector <syntax::Var> _vars;
	std::vector <::syntax::Var> _closure;
	
	std::vector <syntax::Expression> _tmps;
	bool _isVariadic;
	bool _isForced = false;
	syntax::Block _block;

	static std::map <std::string, Ymir::Tree> __declared__;
	static std::vector <Ymir::Tree> __contextToAdd__;
	static Ymir::Tree __fn_decl__;
	static Ymir::Tree __fn_closure__;
	
    public:

	IFinalFrame (Symbol, Namespace, std::string, const std::vector<syntax::Var> &, syntax::Block, const std::vector<syntax::Expression>&) ;

	std::string& name ();

	Namespace space ();

	std::string& file ();

	Symbol type ();

	bool& isVariadic ();

	std::vector<syntax::Var> &vars ();
	
	std::vector <syntax::Var>& closure ();
	
	std::vector <syntax::Expression> &tmps ();

	void isForcedDelegate ();
	
	syntax::Block block ();	

	void finalize ();
	
	Ymir::Tree createClosureType ();

	static Ymir::Tree& currentFrame ();
	
	static Ymir::Tree getDeclaredType (const char * name);
	
	static Ymir::Tree getCurrentClosure ();

	static void declareType (std::string &name, Ymir::Tree type);

	static void declareType (const char * name, Ymir::Tree type);	

    private:

	void declArguments (Ymir::Tree);
	
    };

    typedef IFinalFrame* FinalFrame;
    
}
