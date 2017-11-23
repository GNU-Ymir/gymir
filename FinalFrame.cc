#include <ymir/semantic/pack/FinalFrame.hh>

namespace semantic {
    IFinalFrame::IFinalFrame (Symbol type, Namespace space, std::string name, std::vector <syntax::Var> vars, syntax::Block bl, std::vector <syntax::Expression> tmps) :
	_type (type),
	_file (""),
	_space (space),
	_name (name),
	_vars (vars),
	_tmps (tmps),
	_isVariadic (false),
	_block (bl)
    {}
    
    std::string IFinalFrame::name () {
	return this-> _name;
    }

    Namespace IFinalFrame::space () {
	return this-> _space;
    }

    std::string& IFinalFrame::file () {
	return this-> _file;
    }

    Symbol IFinalFrame::type () {
	return this-> _type;
    }

    bool& IFinalFrame::isVariadic () {
	return this-> _isVariadic;
    }

    std::vector<syntax::Var> IFinalFrame::vars () {
	return this-> _vars;
    }

    std::vector <syntax::Expression> IFinalFrame::tmps () {
	return this-> _tmps;
    }

    syntax::Block IFinalFrame::block () {
    	return this-> _block;
    }

    
    
}
