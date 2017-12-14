#pragma once

#include "FrameScope.hh"
#include "Namespace.hh"
#include "Scope.hh"
#include "Module.hh"
#include "../../ast/Instruction.hh"
#include "../../ast/Global.hh"

#include <ymir/utils/Array.hh>
#include <list>

namespace semantic {

    class Table {

	std::list <FrameScope> _frameTable;
	Scope _globalScope;
	Namespace _templateScope;
	std::vector <Module> _importations;
	std::vector <Namespace> _foreigns;
	Namespace _space;
	Namespace _programSpace;
	std::vector <syntax::Global> _globalVars;
	ulong _nbFrame = 0;
	static const int __maxNbRec__ = 300;
	std::vector <syntax::Instruction> _staticInits;
	
    public:
	
	static Table& instance () {
	    return __instance__;
	}
              
	void enterBlock ();

	void quitBlock ();

	void setCurrentSpace (Namespace space);

	void addCall (Word sym);

	void addGlobal (syntax::Global gl);

	std::vector <syntax::Global> globalVars ();

	void enterFrame (Namespace space, std::string name, bool internal);

	void quitFrame ();

	Namespace space ();

	Namespace globalNamespace ();

	Namespace& programNamespace ();
	
	Namespace& templateNamespace ();

	void insert (Symbol info);

	void addStaticInit (syntax::Instruction exp);

	std::vector <syntax::Instruction> staticInits ();

	void purge ();

	Symbol get (std::string name);

	std::vector <Symbol> getAll (std::string name);

	Symbol getLocal (std::string name);

	Symbol local (std::string name);

	bool sameFrame (Symbol sym);

	Symbol getAlike (std::string name);

	FrameReturnInfo& retInfo ();
	
	Module addModule (Namespace space);

	Module getModule (Namespace space);
	
	void addForeignModule (Namespace space);

	std::vector <Module> getAllMod (Namespace space);

	bool isModule (Namespace space);

	void openModuleForSpace (Namespace from, Namespace to);

	void closeModuleForSpace (Namespace from, Namespace to);

	std::vector <Namespace> modules ();

	std::vector <Namespace> modulesAndForeigns ();

	bool moduleExists (Namespace name);

	ulong nbRecursive ();
	
    private:

	Table ();
	Table (Table&);
	Table& operator= (Table&);
	
	static Table __instance__;
	
    };
  
}
