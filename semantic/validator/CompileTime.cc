#include <ymir/semantic/validator/CompileTime.hh>

namespace semantic {

    namespace validator {

	using namespace generator;

	CompileTime::CompileTime (Visitor & context) :
	    _context (context)
	{}

	CompileTime CompileTime::init (Visitor & context) {
	    return CompileTime (context);
	}

	Generator CompileTime::execute (const Generator & gen) {
	    match (gen) {
		of (Fixed, f ATTRIBUTE_UNUSED,
		    return gen;
		);
	    }
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return gen;
	}
	
    }
    
}
