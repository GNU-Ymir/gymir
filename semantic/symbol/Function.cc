#include <ymir/semantic/symbol/Function.hh>
#include <ymir/syntax/visitor/Keys.hh>

namespace semantic {

    Function::Function () :
	ISymbol (lexing::Word::eof ()),
	_table (this),
	_content (syntax::Declaration::empty ())
    {}

    Function::Function (const lexing::Word & name, const syntax::Function & func) :
	ISymbol (name),
	_table (this),
	_content (syntax::Declaration {func.clone ()})
    {}

    Function::Function (const Function & other) :
	ISymbol (other),
	_table (other._table.clone (this)), 
	_content (other._content)
    {}
    
    Symbol Function::init (const lexing::Word & name, const syntax::Function & func) {
	return Symbol {new Function (name, func)};
    }

    Symbol Function::clone () const {
	return Symbol {new Function (*this)};
    }

    bool Function::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Function thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);
    }

    void Function::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }
    
    void Function::replace (const Symbol & sym) {
	this-> _table.replace (sym);
    }

    std::vector<Symbol> Function::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table.get (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector <Symbol> Function::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }
    
    bool Function::equals (const Symbol & other) const {
	if (!other.is <Function> ()) return false;
	if (other.getName ().isSame (this-> getName ())) {
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

    const syntax::Function & Function::getContent () const {
	return this-> _content.to <syntax::Function> ();
    }

    std::string Function::getRealName () const {
	if (this-> getName ().str == Keys::MAIN) return this-> getName ().str;
	else return ISymbol::getRealName ();
    }
    
    std::string Function::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table.getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }
}
