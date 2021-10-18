#include <ymir/semantic/Table.hh>

namespace semantic {

    Table::Table (const Symbol & attach) :
	_attached (attach.getPtr ())
    {}

    std::shared_ptr <Table> Table::init (const Symbol & attach) {
	return std::make_shared <Table> (attach);
    }
    
    // std::shared_ptr<Table> Table::clone (const Symbol & attach) const {
    // 	Table ret (attach);
    // 	for (auto & it : this-> _syms) {
    // 	    auto & jt = ret._syms [it.first];
    // 	    for (auto & cl : it.second) {
		
    // 	    }
    // 	}
    // 	for (auto it : Ymir::r (0, this-> _syms.size ())) {	    
    // 	    auto cl = this-> _syms [it];
    // 	    cl.setReferent (ret._attached);
    // 	    ret. _syms.push_back (cl);
    // 	}

    // 	return std::make_shared<Table> (ret);
    // }

    Symbol Table::getAttach () {
	return Symbol {this-> _attached};
    }
    
    void Table::insert (const Symbol & sym) {	
	Symbol toInsert = sym;
	toInsert.setReferent (Symbol {this-> _attached});
	auto it = this-> _syms.find (toInsert.getName ().getStr ());
	if (it != this-> _syms.end ()) {
	    it-> second.push_back (toInsert);
	} else {
	    this-> _syms [toInsert.getName ().getStr ()].push_back (toInsert);
	}

	if (sym.isPublic ()) {
	    auto it = this-> _public_syms.find (toInsert.getName ().getStr ());
	    if (it != this-> _public_syms.end ()) {
		it-> second.push_back (toInsert);
	    } else {
		this-> _public_syms [toInsert.getName ().getStr ()].push_back (toInsert);
	    }   
	}
	
	println ("Insert : ", sym.getName ());
	this-> _all.push_back (sym);	
    }
    
    void Table::insertTemplate (const Symbol & sym) {
	Symbol toInsert = sym;
	toInsert.setReferent (Symbol {this-> _attached});
	this-> _templates.push_back (toInsert);
    }

    const std::vector <Symbol> & Table::getTemplates () const {
	return this-> _templates;
    }
    
    void Table::replace (const Symbol & sym) {
    	this-> _syms.erase (sym.getName ().getStr ());
    	this-> _public_syms.erase (sym.getName ().getStr ());
    	this-> insert (sym);
    	this-> recomputeAll ();
    }
       
    void Table::get (const std::string & name, std::vector <Symbol> & rets) const {
	auto it = this-> _syms.find (name);
	if (it != this-> _syms.end ()) {
	    rets.insert (rets.end (), it-> second.begin (), it-> second.end ());
	}
    }

    void Table::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	auto it = this-> _public_syms.find (name);
	if (it != this-> _public_syms.end ()) {
	    rets.insert (rets.end (), it-> second.begin (), it-> second.end ());
	}
    }
    
    const std::vector <Symbol> & Table::getAll () const {
	return this-> _all;
    }

    void Table::prune () {
	this-> _all.clear ();
	this-> _syms.clear ();
	this-> _public_syms.clear ();
	this-> _templates.clear ();
    }
    
    void Table::recomputeAll () {
	this-> _all.clear ();
	for (auto & it : this-> _syms) {
	    this-> _all.insert (this-> _all.end (), it.second.begin (), it.second.end ());
	}
    }
}
