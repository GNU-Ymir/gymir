#include "semantic/pack/Scope.hh"
#include "semantic/pack/Table.hh"
#include <algorithm>
#include "spellcheck.h"
#include <cmath>

namespace semantic {

    Scope::Scope () {}

    Symbol Scope::get (std::string name) {
	auto it = this-> local.find (name);
	if (it != this-> local.end ()) {
	    return it-> second [0];
	} else return NULL;
    }    

    Symbol Scope::getPublic (std::string name) {
	auto it = this-> local.find (name);	
	if (it != this-> local.end () && it-> second [0]-> isPublic ()) {
	    return it-> second [0];
	} else return NULL;
    }    
    
    void Scope::set (std::string name, Symbol sym) {
	auto it = this-> local.find (name);
	if (it != this-> local.end ()) {
	    it-> second.push_back (sym);
	} else this-> local [name] = {sym};
    }   
    
    std::vector <Symbol> Scope::getAll (std::string name) {
	auto it = this-> local.find (name);
	if (it != this-> local.end ()) {
	    return it-> second;
	} else return {};
    }
    
    std::vector <Symbol> Scope::getAllPublic (std::string name) {
	auto it = this-> local.find (name);
	if (it != this-> local.end ()) {
	    std::vector <Symbol> ret;
	    for (auto it_ : it-> second)
		if (it_-> isPublic ()) ret.push_back (it_);
	    return ret;
	} else return {};
    }
    
    void Scope::addOpen (Namespace space) {
	this-> imports.push_back (space);
    }

    size_t min (size_t a, size_t b) {
	return a < b ? a : b;
    }
               
    Symbol Scope::getAlike (std::string name) {
	auto min = 3UL < name.length () ? 3UL : name.length ();
	Symbol ret = NULL;
	for (auto& it : this-> local) {
	    auto diff = get_edit_distance (it.first.c_str (), it.first.length (), name.c_str (), name.length ());
	    if (diff < min && diff < name.length ()) {
		ret = it.second [0];
		min = diff;
	    }
	}
	return ret;
    }

    Symbol Scope::getPublicAlike (std::string name) {
	auto min = 3UL;
	Symbol ret = NULL;
	for (auto& it : this-> local) {
	    if (it.second[0]->  isPublic ()) {
		auto diff = get_edit_distance (it.first.c_str (), it.first.length (), name.c_str (), name.length ());
		if (diff < min && diff < name.length ()) {
		    ret = it.second [0];
		    min = diff;
		}
	    }
	}
	return ret;
    }
    
    void Scope::clear () {
	this-> local.clear ();
    }

    
    void Scope::quit (Namespace space) {
	for (auto it : this-> local) {
	    for (auto it_ : it.second) it_-> quit (space);
	}

	for (auto space_ : this-> imports) {
	    Table::instance ().closeModuleForSpace (space, space_);
	}	
    }

    std::string Scope::toString () {
	Ymir::OutBuffer buf ("syms : {\n");
	for (auto it : this-> local) {
	    buf.write ("\t", it.first, " : [");
	    /*for (auto it_ : it.second)
	      buf.write (it_-> typeString ());*/
	    buf.write ("]\n");
	}
	buf.write ("}");
	return buf.str ();
    }
    
}
