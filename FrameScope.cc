#include "FrameScope.hh"

namespace Semantic {

    FrameScope::FrameScope (const std::string &space)
	: space_name (space)
    {
	local.push_front (Scope());
    }

    void FrameScope::enterBlock () {
	local.push_front (Scope ());
    }

    void FrameScope::quitBlock () {
	local.pop_front ();
    }

    void FrameScope::addImport (Package * pck) {
	this->local.front ().addImport(pck);
    }

    void FrameScope::insert (const std::string & name, SymbolPtr sym) {
	this->local.front ().insert (name, sym);
    }
    
    SymbolPtr FrameScope::get (const std::string & name) {
	for (auto &it : local) {
	    auto is = it.get (name);
	    if (!is-> isVoid ()) return is;
	}
	return Symbol::empty ();
    }

    std::string FrameScope::space () const {
	return (this->space_name);
    }



}
