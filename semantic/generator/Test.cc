#include <ymir/semantic/generator/Test.hh>
#include <ymir/semantic/generator/value/VarDecl.hh>

namespace semantic {
    namespace generator {

	Test::Test () :
	    IGenerator (lexing::Word::eof (), ""),
	    _value (Generator::empty ())
	{
	}

	Test::Test (const lexing::Word & loc, const std::string & name, const Generator & value) :
	    IGenerator (loc, name),
	    _value (value)
	{}

	Generator Test::init (const lexing::Word & loc, const std::string & name, const Generator & value) {
	    return Generator { new (NO_GC) Test (loc, name, value) };
	}

	Generator Test::clone () const {
	    return Generator { new (NO_GC) Test (*this) };
	}

	bool Test::equals (const Generator & other) const {
	    if (!other.is <Test> ()) return false;
	    return other.getName () == this-> getName ();
	}

	const Generator & Test::getValue () const {
	    return this-> _value;
	}
	
    }
}
