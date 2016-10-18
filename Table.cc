#include "Table.hh"

namespace Semantic {

    Table Table::inst;
    
    void Table::enterBlock () {
	frameTable.front().enterBlock ();
    }

    void Table::quitBlock () {
	frameTable.front ().quitBlock ();
    }

    void Table::enterFrame (const std::string& space) {
	frameTable.push_front (FrameScope (space));
	return_types.push_front (NULL);
    }

    void Table::quitFrame () {
	if (!frameTable.empty ()) {
	    frameTable.pop_front ();
	    return_types.pop_front ();
	}
    }

    void Table::addImport (Package * pck) {
	globalScope.addImport (pck);
    }

    void Table::insert (const std::string & name, SymbolPtr sym) {
	if (frameTable.empty ()) {
	    globalScope.insert (name, sym);
	} else {
	    frameTable.front ().insert (name, sym);
	}
    }

    SymbolPtr Table::get (const std::string & name) {
	if (frameTable.empty()) {
	    return globalScope.get (name);
	} else {
	    auto it = frameTable.front ().get (name);
	    if (it-> isVoid ())
		return globalScope.get (name);
	    else return it;
	}
    }

    std::string Table::space () const {
	if (frameTable.empty ())
	    return "";
	else return frameTable.front ().space ();
    }

    TypeInfo *& Table::return_type () {
	return return_types.front ();
    }

    bool Table::alreadyReturned () const {
	return false;
    }
    
}
