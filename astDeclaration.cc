#include <ymir/ast/_.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/FrameTable.hh>
#include <ymir/semantic/pack/PureFrame.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/utils/Mangler.hh>

namespace syntax {

    using namespace semantic;
    
    void IFunction::declare ()  {
	if (this-> ident == Keys::MAIN) {
	    FrameTable::instance ().insert (new IPureFrame (Table::instance ().space (), this));							    
	} else {
	    auto fr = verifyPure (Table::instance ().space ());
	    auto space = Table::instance ().space ();
	    auto it = Table::instance ().getLocal (this-> ident.getStr ());
	    if (it != NULL) {
		if (!it-> type-> is<IFunctionInfo> ()) {
		    Ymir::Error::shadowingVar (ident, it-> sym);
		}		
	    }
	    auto fun = new IFunctionInfo (space, this-> ident.getStr ());
	    fun-> set (fr);
	    Table::instance ().insert (new ISymbol (this-> ident, fun));
	}
    }

    Frame IFunction::verifyPure (Namespace space) {
	if (this-> tmps.size () != 0) {
	    // auto isPure = verifyTemplates ();
	    // auto ret = new TemplateFrame (space, this);
	    // if (!isPure) return ret;
	    // for (auto it : this-> params) {
	    // 	if (!it-> is<ITypedVar> ()) return ret;
	    // }

	    // ret-> isPure = true;
	    // FrameTable::instance ().insert (ret);
	    // return ret;
	}

	for (auto it : this-> params) {
	    if (!it-> is<ITypedVar> ()) {
		return new IUnPureFrame (space, this);
	    }
	}

	auto fr = new IPureFrame (space, this);
	FrameTable::instance ().insert (fr);
	return fr;
    }
    
    void IImpl::declare () {}
    
    void IProgram::declare () {
	std::string name = LOCATION_FILE (this-> locus.getLocus ());
	auto dot = name.find_last_of ('.');
	if (dot != name.npos && name.substr (dot, name.length () - dot) == ".yr") {
	    name = name.substr (0, dot);
	}

	Table::instance ().setCurrentSpace ({Mangler::mangle_file (name)});
	Table::instance ().programNamespace () = Table::instance ().globalNamespace ();
	Table::instance ().addForeignModule (Table::instance ().globalNamespace ());

	for (auto it : this-> decls) {
	    it-> declare ();
	}
	
    }    

    void IProto::declare () {       
	Namespace space (this-> space != "" ? this-> space : Table::instance ().space ());
	
	for (auto it : Ymir::r (0, this-> _params.size ())) {
	    if (!this-> _params [it]-> is<ITypedVar> ()) {
		this-> _params [it] = new ITypedVar (Word (this-> _params [it]-> token.getLocus (), "_"), this-> _params [it]);
	    }
	}
	

	auto fr = new IExternFrame (space, this-> from, this);
	auto fun = new IFunctionInfo (space, this-> ident.getStr ());
	fun-> set (fr);
	Table::instance ().insert (new ISymbol (this-> ident, fun));
    }
    
    
}
