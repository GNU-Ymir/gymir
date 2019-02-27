#include <ymir/syntax/Decorator.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/errors/Error.hh>

namespace syntax {

    DecoratorWord::DecoratorWord (const lexing::Word & loc, Decorator deco) :
	_value (deco),
	_loc (loc)
    {}


    std::vector <std::string> DecoratorWord::members () {
	return {
	    Keys::REF, Keys::CONST, Keys::MUTABLE, Keys::STATIC, Keys::IMMUTABLE
		};
    }
    
    DecoratorWord DecoratorWord::init (const lexing::Word & content) {
	if (content == Keys::REF) return DecoratorWord (content, Decorator::REF);
	if (content == Keys::CONST) return DecoratorWord (content, Decorator::CONST);
	if (content == Keys::MUTABLE) return DecoratorWord (content, Decorator::MUT);
	if (content == Keys::IMMUTABLE) return DecoratorWord (content, Decorator::CTE);
	if (content == Keys::STATIC) return DecoratorWord (content, Decorator::STATIC);

	Ymir::Error::halt ("%(r) - reaching unhandled point", "Critical");
	return DecoratorWord (content, Decorator::REF);
    }

    const lexing::Word & DecoratorWord::getLocation () const {
	return this-> _loc;
    }

    Decorator DecoratorWord::getValue () const {
	return this-> _value;
    }

}
