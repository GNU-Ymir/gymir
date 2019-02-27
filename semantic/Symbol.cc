#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/utils/Path.hh>
#include <ymir/semantic/symbol/Module.hh>

namespace semantic {

    Symbol Symbol::__empty__ (Symbol::empty ());
        
    std::map <std::string, Symbol> Symbol::__imported__;
    
    ISymbol::ISymbol () :
	_name (lexing::Word::eof ()),
	_referent (nullptr)
    {}

    ISymbol::ISymbol (const lexing::Word & name)
	: _name (name),
	  _referent (nullptr)
    {}

    bool ISymbol::isOf (const ISymbol *) const {
	return false;
    }
    
    const lexing::Word & ISymbol::getName () const {
	return this-> _name;
    }

    void ISymbol::setName (const std::string & name) {
	this-> _name = {this-> _name, name};
    }
    
    void ISymbol::insert (const Symbol &) {
	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::INSERT_NO_TABLE));
    }

    void ISymbol::replace (const Symbol &) {
	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::INSERT_NO_TABLE));
    }

    std::vector <Symbol> ISymbol::get (const std::string & name) const {
	return this-> getReferent ().get (name);
    }

    std::vector <Symbol> ISymbol::getLocal (const std::string &) const {
	return {};
    }    

    Symbol ISymbol::getReferent () const {
	return Symbol {this-> _referent};
    }
    
    void ISymbol::setReferent (const Symbol & ref) {
	this-> _referent = ref.getRef ();
    }
    
    std::string ISymbol::getRealName () const {
	if (this-> _referent == nullptr) return this-> _name.str;
	else {
	    auto ft = this-> _referent-> getRealName ();
	    if (ft != "")
		return ft + "::" + this-> _name.str;
	    else return this-> _name.str;
	}
    }
    
    ISymbol::~ISymbol () {}

    Symbol::Symbol (ISymbol * value) : RefProxy<ISymbol, Symbol> (value)
    {}
    
    Symbol Symbol::empty () {
	return Symbol {nullptr};
    }
    
    bool Symbol::isEmpty () const {
	return this-> _value == nullptr;
    }

    const lexing::Word & Symbol::getName () const {	
	if (this-> _value == nullptr)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return this-> _value-> getName ();
    }

    void Symbol::setName (const std::string & name) {	
	if (this-> _value == nullptr)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return this-> _value-> setName (name);
    }

    void Symbol::insert (const Symbol & sym) {
	if (this-> _value != nullptr)
	    this-> _value-> insert (sym);
	else {
	    // We don't do anything, it is more convinient for global modules
	}
    }

    void Symbol::replace (const Symbol & sym) {
	if (this-> _value != nullptr)
	    this-> _value-> replace (sym);
	else {
	    // We don't do anything, it is more convinient for global modules
	}
    }
    
    std::vector <Symbol> Symbol::get (const std::string & name) const {
	if (this-> _value == nullptr)
	    return {};
	return this-> _value-> get (name);
    }

    Symbol Symbol::getReferent () const {
	if (this-> _value == nullptr)
	    return Symbol::__empty__;
	return this-> _value-> getReferent ();
    }

    void Symbol::setReferent (const Symbol & ref) {
	if (this-> _value != nullptr)
	    this-> _value-> setReferent (ref);
    }
    
    std::string Symbol::getRealName () const {
	if (this-> _value != nullptr) return this-> _value-> getRealName ();
	else {
	    return "";
	}
    }

    std::vector <Symbol> Symbol::getLocal (const std::string & name) const {
	if (this-> _value == nullptr) return {};
	else return this-> _value-> getLocal (name);
    }
    
    bool Symbol::equals (const Symbol & other) const {
	if (this-> _value == nullptr) return other._value == nullptr;
	return this-> _value-> equals (other);
    }

    std::string Symbol::formatTree (int padd) const {
	if (this-> _value == nullptr) return "";
	return this-> _value-> formatTree (padd);
    }
    
    const Symbol & Symbol::getModule (const std::string & name) {
	auto sym = __imported__.find (name);
	if (sym == __imported__.end ()) return Symbol::__empty__;
	else return sym-> second;
    }

    Symbol Symbol::getModuleByPath (const std::string & path_) {
	auto path = Ymir::Path {path_, "::"};
	auto mod = Symbol::getModule (path.getFiles () [0]);
	auto files = path.getFiles ();
	auto submods = std::vector <std::string> {files.begin () + 1, files.end ()};
	for (auto & name : submods) {
	    auto inners = mod.getLocal (name);
	    bool succeed = false;
	    for (auto & inner_sym : inners) {
		if (inner_sym.is <Module> ()) {
		    succeed = true;
		    mod = inner_sym;
		}
	    }
	    if (!succeed) return Symbol::__empty__;
	}
	return mod;
    }
    
    void Symbol::registerModule (const std::string & name, const Symbol & sym) {
	__imported__.erase (name); // We start by erase it, if it exists
	__imported__.emplace (name, sym);
    }

    const std::map <std::string, Symbol> & Symbol::getAllModules () {
	return __imported__;
    }

    void Symbol::clearModule () {
	__imported__.clear ();
    }
    
}
