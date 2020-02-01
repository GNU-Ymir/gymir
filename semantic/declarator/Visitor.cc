#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/syntax/visitor/Visitor.hh>
#include <ymir/semantic/symbol/_.hh>
#include <ymir/utils/Path.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/global/State.hh>
#include <dirent.h>
#include <algorithm>

using namespace Ymir;

namespace semantic {

    namespace declarator {

	std::set <std::string> Visitor::__imported__;
	
	Visitor::Visitor () {}
    
	Visitor Visitor::init () {
	    return Visitor ();
	}

	semantic::Symbol Visitor::visit (const syntax::Declaration & ast) {
	    match (ast) {
		of (syntax::Module, mod, 
		    return visitModule (mod);		
		);
	    
		of (syntax::Function, func,
		    return visitFunction (func);
		);

		of (syntax::Struct, str,
		    return visitStruct (str);
		);

		of (syntax::Alias, al,
		    return visitAlias (al);
		);

		of (syntax::DeclBlock, bl,
		    return visitBlock (bl);
		);

		of (syntax::ExternBlock, ex_bl,
		    return visitExtern (ex_bl);
		);
		
		of (syntax::Class, cls,
		    return visitClass (cls);
		);

		of (syntax::Trait, trait,
		    return visitTrait (trait);
		);

		of (syntax::Mixin, impl,
		    return visitImpl (impl);
		);
		
		of (syntax::Global, glb,
		    return visitGlobal (glb);
		);

		of (syntax::Import, im,
		    return visitImport (im);
		);

		of (syntax::Template, tep,
		    return visitTemplate (tep);
		);

		of (syntax::Enum, en,
		    return visitEnum (en);
		);
		
		of (syntax::ExpressionWrapper, wrap, {
			match (wrap.getContent ()) {
			    of (syntax::VarDecl, decl,
				return visitVarDecl (decl);
			    );		    
			}
		
			Error::halt ("%(r) - reaching impossible point", "Critical");
			return Symbol::empty ();
		    }
		);
	    }

	    Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Symbol::empty ();
	}    

	semantic::Symbol Visitor::visitModule (const syntax::Module & mod) {
	    auto path = Path {mod.getIdent ().str, "::"};
	    if (mod.isGlobal ()) {
		auto file_location = Path {mod.getIdent ().locFile}.stripExtension ();
		if (path.getFiles ().size () == 0) {
		    path = Path (file_location.getFiles ().back ());
		}
		__imported__.emplace (file_location.toString ());
		if (!mod.getIdent ().isEof () && !Path {mod.getIdent ().str, "::"}.isRelativeFrom (file_location)) {
		    Ymir::Error::occur (mod.getIdent (), ExternalError::get (WRONG_MODULE_NAME), mod.getIdent ().str, Path {mod.getIdent ().str, "::"}.toString () + ".yr");
		}
	    }


	    pushReferent (Module::init ({mod.getIdent (), path.fileName ().toString ()}));	    
	    getReferent ().insert (ModRef::init (mod.getIdent (), path.getFiles ()));

	    if (mod.isGlobal () && !global::State::instance ().isStandalone ())
		importAllCoreFiles ();	    
	    
	    for (auto & it : mod.getDeclarations ()) {
		visit (it);
	    }

	    auto ret = popReferent ();
	    auto modules = path.getFiles ();
	    
	    if (mod.isGlobal () && modules.size () > 1) {
		auto glob = Symbol::getModule (modules [0]);
		if (glob.isEmpty ()) {
		    glob = Module::init ({mod.getIdent (), modules [0]});
		    pushReferent (glob);
		} else pushReferent (glob);
		
		createSubModules (mod.getIdent (), std::vector <std::string> (modules.begin () + 1, modules.end ()), ret);
		
		glob = popReferent ();
		Symbol::registerModule (modules [0], glob);
		return ret;
	    } else if (mod.isGlobal ()) {
		if (modules.size () == 1) 
		    Symbol::registerModule (modules [0], ret);
	    } else getReferent ().insert (ret);
	    
	    return ret;
	}

	void Visitor::createSubModules (const lexing::Word & loc, const std::vector <std::string> & names, semantic::Symbol & last) {
	    if (names.size () == 1) {
		getReferent ().insert (last);
	    } else if (names.size () != 0) {
		auto symbols = getReferent ().getLocal (names [0]);
		for (auto & sym : symbols) {
		    if (sym.is <Module> ()) {
			pushReferent (sym);
			std::vector<std::string> modules (names.begin () + 1, names.end ());
			createSubModules (loc, modules, last);
			auto mod = popReferent ();
			getReferent ().insert (mod);
			return;
		    }
		}
		pushReferent (Module::init ({loc, names [0]}));
		std::vector<std::string> modules (names.begin () + 1, names.end ());
		createSubModules (loc, modules, last);
		auto mod = popReferent ();
		getReferent ().insert (mod);
	    }
	}
	
