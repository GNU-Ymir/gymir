#include <ymir/semantic/pack/FrameScope.hh>
#include <ymir/errors/Error.hh>
#include <algorithm>

namespace semantic {

    ITreeInfo::ITreeInfo (std::string name) :
	_name (name)
    {}
	
    Word ITreeInfo::ident () {
	return this-> _ident;
    }

    Word ITreeInfo::exists (std::string name) {
	if (this-> _ident == name) return this-> _ident;
	else if (this-> _father) return this-> _father-> exists (name);
	return Word::eof ();
    }

    void ITreeInfo::ident (Word name) {
	auto ex = this-> exists (name.getStr ());
	if (!ex.isEof ()) {
	    Ymir::Error::multipleLoopName (name, ex);
	}
	this-> _ident = name;
    }

    void ITreeInfo::returned () {
	this-> _returned = true;
    }

    void ITreeInfo::breaked () {
	this-> _breaked = true;
    }

    std::string ITreeInfo::name () {
	return this-> _name;
    }

    ITreeInfo* ITreeInfo::enterBlock (std::string sc) {
	this-> _infos.push_back (new (Z0)  ITreeInfo (sc));
	this-> _infos.back ()-> _father = this;
	return this-> _infos.back ();
    }

    bool ITreeInfo::hasBreaked () {
	if (this-> _infos.size () == 0) return this-> _breaked;
	ulong nb = 0;
	std::string need = "";
	bool allNeed = true;
	for (auto it : this-> _infos) {
	    if (!allNeed && need == it-> _name) allNeed = true;

	    if (it-> _breaked && it-> _name == "true") {
		/* in case of : '{ break; }'*/
		this-> _breaked = true;
	    } else if (it-> _breaked && it-> _name == "else") {
		nb ++;
	    } else if (it-> _breaked && it-> _name == "if") {
		nb ++;
		allNeed = false;
		need = "true";
	    } else if (it-> _breaked && it-> _name != "while" && it-> _name != "for") {
		nb ++;
	    }
	}

	if (nb == this-> _infos.size () && allNeed) {
	    this-> _breaked = true;
	}
	return this-> _breaked;
    }

    bool ITreeInfo::hasReturned () {
	if (this-> _infos.size () == 0) return this-> _returned;
	ulong nb = 0;
	std::string need = "";
	bool allNeed = true;
	for (auto it : this-> _infos) {
	    if (!allNeed && need == it-> _name) allNeed = true;

	    if (it-> _returned && it-> _name == "true") {
		/* in case of : '{ break; }'*/
		this-> _returned = true;
	    } else if (it-> _returned && it-> _name == "else") {
		nb ++;
	    } else if (it-> _returned && it-> _name == "if") {
		nb ++;
		allNeed = false;
		need = "else";
	    } else if (it-> _returned && it-> _name != "while" && it-> _name != "for") {
		nb ++;
	    }
	}

	if (nb == this-> _infos.size () && allNeed) {
	    this-> _returned = true;
	}
	return this-> _returned;
    }

    ITreeInfo* ITreeInfo::quitBlock () {
	hasBreaked ();
	hasReturned ();
	return this-> _father;
    }

    bool ITreeInfo::retract () {
	if (this-> _infos.size () == 0) {
	    if (this-> _father != NULL) return this-> _father-> retract ();
	    else return this-> _returned;
	}
	hasReturned ();
	if (this-> _father)
	    return this-> _father-> retract ();
	else return this-> _returned;
    }

    long ITreeInfo::rewind (std::string name, long nb) {
	nb ++;
	if (this-> _ident == name) return nb;
	else if (this-> _father) return this-> _father-> rewind (name, nb);
	else return -1;
    }

    long ITreeInfo::rewind (const std::vector<std::string> & types, long nb) {
	nb ++;
	if (canFind (types, this-> _name))
	    return nb;
	else if (this-> _father) return this-> _father-> rewind (types, nb);
	return -1;
    }

    void ITreeInfo::print (int i) {
	printf ("%*c%s, %s {%s",
		i, ' ',
		this-> _name.c_str (), this-> _ident.getStr ().c_str (),
		this-> _returned ? ":true" : ""
	);

	for (auto it : this-> _infos)
	    it-> print (i + 4);
	printf ("%*c}", i, ' ');
    }

    FrameReturnInfo& FrameReturnInfo::empty () {
	return __empty__;
    }

    void FrameReturnInfo::returned () {
	if (this-> _retInfo) 
	    this-> _retInfo-> returned ();
    }

    void FrameReturnInfo::breaked () {
	this-> _retInfo-> breaked ();
    }
    
    bool& FrameReturnInfo::changed () {
	return this-> _changed;
    }

