#include <ymir/syntax/declaration/ExpressionWrapper.hh>

namespace syntax {

    ExpressionWrapper::ExpressionWrapper () :
	_content (Expression::empty ())
    {}

    Declaration ExpressionWrapper::init () {
	return Declaration {new (Z0) ExpressionWrapper ()};
    }

    Declaration ExpressionWrapper::init (const ExpressionWrapper & gl) {
	auto ret = new (Z0) ExpressionWrapper ();
	ret-> _content = gl._content;
	return Declaration {ret};
    }

    Declaration ExpressionWrapper::init (const Expression & content) {
	auto ret = new (Z0) ExpressionWrapper ();
	ret-> _content = content;
	return Declaration {ret};
    }

    Declaration ExpressionWrapper::clone () const {
	return ExpressionWrapper::init (*this);
    }

    bool ExpressionWrapper::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	ExpressionWrapper thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }

    const Expression & ExpressionWrapper::getContent () const {
	return this-> _content;
    }

    void ExpressionWrapper::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Expression> : ");	
    }
    
}
