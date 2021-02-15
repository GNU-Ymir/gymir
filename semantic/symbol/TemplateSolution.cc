#include <ymir/semantic/symbol/TemplateSolution.hh>
#include <ymir/semantic/generator/value/TemplateSyntaxWrapper.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Match.hh>
#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    TemplateSolution::TemplateSolution () :
	ISymbol (lexing::Word::eof (), "", false)
    {}
    

    TemplateSolution::TemplateSolution (const lexing::Word & loc, const std::string & comments, const std::vector<syntax::Expression> & templs, const std::map<std::string, syntax::Expression> & params, const std::vector <std::string> & nameOrders, bool isWeak) :
	ISymbol (loc, comments, isWeak),
	_templs (templs), 
	_params (params),
	_nameOrder (nameOrders)
    {}
    
    Symbol TemplateSolution::init (const lexing::Word & name, const std::string & comments, const std::vector <syntax::Expression> & templs, const std::map <std::string, syntax::Expression> & mapping, const std::vector <std::string> & nameOrders, bool isWeak) {
	auto ret =  Symbol {new (NO_GC) TemplateSolution (name, comments, templs, mapping, nameOrders, isWeak)};
	ret.to <TemplateSolution> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    void TemplateSolution::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }

    void TemplateSolution::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    void TemplateSolution::get (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().get (name, rets);
	this-> _table-> get (name, rets);	
    }

    void TemplateSolution::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().getPublic (name, rets);
	this-> _table-> getPublic (name, rets);
    }
    
    void TemplateSolution::getLocal (const std::string & name, std::vector<Symbol> & rets) const {
	this-> _table-> get (name, rets);
    }

    void TemplateSolution::getLocalPublic (const std::string & name, std::vector<Symbol> & rets) const {
	this-> _table-> getPublic (name, rets);
    }

    const std::vector <Symbol> & TemplateSolution::getAllLocal () const {
	return this-> _table-> getAll ();
    }
    
    bool TemplateSolution::equals (const Symbol & other, bool parent) const {
	if (!other.is<TemplateSolution> ()) return false;
	if (this-> getName ().isSame (other.getName ())) {
	    auto & ot = other.to<TemplateSolution> ();	    
	    for (auto & it : _params) {
		auto _it = ot._params.find (it.first);
		if (_it == ot._params.end ()) return false;
		else if (_it-> second.is <generator::TemplateSyntaxWrapper> () && it.second.is <generator::TemplateSyntaxWrapper> ()) {		    
		    if (!_it-> second.to <generator::TemplateSyntaxWrapper> ().getContent ().equals (it.second.to <generator::TemplateSyntaxWrapper> ().getContent ())) {
			return false;
		    }
		} else if (_it-> second.is <generator::TemplateSyntaxList> () && it.second.is <generator::TemplateSyntaxList> ()) {
		    if (_it-> second.to <generator::TemplateSyntaxList> ().getContents ().size () != it.second.to <generator::TemplateSyntaxList> ().getContents ().size ())
			return false;
		    for (auto j : Ymir::r (0, _it-> second.to <generator::TemplateSyntaxList> ().getContents ().size ())) {
			if (!_it-> second.to <generator::TemplateSyntaxList> ().getContents ()[j].equals (it.second.to <generator::TemplateSyntaxList> ().getContents () [j]))
			    return false;
		    }
		} else if (_it-> second.prettyString () != it.second.prettyString ()) // Single value
		    return false;
	    }

	    if (_nameOrder.size () != ot._nameOrder.size ()) return false;
	    for (auto it : Ymir::r (0, _nameOrder.size ())) {
		if (_nameOrder [it] != ot._nameOrder [it]) return false;
	    }
	    
	    auto & inner = this-> getAllLocal ();
	    auto & aux_inner = ot.getAllLocal ();
	    if (inner.size () != aux_inner.size ()) return false;
	    for (auto it : Ymir::r (0, inner.size ())) {
		if (!inner [it].equals (aux_inner [it], false)) return false;
	    }

	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else 
	    return false;
    }    

    std::string TemplateSolution::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table-> getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }
    
    std::string TemplateSolution::getRealName () const {
	Ymir::OutBuffer buf;
	buf.writef ("%", this-> getName ().getStr ());
	int i = 0;
	buf.write ("(");
	for (auto & it : _nameOrder) {
	    if (i != 0)
		buf.write (",");
	    buf.write (this-> _params.find (it)-> second.prettyString ()); // [] on map discard const qualifier ?!!
	    i += 1;
	}
	buf.write (")");
	
	if (this-> getReferent ().isEmpty ()) return buf.str ();
	else {
	    auto ft = this-> getReferent ().getRealName ();
	    if (ft != "") return ft + "::" + buf.str ();
	    else return buf.str ();
	}
    }

    std::string TemplateSolution::getSolutionName () const {
	Ymir::OutBuffer buf;
	buf.writef ("%", this-> getName ().getStr ());
	int i = 0;
	buf.write ("(");
	for (auto & it : _nameOrder) {
	    if (i != 0)
		buf.write (",");
	    buf.write (this-> _params.find (it)-> first,"=> ",this-> _params.find (it)-> second.prettyString ()); // [] on map discard const qualifier ?!!
	    i += 1;
	}
	buf.write (")");
	
	return buf.str ();	
    }

    
    std::string TemplateSolution::getMangledName () const {
	Ymir::OutBuffer buf;
	buf.write (this-> getName ().getStr ());
	generator::Mangler mangler = generator::Mangler::init ();
	for (auto & it : _nameOrder) {
	    auto second = this-> _params.find (it)-> second;
	    if (second.is <generator::TemplateSyntaxWrapper> ()) {
		match (second.to <generator::TemplateSyntaxWrapper> ().getContent ()) {
		    of (generator::Type, t, {
			    auto aux = generator::Type::init (t, false, t.isRef ());
			    buf.write (Ymir::format ("N%", mangler.mangle (aux))); // [] on map discard const qualifier ?!!	    
			}
		    ) else {
			buf.write (Ymir::format ("N%", mangler.mangle (second.to <generator::TemplateSyntaxWrapper> ().getContent ()))); // [] on map discard const qualifier ?!!
		    }
		}
	    } else {
		Ymir::OutBuffer innerBuf;
		for (auto & it : second.to <generator::TemplateSyntaxList> ().getContents ()) {
		    match (it) {
			of (generator::Type, t, {
				auto aux = generator::Type::init (t, false, t.isRef ());
				innerBuf.writef ("N%", mangler.mangle (aux));
			    }
			) else innerBuf.writef ("N%", mangler.mangle (it));
		    }
		}
		buf.writef ("V%", innerBuf.str ());
	    }
	}
	
	if (this-> getReferent ().isEmpty ()) return buf.str ();
	else {
	    auto ft = this-> getReferent ().getMangledName ();
	    if (ft != "") return ft + "::" + buf.str ();
	    else return buf.str ();
	}
    }

    const std::vector <syntax::Expression> & TemplateSolution::getTempls () const {
	return this-> _templs;
    }

    const std::map <std::string, syntax::Expression> & TemplateSolution::getParams () const {
	return this-> _params;
    }
    
}
