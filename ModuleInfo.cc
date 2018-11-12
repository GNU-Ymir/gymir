#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/types/ModuleInfo.hh>
#include <ymir/ast/Var.hh>

namespace semantic {
    
    using namespace syntax;
    
    IModuleInfo::IModuleInfo (Namespace space, syntax::ModDecl decl) :
	IInfoType (true),
	content (NULL),
	decl (decl),
	space (space)
    {}
    
    IModuleInfo::IModuleInfo (Module content) :
	IInfoType (true),
	content (content),
	decl (NULL),
	space ("")
    {}

    bool IModuleInfo::isSame (InfoType other) {
	return this == other;
    }

    InfoType IModuleInfo::onClone () {
	return this;
    }
    
    Module IModuleInfo::get () {
	if (!this-> content) {
	    auto info = this-> TempOp ({});
	    if (info) return info-> to <IModuleInfo> ()-> content;
	}
	return this-> content;
    }
    
    InfoType IModuleInfo::TempOp (const std::vector<::syntax::Expression> & tmps) {
	if (this-> decl) {
	    auto res = TemplateSolver::instance ().solve (this-> decl-> getTemplates (), tmps);
	    if (!res.valid || !TemplateSolver::instance ().isSolved (this-> decl-> getTemplates (), res))
		return NULL;
	    auto mod = this-> decl-> templateDeclReplace (res.elements)-> to <IModDecl> ();
	    Ymir::OutBuffer name;
	    name.write (this-> decl-> getIdent ().getStr (), "(");
	    for (auto it = res.elements.begin (); it != res.elements.end (); ) {
		if (auto params = it-> second-> to <IParamList> ()) {
		    name.write ("{");
		    for (auto it_ : Ymir::r (0, params-> getParamTypes ().size ())) {
			name.write (params-> getParamTypes () [it_]-> typeString ());
			if (it_ < (int) params-> getParamTypes ().size () - 1)
			    name.write (", ");
		    }
		} else {
		    if (it-> second == NULL || it-> second-> info == NULL || it-> second-> info-> type () == NULL)
			return NULL;
		    name.write (it-> first, ":", it-> second-> info-> typeString ());
		}
		++it;
		if (it != res.elements.end ())
		    name.write (", ");
	    }
	    name.write (")");
	    
	    mod-> ident ().setStr (name.str ());
	    auto globSpace = Table::instance ().space ();
	    auto space = Namespace (this-> space, mod-> getIdent ().getStr ());
	    auto mod_ = Table::instance ().addModule (space);
	    for (auto it : mod-> getDecls ()) {
		it-> declare (mod_);
	    }
	    
	    auto info = new (Z0) IModuleInfo (mod_);
	    Table::instance ().setCurrentSpace (globSpace);
	    return info;
	} return NULL;
    }

    std::string IModuleInfo::typeString () {
	if (this-> content)
	    return "Module::" + this-> content-> space ().toString ();
	else
	    return "Module::" + this-> decl-> getIdent ().getStr ();
    }

    std::string IModuleInfo::innerTypeString () {
	if (this-> content)
	    return "Module::" + this-> content-> space ().toString ();
	else
	    return "Module::" + this-> decl-> getIdent ().getStr ();
    }

    std::string IModuleInfo::innerSimpleTypeString () {
	Ymir::Error::assert ("!!!");
	return "";
    }

    std::string IModuleInfo::simpleTypeString () {
	Ymir::Error::assert ("!!!");
	return "";
    }
    
    const char * IModuleInfo::getId () {
	return IModuleInfo::id ();
    }        

}
