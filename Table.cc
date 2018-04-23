#include "semantic/pack/Table.hh"
#include "semantic/pack/ExternFrame.hh"
#include <algorithm>
#include <ymir/utils/Array.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/semantic/value/Value.hh>

namespace semantic {

    Table Table::__instance__;

    Table::Table () :
	_templateScope (),
	_space (""),
	_programSpace ("")	
    {
	_templateScope.push_front (Namespace {""});
    }

    void Table::enterBlock () {
	if (!this-> _frameTable.empty ()) {
	    this-> _frameTable.front ().enterBlock ();
	}
    }

    void Table::enterPhantomBlock () {
	FrameScope scope {{this-> getCurrentSpace (), "__"}};
	scope.isPhantom () = true;
	scope.setInternal (&this-> _frameTable.front ());
	this-> _frameTable.push_front (scope);
	this-> _templateScope.push_front (Namespace {""});
	this-> _nbFrame ++;
    }

    void Table::quitBlock () {
	if (!this-> _frameTable.empty ()) {
	    this-> _frameTable.front ().quitBlock ();
	}
    }

    Namespace Table::getCurrentSpace () {
	if (!this-> _frameTable.empty ()) {
	    return this-> _frameTable.front ().space ();
	} else {
	    return this-> _space;
	}
    }
    
    void Table::setCurrentSpace (Namespace space) {
	if (!this-> _frameTable.empty ()) {
	    this-> _frameTable.front ().space () = space;
	} else {
	    this-> _space = space;
	}
    }

    bool Table::addCall (Word sym) {
	if (this-> _nbFrame > __maxNbRec__) {
	    Ymir::Error::recursiveExpansion (sym);
	    return false;
	}
	return true;
    }

    void Table::addGlobal (syntax::Global gl) {
	this-> _globalVars.push_back (gl);
    }

    std::vector <syntax::Global> Table::globalVars () {
	return this-> _globalVars;
    }

    void Table::enterFrame (Namespace space, std::string name, std::vector <syntax::Expression> & tmps, const std::vector <Word> & context, bool internal) {
	Ymir::OutBuffer buf ("0_", name);
	if (tmps.size () != 0) {
	    buf.write ("(");
	    for (auto it : tmps) {
		if (auto ps = it-> to <syntax::IParamList> ()) {
		    buf.write ("{");
		    for (auto it_ : Ymir::r (0, ps-> getParams ().size ())) {
			buf.write (ps-> getParams () [it_]-> info-> typeString ());
			if (it_ < (int) ps-> getParams ().size () - 1)
			    buf.write (",");
		    }
		    buf.write ("}");
		} else if (it-> info) {		    
		    if (it-> info-> isImmutable ()) buf.write (it-> info-> value ()-> toString ());
		    else buf.write (it-> info-> typeString ());		    
		} else buf.write (it-> prettyPrint ());
		if (it != tmps.back ())
		    buf.write (",");
	    }
	    buf.write (")");
	}

	FrameScope scope {{space, buf.str ()}};
	scope.setContext (context);
	if (internal) scope.setInternal (&this-> _frameTable.front ());
	this-> _frameTable.push_front (scope);
	this-> _templateScope.push_front (Namespace {""});
	this-> _nbFrame ++;
    }

    bool Table::hasCurrentContext (const std::string & uda) {
	if (!this-> _frameTable.empty ()) {
	    return this-> _frameTable.back ().hasContext (uda);
	} else return false;
    }
    
