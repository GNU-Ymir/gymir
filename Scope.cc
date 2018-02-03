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
    
    size_t levenshteinDistance(const char* s, size_t n, const char* t, size_t m) {
	++n; ++m;
	size_t* d = new (Z0) size_t[n * m];
	memset(d, 0, sizeof(size_t) * n * m);
	for (size_t i = 1, im = 0; i < m; ++i, ++im) {
	    for (size_t j = 1, jn = 0; j < n; ++j, ++jn) {
		if (s[jn] == t[im]) {
		    d[(i * n) + j] = d[((i - 1) * n) + (j - 1)];
		} else {
		    d[(i * n) + j] = min (d[(i - 1) * n + j] + 1, /* A deletion. */
					 min (d[i * n + (j - 1)] + 1, /* An insertion. */
					     d[(i - 1) * n + (j - 1)] + 1)); /* A substitution. */
		}
	    }	
	}	
	size_t r = d[n * m - 1];
	delete [] d;
	return r;
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
	    auto diff = levenshteinDistance (it.first.c_str (), it.first.length (), name.c_str (), name.length ());
	    if (diff < min && diff < name.length ()) {
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
