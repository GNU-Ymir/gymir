#include <ymir/semantic/generator/value/Class.hh>
#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/semantic/generator/value/VarDecl.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {
	
	Class::Class () :
	    Value (),
	    _ref (Symbol::__empty__)
	{
	}

	Class::Class (const lexing::Word & loc, const Symbol & ref) :
	    Value (loc, loc.str, Void::init (loc)),
	    _ref (ref)
	{}

	Generator Class::init (const lexing::Word & loc, const Symbol & ref) {
	    return Generator {new (Z0) Class (loc, ref)};
	}

	Generator Class::clone () const {
	    return Generator {new (Z0) Class (*this)};
	}
		
	bool Class::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Class thisClass; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisClass) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Class::equals (const Generator & gen) const {
	    if (!gen.is<Class> ()) return false;
	    auto str = gen.to <Class> ();
	    return this-> _ref.isSameRef (str._ref);
	}

	std::string Class::getName () const {
	    return this-> _ref.getRealName ();
	}
	
	std::string Class::prettyString () const {
	    std::vector <std::string> content;
	    for (auto & it : this-> _fields) {
		content.push_back (Ymir::entab (it.prettyString ()));
		if (content.back ().size () != 0 && content.back ().back () == '\n')
		    content.back () = content.back ().substr (0, content.back ().size () - 1);
	    }
	    return Ymir::format ("% (%)", this-> _ref.getRealName (), content);
	}

	const Symbol & Class::getRef () const {
	    return this-> _ref;
	}
	
    }
}
