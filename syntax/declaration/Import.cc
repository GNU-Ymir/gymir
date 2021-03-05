#include <ymir/syntax/declaration/Import.hh>
#include <ymir/utils/Path.hh>

namespace syntax {

    Import::Import (const lexing::Word & loc, const std::string & comment, const lexing::Word & module) :
	IDeclaration (loc, comment),
	_module (module)
    {}
    
    Declaration Import::init (const lexing::Word & loc, const std::string & comment, const lexing::Word & module) {
	return Declaration {new (NO_GC) Import (loc, comment, module)};
    }

    void Import::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Import> ", i, '\t');
	stream.writeln (this-> _module);	
    }    
    
    const lexing::Word & Import::getModule () const {
	return this-> _module;
    }
    
    std::string Import::getPath () const {
	auto path = Ymir::Path {this-> _module.getStr (), "::"};
	return path.toString ();
    }

}
