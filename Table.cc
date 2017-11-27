#include "semantic/pack/Table.hh"
#include "semantic/pack/ExternFrame.hh"
#include <algorithm>
#include <ymir/utils/Array.hh>

namespace semantic {

    Table Table::__instance__;

    Table::Table () :
	_templateScope (""),
	_space (""),
	_programSpace ("")	
    {}

    void Table::enterBlock () {
	if (!this-> _frameTable.empty ()) {
	    this-> _frameTable.front ().enterBlock ();
	}
    }

    void Table::quitBlock () {
	if (!this-> _frameTable.empty ()) {
	    this-> _frameTable.front ().quitBlock ();
	}
    }

    void Table::setCurrentSpace (Namespace space) {
	if (!this-> _frameTable.empty ()) {
	    this-> _frameTable.front ().space () = space;
	} else {
	    this-> _space = space;
	}
    }

    void Table::addCall (Word sym) {
	if (this-> _nbFrame > __maxNbRec__) {
	    Ymir::Error::recursiveExpansion (sym);
	}
    }

    void Table::addGlobal (syntax::Global gl) {
	this-> _globalVars.push_back (gl);
    }

    std::vector <syntax::Global> Table::globalVars () {
	return this-> _globalVars;
    }

    void Table::enterFrame (Namespace space, std::string name, bool internal) {
	this-> _frameTable.push_front ({{space, name}, internal});
	this-> _nbFrame ++;
    }

    void Table::quitFrame () {
	if (!this-> _frameTable.empty ()) {
	    this-> _frameTable.pop_front ();
	    this-> _nbFrame --;
	}
    }

    Namespace Table::space () {
	if (this-> _frameTable.empty ()) return this-> _space;
	return this-> _frameTable.front ().space ();
    }

    Namespace Table::globalNamespace () {
	return this-> _space;
    }

    Namespace& Table::programNamespace () {
	return this-> _programSpace;
    }

    Namespace& Table::templateNamespace () {
	return this-> _templateScope;
    }

    void Table::insert (Symbol info) {
	if (this-> _frameTable.empty ()) {
	    this-> _globalScope.set (info-> sym.getStr (), info);
	} else {
	    this-> _frameTable.front (). set (info-> sym.getStr (), info);
	}
    }

    void Table::addStaticInit (syntax::Instruction exp) {
	this-> _staticInits.push_back (exp);
    }

    std::vector <syntax::Instruction> Table::staticInits () {
	return this-> _staticInits;
    }

    void Table::purge () {
	this-> _globalScope.clear ();
	this-> _frameTable.clear ();
	this-> _staticInits.clear ();
	this-> _globalVars.clear ();
	IExternFrame::clear ();
    }

    Symbol Table::get (std::string name) {
	Symbol ret;
	Namespace last = this-> _space;
	if (!this-> _frameTable.empty ()) {
	    ret = this-> _frameTable.front ().get (name);
	    if (ret) return ret;

	    for (auto it : this-> _frameTable) {
		if (it.space ().isAbsSubOf (last)) {
		    ret = it.get (name);
		    if (ret && ret-> isScoped ()) return ret;
		    else ret = NULL;
		    last = it.space ();
		} else if (this-> _space != last) break;	    
	    }
	}

	if (ret == NULL) ret = this-> _globalScope.get (name);
	if (ret == NULL) {
	    auto mods = this-> getAllMod (this-> _space);
	    for (auto it : mods) {
		ret = it->  get (name);
		if (ret != NULL) return ret;
	    }
	}
	
	return ret;
    }

    std::vector <Symbol> Table::getAll (std::string name) {
	std::vector <Symbol> alls;
	Namespace last = this-> _space;
	if (!this-> _frameTable.empty ()) {
	    alls = this-> _frameTable.front ().getAll (name);

	    for (auto it : this-> _frameTable) {
		if (it.space ().isAbsSubOf (last)) {
		    auto aux = it.getAll (name);
		    for (auto at : aux)
			if (at-> isScoped ()) alls.push_back (at);
		    last = it.space ();
		} else if (this-> _space != last) break;
	    }
	}

	auto aux = this-> _globalScope.getAll (name);
	alls.insert (alls.end (), aux.begin (), aux.end ());
	auto mods = this-> getAllMod (this-> _space);
	for (auto m : mods) {
	    if (!m-> space ().isSubOf (this-> _space)) {
		aux = m-> getAll (name);
		alls.insert (alls.end (), aux.begin (), aux.end ());
	    }
	}
	return alls;
    }

