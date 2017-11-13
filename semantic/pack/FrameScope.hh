#pragma once

#include <gc/gc_cpp.h>
#include "../../syntax/Word.hh"
#include "Symbol.hh"
#include <vector>

namespace semantic {

    class ITreeInfo : public gc {
	std::string name;
	Word ident;
	std::vector <ITreeInfo*> infos;
	bool returned;
	bool breaked;
	ITreeInfo* father;

    public:
	
    };

    typedef ITreeInfo* TreeInfo;
    
    struct FrameReturnInfo {
	Symbol info;
	bool isImmutable;
	std::string currentBlock;
	TreeInfo retInfo = NULL;
	bool changed = false;
    };
    

    class IFrameScope : public gc {
    };
    

    typedef IFrameScope* FrameScope;
    
}
