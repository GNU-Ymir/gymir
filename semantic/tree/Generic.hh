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
    
    Tree makeAuxVar (location_t locus, ulong id, Ymir::Tree type);

    Tree makeLabel (location_t loc, std::string &name);

    Tree makeLabel (location_t loc, const char* name);

    Tree getArrayRef (location_t, Tree array, Tree inner, ulong index);

    Tree getPointerUnref (location_t, Tree ptr, Tree inner, ulong index);

    Tree getArrayRef (location_t, Tree array, Tree inner, Tree index);

    Tree getPointerUnref (location_t, Tree ptr, Tree inner, Tree index);

    Tree getAddr (Tree elem);

    Tree getAddr (location_t, Tree elem);

    real_value makeRealValue (float elem);

    real_value makeRealValue (double elem);
    
}
