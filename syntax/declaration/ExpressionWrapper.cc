#include <ymir/syntax/declaration/ExpressionWrapper.hh>

namespace syntax {

    ExpressionWrapper::ExpressionWrapper () :
	IDeclaration (lexing::Word::eof (), ""),
	_content (Expression::empty ())
    {}

    ExpressionWrapper::ExpressionWrapper (const lexing::Word & loc, const std::string & comment, const Expression & content) :
	IDeclaration (loc, comment),
	_content (content)
    {}
    
    Declaration ExpressionWrapper::init (const lexing::Word & loc, const std::string & comment, const Expression & content) {
	return Declaration {new (NO_GC) ExpressionWrapper (loc, comment, content)};
    }

    const Expression & ExpressionWrapper::getContent () const {
	return this-> _content;
    }

    void ExpressionWrapper::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Expression> : ");	
    }
    
}
