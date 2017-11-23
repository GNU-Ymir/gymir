#include "semantic/pack/Scope.hh"
#include "semantic/pack/Table.hh"
#include <algorithm>
#include <cmath>

namespace semantic {

    Scope::Scope () {}

    Symbol Scope::get (std::string name) {
	auto it = this-> local.find (name);
	if (it != this-> local.end ()) {
	    return it-> second [0];
	} else return NULL;
    }    

    void Scope::set (std::string name, Symbol sym) {
	auto it = this-> local.find (name);
	if (it != this-> local.end ()) {
	    it-> second [0] = (sym);
	} else this-> local [name] = {sym};
    }   
    
    std::vector <Symbol> Scope::getAll (std::string name) {
	auto it = this-> local.find (name);
	if (it != this-> local.end ()) {
	    return it-> second;
	} else return {};
    }

    void Scope::addOpen (Namespace space) {
	this-> imports.push_back (space);
    }

    unsigned int levenshteinDistance (std::string fst, std::string scd) {
	auto len1 = fst.size (), len2 = scd.size ();
	std::vector <unsigned int> col (len2 + 1), prevCol (len2 + 1);

	for (unsigned int i = 0 ; i < prevCol.size (); i++) {
	    prevCol [i] = i;
	}
	for (unsigned int i = 0 ; i < len1 ; i++) {
	    col [0] = i + 1;
	    for (unsigned int j = 0 ; j < len2 ; j++) {
		col [j + 1] = std::min ({prevCol [1 + j] + 1, col [j] + 1,  prevCol [j] + (fst [i] == scd [i] ? 0 : 1)});
		col.swap (prevCol);
	    }
	}
	return prevCol [len2];
    }
    
    Symbol Scope::getAlike (std::string name) {
	auto min = 3UL;
	Symbol ret = NULL;
	for (auto& it : this-> local) {
	    auto diff = levenshteinDistance (it.first, name);
	    if (diff < min) {
		ret = it.second [0];
		min = diff;
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

}
