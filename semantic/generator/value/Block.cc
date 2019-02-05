#include <ymir/semantic/generator/value/Block.hh>

namespace semantic {

    namespace generator {

	Block::Block () :
	    Value ()
	{}

	Block::Block (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) :
	    Value (loc, type),
	    _content (content)
	{}
       
	Generator Block::init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & values) {
	    return Generator {new (Z0) Block (loc, type, values)};
	}

	Generator Block::clone () const {
	    return Generator {new (Z0) Block (*this)};
	}

	bool Block::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Block thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Block::equals (const Generator & gen) const {
	    if (!gen.is <Block> ()) return false;
	    auto block = gen.to<Block> ();
	    if (block._content.size () != this-> _content.size ()) return false;
	    if (!this-> getType ().equals (block.getType ())) return false;

	    for (auto it : Ymir::r (0, this-> _content.size ())) {
		if (!this-> _content [it].equals (block._content [it])) return false;
	    }
	    return true;
	}

	const std::vector <Generator> & Block::getContent () const {
	    return this-> _content;
	}

	void Block::setContent (const std::vector<Generator> & content) {
	    this-> _content = content;
	}
	
    }
    
}