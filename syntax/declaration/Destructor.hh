#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/syntax/Decorator.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    class Destructor : public Function {
    private :

	Destructor () ;

	Destructor (const lexing::Word & name, const std::string & comments, const syntax::Expression & param, const syntax::Expression & body);

    public :

	static Declaration init (const lexing::Word & name, const std::string & comments, const syntax::Expression & param, const syntax::Expression & body);
	
    };
    
}
