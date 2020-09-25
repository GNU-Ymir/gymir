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

	semantic::Symbol Visitor::visit (const syntax::Declaration ast) {
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

		of (syntax::Macro, mc,
		    return visitMacro (mc);
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
	    
	    Ymir::OutBuffer buf;
	    ast.treePrint (buf);
	    println (buf);
	    Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Symbol::empty ();
	}    

	semantic::Symbol Visitor::visitModule (const syntax::Module mod) {
	    auto path = Path {mod.getLocation ().str, "::"};
	    if (mod.isGlobal ()) {
		auto file_location = Path {mod.getLocation ().locFile}.stripExtension ();
		if (path.getFiles ().size () == 0) {
		    path = Path (file_location.getFiles ().back ());
		}
		__imported__.emplace (file_location.toString ());
		if (!mod.getLocation ().isEof () && !Path {mod.getLocation ().str, "::"}.isRelativeFrom (file_location)) {
		    Ymir::Error::occur (mod.getLocation (), ExternalError::get (WRONG_MODULE_NAME), mod.getLocation ().str, Path {mod.getLocation ().str, "::"}.toString () + ".yr");
		}
	    }


	    pushReferent (Module::init ({mod.getLocation (), path.fileName ().toString ()}, this-> _isWeak));	    
	    getReferent ().insert (ModRef::init (mod.getLocation (), path.getFiles (), this-> _isWeak));

	    if (mod.isGlobal () && !global::State::instance ().isStandalone ())
		importAllCoreFiles ();	    
	    
	    for (auto it : mod.getDeclarations ()) {
		visit (it);
	    }

	    auto ret = popReferent ();
	    auto modules = path.getFiles ();
	    
	    if (mod.isGlobal () && modules.size () > 1) {
		auto glob = Symbol::getModule (modules [0]);
		if (glob.isEmpty ()) {
		    glob = Module::init ({mod.getLocation (), modules [0]}, this-> _isWeak);
		    pushReferent (glob);
		} else pushReferent (glob);
		
		createSubModules (mod.getLocation (), std::vector <std::string> (modules.begin () + 1, modules.end ()), ret);
		
		glob = popReferent ();
		Symbol::registerModule (modules [0], glob);
		return ret;
	    } else if (mod.isGlobal ()) {
		if (modules.size () == 1) 
		    Symbol::registerModule (modules [0], ret);
	    } else getReferent ().insert (ret);
	    
	    return ret;
	}

	void Visitor::createSubModules (const lexing::Word & loc, const std::vector <std::string> & names, semantic::Symbol last) {
	    if (names.size () == 1) {
		getReferent ().insert (last);
	    } else if (names.size () != 0) {
		auto symbols = getReferent ().getLocal (names [0]);
		for (auto sym : symbols) {
		    if (sym.is <Module> ()) {
			pushReferent (sym);
			std::vector<std::string> modules (names.begin () + 1, names.end ());
			createSubModules (loc, modules, last);
			auto mod = popReferent ();
			getReferent ().insert (mod);
			return;
		    }
		}
		pushReferent (Module::init ({loc, names [0]}, this-> _isWeak));
		std::vector<std::string> modules (names.begin () + 1, names.end ());
		createSubModules (loc, modules, last);
		auto mod = popReferent ();
		getReferent ().insert (mod);
	    }
	}
	
	semantic::Symbol Visitor::visitFunction (const syntax::Function func, bool isExtern, bool insert) {
	    auto function = Function::init (func.getLocation (), func, this-> _isWeak);
	
	    auto symbols = getReferent ().getLocal (func.getLocation ().str);	    
	    for (auto symbol : symbols) {
		if (!symbol.is <Function> () && !symbol.is <Module> () && !symbol.is <ModRef> () && func.getLocation () != Keys::SELF_TILDE && !symbol.is <Template> ()) {
		    auto note = Ymir::Error::createNote (symbol.getName ());
		    Ymir::Error::occurAndNote (func.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), func.getLocation ().str);
		}
	    }

	    lexing::Word gotSafeOrThrow (lexing::Word::eof ());
	    for (auto & ca : func.getCustomAttributes ()) {
		if (ca == Keys::FINAL_ && function.to<Function> ().isMethod ()) function.to <Function> ().isFinal (true);
		else {
		    Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.str);
		}
	    }

	    
	    if (func.isOver ()) function.to <Function> ().isOver (true);	    
	    function.to <Function> ().setThrowers (func.getThrowers ()); 
		
	    if (!isExtern || !func.getBody ().isEmpty ()) {
		if (func.getPrototype ().isVariadic ()) Ymir::Error::occur (func.getLocation (), ExternalError::get (DECL_VARIADIC_FUNC));
	    }

	    if (insert) 
		getReferent ().insert (function);
	    return function;
	}        

	semantic::Symbol Visitor::visitConstructor (const syntax::Constructor cs) {
	    auto semcs = semantic::Constructor::init (cs.getLocation (), cs, this-> _isWeak);
	    semcs.to <Constructor> ().setThrowers (cs.getThrowers ());
	    for (auto & ca : cs.getCustomAttributes ()) {
		Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.str);		
	    }

	    getReferent ().insert (semcs);
	    return semcs;
	}
	
	semantic::Symbol Visitor::visitStruct (const syntax::Struct str, bool insert) {
	    auto structure = Struct::init (str.getLocation (), str.getDeclarations (), this-> _isWeak);
	
	    auto symbols = getReferent ().getLocal (str.getLocation ().str);	
	    for (auto symbol : symbols) {
		auto note = Ymir::Error::createNote (symbol.getName ());
		Ymir::Error::occurAndNote (str.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), str.getLocation ().str);		
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

	semantic::Symbol Visitor::visitAlias (const syntax::Alias stal) {
	    auto alias = Alias::init (stal.getLocation (), stal.getValue (), this-> _isWeak);

	    auto symbols = getReferent ().getLocal (stal.getLocation ().str);
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (stal.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stal.getLocation ().str);
	    }	    

	    getReferent ().insert (alias);
	    return alias;
	}    

	semantic::Symbol Visitor::visitBlock (const syntax::DeclBlock block) {
	    pushReferent (Module::init (block.getLocation (), this-> _isWeak));
	    // A declaration block is just a list of declaration, we do not enter a new referent
	    for (const syntax::Declaration decl : block.getDeclarations ()) {
	    	visit (decl);		
	    }	    
	    
	    auto ret = popReferent ();
	    auto syms = ret.to <semantic::Module> ().getAllLocal ();
	    if (!block.isPrivate ()) {
		for (auto it : syms)
		    it.setPublic ();		
	    }
	    
	    if (!getReferent ().isEmpty ()) {
		for (auto it : syms) {
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

	semantic::Symbol Visitor::visitExtern (const syntax::ExternBlock ex_block) {
	    if (ex_block.getFrom () == Keys::CLANG) {
		if (!ex_block.getSpace ().isEof ()) 
		    Ymir::Error::occur (ex_block.getSpace (), ExternalError::get (SPACE_EXTERN_C));
		for (const syntax::Declaration ex_decl : ex_block.getDeclaration ().to <syntax::DeclBlock> ().getDeclarations ()) {
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
	
	void Visitor::visitInnerClass (Symbol cls, const std::vector <syntax::Declaration> & decls, bool prv, bool prot, bool pub) {
	    for (auto jt : decls) {
		match (jt) {
		    of (syntax::ExpressionWrapper, wrap, {
			    match (wrap.getContent ()) {
				of (syntax::VarDecl, de ATTRIBUTE_UNUSED, {
					cls.to <semantic::Class> ().addField (wrap.getContent ());
					if (prv)
					    cls.to <semantic::Class> ().setPrivate (de.getLocation ().str);
					else if (prot) {
					    cls.to <semantic::Class> ().setProtected (de.getLocation ().str);
					}
				    } 
				) else of (syntax::Set, se, {
					for (auto it : se.getContent ()) {
					    cls.to <semantic::Class> ().addField (it);
					    if (prv)						
						cls.to <semantic::Class> ().setPrivate (it.to <syntax::VarDecl> ().getLocation ().str);
					    else if (prot) {
						cls.to <semantic::Class> ().setProtected (it.to <syntax::VarDecl> ().getLocation ().str);
					    }
					}
				    }
				) else 
				    Error::halt ("%(r) - reaching impossible point", "Critical");			
			    }
			}
		    ) else of (syntax::DeclBlock, dc, {
			    visitInnerClass (cls, dc.getDeclarations (), dc.isPrivate (), dc.isProt (), dc.isPublic ());
			}
		    ) else of (syntax::Constructor, cs, {
			    auto sym = visitConstructor (cs);
			    sym.to <Constructor> ().setClass (cls);
			    if (pub)  sym.setPublic ();
			    if (prot) sym.setProtected ();					    
			}
		    ) else {
			    auto sym = visit (jt);
			    if (pub)  sym.setPublic ();
			    if (prot) sym.setProtected ();
			}
		}
	    }
	}
	
	semantic::Symbol Visitor::visitClass (const syntax::Class stcls) {
	    auto cls = Class::init (stcls.getLocation (), stcls.getAncestor (), this-> _isWeak);
	
	    auto symbols = getReferent ().getLocal (stcls.getLocation ().str);
	    for (auto symbol : symbols) {
		auto note = Ymir::Error::createNote (symbol.getName ());
		Ymir::Error::occurAndNote (stcls.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stcls.getLocation ().str);		
	    }

	    for (auto & ca : stcls.getAttributes ()) {
		if (ca == Keys::ABSTRACT) cls.to <Class> ().isAbs (true);
		else if (ca == Keys::FINAL_) cls.to <Class> ().isFinal (true);
		else {
		    Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.str);
		}
	    }
	    
	    pushReferent (cls);
	    visitInnerClass (cls, stcls.getDeclarations (), false, true, false);	   

	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}

	semantic::Symbol Visitor::visitTrait (const syntax::Trait sttrait) {
	    auto tr = Trait::init (sttrait.getLocation (), this-> _isWeak);

	    auto symbols = getReferent ().getLocal (sttrait.getLocation ().str);
	    for (auto symbol : symbols) {
	    	auto note = Ymir::Error::createNote (symbol.getName ());
	    	Ymir::Error::occurAndNote (sttrait.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), sttrait.getLocation ().str);
	    }

	    pushReferent (tr);
	    for (auto it : sttrait.getDeclarations ()) {
	    	match (it) {
	    	    of (syntax::DeclBlock, dc, {
	    		    for (auto jt : dc.getDeclarations ()) {
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
	    return tr;
	}

	semantic::Symbol Visitor::visitImpl (const syntax::Mixin stimpl) {
	    auto im = Impl::init (stimpl.getLocation (), stimpl.getMixin (), this-> _isWeak);
	    pushReferent (im);
	    visitInnerClass (im, stimpl.getDeclarations (), false, true, false);	    
	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}
	
	semantic::Symbol Visitor::visitEnum (const syntax::Enum stenm) {
	    auto enm = Enum::init (stenm.getLocation (), stenm.getValues (), stenm.getType (), this-> _isWeak);
	    auto symbols = getReferent ().getLocal (stenm.getLocation ().str);
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (stenm.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stenm.getLocation ().str);
	    }

	    pushReferent (enm);
	    for (auto it : stenm.getValues ()) {
		// Inside an enum the vars are declared using a vardecl expression
		auto en = visit (syntax::ExpressionWrapper::init (it.getLocation (), it));
		en.setPublic ();
	    }

	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}    

	void Visitor::visitInnerMacro (Symbol cls, const std::vector <syntax::Declaration> & decls, bool prv, bool prot, bool pub) {
	    for (auto jt : decls) {
		match (jt) {
		    of (syntax::MacroConstructor, constr, {
			    auto sym = visitMacroConstructor (constr);
			    if (pub) sym.setPublic ();
			    if (prot) sym.setProtected ();
			}
		    ) else of (syntax::MacroRule, rule, {
			    auto sym = visitMacroRule (rule);
			    if (pub) sym.setPublic ();
			    if (prot) sym.setProtected ();
			}
		    ) else of (syntax::DeclBlock, dc, {
			    visitInnerMacro (cls, dc.getDeclarations (), dc.isPrivate (), dc.isProt (), dc.isPublic ());
			}			
		    );			       
		}		
	    }	    
	}
	
	semantic::Symbol Visitor::visitMacro (const syntax::Macro macro) {
	    auto smc = Macro::init (macro.getLocation (), macro.getSkips ());
	    auto symbols = getReferent ().getLocal (macro.getLocation ().str);
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (macro.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), macro.getLocation ().str);
	    }

	    pushReferent (smc);
	    visitInnerMacro (smc, macro.getContent (), false, true, false);
	    
	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}

	semantic::Symbol Visitor::visitMacroConstructor (const syntax::MacroConstructor contr) {
	    auto ret = MacroConstructor::init (contr.getLocation (), syntax::MacroConstructor::init (contr));
	    getReferent ().insert (ret);
	    return ret;
	}

	semantic::Symbol Visitor::visitMacroRule (const syntax::MacroRule rule) {
	    auto ret = MacroRule::init (rule.getLocation (), syntax::MacroRule::init (rule));
	    auto symbols = getReferent ().getLocal (rule.getLocation ().str);
	    for (auto & it : getReferent ().to <Macro> ().getAllInner ()) {
		if (it.is <MacroRule> () && it.getName ().str == rule.getLocation ().str) {
		    auto note = Ymir::Error::createNote (it.getName ());
		    Ymir::Error::occurAndNote (rule.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), rule.getLocation ().str);
		}
	    }
	    
	    getReferent ().insert (ret);
	    return ret;
	}
	
	semantic::Symbol Visitor::visitVarDecl (const syntax::VarDecl stdecl) {
	    auto decl = VarDecl::init (stdecl.getLocation (), stdecl.getDecorators (), stdecl.getType (), stdecl.getValue (), this-> _isWeak);
	    auto symbols = getReferent ().getLocal (stdecl.getLocation ().str);
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (stdecl.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stdecl.getLocation ().str);
	    }

	    getReferent ().insert (decl);	
	    return decl;
	}
    
	semantic::Symbol Visitor::visitGlobal (const syntax::Global stglob) {
	    return visit (syntax::ExpressionWrapper::init (stglob.getLocation (), stglob.getContent ()));	
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
		closedir (dir);
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
			std::list <std::string> errors;
			try {
			    auto synt_module = syntax::Visitor::init (file_path, file).visitModGlobal ();
			    declarator::Visitor::init ().visit (synt_module);
			} catch (Error::ErrorList list) {
			    
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			} 
			fclose (file);
			
			if (errors.size () != 0) 
			    throw Error::ErrorList {errors};
		    }
		}
		
		getReferent ().use (it.second, Symbol::getModuleByPath (it.second));		
	    }
	}

	semantic::Symbol Visitor::visitImport (const syntax::Import imp) {
	    auto path = Path {imp.getModule ().str, "::"};
	    bool success = false;

	    if (__imported__ .find (path.toString ()) == __imported__.end ()) {
		auto file_path = imp.getPath () + ".yr";
		auto file = fopen (file_path.c_str (), "r");
		if (file != NULL) {
		    success = true;
		    // We add a fake module, to prevent infinite import loops
		    __imported__.emplace (path.toString ());
		    std::list <std::string> errors;
		    try {
			auto synt_module = syntax::Visitor::init (file_path, file).visitModGlobal ();
			declarator::Visitor::init ().visit (synt_module);
		    } catch (Error::ErrorList list) {
			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }		    
		    fclose (file);
		    
		    if (errors.size () != 0) {
			auto note = Ymir::Error::createNote (imp.getModule (), ExternalError::get (IN_IMPORT));
			errors.back () = Ymir::Error::addNote (imp.getModule (), errors.back (), note);
			throw Error::ErrorList {errors};
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
			    std::list <std::string> errors;
			    try {
				auto synt_module = syntax::Visitor::init (file_path, file).visitModGlobal ();
				declarator::Visitor::init ().visit (synt_module);
			    } catch (Error::ErrorList list) {				
				errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			    } 
			    fclose (file);
			    
			    if (errors.size () != 0) {
				auto note = Ymir::Error::createNote (imp.getModule (), ExternalError::get (IN_IMPORT));
				errors.back () = Ymir::Error::addNote (imp.getModule (), errors.back (), note);
				throw Error::ErrorList {errors};
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
	
	semantic::Symbol Visitor::visitTemplate (const syntax::Template tep) {
	    std::vector <lexing::Word> used;
	    for (auto par : tep.getParams ()) {
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
	    
	    auto sym = Template::init (tep.getLocation (), tep.getParams (), tep.getContent (), tep.getTest (), tep.getParams (), this-> _isWeak);
	    getReferent ().insert (sym);
	    return sym;
	}

	
	void Visitor::pushReferent (const Symbol sym) {
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

	Symbol Visitor::getReferent () {
	    if (this-> _referent.size () != 0)
		return this-> _referent.front ();
	    else return Symbol::__empty__;
	}

	void Visitor::setWeak () {
	    this-> _isWeak = true;
	}

	bool Visitor::isWeak () const {
	    return this-> _isWeak;
	}
	
    }
    
}
