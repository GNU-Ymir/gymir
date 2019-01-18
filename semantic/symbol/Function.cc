
#include <ymir/semantic/symbol/Function.hh>


namespace semantic {

    Function::Function () :
	ISymbol (lexing::Word::eof ()),
	_table (ITable::init (this)),
	_overload (),
	_content (syntax::Declaration::empty ())
    {}

    Function::Function (const lexing::Word & name, const syntax::Function & func) :
	ISymbol (name),
	_table (ITable::init (this)),
	_overload (),
	_content (syntax::Declaration {func.clone ()})
    {}

    Symbol Function::init (const lexing::Word & name, const syntax::Function & func) {
	return Symbol {new (Z0) Function (name, func)};
    }

    Symbol Function::clone () const {
	return Symbol {new (Z0) Function (*this)};
    }

    bool Function::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Function thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);
    }

    const std::vector <Symbol> & Function::getOverloading () const {
	return this-> _overload;
    }
    
    void Function::setOverloading (const std::vector <Symbol> & overs) {
	this-> _overload = overs;
    }

    void Function::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }

    std::vector<Symbol> Function::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	const Symbol & local = this-> _table.get (name);
	if (!local.isEmpty ())
	    vec.push_back (local);
	return vec;
    }

    const Symbol & Function::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }
    
    bool Function::equals (const Symbol & other) const {
	if (!other.is <Function> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else
	    return false;
    }

    void Function::isPure (bool is) {
	this-> _isPure = is;
    }

    void Function::isInline (bool is) {
	this-> _isInline = is;
    }

    void Function::isSafe (bool is) {
	this-> _isSafe = is;
    }
    
}
