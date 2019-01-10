#pragma once

#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    /**
     * \enum Decorator 
     * The decorators are refering to the a way of typing a variable (const, mutability, ...)
     * Or to the segment of data where the variable will be stored (static, shared, ...)
     */
    enum class Decorator : int {
	REF,
	CONST,
	MUT,
	STATIC, 
	CTE
    };
    
    namespace Decorators {

	std::vector <std::string> members ();

	Decorator init (const lexing::Word & content);
	
    };


}
