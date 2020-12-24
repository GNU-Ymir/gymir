#include <ymir/syntax/declaration/Import.hh>
#include <ymir/utils/Path.hh>

namespace syntax {

    Import::Import () :
	IDeclaration (lexing::Word::eof ())
    {}

    Import::Import (const lexing::Word & loc, const lexing::Word & module) :
	IDeclaration (loc),
	_module (module)
    {}
    
    Declaration Import::init (const lexing::Word & loc, const lexing::Word & module) {
	return Declaration {new (NO_GC) Import (loc, module)};
    }

    bool Import::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Import thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }

    void Import::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Import> ", i, '\t');
	stream.writeln (this-> _module);	
    }    
    
    const lexing::Word & Import::getModule () const {
	return this-> _module;
    }
    
    std::string Import::getPath () const {
	auto path = Ymir::Path {this-> _module.str, "::"};
	return path.toString ();
    }
        
}
