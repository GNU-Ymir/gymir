#include <ymir/syntax/declaration/Template.hh>

namespace syntax {

    Template::Template () :
	IDeclaration (lexing::Word::eof (), ""),
	_content (Declaration::empty ()),
	_test (Expression::empty ())
    {}

    Template::Template (const lexing::Word & loc, const std::string & comment, const std::vector <Expression> & params, const Declaration & content, const Expression & test) :
	IDeclaration (loc, comment),
	_parameters (params),
	_content (content),
	_test (test)
    {}
    
    Declaration Template::init (const lexing::Word & loc, const std::string & comment, const std::vector <Expression> & params, const Declaration & content, const Expression & test) {
	return Declaration {new (NO_GC) Template (loc, comment, params, content, test)};
    }

    void Template::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Template>", i, '\t');
	stream.writeln (this-> getLocation ());
	stream.writefln ("%*<Test>", i + 1, '\t');
	this-> _test.treePrint (stream, i + 2);
	stream.writefln ("%*<Params>", i + 1, '\t');
	for (auto & it : this-> _parameters)
	    it.treePrint (stream, i + 2);
	stream.writefln ("%*<Content>", i + 1, '\t');
	this-> _content.treePrint (stream, i + 2);
    }	
    
    bool Template::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Template thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    const std::vector <Expression> & Template::getParams () const {
	return this-> _parameters;
    }

    const Declaration & Template::getContent () const {
	return this-> _content;
    }

    const Expression & Template::getTest () const {
	return this-> _test;
    }    
    
}
