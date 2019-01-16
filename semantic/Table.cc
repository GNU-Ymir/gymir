#include <ymir/semantic/Table.hh>


namespace semantic {

    ITable::ITable (ISymbol * attach) :
	_attached (attach),
	_proxy (Symbol::empty ())
    {
	this-> _proxy.setRef (attach);
    }
    
    Table ITable::init (ISymbol * attach) {
	return Table {new (Z0) ITable (attach)};
    }

    Table ITable::clone () {
	auto ret = new (Z0) ITable (this-> _attached);
	ret-> _syms = this-> _syms;
	return ret;
    }

    Symbol & ITable::getAttach () {
	return this-> _proxy;
    }

    void ITable::setAttach (ISymbol * attach) {
	this-> _proxy.setRef (attach);
    }
    
    void ITable::insert (const Symbol & sym) {
	auto & name  = sym.getName ().str;
	this-> _syms.emplace (name, sym);
    }

    const Symbol & ITable::get (const std::string & name) const {
	auto ptr = this-> _syms.find (name);
	if (ptr != this-> _syms.end ()) return ptr-> second;
	else return Symbol::__empty__;
    }

    Table::Table (ITable * table) : Proxy<ITable, Table> (table)
    {}

    Table Table::init (ISymbol * sym) {
	return ITable::init (sym);
    }

    Symbol & Table::getAttach () {
	if (this-> _value == nullptr)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return this-> _value-> getAttach ();
    }
   
    void Table::setAttach (ISymbol * attach) {
	if (this-> _value == nullptr)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return this-> _value-> setAttach (attach);
    }
    
    Table Table::empty () {
	return Table {nullptr};
    }

    void Table::insert (const Symbol & sym) {	
	if (this-> _value == nullptr)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	this-> _value-> insert (sym);
    }
    
    const Symbol & Table::get (const std::string & name) const {	
	if (this-> _value == nullptr)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return this-> _value-> get (name);
    }
    
}
