#include <ymir/semantic/generator/type/ClassProxy.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/validator/Visitor.hh>

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

	bool ClassProxy::isCompatible (const Generator & gen) const {
	    if (this-> equals (gen)) return true;
	    return validator::Visitor::isAncestor (this-> getInners ()[0], gen);
	}	

	const ClassRef & ClassProxy::getProxyRef () const {
	    return this-> _proxy.to <ClassRef> ();
	}

	bool ClassProxy::needExplicitAlias () const {
	    return true;
	}

	bool ClassProxy::containPointers () const {
	    return true;
	}

	
	std::string ClassProxy::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.writef ("&(%)::%", this-> _proxy.to <ClassRef> ().computeTypeName (this-> isMutable ()), this-> _ref.to <ClassRef> ().computeTypeName (false), ")");
	    return buf.str ();
	}
		
    }
}