    Symbol Table::getLocal (std::string name) {
	Symbol ret;
	Namespace last = this-> _space;
	if (!this-> _frameTable.empty ()) {
	    ret = this-> _frameTable.front ().get (name);
	    if (ret) return ret;

	    for (auto it : this-> _frameTable) {
		if (it.space ().isAbsSubOf (last)) {
		    ret = it.get (name);
		    if (ret && ret-> isScoped ()) return ret;
		    else ret = NULL;
		    last = it.space ();
		} else if (this-> _space != last) break;	    
	    }
	}

	if (ret == NULL) ret = this-> _globalScope.get (name);
	if (ret == NULL) {
	    auto mods = this-> getAllMod (this-> _space);
	    for (auto it : mods) {
		if (!it-> space ().isSubOf (this-> _space)) {
		    ret = it-> get (name);
		    if (ret != NULL) return ret;
		}
	    }
	}
	
	return ret;
    }

    Symbol Table::local (std::string name) {
	Symbol ret;
	Namespace last = this-> _space;
	if (!this-> _frameTable.empty ()) {
	    ret = this-> _frameTable.front ().get (name);
	    if (ret) return ret;

	    for (auto it : this-> _frameTable) {
		if (it.space ().isAbsSubOf (last)) {
		    ret = it.get (name);
		    if (ret && ret-> isScoped ()) return ret;
		    else ret = NULL;
		    last = it.space ();
		} else if (this-> _space != last) break;	    
	    }
	}

	if (ret == NULL) ret = this-> _globalScope.get (name);
	return ret;
    }

    Symbol Table::getAlike (std::string name) {
	for (auto it : this-> _frameTable) {
	    if (it.space ().isSubOf (this-> _space)) {
		auto ret = it.getAlike (name);
		if (ret) return ret;
	    }
	}
	return this-> _globalScope.getAlike (name);
    }
	
    
    bool Table::sameFrame (Symbol sym) {
	if (this-> _frameTable.empty ()) return true;
	auto ret =this-> _frameTable.front ().get (sym-> sym.getStr ());
	if (ret != NULL) return true;
	return false;
    }

    FrameReturnInfo& Table::retInfo () {
	if (this-> _frameTable.empty ()) return FrameReturnInfo::empty ();
	else return this-> _frameTable.front ().retInfo ();
    }

    Module Table::addModule (Namespace space) {
	Module mod = new IModule (space);
	this-> _importations.push_back (mod);
	return mod;	
    }

    Module Table::getModule (Namespace space) {
	for (auto it : this-> _importations) {
	    if (it-> space () == space) return it;
	}
	return NULL;
    }
    
    void Table::addForeignModule (Namespace space) {
	for (auto it : this-> _foreigns) {
	    if (space.isSubOf (it)) {
	    } else if (it.isSubOf (space)) {
		// TODO
		// Ymir::Error::fatal (
	    }
	}
	this-> _foreigns.push_back (space);
    }

    std::vector <Module> Table::getAllMod (Namespace space) {
	std::vector <Module> alls;
	for (auto it : this-> _importations) {
	    if (it-> authorized (space) || it-> authorized (this-> _templateScope)) {
		auto access = it-> accessible ();
		for (auto sp : access) {
		    auto mod = this-> getModule (sp);
		    if (find (alls, mod) == alls.end ())
			alls.push_back (mod);
		}
	    }
	}
	return alls;
    }
    
    bool Table::isModule (Namespace space) {
	for (auto it : this-> _foreigns) {
	    if (it == space) return true;
	}

	for (auto it : this-> _importations) {
	    if (it-> space () == space) return true;
	}

	return false;
    }

    void Table::openModuleForSpace (Namespace from, Namespace to) {
	for (auto it : this-> _importations) {
	    if (it-> space () == from) {
		it-> addOpen (to);
		if (!this-> _frameTable.empty ())
		    this-> _frameTable.front ().addOpen (it-> space ());
		
		break;
	    }	    
	}
    }

    void Table::closeModuleForSpace (Namespace from, Namespace to) {
	//TODO pareil
	for (auto it : this-> _importations) {
	    if (it-> space () == from) {
		it-> close (to);
		break;
	    }
	}
    }

    std::vector <Namespace> Table::modules () {
	std::vector <Namespace> spaces;
	for (auto it : this-> _importations) {
	    spaces.push_back (it-> space ());
	}
	return spaces;
    }

    std::vector <Namespace> Table::modulesAndForeigns () {
	std::vector <Namespace> spaces;
	for (auto it : this-> _importations) {
	    spaces.push_back (it-> space ());
	}
	
	for (auto it : this-> _foreigns) {
	    spaces.push_back (it);
	}
	
	return spaces;
    }

    bool Table::moduleExists (Namespace name) {
	for (auto it : this-> _importations) {
	    if (it-> space () == name) return true;
	}
	return false;
    }

    ulong Table::nbRecursive () {
	return this-> _nbFrame;
    }
    
}
