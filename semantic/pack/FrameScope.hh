#pragma once

#include <ymir/utils/memory.hh>
#include "../../syntax/Word.hh"
#include <ymir/semantic/pack/Scope.hh>
#include "Symbol.hh"
#include <ymir/utils/Array.hh>
#include <list>

namespace syntax {
    class IVar;
    typedef IVar* Var;
}

namespace semantic {

    class ITreeInfo  {
	std::string _name;
	Word _ident;
	std::vector <ITreeInfo*> _infos;
	bool _returned = false;
	bool _breaked = false;
	ITreeInfo* _father;
	
    public:

	ITreeInfo (std::string name);
	
	Word ident ();

	Word exists (std::string name);

	void ident (Word name);

	void returned ();

	void breaked ();

	std::string name ();

	ITreeInfo* enterBlock (std::string);

	bool hasBreaked ();

	bool hasReturned ();
	
	ITreeInfo* quitBlock ();

	bool retract ();

	long rewind (std::string name, long nb = 0);

	long rewind (const std::vector<std::string> & types, long nb = 0);

	void print (int i = 0);
	
    };

    typedef ITreeInfo* TreeInfo;
    
    struct FrameReturnInfo {

	Symbol info;
	bool _isImmutable;
	std::string deco; 
	std::vector <syntax::Var> closure;
	
    private :
	std::string _currentBlock;
	TreeInfo _retInfo = NULL;
	bool _changed = false;
	bool _closureMoved = false;

    public:
	static FrameReturnInfo& empty ();

	void returned ();

	void breaked ();

	bool& changed ();
	
	bool& isImmutable ();

	bool& closureMoved ();
	
	void enterBlock ();

	void quitBlock ();

	bool retract ();

	bool hasReturned ();

	bool hasBreaked ();

	void setIdent (Word ident);

	std::string& currentBlock ();

	long rewind (std::string name);

	long rewind (const std::vector<std::string> & types);

	void print ();

    private:
	
	static FrameReturnInfo __empty__;
	
    };
    

    class FrameScope {
	FrameReturnInfo _retInfo;
	std::list <Scope> _local;
	FrameScope* _friend = NULL;
	std::vector <Word> _context;
	
	Namespace _space;
	bool phantom;
	
    public:

	FrameScope (Namespace space);

	void enterBlock ();

	void addOpen (Namespace space);

	void setContext (const std::vector<Word> & context);

	bool hasContext (const std::string & uda);
	
	void setInternal (FrameScope* scope);

	bool fromFriend (Symbol sym);
	
	bool isInternal ();

	void quitBlock ();

	void set (std::string, Symbol);

	Symbol get (std::string);

	std::vector <Symbol> getAll (std::string);
	
	Symbol getAlike (std::string);

	bool& isPhantom ();
	
	Namespace& space ();

	FrameReturnInfo& retInfo ();
	
    };
    
    
}
