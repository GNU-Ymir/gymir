#pragma once

#include <ymir/syntax/Word.hh>
#include <ymir/syntax/Token.hh>
#include <ymir/semantic/tree/Tree.hh>

namespace semantic {
    
    namespace OperatorUtils {

	tree_code toGeneric (Word token);
	tree_code toGenericReal (Word token);
	
    }
    
}
