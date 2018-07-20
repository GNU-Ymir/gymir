#include <ymir/ast/_.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/FrameTable.hh>
#include <ymir/semantic/pack/PureFrame.hh>
#include <ymir/semantic/pack/MethodFrame.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/object/_.hh>
#include <ymir/utils/Mangler.hh>
#include <sys/stat.h>
#include <ymir/Parser.hh>
#include <cstdio>
#include <ymir/utils/Options.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/value/LambdaValue.hh>
#include <unistd.h>
#include <dirent.h>

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
	if (!this-> verifUdas ()) return;
	if (this-> ident == Keys::MAIN) {
	    auto it = Table::instance ().get (this-> ident.getStr ());
	    if (it != NULL) {
		Ymir::Error::shadowingVar (ident, it-> sym);
	    }
	    FrameTable::instance ().insert (new (Z0)  IPureFrame (Table::instance ().space (), this));
	    Table::instance ().insert (new (Z0) ISymbol (this-> ident, NULL, new (Z0) IVoidInfo ()));
	} else {
	    auto space = Table::instance ().space ();
	    auto fr = verifyPure (space);
	    auto it = Table::instance ().getLocal (this-> ident.getStr ());
	    Value val = NULL;
	    if (it != NULL) {
		if (!it-> type ()-> is<IFunctionInfo> ()) {
		    Ymir::Error::shadowingVar (ident, it-> sym);
		} else {
		    val = it-> type ()-> value ();
		}
	    }

	    if (val == NULL) val = new (Z0) ILambdaValue (fr);
	    else val-> to <ILambdaValue> ()-> push (fr);
	    
	    auto fun = new (Z0)  IFunctionInfo (space, this-> ident.getStr ());
	    fun-> set (fr);	    
	    fun-> value () = val;
	    Table::instance ().insert (new (Z0)  ISymbol (this-> ident, NULL, fun));
	}
    }

    void IFunction::declare (Module mod)  {
	if (!this-> verifUdas ()) return;
	if (this-> ident == Keys::MAIN) {
	    Ymir::Error::mainInModule (this-> ident);
	    return;
	}
	
	auto fr = verifyPure (mod-> space ());
	auto space = mod-> space ();
	auto it = mod-> get (this-> ident.getStr ());
	Value val = NULL;
	if (it != NULL) {
	    if (!it-> type ()-> is<IFunctionInfo> ()) {
		Ymir::Error::shadowingVar (ident, it-> sym);
		return;
	    } else if (it-> isPublic () != this-> is_public ()) {
		Ymir::Error::shadowingVar (ident, it-> sym, this-> is_public ());
		return;
	    } else val = it-> type ()-> value ();
	}

	if (val == NULL) val = new (Z0) ILambdaValue (fr);
	else val-> to <ILambdaValue> ()-> push (fr);
	
	auto fun = new (Z0)  IFunctionInfo (space, this-> ident.getStr ());
	auto sym = new (Z0)  ISymbol (this-> ident, NULL, fun);
	fun-> set (fr);
	sym-> isPublic () = this-> is_public ();
	fun-> value () = val;
	mod-> insert (sym);	
    }

    void IFunction::declareAsExtern (semantic::Module mod) {
	if (!this-> verifUdas ()) return;
	if (this-> ident != Keys::MAIN) {
	    auto fr = verifyPureExtern (mod-> space ());
	    auto space = mod-> space ();
	    auto it = mod-> get (this-> ident.getStr ());
	    Value val = NULL;
	    if (it != NULL) {
		if (!it-> type ()-> is<IFunctionInfo> ()) {
		    Ymir::Error::shadowingVar (ident, it-> sym);
		    return;
		} else if (it-> isPublic () != this-> is_public ()) {
		    Ymir::Error::shadowingVar (ident, it-> sym, this-> is_public ());
		    return;
		} else val = it-> type ()-> value ();
	    }
	    
	    if (val == NULL) val = new (Z0) ILambdaValue (fr);
	    else val-> to <ILambdaValue> ()-> push (fr);
	
	    auto fun = new (Z0)  IFunctionInfo (space, this-> ident.getStr ());
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, fun);
	    fun-> set (fr);
	    sym-> isPublic () = this-> is_public ();
	    fun-> value () = val;
	    mod-> insert (sym);	
	}
    } 
    
    bool IFunction::verifUdas () {
	for (auto it : this-> attrs) {
	    if (it != Keys::INLINE &&
		it != Keys::SAFE &&
		it != Keys::NOGC &&
		it != Keys::FUNCTIONAL &&
		it != Keys::TRUSTED
	    ) {
		Ymir::Error::undefUda (this-> ident, it);
		return false;
	    } 
	}
	return true;
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

	if (!this-> has (Keys::INLINE)) {	    
	    auto fr = new (Z0)  IExternFrame (space, "", this-> toProto ());
	    fr-> attributes () = this-> attrs;
	    FrameTable::instance ().insert (fr);
	    return fr;
	} else {
	    auto fr = new (Z0)  IPureFrame (space, this);
	    FrameTable::instance ().insert (fr);
	    return fr;
	}
    }


    Proto IFunction::toProto () {
	delete this-> block;
	this-> block = NULL;
	auto ret = new (Z0)  IProto (this-> ident, this-> params, false);
	ret-> type () = this-> type;
	ret-> retDeco () = this-> retDeco;
	ret-> externLang () = this-> _externLang;
	ret-> externLangSpace () = this-> _externLangSpace;
	return ret;
    }
    
    Frame IFunction::verifyPure (Namespace space) {
	if (this-> tmps.size () != 0) {
	    if (this-> _externLang != "") {
		Ymir::Error::unpureExternC (this-> ident);
	    } 
	    
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
	fr-> externLang () = this-> _externLang;
	FrameTable::instance ().insert (fr);
	return fr;
    }

    bool IFunction::verifyTemplates () {		
	std::vector <Word> dones;
	for (auto it : this-> tmps) {
	    if (auto var = it-> to <IVar> ()) {
		for (auto w : dones) {
		    if (var-> token.getStr () == w.getStr ()) {
			Ymir::Error::shadowingVar (var-> token, w);
			return false;
		    }
		}
		dones.push_back (var-> token);
	    }
	}
	return false;
    }
    
    void IDeclaration::declareAsExtern (semantic::Module) {
	Ymir::Error::assert ("TODO");
    }

    void IDeclaration::declare (semantic::Module) {
	Ymir::Error::assert ("TODO");
    }
    
    void IModDecl::declare () {
	auto get = Table::instance ().getLocal (this-> ident.getStr ());
	if (get != NULL) {
	    Ymir::Error::shadowingVar (this-> ident, get-> sym);
	    return;
	}
	if (this-> isGlobal ()) {
	    Ymir::Error::moduleNotFirst (this-> ident);
	} else if (this-> tmps.size () == 0) {
	    auto globSpace = Table::instance ().space ();
	    auto space = Namespace (Table::instance ().space (), this-> ident.getStr ());
	    auto mod = Table::instance ().addModule (space);
	    std::vector <Use> later;
	    for (auto it : this-> decls) {
		if (auto use = it-> to<IUse> ())
		    later.push_back (use);
		else it-> declare (mod);
	    }

	    for (auto it : later)
		it-> declare (mod);
	    
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, new (Z0) IModuleInfo (mod));
	    sym-> isPublic () = this-> is_public ();
	    Table::instance ().insert (sym);
	    //TODO insert ModuleAccessor
	    Table::instance ().setCurrentSpace (globSpace);
	} else {
	    auto globSpace = Table::instance ().space ();

	    auto sym = new (Z0) ISymbol (this-> ident, NULL, new (Z0) IModuleInfo (globSpace, this));
	    sym-> isPublic () = this-> is_public ();
	    Table::instance ().insert (sym);	    
	}
    }
    
    void IModDecl::declare (semantic::Module mod) {
	auto get = mod-> get (this-> ident.getStr ());
	if (get != NULL) {
	    Ymir::Error::shadowingVar (this-> ident, get-> sym);
	    return;
	}
	if (this-> isGlobal ()) {
	    Ymir::Error::moduleNotFirst (this-> ident);
	} else if (this-> tmps.size () == 0) {
	    auto globSpace = mod-> space ();
	    auto space = Namespace (mod-> space (), this-> ident.getStr ());
	    auto mod_ = Table::instance ().addModule (space);
	    std::vector <Use> later;
	    for (auto it : this-> decls) {
		if (auto use = it-> to<IUse> ())
		    later.push_back (use);
		else it-> declare (mod_);
	    }

	    for (auto it : later)
		it-> declare (mod_);
	    mod-> addOpen (space);
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, new (Z0) IModuleInfo (mod_));
	    sym-> isPublic () = this-> is_public ();
	    mod-> insert (sym);
	} else {
	    auto globSpace = mod-> space ();
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, new (Z0) IModuleInfo (globSpace, this));
	    sym-> isPublic () = this-> is_public ();
	    mod-> insert (sym);	    
	}
    }
    
    void IModDecl::declareAsExtern (semantic::Module mod) {
	auto get = mod-> get (this-> ident.getStr ());
	if (get != NULL) {
	    Ymir::Error::shadowingVar (this-> ident, get-> sym);
	    return;
	}

	if (this-> isGlobal ()) {
	    Ymir::Error::moduleNotFirst (this-> ident);
	} else if (this-> tmps.size () == 0) {	
	    auto globSpace = mod-> space ();
	    auto space = Namespace (mod-> space (), this-> ident.getStr ());
	    auto mod_ = Table::instance ().addModule (space);

	    std::vector <Use> later;
	    for (auto it : this-> decls) {
		if (auto use = it-> to<IUse> ())
		    later.push_back (use);
		else it-> declareAsExtern (mod_);
	    }

	    for (auto it : later)
		it-> declareAsExtern (mod_);
	    
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, new (Z0) IModuleInfo (mod_));
	    sym-> isPublic () = this-> is_public ();
	    mod-> insert (sym);
	} else {
	    auto globSpace = mod-> space ();
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, new (Z0) IModuleInfo (globSpace, this));
	    sym-> isPublic () = this-> is_public ();
	    mod-> insert (sym);	    
	} 
    }

    void IProgram::detachFile (std::string & file, std::string & path) {
	auto last = path.find_last_of ('/');
	if (last == std::string::npos) {
	    file = path;
	    path = "";
	} else {
	    file = path.substr (last + 1);
	    path = path.substr (0, last);
	}
    }
    
    void IProgram::detachSpace (std::string & file, std::string & path) {
	auto last = path.find_last_of ('.');
	if (last == std::string::npos) {
	    file = path;
	    path = "";
	} else {
	    file = path.substr (last + 1);
	    path = path.substr (0, last);
	}
    }
    
    bool IProgram::verifyMatch (Word & loc, std::string file, std::string mod) {
	while (mod.length () != 0) {
	    std::string f1, f2;
	    detachFile (f1, file);
	    detachSpace (f2, mod);
	    if (f1 != f2) {
		Ymir::Error::moduleDontMatch (loc);
		return false;
	    }
	}
	return true;
    }
    
    void IProgram::declare () {
	std::string name = this-> locus.getFile ();
	auto dot = name.find_last_of ('.');
	if (dot != name.npos && name.substr (dot, name.length () - dot) == ".yr") {
	    name = name.substr (0, dot);
	}

	Table::instance ().setCurrentSpace ({Mangler::mangle_file (name)});
	Table::instance ().programNamespace () = Table::instance ().globalNamespace ();
	Table::instance ().addForeignModule (Table::instance ().globalNamespace ());
	
	auto begin = 0;
	if (this-> decls.size () != 0) {
	    if (auto mod = this-> decls [0]-> to <IModDecl> ()) {
		if (mod-> isGlobal () && verifyMatch (mod-> getIdent (), name, mod-> getIdent ().getStr ())) {
		    auto str = mod-> getIdent ().getStr ();
		    auto space = Mangler::mangle_file (str);
		    Table::instance ().setCurrentSpace ({space});
		    Table::instance ().programNamespace () = Table::instance ().globalNamespace ();
		    Table::instance ().addForeignModule (Table::instance ().globalNamespace ());
		    begin ++;
		}
	    }
	}

	this-> importAllCoreFiles ();
	
	std::vector <Use> later;
	for (auto it : Ymir::r (begin, this-> decls.size ())) {
	    if (auto use = this-> decls [it]-> to <IUse> ()) {
		later.push_back (use);
	    } else this-> decls [it]-> declare ();
	}	    

	for (auto it : later)
	    it-> declare ();
    }    

    void IProgram::declareAsExtern (std::string name, semantic::Module mod) {
	if (extension (name) == ".yr") {
	    name = name.substr (0, name.length () - 3);
	}

	Table::instance ().setCurrentSpace (Namespace (Mangler::mangle_file (name)));
	auto begin = 0;
	if (this-> decls.size () != 0) {
	    if (auto mod_ = this-> decls [0]-> to <IModDecl> ()) {
		if (mod_-> isGlobal () && verifyMatch (mod_-> getIdent (), name, mod_-> getIdent ().getStr ())) { 
		    auto str = mod_-> getIdent ().getStr ();
		    auto space = Mangler::mangle_file (str);
		    Table::instance ().setCurrentSpace ({space});
		    begin ++;
		}
	    }
	}
	
	this-> importAllCoreFilesAsExtern (mod);

	std::vector <Use> later;
	for (auto it : Ymir::r (begin, this-> decls.size ())) {
	    if (auto use = this-> decls [it]-> to <IUse> ()) {
		later.push_back (use);
	    } else this-> decls [it]-> declareAsExtern (mod);
	}	    

	for (auto it : later)
	    it-> declareAsExtern (mod);
    }

    void IProgram::importAllCoreFiles () {
	if (!Options::instance ().isStandalone ()) {
	    std::string path = Options::instance ().prefixIncludeDir ();
	    if (path [path.size () - 1] == '/') path = path + "ymir/core/";
	    else path = path + "/ymir/core/";
	
	    DIR* dir = opendir (path.c_str ());
	    dirent* ent;
	    if (dir != NULL) {
		while ((ent = readdir (dir)) != NULL) {
		    std::string name = ent-> d_name;
		    if (ent-> d_type == DT_REG &&
			name.size () > 3 && 
			name [name.size () - 1] == 'r'
			&& name [name.size () - 2] == 'y' &&
			name [name.size () - 3] == '.') {
			Word file;
			file.setStr ("core/" + name.substr (0, name.size () - 3));
			auto import = new (Z0) IImport (Word::eof (), {file});
			import-> is_public (false);
			import-> declare ();
		    }
		}
		closedir (dir);
	    }
	}
    }    

    void IProgram::importAllCoreFilesAsExtern (semantic::Module mod) {
	if (!Options::instance ().isStandalone ()) {
	    std::string path = Options::instance ().prefixIncludeDir ();
	    if (path [path.size () - 1] == '/') path = path + "ymir/core/";
	    else path = path + "/ymir/core/";
	
	    DIR* dir = opendir (path.c_str ());
	    dirent* ent;
	    if (dir != NULL) {
		while ((ent = readdir (dir)) != NULL) {
		    std::string name = ent-> d_name;
		    if (ent-> d_type == DT_REG &&
			name.size () > 3 && 
			name [name.size () - 1] == 'r'
			&& name [name.size () - 2] == 'y' &&
			name [name.size () - 3] == '.') {
			Word file;
			file.setStr ("core/" + name.substr (0, name.size () - 3));
			auto import = new (Z0) IImport (Word::eof (), {file});
			import-> is_public (false);
			import-> declareAsExtern (mod);
		    }
		}
		closedir (dir);
	    }
	}
    }
    
    void IProto::declare () {
	Namespace space (this-> space != "" ? this-> space : Table::instance ().space ());
		
	auto fr = new (Z0)  IExternFrame (space, this-> from, this);
	auto fun = new (Z0)  IFunctionInfo (space, this-> ident.getStr ());
	fun-> set (fr);
	Table::instance ().insert (new (Z0)  ISymbol (this-> ident, NULL, fun));
    }

    void IProto::declare (semantic::Module mod) {       
	Namespace space (this-> space != "" ? this-> space : mod-> space ());
		
	auto fr = new (Z0)  IExternFrame (space, this-> from, this);
	auto fun = new (Z0)  IFunctionInfo (space, this-> ident.getStr ());
	auto sym = new (Z0)  ISymbol (this-> ident, NULL, fun);
	sym-> isPublic () = this-> is_public ();
	fun-> set (fr);
	mod-> insert (sym);
    }
    
    void IProto::declareAsExtern (semantic::Module mod) {
	Namespace space (this-> space != "" ? this-> space : mod-> space ());
	auto fr = new (Z0)  IExternFrame (space, this-> from, this);
	fr-> isPrivate () = !this-> isPublic;
	auto fun = new (Z0)  IFunctionInfo (space, this-> ident.getStr ());
	auto sym = new (Z0)  ISymbol (this-> ident, NULL, fun);
	sym-> isPublic () = this-> is_public ();
	fun-> set (fr);
	mod-> insert (sym);
    }

    std::string IImport::firstExistingPath (std::string file) {
	if (exists (file)) return file;
	std::vector <std::string> paths = Options::instance ().includeDirs ();
	for (auto it : paths) {
	    auto current = it + file;
	    Ymir::log ("Test import : ", current);
	    if (exists (current))
		return current;
	}
	return "";
    }

    Symbol IImport::createAndDeclareSubMod (Module& last, Namespace space, int current) {
	if (current == (int) space.innerMods ().size ()) {
	    last = Table::instance ().addModule (space);
	    auto sym = new (Z0) ISymbol ({this-> ident, space.innerMods ().back ()}, NULL, new (Z0) IModuleInfo (last));
	    return sym;
	} else {
	    auto innerMods = space.innerMods ();
	    Ymir::OutBuffer buf;
	    for (auto it : Ymir::r (0, current)) {
		buf.write (innerMods [it]);
		if (it != current - 1) buf.write (".");
	    }

	    auto next = createAndDeclareSubMod (last, space, current + 1);	    
	    Namespace current (buf.str ());
	    Module mod;
	    if (Table::instance ().isModule (current)) {
		mod = Table::instance ().getModule (current);
	    } else mod = Table::instance ().addModule (current);

	    mod-> addOpen (next-> type ()-> to <IModuleInfo> ()-> get ()-> space ());
	    mod-> insert (next);
	    mod-> isAuto () = false;
	    auto sym = new (Z0) ISymbol ({this-> ident, current.innerMods ().back ()},
					 NULL,
					 new (Z0) IModuleInfo (mod));
	    return sym;
	}
    }    
    
    void IImport::declare () {
	auto globSpace = Table::instance ().space ();
	for (auto it : this-> params) {
	    ulong nbErrorBeg = Ymir::Error::nb_errors;
	    std::string name = firstExistingPath (it.getStr () + ".yr");
	    if (name == "") {
		name = it.getStr () + ".yr";
		std::string path = Options::instance ().prefixIncludeDir ();
		name = path + "ymir/" + name;
		if (!exists (name)) {
		    Ymir::Error::moduleDontExist (this-> ident, it);
		    return;
		}
	    }

	    Namespace space (it.getStr ());
	    if (!Table::instance ().moduleExists (space)) {
	    	auto file = fopen (name.c_str (), "r");
		if (file) { 
		    Ymir::Parser parser (name.c_str (), file);
		    Module mod;
		    auto sym = this-> createAndDeclareSubMod (mod, space, 1);
		    mod-> addOpen (globSpace);
		    auto prg = parser.syntax_analyse ();
		    fclose (file);
		    prg-> declareAsExtern (it.getStr (), mod);
		    Table::instance ().insert (sym);
		} else {
		    Ymir::Error::permissionDenied (name);
		}
	    }

	    Table::instance ().openModuleForSpace (space, globSpace);
	    Table::instance ().setCurrentSpace (globSpace);

	    if (Ymir::Error::nb_errors - nbErrorBeg) {
		Ymir::Error::importError (this-> ident);
	    }
	}	
    }

    void IImport::declare (semantic::Module mod_) {
	auto globSpace = mod_-> space ();
	for (auto it : this-> params) {
	    ulong nbErrorBeg = Ymir::Error::nb_errors;
	    std::string name = firstExistingPath (it.getStr () + ".yr");
	    if (name == "") {
		name = it.getStr () + ".yr";
		std::string path = Options::instance ().prefixIncludeDir ();
		name = path + "ymir/" + name;
		if (!exists (name)) {
		    Ymir::Error::moduleDontExist (this-> ident, it);
		    return;
		}
	    }
	    
	    Namespace space (it.getStr ());
	    if (!Table::instance ().moduleExists (space)) {
		auto file = fopen (name.c_str (), "r");
		if (file) {
		    Ymir::Parser parser (name.c_str (), file);
		    Module mod;
		    auto sym = this-> createAndDeclareSubMod (mod, space, 1);
		    mod-> addOpen (globSpace);
		    auto prg = parser.syntax_analyse ();
		    fclose (file);
		    prg-> declareAsExtern (it.getStr (), mod);
		    if (this-> isPublic) {
			mod_-> addPublicOpen (mod-> space ());		    
		    }
		    mod_-> insert (sym);
		} else {
		    Ymir::Error::permissionDenied (name);
		}
	    }
	    
	    //Table::instance ().openModuleForSpace (space, globSpace);
	    if (Ymir::Error::nb_errors - nbErrorBeg) {
		Ymir::Error::importError (this-> ident);
	    }
	}	
    }

    void IImport::declareAsExtern (semantic::Module mod_) {
	auto globSpace = Table::instance ().space ();
	for (auto it : this-> params) {
	    ulong nbErrorBeg = Ymir::Error::nb_errors;
	    std::string name = firstExistingPath (it.getStr () + ".yr");
	    if (name == "") {
		name = it.getStr () + ".yr";
		std::string path = Options::instance ().prefixIncludeDir ();
		name = path + "ymir/" + name;
		if (!exists (name)) {
		    Ymir::Error::moduleDontExist (this-> ident, it);
		    return;
		}
	    }

	    Namespace space (it.getStr ());
	    if (!Table::instance ().moduleExists (space)) {
		auto file = fopen (name.c_str (), "r");
		if (file) {
		    Ymir::Parser parser (name.c_str (), file);
		    Module mod;
		    auto sym = this-> createAndDeclareSubMod (mod, space, 1);
		    mod-> addOpen (globSpace);
		    auto prg = parser.syntax_analyse ();
		    prg-> declareAsExtern (it.getStr (), mod);
		    if (this-> isPublic) {
			mod_-> addPublicOpen (mod-> space ());
		    }
		    mod_-> insert (sym);
		} else {
		    Ymir::Error::permissionDenied (name);
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
	if (!verifUdas ()) return;	
	auto exist = Table::instance ().getLocal (this-> ident.getStr ());
	if (exist) {
	    Ymir::Error::shadowingVar (this-> ident, exist-> sym);
	} else {
	    auto str = new (Z0) IStructCstInfo (this-> ident, Table::instance ().space (), this-> ident.getStr (), this-> tmps, this-> _udas, this-> _isUnion);

	    str-> isPublic (true);
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, str);
	    Table::instance ().insert (sym);
	    if (this-> tmps.size () == 0)
		FrameTable::instance ().insert (str);
	    for (auto it : this-> params) {
		str-> addAttrib (it-> to <ITypedVar> ());		
	    }
	}
    }

    void IStruct::declare (semantic::Module mod) {
	if (!verifUdas ()) return;
	auto exist = mod-> get (this-> ident.getStr ());
	if (exist) {
	    Ymir::Error::shadowingVar (this-> ident, exist-> sym);
	} else {
	    auto str = new (Z0) IStructCstInfo (
		this-> ident, 
		mod-> space (),
		this-> ident.getStr (),
		this-> tmps,
		this-> _udas,
		this-> _isUnion
	    );

	    str-> isPublic (this-> is_public ());
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, str);
	    sym-> isPublic () = this-> is_public ();
	    mod-> insert (sym);
	    if (this-> tmps.size () == 0)
		FrameTable::instance ().insert (str);
	    for (auto it : this-> params) {
		str-> addAttrib (it-> to <ITypedVar> ());		
	    }
	}
    }
    
    void IStruct::declareAsExtern (Module mod) {
	if (!verifUdas ()) return;
	auto exist = mod-> get (this-> ident.getStr ());
	if (exist) {
	    Ymir::Error::shadowingVar (this-> ident, exist-> sym);
	} else {
	    auto str = new (Z0) IStructCstInfo (
		this-> ident,
		mod-> space (),
		this-> ident.getStr (),
		this-> tmps,
		this-> _udas,
		this-> _isUnion
	    );
	    
	    str-> isPublic (this-> is_public ());
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, str);
	    sym-> isPublic () = this-> is_public ();
	    mod-> insert (sym);
	    for (auto it : this-> params) {
		str-> addAttrib (it-> to <ITypedVar> ());		
	    }
	}
    }
    
    bool IStruct::verifUdas () {
	for (auto it : this-> _udas) {
	    if (it != Keys::PACKED ||
		(it == Keys::PACKED && this-> _isUnion)
	    ) {
		Ymir::Error::undefUda (this-> ident, it);
		return false;
	    }
	}
	return true;
    }

    
    void IEnum::declare () {
	auto exist = Table::instance ().getLocal (this-> ident.getStr ());
	if (exist) {
	    Ymir::Error::shadowingVar (this-> ident, exist-> sym);
	} else {
	    Symbol type = NULL;
	    Expression fst = NULL;
	    if (this-> type != NULL) {
		auto ftype = this-> type-> toType ();
		if (ftype == NULL) return;
		type = ftype-> info;
	    } else {
		fst = this-> values [0]-> expression ();
		if (fst == NULL) return;
		type = fst-> info;
	    }
	    auto space = Table::instance ().space ();
	    auto en = new (Z0) IEnumCstInfo (space, this-> ident.getStr (), type-> type ());
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, en);
	    sym-> isPublic () = true;

	    for (auto i : Ymir::r (0, this-> names.size ())) {
		if (i == 0 && fst) {
		    en-> addAttrib (this-> names [i].getStr (), fst, NULL);
		} else {
		    auto val = this-> values [i]-> expression ();
		    if (val == NULL) return;
		    auto comp = val-> info-> type ()-> CompOp (type-> type ());
		    if (comp != NULL)
			en-> addAttrib (this-> names [i].getStr (), val, comp);
		    else {
			Ymir::Error::incompatibleTypes (this-> names [i], val-> info, type-> type ());
			return;
		    }			
		}
	    }
	    
	    Table::instance ().insert (sym);	    
	}
    }

    void IEnum::declare (semantic::Module mod) {
	auto exist = mod-> get (this-> ident.getStr ());
	if (exist) {
	    Ymir::Error::shadowingVar (this-> ident, exist-> sym);
	} else {
	    Symbol type = NULL;
	    Expression fst = NULL;
	    if (this-> type != NULL) {
		auto ftype = this-> type-> toType ();
		if (ftype == NULL) return;
		type = ftype-> info;
	    } else {
		fst = this-> values [0]-> expression ();
		if (fst == NULL) return;
		type = fst-> info;
	    }
	    auto space = mod-> space ();
	    auto en = new (Z0) IEnumCstInfo (space, this-> ident.getStr (), type-> type ());
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, en);
	    sym-> isPublic () = true;

	    for (auto i : Ymir::r (0, this-> names.size ())) {
		if (i == 0 && fst) {
		    en-> addAttrib (this-> names [i].getStr (), fst, NULL);
		} else {
		    auto val = this-> values [i]-> expression ();
		    if (val == NULL) return;
		    auto comp = val-> info-> type ()-> CompOp (type-> type ());
		    if (comp != NULL)
			en-> addAttrib (this-> names [i].getStr (), val, comp);
		    else {
			Ymir::Error::incompatibleTypes (this-> names [i], val-> info, type-> type ());
			return;
		    }			
		}
	    }
	    
	    mod-> insert (sym);	    
	}
    }

    void IEnum::declareAsExtern (Module mod) {
	if (!this-> isPublic) return;
	auto exist = mod-> get (this-> ident.getStr ());
	if (exist) {
	    Ymir::Error::shadowingVar (this-> ident, exist-> sym);
	} else {
	    Symbol type = NULL;
	    Expression fst = NULL;
	    if (this-> type != NULL) {
		auto ftype = this-> type-> toType ();
		if (ftype == NULL) return;
		type = ftype-> info;
	    } else {
		fst = this-> values [0]-> expression ();
		if (fst == NULL) return;
		type = fst-> info;
	    }
	    auto space = mod-> space ();
	    auto en = new (Z0) IEnumCstInfo (space, this-> ident.getStr (), type-> type ());
	    auto sym = new (Z0) ISymbol (this-> ident, NULL, en);
	    sym-> isPublic () = true;

	    for (auto i : Ymir::r (0, this-> names.size ())) {
		if (i == 0 && fst) {
		    en-> addAttrib (this-> names [i].getStr (), fst, NULL);
		} else {
		    auto val = this-> values [i]-> expression ();
		    if (val == NULL) return;
		    auto comp = val-> info-> type ()-> CompOp (type-> type ());
		    if (comp != NULL)
			en-> addAttrib (this-> names [i].getStr (), val, comp);
		    else {
			Ymir::Error::incompatibleTypes (this-> names [i], val-> info, type-> type ());
			return;
		    }			
		}
	    }
	    
	    mod-> insert (sym);	    
	}
    }

    void IGlobal::declare () {
	//auto space = Table::instance ().space ();
	this-> sym = new (Z0) ISymbol (this-> ident, NULL, NULL);
	if (this-> expr) {
	    this-> expr = this-> expr-> expression ();
	    if (expr == NULL) return;

	    if (!expr-> info-> isImmutable ()) {
		Ymir::Error::notImmutable (expr-> token, expr-> info);
		return;
	    }

	    sym-> type (expr-> info-> type ()-> clone ());
	    sym-> value () = this-> _isImut ? expr-> info-> value () : NULL;
	} else {
	    if (auto var = this-> type-> to <IVar> ()) {
		auto type = var-> asType ();
		if (type == NULL) return;
		sym-> type (type-> info-> type ());
	    } else {
		auto expType = this-> type-> expression ();
		if (expType == NULL) return;
		sym-> type (expType-> info-> type ());
	    }
	}
	
	if (!this-> _isImut)
	    sym-> isConst (false);
	
	sym-> space () = Table::instance ().space ();	
	Table::instance ().insert (sym);
	if (this-> isExternal) {
	    if (this-> fromC ()) sym-> space () = {""};
	    FrameTable::instance ().insertExtern (sym);
	    Ymir::declareGlobalExtern (sym);
	} else if (!this-> _isImut) {
	    FrameTable::instance ().insert (this);
	    Ymir::declareGlobal (this-> sym, this-> getExpr ());
	}
    }

    void IGlobal::declare (semantic::Module mod) {
	//auto space = Table::instance ().space ();
	this-> sym = new (Z0) ISymbol (this-> ident, NULL, NULL);
	if (this-> expr) {
	    this-> expr = this-> expr-> expression ();
	    if (expr == NULL) return;

	    if (!expr-> info-> isImmutable ()) {
		Ymir::Error::notImmutable (this-> expr-> token, expr-> info);
		return;
	    }

	    sym-> type (expr-> info-> type ()-> clone ());	    
	    sym-> value () = this-> _isImut ? expr-> info-> value () : NULL;
	} else {
	    if (auto var = this-> type-> to <IVar> ()) {
		auto type = var-> asType ();
		if (type == NULL) return;
		sym-> type (type-> info-> type ());
	    } else {
		auto expType = this-> type-> expression ();
		if (expType == NULL) return;
		sym-> type (expType-> info-> type ());
	    }
	}
	
	if (!this-> _isImut)
	    sym-> isConst (false);
	
	sym-> space () = mod-> space ();
	sym-> isPublic () = this-> is_public ();
	mod-> insert (sym);
	if (this-> isExternal) {
	    if (this-> fromC ()) sym-> space () = {""};
	    FrameTable::instance ().insertExtern (sym);
	    Ymir::declareGlobalExtern (sym);
	} else if (!this-> _isImut) {
	    FrameTable::instance ().insert (this);
	    Ymir::declareGlobal (this-> sym, this-> getExpr ());
	}
    }

    void IGlobal::declareAsExtern (semantic::Module mod) {
	//auto space = Table::instance ().space ();
	this-> sym = new (Z0) ISymbol (this-> ident, NULL, NULL);
	if (this-> expr) {
	    this-> expr = this-> expr-> expression ();
	    if (expr == NULL) return;

	    if (!expr-> info-> isImmutable ()) {
		Ymir::Error::notImmutable (expr-> token, expr-> info);
		return;
	    }

	    sym-> type (expr-> info-> type ()-> clone ());
	    sym-> value () = this-> _isImut ? expr-> info-> value () : NULL;
	} else {
	    if (auto var = this-> type-> to <IVar> ()) {
		auto type = var-> asType ();
		if (type == NULL) return;
		sym-> type (type-> info-> type ());
	    } else {
		auto expType = this-> type-> expression ();
		if (expType == NULL) return;
		sym-> type (expType-> info-> type ());
	    }
	}
	
	if (!this-> _isImut)
	    sym-> isConst (false);
	
	sym-> space () = mod-> space ();
	if (this-> fromC ()) sym-> space () = {""};
	FrameTable::instance ().insertExtern (sym);
	Ymir::declareGlobalExtern (sym);
	sym-> isPublic () = this-> is_public ();	
	mod-> insert (sym);
    }
    
    void ISelf::declare () {
	if (Table::instance ().constructor () != NULL) {
	    Ymir::Error::shadowingVar (
		this-> ident,
		Table::instance ().constructor ()-> ident ()
	    );
	    return;
	}
	
	auto space = Table::instance ().space ();
	auto fr = new (Z0) IPureFrame (space, this);	
	FrameTable::instance ().insert (fr);
	Table::instance ().constructor () = fr;
    }

    void ISelf::declare (semantic::Module mod) {
	if (mod-> constructor () != NULL) {
	    Ymir::Error::shadowingVar (
		this-> ident,
		mod-> constructor ()-> ident ()
	    );
	    return;
	}
	
	auto space = mod-> space ();
	auto fr = new (Z0) IPureFrame (space, this);	
	FrameTable::instance ().insert (fr);
	mod-> constructor () = fr;
    }

    void ISelf::declareAsExtern (semantic::Module mod) {
	if (mod-> constructor () != NULL) {
	    Ymir::Error::shadowingVar (
		this-> ident,
		mod-> constructor ()-> ident ()
	    );
	    return;
	}
	
	auto space = mod-> space ();
	auto fr = new (Z0) IExternFrame (space, "", this-> toProto ());	
	FrameTable::instance ().insert (fr);
	mod-> constructor () = fr;
    }

    void IDestSelf::declare () {
	if (Table::instance ().destructor () != NULL) {
	    Ymir::Error::shadowingVar (
		this-> ident,
		Table::instance ().destructor ()-> ident ()
	    );
	    return;
	}
	
	auto space = Table::instance ().space ();
	auto fr = new (Z0) IPureFrame (space, this);	
	FrameTable::instance ().insert (fr);
	Table::instance ().destructor () = fr;
    }

    void IDestSelf::declare (semantic::Module mod) {
	if (mod-> destructor () != NULL) {
	    Ymir::Error::shadowingVar (
		this-> ident,
		mod-> destructor ()-> ident ()
	    );
	    return;
	}
	
	auto space = mod-> space ();
	auto fr = new (Z0) IPureFrame (space, this);	
	FrameTable::instance ().insert (fr);
	mod-> destructor () = fr;
    }

    void IDestSelf::declareAsExtern (semantic::Module mod) {
	if (mod-> destructor () != NULL) {
	    Ymir::Error::shadowingVar (
		this-> ident,
		mod-> destructor ()-> ident ()
	    );
	    return;
	}
	
	auto space = mod-> space ();
	auto fr = new (Z0) IExternFrame (space, "", this-> toProto ());	
	FrameTable::instance ().insert (fr);
	mod-> destructor () = fr;
    }
    
    void IUse::declare () {
	auto sym = this-> mod-> expression ();
	if (sym == NULL) return;
	if (auto mod = sym-> info-> type ()-> to <IModuleInfo> ()) {
	    auto content = mod-> get ();
	    if (content == NULL) {
		Ymir::Error::uninitVar (this-> mod-> token, this-> mod-> token);
		return;
	    } else if (!content-> isAuto ()) {
		Ymir::Error::implicitModule (this-> mod-> token);
		return;
	    }

	    auto space = Table::instance ().space ();
	    content-> addOpen (space);
	} else {
	    Ymir::Error::incompatibleTypes (this-> loc, sym-> info, "module");
	}
    }

    void IUse::declare (semantic::Module mod) {
	auto sym = this-> mod-> expression ();
	if (sym == NULL) return;
	if (auto mod_ = sym-> info-> type ()-> to <IModuleInfo> ()) {
	    auto content = mod_-> get ();
	    if (content == NULL) {
		Ymir::Error::uninitVar (this-> mod-> token, this-> mod-> token);
		return;
	    } else if (!content-> isAuto ()) {
		Ymir::Error::implicitModule (this-> mod-> token);
		return;
	    }

	    auto space = mod-> space ();
	    content-> addOpen (space);
	} else {
	    Ymir::Error::incompatibleTypes (this-> loc, sym-> info, "module");
	}
    }

    void IUse::declareAsExtern (semantic::Module mod) {
	auto sym = this-> mod-> expression ();
	if (sym == NULL) return;
	if (auto mod_ = sym-> info-> type ()-> to <IModuleInfo> ()) {
	    auto content = mod_-> get ();
	    if (content == NULL) {
		Ymir::Error::uninitVar (this-> mod-> token, this-> mod-> token);
		return;
	    } else if (!content-> isAuto ()) {
		Ymir::Error::implicitModule (this-> mod-> token);
		return;
	    }
	    
	    auto space = mod-> space ();
	    content-> addOpen (space);
	} else {
	    Ymir::Error::incompatibleTypes (this-> loc, sym-> info, "module");
	}
    }

    void IMacro::declare () {
	auto space = Table::instance ().space ();
	auto it = Table::instance ().getLocal (this-> ident.getStr ());
	if (it != NULL) {
	    Ymir::Error::shadowingVar (ident, it-> sym);
	    return;
	}

	auto mac = new (Z0) IMacroInfo (space, this-> ident.getStr (), this);
	Table::instance ().insert (new (Z0) ISymbol (this-> ident, NULL, mac));
    }

    void IMacro::declare (semantic::Module mod) {
	auto space = mod-> space ();
	auto it = mod-> get (this-> ident.getStr ());
	if (it != NULL) {
	    Ymir::Error::shadowingVar (ident, it-> sym);
	    return;
	}

	auto mac = new (Z0) IMacroInfo (space, this-> ident.getStr (), this);
	auto sym = new (Z0) ISymbol (this-> ident, NULL, mac);
	sym-> isPublic () = this-> is_public ();
	mod-> insert (sym);
    }
	    
    void IMacro::declareAsExtern (semantic::Module mod) {
	auto space = mod-> space ();
	auto it = mod-> get (this-> ident.getStr ());
	if (it != NULL) {
	    Ymir::Error::shadowingVar (ident, it-> sym);
	    return;
	}

	auto mac = new (Z0) IMacroInfo (space, this-> ident.getStr (), this);
	auto sym = new (Z0) ISymbol (this-> ident, NULL, mac);
	sym-> isPublic () = this-> is_public ();
	mod-> insert (sym);
    }

    void ITypeCreator::declare () {
	auto space = Table::instance ().space ();
	auto it = Table::instance ().getLocal (this-> _ident.getStr ());
	if (it != NULL) {
	    Ymir::Error::shadowingVar (this-> _ident, it-> sym);
	}

	auto type = new (Z0) IAggregateCstInfo (this-> _ident, space, this-> _ident.getStr (), this-> _tmps, this-> _who, this-> _isUnion, this-> _form == TypeForm::OVER);
	if (this-> _destr.size () > 1)
	    Ymir::Error::multipleDestr (this-> _ident);
	
	if (this-> _tmps.size () == 0) {
	    if (this-> _destr.size () == 1) {
		type-> getDestructor () = this-> _destr [0]-> declare (type)-> to <IFunctionInfo> ();
	    }

	    for (auto cst : this-> _constr) {
		auto res = cst-> declare  (type);
		if (res)
		    type-> getConstructors ().push_back (res-> to <IFunctionInfo> ());
	    }

	    for (auto meth : this-> _methods) {
		bool isMethod = false;
		auto info_ = meth-> declare (type, isMethod);
		if (info_) {
		    auto info = info_-> to <IFunctionInfo> ();
		    if (!isMethod) type-> getStaticMethods ().push_back (info);
		    else type-> getMethods ().push_back (info);
		}
	    }

	    FrameTable::instance ().insert (type);	    
	} else {
	    type-> creator () = this;
	}
	
	auto sym = new (Z0) ISymbol (this-> _ident, NULL, type);
	Table::instance ().insert (sym);
    }

    void ITypeCreator::declare (semantic::Module mod) {
	auto space = mod-> space ();
	auto it = mod-> get (this-> _ident.getStr ());
	if (it != NULL) {
	    Ymir::Error::shadowingVar (this-> _ident, it-> sym);
	}

	auto type = new (Z0) IAggregateCstInfo (this-> _ident, space, this-> _ident.getStr (), this-> _tmps, this-> _who, this-> _isUnion, this-> _form == TypeForm::OVER);

	if (this-> _destr.size () > 1) 
	    Ymir::Error::multipleDestr (this-> _ident);
	
	if (this-> _tmps.size () == 0) {
	    if (this-> _destr.size () == 1) {
		type-> getDestructor () = this-> _destr [0]-> declare (type)-> to <IFunctionInfo> ();
	    }

	    for (auto cst : this-> _constr) {
		auto res = cst-> declare (type);
		if (res)
		    type-> getConstructors ().push_back (res-> to <IFunctionInfo> ());
	    }

	    for (auto meth : this-> _methods) {
		bool isMethod = false;
		auto info_ = meth-> declare (type, isMethod);
		if (info_) {
		    auto info = info_-> to <IFunctionInfo> ();
		    if (!isMethod) type-> getStaticMethods ().push_back (info);
		    else type-> getMethods ().push_back (info);
		}
	    }

	    FrameTable::instance ().insert (type);	    	
	} else {
	    type-> creator () = this;
	}
	
	auto sym = new (Z0) ISymbol (this-> _ident, NULL, type);
	mod-> insert (sym);
    }
    
    void ITypeCreator::declareAsExtern (semantic::Module mod) {
	auto space = mod-> space ();
	auto it = mod-> get (this-> _ident.getStr ());
	if (it != NULL) {
	    Ymir::Error::shadowingVar (this-> _ident, it-> sym);
	}

	auto type = new (Z0) IAggregateCstInfo (this-> _ident, space, this-> _ident.getStr (), this-> _tmps, this-> _who, this-> _isUnion, this-> _form == TypeForm::OVER);
	type-> isExtern () = true;
	
	if (this-> _destr.size () > 1) 
	    Ymir::Error::multipleDestr (this-> _ident);

	if (this-> _tmps.size () == 0) {
	    if (this-> _destr.size () == 1) {
		type-> getDestructor () = this-> _destr [0]-> declare (type, true)-> to <IFunctionInfo> ();
	    }

	    for (auto cst : this-> _constr) {
		auto res = cst-> declare  (type, true);
		if (res)
		    type-> getConstructors ().push_back (res-> to <IFunctionInfo> ());
	    }

	    for (auto meth : this-> _methods) {
		bool isMethod = false;
		auto info_ = meth-> declare (type, isMethod, true);
		if (info_) {
		    auto info = info_-> to <IFunctionInfo> ();
		    if (!isMethod) type-> getStaticMethods ().push_back (info);
		    else type-> getMethods ().push_back (info);
		}
	    }

	} else {
	    type-> creator () = this;
	}
	
	auto sym = new (Z0) ISymbol (this-> _ident, NULL, type);
	mod-> insert (sym);
    }
    
    
    InfoType ITypeConstructor::declare (AggregateCstInfo info, bool isExternal) {
	auto space = Namespace (info-> space (), info-> name ());
	bool addable = true;
	for (auto it : this-> _params) {
	    if (!it-> is<ITypedVar> ()) {
		addable = false;
	    }
	}
	auto fr = new (Z0) IMethodFrame (space, Keys::INIT,  info, this);
	fr-> isExtern () = isExternal;
	
	auto func = new (Z0) IFunctionInfo (space, Keys::INIT);

	if (addable && !isExternal)
	    FrameTable::instance ().insert (fr);
	func-> set (fr);
	func-> isConstr () = (true);
	return func;
    }

    InfoType ITypeMethod::declare (AggregateCstInfo info, bool& method, bool isExternal) {
	auto space = Namespace (info-> space (), info-> name ());

	bool addable = this-> tmps.size () == 0;
	method = false;
	bool needConst = false;
	if (this-> params.size () > 0) {
	    if (this-> params [0]-> token.getStr () == Keys::SELF) method = true;
	    if (method) {
		if (this-> params [0]-> is <ITypedVar> ()) {
		    Ymir::Error::selfAlwaysInfered (this-> params [0]-> token);
		    return NULL;
		} else if (this-> params [0]-> deco.getStr () == Keys::CONST) {
		    needConst = true;
		} else if (this-> params [0]-> deco.getStr () == Keys::REF) {
		    Ymir::Error::selfAlwaysRef (this-> params [0]-> token);
		}
		
		for (auto it : Ymir::r (1, this-> params.size ())) {
		    if (!this-> params [it]-> is <ITypedVar> ())
			addable = false;
		}
	    } else {
		for (auto it : Ymir::r (0, this-> params.size ())) {
		    if (!this-> params [it]-> is<ITypedVar> ())
			addable = false;
		}
	    }
	}
	
	if (!method && this-> name () == Keys::INIT) {
	    Ymir::Error::staticMethodInit (this-> ident);
	    return NULL;
	}
	
	Frame fr = NULL;
	if (method) {
	    fr = new (Z0) IMethodFrame (space, this-> name (), info, this);
	    fr-> to <IMethodFrame> ()-> isExtern () = isExternal;
	    fr-> to <IMethodFrame> ()-> isVirtual () = addable;
	    fr-> to <IMethodFrame> ()-> needConst () = needConst;
	} else if (addable) {
	    if (!isExternal)
		fr = new (Z0) IPureFrame (space, this);
	    else
		fr = new (Z0) IExternFrame (space, "", this-> toProto ());
	} else fr = new (Z0) IUnPureFrame (space, this);
	    
	auto func = new (Z0) IFunctionInfo (space, this-> name ());
	if (addable) {
	    FrameTable::instance ().insert (fr);
	    func-> isVirtual () = true;
	    func-> isOver () = this-> _isOver;
	}
	
	func-> set (fr);	
	return func;
    }

    InfoType ITypeDestructor::declare (AggregateCstInfo info, bool isExternal) {
	auto space = Namespace (info-> space (), info-> name ());
	auto fr = new (Z0) IMethodFrame (space, Keys::DELETE, info, this);
	fr-> isExtern () = isExternal;
	
	auto func = new (Z0) IFunctionInfo (space, Keys::DELETE);

	if (!isExternal) FrameTable::instance ().insert (fr);
	func-> set (fr);
	//func-> isConstr (true);
	return func;
    }
    
    void ITypeMethod::declare () {	
	Ymir::Error::assert ("!!?");
    }
    
    void ITypeMethod::declare (semantic::Module) {
	Ymir::Error::assert ("!!?");
    }
    
    void ITypeMethod::declareAsExtern (semantic::Module) {
	Ymir::Error::assert ("!!?");
    }
    
}
