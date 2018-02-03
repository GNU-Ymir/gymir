#include <ymir/ast/_.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/FrameTable.hh>
#include <ymir/semantic/pack/PureFrame.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/utils/Mangler.hh>
#include <sys/stat.h>
#include <ymir/Parser.hh>
#include <cstdio>

namespace syntax {

    using namespace semantic;

    bool exists (std::string& file) {
	struct stat buffer;
	return (stat (file.c_str (), &buffer) == 0);
    }

    std::string extension (std::string & name) {
	size_t i = name.find_last_of (".");
	if (i == std::string::npos) return "";
	return name.substr (i);
    }
    
    void IFunction::declare ()  {
	if (this-> ident == Keys::MAIN) {
	    FrameTable::instance ().insert (new (Z0)  IPureFrame (Table::instance ().space (), this));							    
	} else {
	    auto fr = verifyPure (Table::instance ().space ());
	    auto space = Table::instance ().space ();
	    auto it = Table::instance ().getLocal (this-> ident.getStr ());
	    if (it != NULL) {
		if (!it-> type-> is<IFunctionInfo> ()) {
		    Ymir::Error::shadowingVar (ident, it-> sym);
		}		
	    }
	    auto fun = new (Z0)  IFunctionInfo (space, this-> ident.getStr ());
	    fun-> set (fr);
	    Table::instance ().insert (new (Z0)  ISymbol (this-> ident, fun));
	}
    }

    void IFunction::declareAsExtern (semantic::Module mod) {
	if (this-> ident != Keys::MAIN) {
	    auto fr = verifyPureExtern (Table::instance ().space ());
	    auto space = mod-> space ();
	    auto it = mod-> get (this-> ident.getStr ());
	    if (it != NULL) {
		if (!it-> type-> is<IFunctionInfo> ()) {
		    Ymir::Error::shadowingVar (ident, it-> sym);
		}
	    }
	    auto fun = new (Z0)  IFunctionInfo (space, this-> ident.getStr ());
	    auto sym = new (Z0) ISymbol (this-> ident, fun);
	    fun-> set (fr);
	    sym-> isPublic () = this-> is_public ();
	    mod-> insert (sym);	
	}
    }

    Frame IFunction::verifyPureExtern (Namespace space) {
	if (this-> tmps.size () != 0) {
	    auto isPure = verifyTemplates ();
	    auto ret = new (Z0)  ITemplateFrame (space, this);
	    if (!isPure) return ret;
	    for (auto it : this-> params) {
	    	if (!it-> is<ITypedVar> ()) return ret;
	    }

	    ret-> isPure (true);
	    ret-> isExtern () = true;
	    FrameTable::instance ().insert (ret);
	    return ret;	
	}

	for (auto it : this-> params) {
	    if (!it-> is<ITypedVar> ()) {
		return new (Z0)  IUnPureFrame (space, this);
	    }
	}
	
	auto fr = new (Z0)  IExternFrame (space, "", this-> toProto ());
	FrameTable::instance ().insert (fr);
	return fr;
    }


    Proto IFunction::toProto () {
	delete this-> block;
	this-> block = NULL;
	return new (Z0)  IProto (this-> ident, this-> params, false);
    }
    
    Frame IFunction::verifyPure (Namespace space) {
	if (this-> tmps.size () != 0) {
	    auto isPure = verifyTemplates ();
	    auto ret = new (Z0)  ITemplateFrame (space, this);
	    if (!isPure) return ret;
	    for (auto it : this-> params) {
	    	if (!it-> is<ITypedVar> ()) return ret;
	    }

	    ret-> isPure (true);
	    FrameTable::instance ().insert (ret);
	    return ret;
	}

	for (auto it : this-> params) {
	    if (!it-> is<ITypedVar> ()) {
		return new (Z0)  IUnPureFrame (space, this);
	    }
	}

	auto fr = new (Z0)  IPureFrame (space, this);
	FrameTable::instance ().insert (fr);
	return fr;
    }

    bool IFunction::verifyTemplates () {		
	return false;
    }
    
    void IImpl::declare () {}

    void IDeclaration::declareAsExtern (semantic::Module) {
	Ymir::Error::assert ("TODO");
    }
    
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

    void IProgram::declareAsExtern (std::string name, semantic::Module mod) {
	if (extension (name) == ".yr") {
	    name = name.substr (0, name.length () - 3);
	}

	Table::instance ().setCurrentSpace (Namespace (Mangler::mangle_file (name)));
	for (auto it : this-> decls) {
	    it-> declareAsExtern (mod);
	}
    }

    
    void IProto::declare () {       
	Namespace space (this-> space != "" ? this-> space : Table::instance ().space ());
		
	auto fr = new (Z0)  IExternFrame (space, this-> from, this);
	auto fun = new (Z0)  IFunctionInfo (space, this-> ident.getStr ());
	fun-> set (fr);
	Table::instance ().insert (new (Z0)  ISymbol (this-> ident, fun));
    }

