#pragma once

#include <gc/gc_cpp.h>
#include "../../syntax/Word.hh"
#include <ymir/semantic/pack/Scope.hh>
#include "Symbol.hh"
#include <ymir/utils/Array.hh>
#include <list>

namespace semantic {

    class ITreeInfo : public gc {
	std::string _name;
	Word _ident;
	std::vector <ITreeInfo*> _infos;
	bool _returned;
	bool _breaked;
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

    private :
	std::string _currentBlock;
	TreeInfo _retInfo = NULL;
	bool _changed = false;

    public:
	static FrameReturnInfo& empty ();

	void returned ();

	void breaked ();

	bool& changed ();

	bool& isImmutable ();

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
	Namespace _space;
	bool _isInternal;

    public:

	FrameScope (Namespace space, bool isInternal);

	void enterBlock ();

	void addOpen (Namespace space);

	bool& isInternal ();

	void quitBlock ();

	void set (std::string, Symbol);

	Symbol get (std::string);

	std::vector <Symbol> getAll (std::string);
	
	Symbol getAlike (std::string);

	Namespace& space ();

	FrameReturnInfo& retInfo ();
	
    };
    
    
}
