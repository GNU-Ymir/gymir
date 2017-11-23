#pragma once

#include <gc/gc_cpp.h>
#include <ymir/semantic/pack/Symbol.hh>
#include <ymir/semantic/pack/Namespace.hh>
#include <ymir/ast/Var.hh>
#include <vector>
#include <string>

namespace semantic {

    class IFinalFrame : public gc {
    private :

	Symbol _type;
	std::string _file;
	Namespace _space;
	std::string _name;
	std::vector <syntax::Var> _vars;
	std::vector <syntax::Expression> _tmps;
	bool _isVariadic;
	syntax::Block _block;

    public:

	IFinalFrame (Symbol, Namespace, std::string, std::vector <syntax::Var>, syntax::Block, std::vector<syntax::Expression>) ;

	std::string name ();

	Namespace space ();

	std::string& file ();

	Symbol type ();

	bool& isVariadic ();

	std::vector<syntax::Var> vars ();

	std::vector <syntax::Expression> tmps ();

	syntax::Block block ();	
	
    };

    typedef IFinalFrame* FinalFrame;
    
}
