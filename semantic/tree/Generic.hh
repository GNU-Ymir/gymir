#pragma once

#include <vector>
#include "Tree.hh"

namespace syntax {
    class IExpression;
    typedef IExpression* Expression;    
}

namespace semantic {    
    class IInfoType;
    typedef IInfoType* InfoType;

    class ISymbol;
    typedef ISymbol* Symbol;
    
}

namespace Ymir {    
        
    Tree makeField (semantic::InfoType type, std::string name);
    
    Tree getField (location_t locus, Tree obj, std::string name);

    Tree getField (location_t locus, Tree obj, ulong it);
    
    Tree makeTuple (std::string name, const std::vector<semantic::InfoType> & types, const std::vector<std::string>& attrs, bool packed = false);

    Tree makeTuple (std::string name, const std::vector<semantic::InfoType> & types);

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

    void declareGlobal (semantic::Symbol sym, syntax::Expression);
    
    void declareGlobalExtern (semantic::Symbol sym);

    void finishCompilation ();
    
    real_value makeRealValue (float elem);

    real_value makeRealValue (double elem);

    tree promote (tree type);

    Tree compoundExpr (location_t, Tree arg0, Tree arg1);
    
}

//tree convert (tre
