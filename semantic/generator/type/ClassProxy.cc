#include <ymir/semantic/generator/type/ClassProxy.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/Error.hh>

namespace semantic {
    namespace generator {
	
	ClassProxy::ClassProxy () :
	    ClassPtr (lexing::Word::eof (), Generator::empty ()),
	    _proxy (Generator::empty ())
	{}

	ClassProxy::ClassProxy (const lexing::Word & loc, const Generator & type, const Generator & ancestor) :
	    ClassPtr (loc, type),
	    _proxy (ancestor)
	{}

	Generator ClassProxy::init (const lexing::Word & loc, const Generator & type, const Generator & ancestor) {
	    return Generator {new (NO_GC) ClassProxy (loc, type, ancestor)};
	}

	Generator ClassProxy::clone () const {
	    return Generator {new (NO_GC) ClassProxy (*this)};
	}
		
	bool ClassProxy::equals (const Generator & gen) const {
	    if (!gen.is<ClassProxy> ()) return false;
	    auto array = gen.to <ClassProxy> ();
	    return this-> _proxy.equals (array._proxy) && ClassPtr::equals (gen);
	}

	const ClassRef & ClassProxy::getProxyRef () const {
	    return this-> _proxy.to <ClassRef> ();
	}
	
	std::string ClassProxy::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.writef ("&(%)::%", this-> _proxy.to <ClassRef> ().getTypeName (this-> isMutable ()), this-> _ref.to <ClassRef> ().getTypeName (false), ")");
	    return buf.str ();
	}
		
    }
}
