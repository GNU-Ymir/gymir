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
    
    Tree TypeInfo::buildBinaryOp (Syntax::Expression* left, Syntax::Expression* right) {
	return this->binopFoo (left, right);
    }

    Tree TypeInfo::buildUnaryOp (Syntax::Expression* elem) {
	return this->unopFoo (elem);
    }

    Tree TypeInfo::buildMultOp (Syntax::Expression* left, Syntax::Expression* rights) {
	return this->multFoo (left, rights);
    }
        
    

}
