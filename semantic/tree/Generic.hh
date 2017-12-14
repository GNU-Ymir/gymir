#pragma once

#include <vector>
#include "Tree.hh"

namespace semantic {    
    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace Ymir {    
        
    Tree makeField (semantic::InfoType type, std::string name);
    
    Tree getField (location_t locus, Tree obj, std::string name);
    
    Tree makeTuple (std::string name, std::vector <semantic::InfoType> types, std::vector<std::string> attrs);

    Tree makeStructType (std::string name, int nbFields, ...);
    
}
