#include "TypeInfo.hh"
#include "Expression.hh"
#include "IntInfo.hh"

using namespace Ymir;

namespace Semantic {

    std::vector<TypeInfo*> TypeInfo::all;
    std::map<std::string, TypeInfo::TypeCreator> TypeInfo::creators;
    
    TypeInfo::TypeInfo () {
	id = all.size ();
	all.push_back (this);
    }
    
    Tree TypeInfo::buildBinaryOp (location_t locus, Syntax::Expression* left, Syntax::Expression* right) {
	return this->binopFoo (locus, left, right);
    }

    Tree TypeInfo::buildUnaryOp (Syntax::Expression* elem) {
	return this->unopFoo (elem);
    }

    Tree TypeInfo::buildMultOp (location_t locus, Syntax::Expression* left, Syntax::Expression* rights) {
	return this->multFoo (locus, left, rights);
    }
        
    
    TypeInfo * TypeInfo::typeOp (Syntax::Expression *) {
	return NULL;
    }


}
