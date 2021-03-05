#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/utils/Path.hh>
#include <ymir/semantic/symbol/Module.hh>
#include <ymir/semantic/symbol/TemplateSolution.hh>
#include <ymir/semantic/generator/Mangler.hh>

namespace semantic {

    ulong __GLOBAL_TIME__ = 0;
    
    Symbol Symbol::__empty__ (Symbol::empty ());
        
    std::map <std::string, Symbol> Symbol::__imported__;
    std::map <std::string, Symbol> Symbol::__fast_mod_access__;
      
    ISymbol::ISymbol (const lexing::Word & name, const std::string & comments, bool isWeak)
	: _name (name),
	  _comments (comments),
	  _isWeak (isWeak)
    {}
        
    const lexing::Word & ISymbol::getName () const {
	return this-> _name;
    }

    const std::string & ISymbol::getComments () const {
	return this-> _comments;
    }
        
    void ISymbol::insert (const Symbol &) {
	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::INSERT_NO_TABLE));
    }

    void ISymbol::insertTemplate (const Symbol &) {
	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::INSERT_NO_TABLE));
    }

    void ISymbol::getTemplates (std::vector<Symbol> &) const {
	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::INSERT_NO_TABLE));
    }

    // void ISymbol::replace (const Symbol &) {
    // 	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::INSERT_NO_TABLE));
    // }

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

    void ISymbol::setWeak () {
	this-> _isWeak = true;
    }

    bool ISymbol::isWeak () const {
	return this-> _isWeak;
    }

    void ISymbol::setTrusted () {
	this-> _isTrusted = true;
    }
    
    bool ISymbol::isTrusted () const {
	if (this-> _isTrusted) return true;
	if (this-> getReferent ().isEmpty ()) return false;
	return this-> getReferent ().isTrusted ();
    }
    
    void ISymbol::setProtected () {
	this-> _isPublic = false;
	this-> _isProtected = true;
    }

    bool ISymbol::isProtected () const {
	return this-> _isProtected;
    }
    
    void ISymbol::get (const std::string & name, std::vector <Symbol> & ret) const {
	this-> getReferent ().get (name, ret);
    }

    void ISymbol::getPrivate (const std::string & name, std::vector <Symbol> & ret) const {
	this-> get (name, ret);	
	this-> getReferent ().getPrivate (name, ret);
	
	Symbol::mergeEqSymbols (ret);
    }
    
    void ISymbol::getPublic (const std::string & name, std::vector <Symbol> & ret) const {
	this-> getReferent ().getPublic (name, ret);
    }

    void ISymbol::getLocal (const std::string &, std::vector <Symbol> &) const {}    

    void ISymbol::getLocalPublic (const std::string &, std::vector <Symbol> &) const {
    }    
    
    const std::map <std::string, Symbol> & ISymbol::getUsedSymbols () const {
	return this-> _used;
    }
    
    Symbol ISymbol::getReferent () const {
	return Symbol {this-> _referent};	
    }
    
    void ISymbol::setReferent (const Symbol & ref) {
	this-> _referent = ref.getPtr ();
    }
    
    std::string ISymbol::getRealName () const {
	if (!this-> _referent.lock ()) return this-> _name.getStr ();
	else {
	    auto ft = (Symbol {this-> _referent}).getRealName ();
	    if (ft != "")
		return ft + "::" + this-> _name.getStr ();
	    else return this-> _name.getStr ();
	}
    }
    
    std::string ISymbol::getMangledName () const {
	std::string ret;
	if (!this-> _referent.lock ()) ret = this-> _name.getStr ();
	else {
	    auto ft = (Symbol {this-> _referent}).getMangledName ();
	    if (ft != "")
		ret = ft + "::" + this-> _name.getStr ();
	    else ret = this-> _name.getStr ();
	}
	
	return ret;
    }

    ISymbol::~ISymbol () {}

    Symbol::Symbol (ISymbol * value) : RefProxy<ISymbol, Symbol> (value)
    {}

    Symbol::Symbol (const std::shared_ptr <ISymbol> & sym) : RefProxy<ISymbol, Symbol> (sym)
    {}

    Symbol::Symbol (const std::weak_ptr <ISymbol> & sym) : RefProxy<ISymbol, Symbol> (sym.lock ())
    {}
    
    Symbol::~Symbol () {}
    
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

    const std::string & Symbol::getComments () const {
	if (this-> _value == nullptr)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return this-> _value-> getComments ();
    }
    

    void Symbol::insert (const Symbol & sym) {
	if (this-> _value != nullptr) {	    
	    this-> _value-> insert (sym);
	} else {
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
    
    void Symbol::getTemplates (std::vector <Symbol> & ret) const {
	if (this-> _value == nullptr) {
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	}
	
	this-> _value-> getTemplates (ret);
    }

    std::vector <Symbol> Symbol::getTemplates () const {
	std::vector <Symbol> rets;
	this-> getTemplates (rets);
	return rets;
    }    
    
    // void Symbol::replace (const Symbol & sym) {
    // 	if (this-> _value != nullptr)
    // 	    this-> _value-> replace (sym);
    // 	else {
    // 	    // We don't do anything, it is more convinient for global modules
    // 	}
    // }
    
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
    
    void Symbol::setWeak () {
	if (this-> _value != nullptr)
	    this-> _value-> setWeak ();
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	}
    }

    bool Symbol::isWeak () const {
	if (this-> _value != nullptr)
	    return this-> _value-> isWeak ();
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	    return false;
	}
    }

    void Symbol::setTrusted () {
	if (this-> _value != nullptr)
	    this-> _value-> setTrusted ();
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	}
    }

    bool Symbol::isTrusted () const {
	if (this-> _value != nullptr)
	    return this-> _value-> isTrusted ();
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	    return false;
	}
    }

    
    void Symbol::setProtected () {
	if (this-> _value != nullptr)
	    this-> _value-> setProtected ();
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	}
    }

    bool Symbol::isProtected () const {
	if (this-> _value != nullptr)
	    return this-> _value-> isProtected ();
	else {
	    // We cannot use a symbol outside of any scope
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	    return false;
	}
    }
       
    std::vector <Symbol> Symbol::get (const std::string & name) {
	std::vector <Symbol> rets;
	this-> get (name, rets);
	return rets;
    }

    void Symbol::get (const std::string & name, std::vector <Symbol> & ret) {
	if (this-> _value == nullptr) return;

	this-> _value-> get (name, ret);
	for (auto & it : this-> _value-> getUsedSymbols ()) {
	    auto mod = it.second;
	    if (it.second.isEmpty ()) {
		mod = getModuleByPath (it.first);
		this-> _value-> use (it.first, mod);
	    }

	    if (!mod.isEmpty ()) {
		mod.getUsed (name, ret);		
	    }
	}	           
	
	Symbol::mergeEqSymbols (ret);
    }

    std::vector <Symbol> Symbol::getPrivate (const std::string & name) {
	std::vector <Symbol> rets;
	this-> getPrivate (name, rets);
	return rets;
    }
    
    void Symbol::getPrivate (const std::string & name, std::vector <Symbol>& ret) {
	if (this-> _value == nullptr) return;

	static std::set <std::shared_ptr<ISymbol> > current;
	
	if (current.find (this-> _value) == current.end ()) {	    
	    current.emplace (this-> _value);
	    this-> _value-> getPrivate (name, ret);
	    for (auto & it : this-> _value-> getUsedSymbols ()) {
		auto mod = it.second;
		if (it.second.isEmpty ()) {
		    mod = getModuleByPath (it.first);
		    this-> _value-> use (it.first, mod);
		}
		
		if (!mod.isEmpty ()) {
		    mod.getLocal (name, ret);				
		}
	    }
	    
	    current.erase (this-> _value);
	}
	
	Symbol::mergeEqSymbols (ret);
    }

    std::vector <Symbol> Symbol::getPublic (const std::string & name) {
	std::vector <Symbol> rets;
	this-> getPublic (name, rets);
	return rets;
    }

    void Symbol::getPublic (const std::string & name, std::vector <Symbol> & ret) {
	if (this-> _value == nullptr) return;
	static std::set <std::shared_ptr <ISymbol> > current;
	
	if (current.find (this-> _value) == current.end ()) {	    
	    current.emplace (this-> _value);
	    this-> _value-> getPublic (name, ret);	    
	    for (auto & it : this-> _value-> getUsedSymbols ()) {
		auto mod = it.second;
		if (it.second.isEmpty ()) {
		    mod = getModuleByPath (it.first);
		    this-> _value-> use (it.first, mod);
		}
		
		if (!mod.isEmpty () && mod.isPublic ()) {
		    mod.getPublic (name, ret);		
		}
	    }
	
	    current.erase (this-> _value);
	}
	
	Symbol::mergeEqSymbols (ret);
    }

	std::vector <Symbol> Symbol::getUsed (const std::string & name) {
	std::vector <Symbol> rets;
	this-> getUsed (name, rets);
	return rets;
    }

    
    void Symbol::getUsed (const std::string & name, std::vector <Symbol> & ret) {
	if (this-> _value == nullptr) return;

	static std::set <std::shared_ptr <ISymbol> > current;
	if (current.find (this-> _value) == current.end ()) {
	    current.emplace (this-> _value);
	    this-> _value-> getPublic (name, ret);
	    for (auto & it : this-> _value-> getUsedSymbols ()) {
		auto mod = it.second;
		if (it.second.isEmpty ()) {
		    mod = getModuleByPath (it.first);
		    this-> _value-> use (it.first, mod);
		}

		if (!mod.isEmpty () && mod.isPublic ()) {
		    mod.getUsed (name, ret);		    
		}
	    }
	    current.erase (this-> _value);
	}
	
	Symbol::mergeEqSymbols (ret);
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
	if (this-> _value != nullptr) {	    
	    this-> _value-> setReferent (ref);
	}
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

    void Symbol::getLocal (const std::string & name, std::vector <Symbol> & ret) const {
	if (this-> _value != nullptr) 
	    this-> _value-> getLocal (name, ret);
    }

    std::vector <Symbol> Symbol::getLocal (const std::string & name) const {
	std::vector <Symbol> ret;
	this-> getLocal (name, ret);
	return ret;
    }
    
    void Symbol::getLocalPublic (const std::string & name, std::vector <Symbol> & ret) const {
	if (this-> _value != nullptr) 
	    this-> _value-> getLocalPublic (name, ret);
    }

    std::vector <Symbol> Symbol::getLocalPublic (const std::string & name) const {
	std::vector <Symbol> rets;
	this-> getLocalPublic (name, rets);
	return rets;
    }
    
    bool Symbol::equals (const Symbol & other, bool parent) const {
	if (this-> _value == nullptr) return other._value == nullptr;
	return this-> _value-> equals (other, parent);
    }

    bool Symbol::isSameRef (const Symbol & other) const {
	if (this-> _value == nullptr) return false;
	return this-> _value.get () == other._value.get ();
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
	auto it = __fast_mod_access__.find (path_);
	if (it != __fast_mod_access__.end ()) {
	    return it-> second;
	} 
	
	auto path = Ymir::Path {path_, "::"};	
	auto mod = Symbol::getModule (path.getFiles () [0]);
	auto files = path.getFiles ();
	auto submods = std::vector <std::string> {files.begin () + 1, files.end ()};
	for (auto & name : submods) {
	    std::vector <Symbol> inners;
	    
	    mod.getLocal (name, inners);
	    bool succeed = false;
	    for (const auto & inner_sym : inners) {
		if (inner_sym.is <Module> ()) {
		    succeed = true;
		    mod = inner_sym;
		    break;
		}
	    }
	    if (!succeed) return Symbol::__empty__;
	}
	
	if (!mod.isEmpty ()) {
	    __fast_mod_access__.emplace (path_, mod);	    
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

    void Symbol::purge () {
	__imported__.clear ();
	__fast_mod_access__.clear ();
    }
    
    void Symbol::mergeEqSymbols (std::vector <Symbol> & multSym) {
	std::vector <Symbol> result;
	result.reserve (multSym.size ());
	for (auto & it : multSym) {
	    bool add = true;
	    for (auto & zt : result)
		if (zt.isSameRef (it)) { add = false; break; }
	    
	    if (add)
		result.push_back (it);
	}
	
	multSym = result;
    }

    
}
