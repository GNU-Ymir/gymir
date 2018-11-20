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
    
    Tree getFieldDecl (Tree type, std::string name);

    std::vector<Tree> getFieldDecls (Tree type);
    
    Tree makeUnion (std::string name, const std::vector<semantic::InfoType> & types, const std::vector<std::string>& attrs);
    
    Tree makeTuple (std::string name, const std::vector<semantic::InfoType> & types, const std::vector<std::string>& attrs, bool packed = false);

    Tree makeEmptyTuple (ulong size);
        
    Tree makeUnion (std::string name, const std::vector<semantic::InfoType> & types);

    Tree makeTuple (std::string name, const std::vector<semantic::InfoType> & types);

    Tree makeStructType (std::string name, int nbFields, ...);

    Tree makeAuxVar (location_t locus, const std::string& name, Ymir::Tree type);
    
    Tree makeAuxVar (location_t locus, ulong id, Ymir::Tree type);

    Tree makeLabel (location_t loc, std::string &name);

    Tree makeLabel (location_t loc, const char* name);
    
    Tree makeVectorCst (Tree, std::string name, std::vector <Tree> values);
    
    Tree getArrayRef (location_t, Tree array, Tree inner, ulong index);

    Tree getPointerUnref (location_t, Tree ptr, Tree inner, ulong index);

    Tree getArrayRef (location_t, Tree array, Tree inner, Tree index);

    Tree getPointerUnref (location_t, Tree ptr, Tree inner, Tree index);

    Tree getAddr (Tree elem);

    Tree getAddr (location_t, Tree elem);

    void declareGlobal (semantic::Symbol sym, syntax::Expression);

    void declareGlobalWeak (semantic::Symbol sym, syntax::Expression);

    Ymir::Tree declareGlobalWeak (const std::string &name, Ymir::Tree type, Ymir::Tree value);
    
    void declareGlobalExtern (semantic::Symbol sym);

    Tree declareVtable (const std::string&, Tree type, Tree value);
    
    Tree declareVtableExtern (const std::string&, Tree type);

    Tree getVtable (const std::string & name);
    
    void finishCompilation ();
    
    real_value makeRealValue (float elem);

    real_value makeRealValue (double elem);

    tree promote (tree type);

    Tree compoundExpr (location_t, Tree arg0, Tree arg1);
    
    Tree compoundExpr (location_t, TreeStmtList arg0, Tree arg1);

    Tree getExpr (TreeStmtList& list, Tree arg);

    Tree getExpr (TreeStmtList& list, syntax::Expression arg);

    Tree callLib (location_t loc, const std::string & name, Tree ret, std::vector <Tree> params);
}

//tree convert (tre
