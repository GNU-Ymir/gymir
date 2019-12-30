#include <ymir/semantic/generator/value/Class.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
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
	    Value (loc, loc.str, NoneType::init (loc, "class " + ref.getRealName ())),
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

	const std::vector <generator::Generator> & Class::getFields () const {
	    return this-> _fields;
	}

	void Class::setFields (const std::vector <generator::Generator> & fields) {
	    this-> _fields = fields;
	}

	const std::vector <generator::Generator> & Class::getVtable () const {
	    return this-> _vtable;
	}

	void Class::setVtable (const std::vector <generator::Generator> & vtable) {
	    this-> _vtable = vtable;
	}
	
	std::string Class::getName () const {
	    return this-> _ref.getRealName ();
	}
	
	std::string Class::prettyString () const {
	    return Ymir::format ("%", this-> _ref.getRealName ());
	}

	const Symbol & Class::getRef () const {
	    return this-> _ref;
	}
	
    }
}