	semantic::Symbol Visitor::visitFunction (const syntax::Function & func, bool isExtern, bool insert) {
	    auto function = Function::init (func.getName (), func);
	
	    auto symbols = getReferent ().getLocal (func.getName ().str);	    
	    for (auto & symbol : symbols) {
		if (!symbol.is <Function> () && !symbol.is <Module> () && !symbol.is <ModRef> () && func.getName () != Keys::SELF_TILDE && !symbol.is <Template> ()) {
		    auto note = Ymir::Error::createNote (symbol.getName ());
		    Ymir::Error::occurAndNote (func.getName (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), func.getName ().str);
		}
	    }
	
	    for (auto & ca : func.getCustomAttributes ()) {
		if (ca == Keys::SAFE) function.to <Function> ().isSafe (true);
		else if (ca == Keys::PURE) function.to <Function> ().isPure (true);
		else {
		    Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.str);
		}
	    }

	    
	    if (func.isOver ()) function.to <Function> ().isOver (true);	    

	    if (!isExtern || !func.getBody ().getBody ().isEmpty ()) {
		if (func.getPrototype ().isVariadic ()) Ymir::Error::occur (func.getName (), ExternalError::get (DECL_VARIADIC_FUNC));
	    }

	    if (insert) 
		getReferent ().insert (function);
	    return function;
	}        

	semantic::Symbol Visitor::visitConstructor (const syntax::Constructor & cs) {
	    auto semcs = semantic::Constructor::init (cs.getName (), cs);
	    getReferent ().insert (semcs);
	    return semcs;
	}
	
