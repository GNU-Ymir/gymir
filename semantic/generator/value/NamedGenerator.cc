#include <ymir/semantic/generator/value/NamedGenerator.hh>

namespace semantic {
    namespace generator {

	NamedGenerator::NamedGenerator () :
	    Value (lexing::Word::eof (), Generator::empty ()),
	    _content (Generator::empty ())
	{}
    
	NamedGenerator::NamedGenerator (const lexing::Word & loc, const Generator & content) :
	    Value (loc, content.to<Value> ().getType ()),
	    _content (content)
	{}
    
	Generator NamedGenerator::init (const lexing::Word & location, const Generator & content) {
	    return Generator {new (Z0) NamedGenerator (location, content)};
	}
        
	Generator NamedGenerator::clone () const {
	    return Generator {new NamedGenerator (*this)};
	}

	bool NamedGenerator::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    NamedGenerator thisType; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	    return Value::isOf (type);
	}

	bool NamedGenerator::equals (const Generator & gen) const {
	    if (!gen.is <NamedGenerator> ()) return false;
	    auto named = gen.to<NamedGenerator> ();	    
	    if (named.getLocation ().str != this-> getLocation ().str) return false;
	    return this-> _content.equals (named.getContent ());
	}

	const Generator & NamedGenerator::getContent () const {
	    return this-> _content;
	}
    
    }
}
