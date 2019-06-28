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

    class DecoratorWord {

	Decorator _value;

	lexing::Word _loc;

    private :

	DecoratorWord (const lexing::Word & loc, Decorator deco);
	
    public :
	
	static std::vector <std::string> members ();
	
	static DecoratorWord init (const lexing::Word & content);

	static DecoratorWord init (Decorator deco);

	const lexing::Word & getLocation () const;

	Decorator getValue () const;
	
	
    };
    
       
}
