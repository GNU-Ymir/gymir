#include <ymir/syntax/Decorator.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/errors/Error.hh>

namespace syntax {
    namespace Decorators {

	std::vector <std::string> members () {
	    return {
		Keys::REF, Keys::CONST, Keys::MUTABLE, Keys::STATIC, Keys::IMMUTABLE
	    };
	}

	Decorator init (const lexing::Word & content) {
	    if (content == Keys::REF) return Decorator::REF;
	    if (content == Keys::CONST) return Decorator::CONST;
	    if (content == Keys::MUTABLE) return Decorator::MUT;
	    if (content == Keys::IMMUTABLE) return Decorator::CTE;
	    if (content == Keys::STATIC) return Decorator::STATIC;

	    Ymir::Error::halt ("%(r) - reaching unhandled point", "Critical");
	    return Decorator::REF;
	}
	

    }
}
