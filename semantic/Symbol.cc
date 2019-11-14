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

    void ISymbol::insertTemplate (const Symbol &) {
	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::INSERT_NO_TABLE));
    }

    void ISymbol::replace (const Symbol &) {
	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::INSERT_NO_TABLE));
    }

    void ISymbol::use (const std::string & name, const Symbol & sym) {
	auto ptr = this-> _used.find (name);
	if (ptr == this-> _used.end ()) {
	    this-> _used.emplace (name, sym);
	} else {
	    ptr-> second = sym;
	}
    }

    void ISymbol::unuse (const std::string & name) {
	this-> _used.erase (name);
    }

    void ISymbol::setPublic () {
	this-> _isPublic = true;
    }

    bool ISymbol::isPublic () const {
	return this-> _isPublic;
    }
    
    std::vector <Symbol> ISymbol::get (const std::string & name) const {
	return this-> getReferent ().get (name);
    }

    std::vector <Symbol> ISymbol::getPublic (const std::string & name) const {
	return this-> getReferent ().getPublic (name);
    }

    std::vector <Symbol> ISymbol::getLocal (const std::string &) const {
	return {};
    }    

    const std::map <std::string, Symbol> & ISymbol::getUsedSymbols () const {
	return this-> _used;
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
    
    std::string ISymbol::getMangledName () const {
	if (this-> _referent == nullptr) return this-> _name.str;
	else {
	    auto ft = this-> _referent-> getMangledName ();
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

    void Symbol::insertTemplate (const Symbol & sym) {
	if (this-> _value != nullptr) {
	    this-> _value-> insertTemplate (sym);
	} else {
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
    
    void Symbol::use (const std::string & name, const Symbol & sym) {
	if (this-> _value != nullptr)
	    this-> _value-> use (name, sym);
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	}
    }    

    void Symbol::unuse (const std::string & name) {
	if (this-> _value != nullptr)
	    this-> _value-> unuse (name);
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	}
    }

    void Symbol::setPublic () {
	if (this-> _value != nullptr)
	    this-> _value-> setPublic ();
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	}
    }

    bool Symbol::isPublic () const {
	if (this-> _value != nullptr)
	    return this-> _value-> isPublic ();
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	    return false;
	}
    }

    
    std::vector <Symbol> Symbol::get (const std::string & name) const {
	if (this-> _value == nullptr)
	    return {};

	auto ret = this-> _value-> get (name);
	for (auto & it : this-> _value-> getUsedSymbols ()) {
	    auto local_ret = it.second.getUsed (name);
	    ret.insert (ret.end (), local_ret.begin (), local_ret.end ());
	}
	
	return Symbol::mergeEqSymbols (ret);
    }

    std::vector <Symbol> Symbol::getPublic (const std::string & name) const {
	if (this-> _value == nullptr)
	    return {};

	auto ret = this-> _value-> getPublic (name);
	for (auto & it : this-> _value-> getUsedSymbols ()) {
	    if (it.second.isPublic ()) {
		auto local_ret = it.second.getPublic (name);
		ret.insert (ret.end (), local_ret.begin (), local_ret.end ());
	    }	    
	}
	
	return Symbol::mergeEqSymbols (ret);
    }
    
    std::vector <Symbol> Symbol::getUsed (const std::string & name) const {
	if (this-> _value == nullptr)
	    return {};
	
	auto ret = this-> _value-> getPublic (name);
	for (auto & it : this-> _value-> getUsedSymbols ()) {
	    if (it.second.isPublic ()) {
		auto local_ret = it.second.getUsed (name);
		ret.insert (ret.end (), local_ret.begin (), local_ret.end ());
	    }
	}
	
	return Symbol::mergeEqSymbols (ret);
    }

    const std::map <std::string, Symbol> & Symbol::getUsedSymbols () const {
	if (this-> _value != nullptr)
	    return this-> _value-> getUsedSymbols ();
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	    return Symbol::__empty__.getUsedSymbols (); // ....
	    // We don't get there anyway
	}
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

    std::string Symbol::getMangledName () const {
	if (this-> _value != nullptr) return this-> _value-> getMangledName ();
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

    bool Symbol::isSameRef (const Symbol & other) const {
	if (this-> _value == nullptr) return false;
	return this-> _value == other._value;
    }

    std::string Symbol::formatTree (int padd) const {
	if (this-> _value == nullptr) return "";
	auto ret = this-> _value-> formatTree (padd);
	if (this-> _value-> getUsedSymbols ().size () != 0) {
	    Ymir::OutBuffer buf (ret);
	    buf.writefln ("%*- %", padd, "|\t", "USED : ");
	    for (auto & it : this-> _value-> getUsedSymbols ()) {
		buf.writefln ("%*- %", padd + 1, "|\t", it.first);
	    }	
	    return buf.str ();
	} return ret;
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

    std::vector <Symbol> Symbol::mergeEqSymbols (const std::vector <Symbol> & multSym) {
	std::vector <Symbol> result;
	for (auto & it : multSym) {
	    bool add = true;
	    for (auto & zt : result)
		if (zt.isSameRef (it)) { add = false; break; }
	    if (add)
		result.push_back (it);
	}
	return result;
    }

    
}