    bool& FrameReturnInfo::isImmutable () {
	return this-> _isImmutable;
    }

    bool& FrameReturnInfo::closureMoved () {
	return this-> _closureMoved;
    }
    
    void FrameReturnInfo::enterBlock () {
	if (this-> _retInfo) {
	    this-> _retInfo = this-> _retInfo-> enterBlock (this-> _currentBlock);
	} else this-> _retInfo = new (Z0)  ITreeInfo (this-> _currentBlock);
    }

    void FrameReturnInfo::quitBlock () {
	if (this-> _retInfo) {
	    this-> _retInfo = this-> _retInfo-> quitBlock ();
	    if (this-> _retInfo)
		this-> _currentBlock = this-> _retInfo-> name ();
	}
    }

    bool FrameReturnInfo::retract () {
	return this-> _retInfo ?
	    this-> _retInfo-> retract () :
	    false;	    
    }

    bool FrameReturnInfo::hasReturned () {
	return this-> _retInfo ?
	    this-> _retInfo-> hasReturned () :
	    false;	    
    }

    bool FrameReturnInfo::hasBreaked () {
	return this-> _retInfo ?
	    this-> _retInfo-> hasBreaked () :
	    false;	    
    }

    void FrameReturnInfo::setIdent (Word ident) {
	if (this-> _retInfo)
	    this-> _retInfo-> ident (ident);
    }

    std::string& FrameReturnInfo::currentBlock () {
	return this-> _currentBlock;
    }

    long FrameReturnInfo::rewind (std::string name) {
	return this-> _retInfo-> rewind (name);
    }

    long FrameReturnInfo::rewind (const std::vector<std::string> & types) {
	return this-> _retInfo-> rewind (types);
    }
    
    void FrameReturnInfo::print () {
	this-> _retInfo-> print ();
    }

    FrameReturnInfo FrameReturnInfo::__empty__;

    FrameScope::FrameScope (Namespace space) :
	_friend (NULL),
	_space (space)
    {
	this-> _retInfo.currentBlock () = "";
	this-> enterBlock ();
    }

    void FrameScope::enterBlock () {
	this-> _local.push_front ({});
	this-> _retInfo.enterBlock ();
    }

    ulong FrameScope::getCurrentLifeTime () {
	return this-> _local.size ();
    }
    
    void FrameScope::setContext (const std::vector <Word> & context) {
	this-> _context = context;
    }

    bool FrameScope::hasContext (const std::string & uda) {
	for (auto it : this-> _context) {
	    if (it.getStr () == uda)
		return true;
	}
	return false;
    }
    
    void FrameScope::addOpen (Namespace space) {
	this-> _local.front ().addOpen (space);
    }

    void FrameScope::setInternal (FrameScope* scope) {
	this-> _friend = scope;
    }
    
    bool FrameScope::isInternal () {
	return this-> _friend != NULL;
    }

    void FrameScope::quitBlock () {
	this-> _retInfo.quitBlock ();
	if (!this-> _local.empty ()) {
	    this-> _local.front ().quit (this-> _space);
	    this-> _local.pop_front ();
	}
    }

    void FrameScope::set (std::string name, Symbol info) {
	if (!this-> _local.empty ()) {
	    this-> _local.front ().set (name, info);
	}
    }

    Symbol FrameScope::get (std::string name) {
	for (auto it : this-> _local) {
	    auto t = it.get (name);
	    if (t) return t;
	}
	
	if (this-> _friend) return this-> _friend-> get (name);
	return NULL;	
    }

    bool FrameScope::fromFriend (Symbol sym) {
	if (!this-> _friend) return false;
	auto name = sym-> sym.getStr ();
	for (auto it : this-> _local) {
	    auto t = it.get (name);
	    if (t) return false;
	}
	
	if (this-> _friend-> get (name)) return true;
	return false;
    }
    
    std::vector <Symbol> FrameScope::getAll (std::string name) {
	std::vector <Symbol> syms;
	for (auto it : this-> _local) {
	    auto alls = it.getAll (name);
	    syms.insert (syms.end (), alls.begin (), alls.end ());
	}
	return syms;
    }
	
    Symbol FrameScope::getAlike (std::string name) {
	for (auto it : this-> _local) {
	    auto t = it.getAlike (name);
	    if (t) return t;
	}
	if (this-> _friend) return this-> _friend-> get (name);
	return NULL;
    }

    bool &FrameScope::isPhantom () {
	return this-> phantom;
    }
    
    Namespace& FrameScope::space () {
	return this-> _space;
    }

    FrameReturnInfo& FrameScope::retInfo () {
	return this-> _retInfo;
    }

    

}
