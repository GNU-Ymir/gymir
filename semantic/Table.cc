#include <ymir/semantic/Table.hh>

namespace semantic {

    Table::Table (ISymbol * attach) :
	_attached (attach)
    {
    }
    
    Table Table::init (ISymbol * attach) {
	return Table (attach);
    }

    Table Table::clone (ISymbol * attach) const {
	Table ret (attach);
	for (auto it : Ymir::r (0, this-> _syms.size ())) {	    
	    auto cl = this-> _syms [it];
	    cl.setReferent (ret._attached);
	    ret. _syms.push_back (cl);
	}

	return ret;
    }

    ISymbol * Table::getAttach () {
	return this-> _attached;
    }

    void Table::setAttach (ISymbol * attach) {
	this-> _attached = attach;
    }
    
    void Table::insert (const Symbol & sym) {
	Symbol toInsert = sym;
	toInsert.setReferent (this-> _attached);
	this-> _syms.push_back (toInsert);
    }

    void Table::replace (const Symbol & sym) {
	Symbol toInsert = sym;
	toInsert.setReferent (this-> _attached);
	for (auto it : Ymir::r (0, this-> _syms.size ())) {
	    if (this-> _syms [it].getName ().str == sym.getName ().str) {
		this-> _syms [it] = sym;
		return;
	    }
	}
	this-> _syms.push_back (sym);
    }
    
    std::vector <Symbol> Table::get (const std::string & name) const {
	std::vector <Symbol> rets;
	for (auto & s : this-> _syms) {
	    if (s.getName ().str == name) {
		rets.push_back (s);
	    }
	}
	
	return rets;
    }

    const std::vector <Symbol> & Table::getAll () const {
	return this-> _syms;
    }
}
