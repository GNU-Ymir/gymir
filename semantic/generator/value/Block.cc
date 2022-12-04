#include <ymir/semantic/generator/value/Block.hh>

namespace semantic {

    namespace generator {

	Block::Block () :
	    Value ()
	{}

	Block::Block (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content, bool isLvalue) :
	    Value (loc, type),
	    _content (content)
	{
	    this-> isLvalue (isLvalue);
	    std::vector <Generator> thr;
	    for (auto & it : this-> _content) {
		auto &ith = it.getThrowers ();
		thr.insert (thr.end (), ith.begin (), ith.end ());
	    }
	    
	    this-> setThrowers (thr);
	}
       
	Generator Block::init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & values) {
	    return Generator {new (NO_GC) Block (loc, type, values, false)};
	}

	Generator LBlock::init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & values) {
	    return Generator {new (NO_GC) Block (loc, type, values, true)};
	}

	Generator Block::clone () const {
	    return Generator {new (NO_GC) Block (*this)};
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

	std::string Block::prettyString () const {
	    if (this-> _content.size () == 1) {
		auto inner = this-> _content[0].prettyString ();
		if (inner.length () != 0) {
		    return inner;
		} else {
		    return "{}";
		}
	    } else {
		Ymir::OutBuffer buf ("{");
		uint64_t i = 0;
		for (auto & it : this-> _content) {
		    buf.write (Ymir::entab ("\n" + it.prettyString ()));
		    i += 1;
		    if (i != this-> _content.size ())
		    buf.write (";");
		}
		buf.writeln ("\n}");
		return buf.str ();
	    }
	}
	
    }
    
}