	semantic::Symbol Visitor::visitStruct (const syntax::Struct & str, bool insert) {
	    auto structure = Struct::init (str.getName (), str.getDeclarations ());
	
	    auto symbols = getReferent ().getLocal (str.getName ().str);	
	    for (auto & symbol : symbols) {
		auto note = Ymir::Error::createNote (symbol.getName ());
		Ymir::Error::occurAndNote (str.getName (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), str.getName ().str);		
	    } 
	
	    for (auto & ca : str.getCustomAttributes ()) {
		if (ca == Keys::PACKED) {
		    structure.to<Struct> ().isPacked (true);
		} else if (ca == Keys::UNION) {
		    structure.to<Struct> ().isUnion (true);
		} else Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.str);

		if (structure.to <Struct> ().isUnion () && structure.to <Struct> ().isPacked ()) {
		    Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::PACKED_AND_UNION));
		}
	    }
	    
	    if (insert)
		getReferent ().insert (structure);	    
	    return structure;
	}

	semantic::Symbol Visitor::visitAlias (const syntax::Alias & stal) {
	    auto alias = Alias::init (stal.getName (), stal.getValue ());

	    auto symbols = getReferent ().getLocal (stal.getName ().str);
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (stal.getName (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stal.getName ().str);
	    }	    

	    getReferent ().insert (alias);
	    return alias;
	}    

	semantic::Symbol Visitor::visitBlock (const syntax::DeclBlock & block) {
	    pushReferent (Module::init (block.getLocation ()));
	    // A declaration block is just a list of declaration, we do not enter a new referent
	    for (const syntax::Declaration & decl : block.getDeclarations ()) {
		visit (decl);		
	    }

	    auto ret = popReferent ();
	    auto syms = ret.to <semantic::Module> ().getAllLocal ();
	    if (!block.isPrivate ()) {
		for (auto & it : syms)
		    it.setPublic ();		
	    }
	    
	    if (!getReferent ().isEmpty ()) {
		for (auto & it : syms) {
		    getReferent ().insert (it);
		}

		for (auto it : ret.getUsedSymbols ()) {
		    if (!block.isPrivate () && !it.second.isEmpty ()) // Why it can be empty ? 
			it.second.setPublic ();
		    
		    getReferent ().use (it.first, it.second);
		}
	    }
	    
	    return Symbol::empty ();	    
	}

	semantic::Symbol Visitor::visitExtern (const syntax::ExternBlock & ex_block) {
	    if (ex_block.getFrom () == Keys::CLANG) {
		if (!ex_block.getSpace ().isEof ()) 
		    Ymir::Error::occur (ex_block.getSpace (), ExternalError::get (SPACE_EXTERN_C));
		for (const syntax::Declaration & ex_decl : ex_block.getDeclaration ().to <syntax::DeclBlock> ().getDeclarations ()) {
		    match (ex_decl) {
			of (syntax::Function, func,
			    auto decl = visitFunction (func, true);
			    decl.to <semantic::Function> ().setExternalLanguage (ex_block.getFrom ().str);
			) 

			else of (syntax::ExpressionWrapper, wrap, {
				match (wrap.getContent ()) {
				    of (syntax::VarDecl, decl,
					visitVarDecl (decl);
					continue;
				    );		    
				}
			    		
				Error::halt ("%(r) - reaching impossible point", "Critical");
				return Symbol::empty ();
			    }
			) else
				 Ymir::Error::occur (ex_block.getLocation (), ExternalError::get (IMPOSSIBLE_EXTERN));
		    }
		}
	    } else
		Ymir::Error::occur (ex_block.getLocation (), ExternalError::get (IMPOSSIBLE_EXTERN));
	    
	    return Symbol::empty ();
	}
	
	semantic::Symbol Visitor::visitClass (const syntax::Class & stcls) {
	    auto cls = Class::init (stcls.getName (), stcls.getAncestor ());
	
	    auto symbols = getReferent ().getLocal (stcls.getName ().str);
	    for (auto & symbol : symbols) {
		auto note = Ymir::Error::createNote (symbol.getName ());
		Ymir::Error::occurAndNote (stcls.getName (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stcls.getName ().str);		
	    }

	    for (auto & ca : stcls.getAttributes ()) {
		if (ca == Keys::ABSTRACT) cls.to <Class> ().isAbs (true);
		else {
		    Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.str);
		}
	    }
	    
	    pushReferent (cls);
	    for (auto & it : stcls.getDeclarations ()) {
		match (it) {
		    of (syntax::ExpressionWrapper, wrap, {
			    match (wrap.getContent ()) {
				of (syntax::VarDecl, de ATTRIBUTE_UNUSED, {
					cls.to <semantic::Class> ().addField (wrap.getContent ());
					cls.to <semantic::Class> ().setProtected (de.getName ().str);
				    }
				) else of (syntax::Set, se, {
					for (auto & it : se.getContent ()) {
					    cls.to <semantic::Class> ().addField (it);
					    cls.to <semantic::Class> ().setProtected (it.to <syntax::VarDecl> ().getName ().str);
					}
				    }
				) else
				    Error::halt ("%(r) - reaching impossible point", "Critical");			
			    }
			}
		    )
		    else of (syntax::Constructor, cs, {
			    auto sym = visitConstructor (cs);
			    sym.to <Constructor> ().setClass (cls);
			    sym.setPublic ();
			}
		    ) else of (syntax::DeclBlock, dc, {
			    for (auto & jt : dc.getDeclarations ()) {
				match (jt) {
				    of (syntax::ExpressionWrapper, wrap, {
					    match (wrap.getContent ()) {
						of (syntax::VarDecl, de ATTRIBUTE_UNUSED, {
							cls.to <semantic::Class> ().addField (wrap.getContent ());
							if (dc.isPrivate ())
							    cls.to <semantic::Class> ().setPrivate (de.getName ().str);
							if (dc.isProt ())
							    cls.to <semantic::Class> ().setProtected (de.getName ().str);
						    } 
						) else 
						    Error::halt ("%(r) - reaching impossible point", "Critical");			
					    }
					}
				    ) else of (syntax::Constructor, cs, {
					    auto sym = visitConstructor (cs);
					    sym.to <Constructor> ().setClass (cls);
					    if (dc.isPublic ())
						sym.setPublic ();
					    if (dc.isProt ()) 
						sym.setProtected ();					    
					}
				    ) else {
					    auto sym = visit (jt);
					    if (dc.isPublic ())
						sym.setPublic ();
					    if (dc.isProt ())
						sym.setProtected ();
				    }
				}
			    }
			}
		    ) else {			
				auto sym = visit (it);
				sym.setPublic ();
		    }
		}
	    }

	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}

	semantic::Symbol Visitor::visitTrait (const syntax::Trait & sttrait) {
	    auto tr = Trait::init (sttrait.getName ());

	    auto symbols = getReferent ().getLocal (sttrait.getName ().str);
	    for (auto & symbol : symbols) {
		auto note = Ymir::Error::createNote (symbol.getName ());
		Ymir::Error::occurAndNote (sttrait.getName (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), sttrait.getName ().str);
	    }

	    pushReferent (tr);
	    for (auto & it : sttrait.getDeclarations ()) {
		match (it) {
		    of (syntax::DeclBlock, dc, {
			    for (auto & jt : dc.getDeclarations ()) {
				auto sym = visit (jt);
				if (dc.isPublic ())
				    sym.setPublic ();
				if (dc.isProt ())
				    sym.setProtected ();
			    }							    
			}
		    ) else {			
			auto sym = visit (it);
			sym.setPublic ();
		    }
		}
	    }
	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}

	semantic::Symbol Visitor::visitImpl (const syntax::Mixin & stimpl) {
	    auto im = Impl::init (stimpl.getLocation (), stimpl.getMixin ());
	    pushReferent (im);
	    for (auto & it : stimpl.getDeclarations ()) {
		match (it) {
		    of (syntax::DeclBlock, dc, {
			    for (auto & jt : dc.getDeclarations ()) {
				auto sym = visit (jt);
				if (dc.isPublic ())
				    sym.setPublic ();
				if (dc.isProt ())
				    sym.setProtected ();
			    }							    
			}
		    ) else {			
			auto sym = visit (it);
			sym.setPublic ();
		    }
		}
	    }
	    
	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}
	
	semantic::Symbol Visitor::visitEnum (const syntax::Enum & stenm) {
	    auto enm = Enum::init (stenm.getName (), stenm.getValues (), stenm.getType ());
	    auto symbols = getReferent ().getLocal (stenm.getName ().str);
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (stenm.getName (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stenm.getName ().str);
	    }

	    pushReferent (enm);
	    for (auto & it : stenm.getValues ()) {
		// Inside an enum the vars are declared using a vardecl expression
		auto en = visit (syntax::ExpressionWrapper::init (it));
		en.setPublic ();
	    }

	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}    

	semantic::Symbol Visitor::visitVarDecl (const syntax::VarDecl & stdecl) {
	    auto decl = VarDecl::init (stdecl.getName (), stdecl.getDecorators (), stdecl.getType (), stdecl.getValue ());
	    auto symbols = getReferent ().getLocal (stdecl.getName ().str);
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (stdecl.getName (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stdecl.getName ().str);
	    }

	    getReferent ().insert (decl);	
	    return decl;
	}
    
	semantic::Symbol Visitor::visitGlobal (const syntax::Global & stglob) {
	    return visit (syntax::ExpressionWrapper::init (stglob.getContent ()));	
	}

	std::map <std::string, std::string> dirEntries (const std::string & path, const std::string & init) {
	    auto dir = opendir (path.data ());
	    std::map <std::string, std::string> res;
	    if (dir != NULL) {
		auto entry = readdir (dir);
		while (entry != NULL) {
		    auto str = std::string (entry-> d_name);
		    if (entry-> d_type == DT_DIR && str [0] != '.') {
			auto ent = Ymir::Path::build (path, str).toString ();
			auto entries = dirEntries (ent, Ymir::Path::build (init, str).toString ("::"));
			for (auto & it :entries) res.emplace (it.first, it.second);
		    } else if (str.length () >= 4 && str.substr (str.length () - 3, 3) == ".yr") {
			auto name = str.substr (0, str.length () - 3);
			res.emplace (Ymir::Path::build (path, name).toString (), Ymir::Path::build (init, name).toString ("::"));
		    }
		    entry = readdir (dir);
		}
	    }
	    return res;
	}	
	
	void Visitor::importAllCoreFiles () {
	    auto dir = global::State::instance ().getCorePath ();
	    auto entries = dirEntries (dir, "core");
	    for (auto & it : entries) {
		if (__imported__.find (it.first) == __imported__.end ()) {
		    auto file_path = it.first + ".yr";
			
		    auto file = fopen (file_path.c_str (), "r");
		    if (file != NULL) {
			// We add a fake module, to prevent infinite import loops
			__imported__.emplace (it.first);
			std::vector <std::string> errors;
			TRY (
			    auto synt_module = syntax::Visitor::init (file_path, file).visitModGlobal ();
			    declarator::Visitor::init ().visit (synt_module);
			) CATCH (ErrorCode::EXTERNAL) {
			    GET_ERRORS_AND_CLEAR (msgs);
			    errors.insert (errors.end (), msgs.begin (), msgs.end ());
			} FINALLY;
		    
			if (errors.size () != 0) 
			    THROW (ErrorCode::EXTERNAL, errors);			
		    }
		}
		getReferent ().use (it.second, Symbol::getModuleByPath (it.second));		
	    }
	}

	semantic::Symbol Visitor::visitImport (const syntax::Import & imp) {
	    auto path = Path {imp.getModule ().str, "::"};
	    bool success = false;

	    if (__imported__ .find (path.toString ()) == __imported__.end ()) {
		auto file_path = imp.getPath () + ".yr";
		auto file = fopen (file_path.c_str (), "r");
		if (file != NULL) {
		    success = true;
		    // We add a fake module, to prevent infinite import loops
		    __imported__.emplace (path.toString ());
		    std::vector <std::string> errors;
		    TRY (
			auto synt_module = syntax::Visitor::init (file_path, file).visitModGlobal ();
			declarator::Visitor::init ().visit (synt_module);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
		    } FINALLY;
		    
		    if (errors.size () != 0) {
			auto note = Ymir::Error::createNote (imp.getModule (), ExternalError::get (IN_IMPORT));
			errors.push_back (note);
			THROW (ErrorCode::EXTERNAL, errors);
		    }
		}
	    } else success = true;
	    
	    if (!success) {
		for (auto & it : global::State::instance ().getIncludeDirs ()) {
		    path = Path::build (it, Path {imp.getModule ().str, "::"}.toString ());
		    if (__imported__.find (path.toString ()) == __imported__.end ()) {
			auto file_path = path.toString () + ".yr";
			
			auto file = fopen (file_path.c_str (), "r");
			if (file != NULL) {
			    success = true;
			    // We add a fake module, to prevent infinite import loops
			    __imported__.emplace (path.toString ());
			    std::vector <std::string> errors;
			    TRY (
				auto synt_module = syntax::Visitor::init (file_path, file).visitModGlobal ();
				declarator::Visitor::init ().visit (synt_module);
			    ) CATCH (ErrorCode::EXTERNAL) {
				GET_ERRORS_AND_CLEAR (msgs);
				errors.insert (errors.end (), msgs.begin (), msgs.end ());
			    } FINALLY;
		    
			    if (errors.size () != 0) {
				auto note = Ymir::Error::createNote (imp.getModule (), ExternalError::get (IN_IMPORT));
				errors.push_back (note);
				THROW (ErrorCode::EXTERNAL, errors);
			    }
			    break;
			}
		    } else success = true;
		}

		if (!success) {
		    auto path = (Path {imp.getModule ().str, "::"}).toString () + ".yr";
		    Error::occur (imp.getModule (), ExternalError::get (NO_SUCH_FILE), path);
		}
	    }
	    
	    getReferent ().use (imp.getModule ().str , Symbol::getModuleByPath (imp.getModule ().str));
	    return Symbol::empty ();	    		    
	}	
	
	semantic::Symbol Visitor::visitTemplate (const syntax::Template & tep) {
	    std::vector <lexing::Word> used;
	    for (auto & par : tep.getParams ()) {
		match (par) {
		    of (syntax::VariadicVar, vr, 
			for (auto & use : used) {
			    if (use.str == vr.getLocation ().str) {
				Error::occur (vr.getLocation (), ExternalError::get (SHADOWING_DECL), use.str);
			    }
			}
			used.push_back (vr.getLocation ());
		    ) else of (syntax::OfVar, vr,
			       for (auto & use : used) {
				   if (use.str == vr.getLocation ().str) {
				       Error::occur (vr.getLocation (), ExternalError::get (SHADOWING_DECL), use.str);
				   }
			       }
			       used.push_back (vr.getLocation ());
		    ) else of (syntax::VarDecl, vr,
			       for (auto & use : used) {
				   if (use.str == vr.getLocation ().str) {
				       Error::occur (vr.getLocation (), ExternalError::get (SHADOWING_DECL), use.str);
				   }
			       }
			       used.push_back (vr.getLocation ());
		    ) else of (syntax::Var, vr, 
			       for (auto & use : used) {
				   if (use.str == vr.getLocation ().str) {
				       Error::occur (vr.getLocation (), ExternalError::get (SHADOWING_DECL), use.str);
				   }
			       }
			       used.push_back (vr.getLocation ());
		    );			       
		}		
	    }
	    
	    auto sym = Template::init (tep.getLocation (), tep.getParams (), tep.getContent (), tep.getTest (), tep.getParams ());
	    getReferent ().insert (sym);
	    return sym;
	}

	
	void Visitor::pushReferent (const Symbol & sym) {
	    this-> _referent.push_front (sym);
	}

	Symbol Visitor::popReferent () {
	    if (this-> _referent.size () == 0)
		Ymir::Error::halt ("%(r) - poping a symbol referent, while there is none", "Critical");

	    // We can't get a reference since the pop_front will erase the data
	    auto /** & */ ret = this-> _referent.front ();

	    this-> _referent.pop_front ();
	    return ret;
	}

	Symbol & Visitor::getReferent () {
	    if (this-> _referent.size () != 0)
		return this-> _referent.front ();
	    else return Symbol::__empty__;
	}

    }
    
}
