#include <ymir/semantic/symbol/Class.hh>

namespace semantic {

    Class::Class () :
	ISymbol (lexing::Word::eof ()), 
	_table (ITable::init (this)),
	_overload (),
	_ancestor (syntax::Expression::empty ())
    {}

    Class::Class (const lexing::Word & name, const syntax::Expression & ancestor) :
	ISymbol (name),
	_table (ITable::init (this)),
	_overload (),
	_ancestor (ancestor)
    {}

    Symbol Class::init (const lexing::Word & name, const syntax::Expression & ancestor) {
	return Symbol {new (Z0) Class (name, ancestor)};
    }

    Symbol Class::clone () const {
	return Class::init (this-> getName (), this-> _ancestor);
    }

    
    bool Class::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Class thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    const std::vector <Symbol> & Class::getOverloading () const {
	return this-> _overload;
    }

    void Class::setOverloading (const std::vector <Symbol> & overs) {
	this-> _overload = overs;
    }

    void Class::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }

    std::vector <Symbol> Class::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	const Symbol & local = this-> _table.get (name);
	if (!local.isEmpty ())
	    vec.push_back (local);
	return vec;
    }

    const Symbol & Class::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }

    bool Class::equals (const Symbol & other) const {
	if (!other.is <Class> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else return false;
    }

}
