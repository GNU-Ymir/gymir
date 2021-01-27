#include <ymir/semantic/generator/value/ThrowBlock.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {
    namespace generator {

	ThrowBlock::ThrowBlock (const lexing::Word & loc, const Generator & content, const std::string & name) :
	    Value (loc, Void::init (loc)),
	    _content (content),
	    _name (name)
	{
	    this-> setThrowers (content.getThrowers ());
	}

	Generator ThrowBlock::init (const lexing::Word & loc, const Generator & content, const std::string & name) {
	    return Generator {new (NO_GC) ThrowBlock (loc, content, name)};
	}

	const Generator & ThrowBlock::getContent () const {
	    return this-> _content;
	}

	const std::string & ThrowBlock::getName () const {
	    return this-> _name;
	}

	std::string ThrowBlock::prettyString () const {
	    return this-> _content.prettyString ();
	}
	
    }
}
