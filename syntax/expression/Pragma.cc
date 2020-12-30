#include <ymir/syntax/expression/Pragma.hh>

namespace syntax {

    Pragma::Pragma () :
	IExpression (lexing::Word::eof ())
    {}
    
    Pragma::Pragma (const lexing::Word & loc, const std::vector <Expression> & params) :
	IExpression (loc),
	_params (params)
    {}

    Expression Pragma::init (const lexing::Word & location, const std::vector <Expression> & params) {
	return Expression {new (NO_GC) Pragma (location, params)};
    }

    bool Pragma::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Pragma thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Pragma::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Pragma> ", i, '\t');
	stream.writeln (this-> getLocation ());
	for (auto & it : this-> _params)
	    it.treePrint (stream, i + 1);
    }
    
    const std::vector <Expression> & Pragma::getContent () const {
	return this-> _params;
    }

    std::string Pragma::prettyString () const {
	Ymir::OutBuffer buf;
	buf.writef ("__pragma!%(", this-> getLocation ().getStr ());
	int i = 0;
	for (auto &it : this-> _params) {
	    if (i != 0) buf.write (", ");
	    buf.write (it.prettyString ());
	    i += 1;
	}
	buf.write (")");
	return buf.str ();
    }
    
}