    void Table::quitFrame () {
	if (!this-> _frameTable.empty ()) {
	    this-> _frameTable.pop_front ();
	    this-> _templateScope.pop_front ();
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
	return this-> _templateScope.front ();
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
	Namespace last = this-> getCurrentSpace ();
	if (name == "inner")
	    Ymir::log ("Get : ", name, " from : ", last);
	if (!this-> _frameTable.empty ()) {
	    ret = this-> _frameTable.front ().get (name);
	    if (ret) return ret;
	}
	if (name == "inner")
	    Ymir::log ("Global : ", this-> _globalScope.toString());
	
	if (ret == NULL) ret = this-> _globalScope.get (name);
	if (ret == NULL) {
	    //Ymir::log ("Get All mod from : ", getCurrentSpace (), " and : ", this-> _templateScope.front (), " {");
	    auto mods = this-> getAllMod (getCurrentSpace ());
	    // for (auto i : mods)
	    // 	Ymir::log ("\t", i-> space ());
	    // Ymir::log ("}");
	    for (auto it : mods) {
		ret = it->  getFor (name, getCurrentSpace ());
		if (ret != NULL) return ret;
	    }
	}
	
	return ret;
    }

    std::vector <Symbol> Table::getAll (std::string name) {
	std::vector <Symbol> alls;
	Namespace last = this-> getCurrentSpace ();
	if (!this-> _frameTable.empty ()) {
	    alls = this-> _frameTable.front ().getAll (name);
	}

	auto aux = this-> _globalScope.getAll (name);
	alls.insert (alls.end (), aux.begin (), aux.end ());
	auto mods = this-> getAllMod (getCurrentSpace ());
	for (auto m : mods) {
	    if (!m-> space ().isSubOf (this-> _space)) {
		aux = m-> getAll (name);
		alls.insert (alls.end (), aux.begin (), aux.end ());
	    }
	}
	return alls;
    }

    bool Table::isFrameLocal (Symbol sym) {
	if (!this-> _frameTable.empty ()) {
	    auto ret = this-> _frameTable.front ().get (sym-> sym.getStr ());
	    if (ret == sym) return true;
	}
	return false;
    }
    
    Symbol Table::getLocal (std::string name) {
	Symbol ret;
	Namespace last = this-> _space;
	if (!this-> _frameTable.empty ()) {
	    ret = this-> _frameTable.front ().get (name);
	    if (ret) return ret;
	}

	if (ret == NULL) ret = this-> _globalScope.get (name);
	// if (ret == NULL) {
	//     auto mods = this-> getAllMod (this-> _space);
	//     for (auto it : mods) {
	// 	if (!it-> space ().isSubOf (this-> _space)) {
	// 	    ret = it-> get (name);
	// 	    if (ret != NULL) return ret;
	// 	}
	//     }
	// }
	
	return ret;
    }

    Symbol Table::local (std::string name) {
	Symbol ret;
	Namespace last = this-> _space;
	if (!this-> _frameTable.empty ()) {
	    ret = this-> _frameTable.front ().get (name);
	    if (ret) return ret;
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
	auto ret = this-> _globalScope.getAlike (name);
	if (ret) return ret;
	auto mods = this-> getAllMod (getCurrentSpace ());
	for (auto it : mods) {
	    ret = it->  getAlikeFor (name, getCurrentSpace ());
	    if (ret != NULL) return ret;
	}
	return NULL;
    }
	    
    bool Table::sameFrame (Symbol sym) {
	if (this-> _frameTable.empty ()) return true;
	auto ret = this-> _frameTable.front ().get (sym-> sym.getStr ());
	if (this-> _frameTable.front ().fromFriend (sym)) return false;
	if (ret != NULL && !this-> _frameTable.front ().isPhantom ()) return true;
	return false;
    }

    bool Table::parentFrame (Symbol sym) {
	if (!this-> _frameTable.empty ()) {
	    return this-> _frameTable.front ().fromFriend (sym) && !this-> _frameTable.front ().isPhantom ();
	}
	return false;
    }
    
    FrameReturnInfo& Table::retInfo () {
	if (this-> _frameTable.empty ()) return FrameReturnInfo::empty ();
	else return this-> _frameTable.front ().retInfo ();
    }

    Module Table::addModule (Namespace space) {
	Module mod = new (Z0)  IModule (space);
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
	    if (it-> authorized (space) || it-> authorized (this-> _templateScope.front ())) {
		auto access = it-> accessible ();
		for (auto sp : access) {
		    auto mod = this-> getModule (sp);
		    if (find (alls, mod) == alls.end ()) {
			alls.push_back (mod);
		    }
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
	spaces.push_back (this-> _space);
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
	if (name == this-> _space) return true;
	for (auto it : this-> _importations) {
	    if (it-> space () == name) return true;
	}
	return false;
    }

    Frame Table::moduleConstruct (Namespace name) {
	if (name == this-> _space) return this-> _constructor;
	for (auto it : this-> _importations) {
	    if (it-> space () == name) return it-> constructor ();
	}
	return NULL;
    }

    Frame Table::moduleDestruct (Namespace name) {
	if (name == this-> _space) return this-> _destructor;
	for (auto it : this-> _importations) {	    
	    if (it-> space () == name) return it-> destructor ();
	}
	return NULL;
    }
        
    ulong Table::nbRecursive () {
	return this-> _nbFrame;
    }

    Frame & Table::constructor () {
	return this-> _constructor;
    }

    Frame & Table::destructor () {
	return this-> _destructor;
    }
    
}