    void IProto::declareAsExtern (semantic::Module mod) {
	Namespace space (this-> space != "" ? this-> space : mod-> space ());
	auto fr = new (Z0)  IExternFrame (space, this-> from, this);
	fr-> isPrivate () = !this-> isPublic;
	auto fun = new (Z0)  IFunctionInfo (space, this-> ident.getStr ());
	fun-> set (fr);
	mod-> insert (new (Z0)  ISymbol (this-> ident, fun));
    }
    
    void IImport::declare () {
	auto globSpace = Table::instance ().space ();
	for (auto it : this-> params) {
	    ulong nbErrorBeg = Ymir::Error::nb_errors;
	    std::string name = it.getStr () + ".yr";
	    if (!exists (name)) {
		std::string path = cpp_GCC_INCLUDE_DIR;
		if (path [path.length () - 1] != '/') path += "/";
		name = path + "ymir/" + name;
		if (!exists (name)) {
		    Ymir::Error::moduleDontExist (this-> ident, it);
		    return;
		}
	    }
	    Namespace space (it.getStr ());
	    if (!Table::instance ().moduleExists (space)) {
		auto file = fopen (name.c_str (), "r");
		Ymir::Parser parser (name.c_str (), file);
		auto mod = Table::instance ().addModule (space);
		mod-> addOpen (globSpace);
		auto prg = parser.syntax_analyse ();
		fclose (file);
		prg-> declareAsExtern (it.getStr (), mod);
	    }
	    Table::instance ().openModuleForSpace (space, globSpace);
	    Table::instance ().setCurrentSpace (globSpace);
	    if (Ymir::Error::nb_errors - nbErrorBeg) {
		Ymir::Error::importError (this-> ident);
	    }
	}	
    }

    void IImport::declareAsExtern (semantic::Module mod_) {
	auto globSpace = Table::instance ().space ();
	for (auto it : this-> params) {
	    ulong nbErrorBeg = Ymir::Error::nb_errors;
	    std::string name = it.getStr () + ".yr";
	    if (!exists (name)) {
		std::string path = cpp_GCC_INCLUDE_DIR;
		if (path [path.length () - 1] != '/') path += "/";
		name = path + "ymir/" + name;
		if (!exists (name)) {
		    Ymir::Error::moduleDontExist (this-> ident, it);
		    return;
		}
	    }
	    Namespace space (it.getStr ());
	    if (!Table::instance ().moduleExists (space)) {
		Ymir::Parser parser (name.c_str (), fopen (name.c_str (), "r"));
		auto mod = Table::instance ().addModule (space);
		mod-> addOpen (globSpace);
		auto prg = parser.syntax_analyse ();
		prg-> declareAsExtern (it.getStr (), mod);
		if (this-> isPublic) {
		    mod_-> addPublicOpen (mod-> space ());
		}
	    }
	    Table::instance ().openModuleForSpace (space, globSpace);
	    Table::instance ().setCurrentSpace (globSpace);
	    if (Ymir::Error::nb_errors - nbErrorBeg) {
		Ymir::Error::importError (this-> ident);
	    }
	}
    }

    void IStruct::declare () {
	auto exist = Table::instance ().getLocal (this-> ident.getStr ());
	if (exist) {
	    Ymir::Error::shadowingVar (this-> ident, exist-> sym);
	} else {
	    auto str = new (Z0) IStructCstInfo (Table::instance ().space (), this-> ident.getStr (), this-> tmps);

	    str-> isPublic (true);
	    auto sym = new (Z0) ISymbol (this-> ident, str);
	    Table::instance ().insert (sym);
	    for (auto it : this-> params) {
		str-> addAttrib (it-> to <ITypedVar> ());		
	    }
	}
    }
    
    void IEnum::declare () {
	auto exist = Table::instance ().getLocal (this-> ident.getStr ());
	if (exist) {
	    Ymir::Error::shadowingVar (this-> ident, exist-> sym);
	} else {
	    Symbol type = NULL;
	    Expression fst = NULL;
	    if (this-> type != NULL) {
		type = this-> type-> asType ()-> info;
	    } else {
		fst = this-> values [0]-> expression ();
		type = fst-> info;
	    }

	    auto en = new (Z0) IEnumCstInfo (this-> ident.getStr (), type-> type);
	    auto sym = new (Z0) ISymbol (this-> ident, en);
	    sym-> isPublic () = true;

	    for (auto i : Ymir::r (0, this-> names.size ())) {
		if (i == 0 && fst) {
		    en-> addAttrib (this-> names [i].getStr (), fst, NULL);
		} else {
		    auto val = this-> values [i]-> expression ();
		    auto comp = val-> info-> type-> CompOp (type-> type);
		    if (comp != NULL)
			en-> addAttrib (this-> names [i].getStr (), val, comp);
		    else {
			Ymir::Error::incompatibleTypes (this-> ident, val-> info, type-> type);
			return;
		    }			
		}
	    }
	    
	    Table::instance ().insert (sym);	    
	}
    }

}
