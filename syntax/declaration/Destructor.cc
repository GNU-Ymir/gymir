#include <ymir/syntax/declaration/Destructor.hh>

namespace syntax {

    Destructor::Destructor () {}    

    Destructor::Destructor (const lexing::Word & name, const std::string & comments, const syntax::Expression & param, const syntax::Expression & body) :
	Function (name, comments, Function::Prototype::init ({param}, Expression::empty (), false), body, {}, {}, false, false)
    {}
    
    Declaration Destructor::init (const lexing::Word & name, const std::string & comments, const syntax::Expression & param, const syntax::Expression & body) {
	return Declaration {new (NO_GC) Destructor (name, comments, param, body)};
    }
        
}
