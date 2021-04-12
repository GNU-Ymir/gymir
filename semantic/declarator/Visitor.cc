#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/syntax/visitor/Visitor.hh>
#include <ymir/semantic/symbol/_.hh>
#include <ymir/utils/Path.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/semantic/validator/MacroVisitor.hh>
#include <ymir/global/State.hh>
#include <dirent.h>
#include <algorithm>


using namespace Ymir;
using namespace global;

namespace semantic {

    namespace declarator {

	std::set <std::string> Visitor::__imported__;
	
	Visitor::Visitor (bool isTrusted) :
	    _isTrusted (false)
	{}
    
	Visitor Visitor::init (bool isTrusted) {
	    return Visitor (isTrusted);
	}

	semantic::Symbol Visitor::visit (const syntax::Declaration & ast) {
	    match (ast) {
		s_of (syntax::Module, mod) 
		    return visitModule (mod);		
		
		s_of (syntax::Function, func)
		    return visitFunction (func);		
		
		s_of (syntax::Struct, str)
		    return visitStruct (str);		

		s_of (syntax::Aka, al)
		    return visitAka (al);		

		s_of (syntax::DeclBlock, bl)
		    return visitBlock (bl);		

		s_of (syntax::ExternBlock, ex_bl)
		    return visitExtern (ex_bl);		
		
		s_of_u (syntax::Class)
		    return visitClass (ast);		

		s_of (syntax::Trait, trait)
		    return visitTrait (trait);

		s_of_u (syntax::Mixin)
		    return visitImpl (ast);
		
		s_of (syntax::Global, glb)
		    return visitGlobal (glb);

		s_of (syntax::Import, im)
		    return visitImport (im);

		s_of (syntax::Template, tep)
		    return visitTemplate (tep);

		s_of (syntax::Enum, en)
		    return visitEnum (en);

		s_of (syntax::Macro, mc)
		    return visitMacro (mc);
		
		s_of (syntax::ExpressionWrapper, wrap) {
		    match (wrap.getContent ()) {
			s_of (syntax::VarDecl, decl) 
			    return visitVarDecl (decl, wrap.getComments ());		    
		    }
		    
		    Error::halt ("%(r) - reaching impossible point", "Critical");
		    return Symbol::empty ();
		}		
	    }
	    
	    Ymir::OutBuffer buf;
	    ast.treePrint (buf);
	    println (buf);
	    Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Symbol::empty ();
	}    

	semantic::Symbol Visitor::visitModule (const syntax::Module & mod) {
	    auto path = Path {mod.getLocation ().getStr (), "::"};
	    auto modules = path.getFiles ();	    
	    bool isTrusted = path.startWith (Path {CoreNames::get (STD_MODULE), "::"}) || path.startWith (Path {CoreNames::get (CORE_MODULE), "::"}) || path.startWith ({CoreNames::get (ETC_MODULE), "::"});
	    if (mod.isGlobal ()) {
		auto file_location = Path {mod.getLocation ().getFilename ()}.stripExtension ();
		if (path.getFiles ().size () == 0) {
		    path = Path (file_location.getFiles ().back ());
		}
		__imported__.emplace (file_location.toString ());
		if (!mod.getLocation ().isEof () && !Path {mod.getLocation ().getStr (), "::"}.isRelativeFrom (file_location)) {
		    Ymir::Error::occur (mod.getLocation (), ExternalError::get (WRONG_MODULE_NAME), mod.getLocation ().getStr (), Path {mod.getLocation ().getStr (), "::"}.toString () + ".yr");
		}
	    }

	    if (mod.isGlobal () && modules.size () != 0) {
		auto sym = Symbol::getModuleByPath (path.fileName ().toString ());
		if (!sym.isEmpty ()) {
		    pushReferent (sym);
		} else {
		    auto semMod = Module::init (lexing::Word::init (mod.getLocation (), path.fileName ().toString ()), mod.getComments (), this-> _isWeak, this-> _isTrusted || State::instance ().isStandalone () || isTrusted, mod.isGlobal ());
		    pushReferent (semMod);
		}
	    } else {
		auto semMod = Module::init (lexing::Word::init (mod.getLocation (), path.fileName ().toString ()), mod.getComments (), this-> _isWeak, this-> _isTrusted || State::instance ().isStandalone () || isTrusted, mod.isGlobal ());
		pushReferent (semMod);
	    }

	    if (mod.isGlobal () && !State::instance ().isStandalone ()) {
		importAllCoreFiles ();
	    }
	    
	    for (auto it : mod.getDeclarations ()) {
		visit (it);
	    }

	    auto ret = popReferent ();
	    
	    if (mod.isGlobal () && modules.size () > 1) {
		auto glob = Symbol::getModule (modules [0]);
		if (glob.isEmpty ()) {
		    glob = Module::init (lexing::Word::init (mod.getLocation (), modules [0]), mod.getComments (), this-> _isWeak, this-> _isTrusted || State::instance ().isStandalone (), mod.isGlobal ());
		    pushReferent (glob);
		} else pushReferent (glob);

		createSubModules (mod.getLocation (), std::vector <std::string> (modules.begin () + 1, modules.end ()), ret);
		
		glob = popReferent ();
		Symbol::registerModule (modules [0], glob);		
		return ret;
	    } else if (mod.isGlobal ()) {
		if (modules.size () == 1) {
		    Symbol::registerModule (modules [0], ret);
		} else if (modules.size () == 0) {
		    auto path = Path {ret.getRealName (), "::"};
		    auto modules = path.getFiles ();	   		    
		    Symbol::registerModule (modules [0], ret);
		}
	    } else getReferent ().insert (ret);
	    
	    return ret;
	}

	void Visitor::createSubModules (const lexing::Word & loc, const std::vector <std::string> & names, semantic::Symbol last) {
	    if (names.size () == 1) {
		auto symbols = getReferent ().getLocal (names [0]);
		for (auto sym : symbols) {
		    if (sym.is<Module> ()) {
			for (auto &it : sym.to <Module> ().getAllLocal ()) {
			    last.insert (it);
			}
			for (auto & it : sym.getUsedSymbols ()) {
			    last.use (it.first, it.second);
			}
		    }
		}

		getReferent ().insertOrReplace (last);
	    } else if (names.size () != 0) {
		auto symbols = getReferent ().getLocal (names [0]);
		for (auto sym : symbols) {
		    if (sym.is <Module> ()) {
			pushReferent (sym);
			std::vector<std::string> modules (names.begin () + 1, names.end ());
			createSubModules (loc, modules, last);
			auto mod = popReferent ();
			getReferent ().insertOrReplace (mod);
			return;
		    }
		}
		
		auto semMod = Module::init (lexing::Word::init (loc, names [0]), "", this-> _isWeak, this-> _isTrusted || State::instance ().isStandalone (), true);
		pushReferent (semMod);
		std::vector<std::string> modules (names.begin () + 1, names.end ());
		createSubModules (loc, modules, last);
		auto mod = popReferent ();
		getReferent ().insert (mod);
	    }
	}
	
	semantic::Symbol Visitor::visitFunction (const syntax::Function & func, bool isExtern, bool insert) {
	    auto function = Function::init (func.getLocation (), func.getComments (), func, this-> _isWeak);
	
	    auto symbols = getReferent ().getLocal (func.getLocation ().getStr ());	    
	    for (auto symbol : symbols) {
		if (!symbol.is <Function> () && !symbol.is <Module> () && !symbol.is <ModRef> () && func.getLocation () != Keys::SELF_TILDE && !symbol.is <Template> ()) {
		    auto note = Ymir::Error::createNote (symbol.getName ());
		    Ymir::Error::occurAndNote (func.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), func.getLocation ().getStr ());
		}
	    }

	    lexing::Word gotSafeOrThrow (lexing::Word::eof ());
	    for (auto & ca : func.getCustomAttributes ()) {
		if (ca == Keys::FINAL_ && function.to<Function> ().isMethod ()) function.to <Function> ().isFinal (true);
		else {
		    Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.getStr ());
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

	semantic::Symbol Visitor::visitConstructor (const syntax::Constructor & cs) {
	    auto semcs = semantic::Constructor::init (cs.getLocation (), cs.getComments (), cs, this-> _isWeak);
	    semcs.to <Constructor> ().setThrowers (cs.getThrowers ());
	    for (auto & ca : cs.getCustomAttributes ()) {
		Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.getStr ());		
	    }

	    getReferent ().insert (semcs);
	    return semcs;
	}
		
	semantic::Symbol Visitor::visitStruct (const syntax::Struct & str, bool insert) {
	    auto structure = Struct::init (str.getLocation (), str.getComments (), str.getDeclarations (), str.getDeclComments (), this-> _isWeak);
	
	    auto symbols = getReferent ().getLocal (str.getLocation ().getStr ());	
	    for (auto symbol : symbols) {
		auto note = Ymir::Error::createNote (symbol.getName ());
		Ymir::Error::occurAndNote (str.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), str.getLocation ().getStr ());		
	    } 
	
	    for (auto & ca : str.getCustomAttributes ()) {
		if (ca == Keys::PACKED) {
		    structure.to<Struct> ().isPacked (true);
		} else if (ca == Keys::UNION) {
		    structure.to<Struct> ().isUnion (true);
		} else Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.getStr ());

		if (structure.to <Struct> ().isUnion () && structure.to <Struct> ().isPacked ()) {
		    Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::PACKED_AND_UNION));
		}
	    }
	    
	    if (insert)
		getReferent ().insert (structure);	    
	    return structure;
	}

	semantic::Symbol Visitor::visitAka (const syntax::Aka & stal) {
	    auto alias = Aka::init (stal.getLocation (), stal.getComments (), stal.getValue (), this-> _isWeak);

	    auto symbols = getReferent ().getLocal (stal.getLocation ().getStr ());
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (stal.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stal.getLocation ().getStr ());
	    }	    

	    getReferent ().insert (alias);
	    return alias;
	}    

	semantic::Symbol Visitor::visitBlock (const syntax::DeclBlock &  block) {
	    pushReferent (Module::init (block.getLocation (), block.getComments (), this-> _isWeak, this-> _isTrusted || State::instance ().isStandalone (), false));
	    
	    // A declaration block is just a list of declaration, we do not enter a new referent
	    for (syntax::Declaration decl : block.getDeclarations ()) {
		if (block.getDeclarations ().size () == 1)
		    decl.setComments (block.getComments () + decl.getComments ());
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

	semantic::Symbol Visitor::visitExtern (const syntax::ExternBlock & ex_block) {
	    if (ex_block.getFrom () == Keys::CLANG) {
		if (!ex_block.getSpace ().isEof ()) 
		    Ymir::Error::occur (ex_block.getSpace (), ExternalError::get (SPACE_EXTERN_C));
		for (const syntax::Declaration ex_decl : ex_block.getDeclaration ().to <syntax::DeclBlock> ().getDeclarations ()) {
		    match (ex_decl) {
			of (syntax::Function, func) {
			    auto decl = visitFunction (func, true);
			    decl.to <semantic::Function> ().setExternalLanguage (ex_block.getFrom ().getStr ());
			} elof (syntax::ExpressionWrapper, wrap) {
			    match (wrap.getContent ()) {
				of (syntax::VarDecl, decl) {
				    auto dl = visitVarDecl (decl, wrap.getComments ());
				    dl.to <semantic::VarDecl> ().setExternalLanguage (ex_block.getFrom ().getStr ());
				    continue;
				} fo;		    
			    }
			    
			    Error::halt ("%(r) - reaching impossible point", "Critical");
			    return Symbol::empty ();
			} elof (syntax::Global, glb) {
			    auto decl = visitGlobal (glb);
			    decl.to <semantic::VarDecl> ().setExternalLanguage (ex_block.getFrom ().getStr ());
			} elfo {
			    Ymir::OutBuffer buf;
			    ex_decl.treePrint (buf);
			    println (buf.str ());
			    Ymir::Error::occur (ex_block.getLocation (), ExternalError::get (IMPOSSIBLE_EXTERN));
			}
		    }
		}
	    } else {
		Ymir::Error::occur (ex_block.getLocation (), ExternalError::get (IMPOSSIBLE_EXTERN));
	    }
	    
	    return Symbol::empty ();
	}
	
	void Visitor::visitInnerClass (Symbol cls, const std::vector <syntax::Declaration> & decls, bool prv, bool prot, bool pub) {
	    for (auto jt : decls) {
		match (jt) {
		    of (syntax::ExpressionWrapper, wrap) {
			match (wrap.getContent ()) {
			    of (syntax::VarDecl, de) {
				cls.to <semantic::Class> ().addField (wrap.getContent ());					
				cls.to <semantic::Class> ().setFieldComment (de.getLocation ().getStr (), wrap.getComments ());
				if (prv)
				cls.to <semantic::Class> ().setPrivate (de.getLocation ().getStr ());
				else if (prot) {
				    cls.to <semantic::Class> ().setProtected (de.getLocation ().getStr ());
				}
			    } elof (syntax::Set, se) {
				for (auto it : se.getContent ()) {
				    cls.to <semantic::Class> ().addField (it);
				    cls.to <semantic::Class> ().setFieldComment (it.to <syntax::VarDecl> ().getLocation ().getStr (), wrap.getComments ());
				    if (prv) {					
					cls.to <semantic::Class> ().setPrivate (it.to <syntax::VarDecl> ().getLocation ().getStr ());
				    } else if (prot) {
					cls.to <semantic::Class> ().setProtected (it.to <syntax::VarDecl> ().getLocation ().getStr ());
				    }
				}
			    } elof_u (syntax::Assert) {
				cls.to <semantic::Class> ().addAssertion (wrap.getContent ());
				cls.to <semantic::Class> ().addAssertionComments (wrap.getComments ());
			    } elfo {
				Error::halt ("%(r) - reaching impossible point", "Critical");
			    }
			}
		    } elof (syntax::DeclBlock, dc) {			    
			visitInnerClass (cls, dc.getDeclarations (), dc.isPrivate (), dc.isProt (), dc.isPublic ());
		    } elof (syntax::Constructor, cs) {
			auto sym = visitConstructor (cs);
			if (!sym.isEmpty ()) sym.to <Constructor> ().setClass (cls);
			if (!sym.isEmpty () && pub)  sym.setPublic ();
			if (!sym.isEmpty () && prot) sym.setProtected ();					    
		    } elof (syntax::Destructor, dc) {
			if (!cls.to <semantic::Class> ().getDestructor ().isEmpty ()) {
			    auto note = Ymir::Error::createNote (cls.to <semantic::Class> ().getDestructor ().getName ());
			    Ymir::Error::occurAndNote (dc.getLocation (), note, ExternalError::get (MULTIPLE_DESTRUCTOR));
			}

			auto sym = Function::init (dc.getLocation (), dc.getComments (), dc, this-> _isWeak);
			sym.setReferent (cls);
			cls.to <semantic::Class> ().setDestructor (sym);
			// Destructor are always public
		    } elof (syntax::CondBlock, cb) {
			Ymir::Error::occur (cb.getLocation (), ExternalError::get (CONDITIONAL_NON_TEMPLATE_CLASS));
		    } elfo {
			auto sym = visit (jt);
			if (!sym.isEmpty () && pub)  sym.setPublic ();
			if (!sym.isEmpty () && prot) sym.setProtected ();
		    }
		}
	    }
	}
	
	semantic::Symbol Visitor::visitClass (const syntax::Declaration & stdecl) {
	    auto & stcls = stdecl.to <syntax::Class> ();
	    auto cls = Class::init (stcls.getLocation (), stcls.getComments (), stcls.getAncestor (), this-> _isWeak);
	
	    auto symbols = getReferent ().getLocal (stcls.getLocation ().getStr ());
	    for (auto symbol : symbols) {
		auto note = Ymir::Error::createNote (symbol.getName ());
		Ymir::Error::occurAndNote (stcls.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stcls.getLocation ().getStr ());		
	    }

	    for (auto & ca : stcls.getAttributes ()) {
		if (ca == Keys::ABSTRACT) cls.to <Class> ().isAbs (true);
		else if (ca == Keys::FINAL_) cls.to <Class> ().isFinal (true);
		else {
		    Ymir::Error::occur (ca, Ymir::ExternalError::get (Ymir::UNDEFINED_CA), ca.getStr ());
		}
	    }
	    
	    pushReferent (cls);
	    visitInnerClass (cls, stcls.getDeclarations (), false, true, false);	   

	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}
	
	semantic::Symbol Visitor::visitTrait (const syntax::Trait & sttrait) {
	    auto tr = Trait::init (sttrait.getLocation (), sttrait.getComments (), this-> _isWeak);

	    auto symbols = getReferent ().getLocal (sttrait.getLocation ().getStr ());
	    for (auto symbol : symbols) {
	    	auto note = Ymir::Error::createNote (symbol.getName ());
	    	Ymir::Error::occurAndNote (sttrait.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), sttrait.getLocation ().getStr ());
	    }

	    pushReferent (tr);
	    visitInnerTrait (tr, sttrait.getDeclarations (), false, true, false);
	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return tr;
	}

	void Visitor::visitInnerTrait (Symbol tr, const std::vector <syntax::Declaration> & decls, bool prv, bool prot, bool pub) {
	    for (auto it : decls) {
	    	match (it) {
	    	    of (syntax::DeclBlock, dc) {
			visitInnerTrait (tr, dc.getDeclarations (), dc.isPrivate (), dc.isProt (), dc.isPublic ());
		    }
		    elof (syntax::CondBlock, cb) {
			Ymir::Error::occur (cb.getLocation (), ExternalError::get (CONDITIONAL_NON_TEMPLATE_TRAIT));
		    }
		    elof(syntax::ExpressionWrapper, wrap) {
			if (wrap.getContent ().is <syntax::Assert> ()) {
			    tr.to <semantic::Trait> ().addAssertion (wrap.getContent ());
			    tr.to <semantic::Trait> ().addAssertionComments (wrap.getComments ());
			} else {
			    auto sym = visit (it);
			    if (!sym.isEmpty () && prot) sym.setProtected ();
			    if (!sym.isEmpty () && pub) sym.setPublic ();
			}
		    } elfo {			
	    		auto sym = visit (it);
			if (!sym.isEmpty () && prot) sym.setProtected ();
			if (!sym.isEmpty () && pub) sym.setPublic ();
	    	    }
	    	}
	    }  
	}
	
	semantic::Symbol Visitor::visitImpl (const syntax::Declaration & decl) {
	    auto stimpl = decl.to <syntax::Mixin> ();
	    auto im = Impl::init (stimpl.getLocation (), stimpl.getComments (), stimpl.getMixin (), this-> _isWeak);
	    pushReferent (im);
	    visitInnerClass (im, stimpl.getDeclarations (), false, true, false);	    
	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;	    
	}
	
	semantic::Symbol Visitor::visitEnum (const syntax::Enum & stenm) {
	    auto enm = Enum::init (stenm.getLocation (), stenm.getComments (), stenm.getValues (), stenm.getType (), stenm.getFieldComments (), this-> _isWeak);
	    auto symbols = getReferent ().getLocal (stenm.getLocation ().getStr ());
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (stenm.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stenm.getLocation ().getStr ());
	    }

	    pushReferent (enm);
	    for (auto it : stenm.getValues ()) {
		// Inside an enum the vars are declared using a vardecl expression
		auto en = visit (syntax::ExpressionWrapper::init (it.getLocation (), "",it));
		en.setPublic ();
	    }

	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}    

	void Visitor::visitInnerMacro (Symbol cls, const std::vector <syntax::Declaration> & decls, bool prv, bool prot, bool pub) {
	    for (auto jt : decls) {
		match (jt) {
		    of (syntax::MacroConstructor, constr) {
			auto sym = visitMacroConstructor (constr);
			if (!sym.isEmpty () && pub) sym.setPublic ();
			if (!sym.isEmpty () && prot) sym.setProtected ();
		    } elof (syntax::MacroRule, rule) {
			auto sym = visitMacroRule (rule);
			if (!sym.isEmpty () && pub) sym.setPublic ();
			if (!sym.isEmpty () && prot) sym.setProtected ();
		    } elof (syntax::DeclBlock, dc) {
			visitInnerMacro (cls, dc.getDeclarations (), dc.isPrivate (), dc.isProt (), dc.isPublic ());
		    } fo;		       
		}		
	    }	    
	}
	
	semantic::Symbol Visitor::visitMacro (const syntax::Macro & macro) {
	    auto smc = Macro::init (macro.getLocation (), macro.getComments ());
	    auto symbols = getReferent ().getLocal (macro.getLocation ().getStr ());
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (macro.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), macro.getLocation ().getStr ());
	    }

	    pushReferent (smc);
	    visitInnerMacro (smc, macro.getContent (), false, true, false);
	    
	    auto ret = popReferent ();
	    getReferent ().insert (ret);
	    return ret;
	}

	semantic::Symbol Visitor::visitMacroConstructor (const syntax::MacroConstructor & contr) {
	    auto ret = MacroConstructor::init (contr.getLocation (), contr.getComments (), syntax::MacroConstructor::init (contr));
	    getReferent ().insert (ret);
	    return ret;
	}

	semantic::Symbol Visitor::visitMacroRule (const syntax::MacroRule & rule) {
	    auto known_rules = semantic::validator::MacroVisitor::getKnwonRules ();
	    if (std::find (known_rules.begin (), known_rules.end (), rule.getLocation ().getStr ()) != known_rules.end ()) {
		Ymir::Error::occur (rule.getLocation (), Ymir::ExternalError::get (Ymir::RESERVED_RULE_NAME), rule.getLocation ().getStr ());
	    }
	    
	    auto ret = MacroRule::init (rule.getLocation (), rule.getComments (), syntax::MacroRule::init (rule));
	    auto symbols = getReferent ().getLocal (rule.getLocation ().getStr ());
	    for (auto & it : getReferent ().to <Macro> ().getAllInner ()) {
		if (it.is <MacroRule> () && it.getName ().getStr () == rule.getLocation ().getStr ()) {
		    auto note = Ymir::Error::createNote (it.getName ());
		    Ymir::Error::occurAndNote (rule.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), rule.getLocation ().getStr ());
		}
	    }
	    
	    getReferent ().insert (ret);
	    return ret;
	}
	
	semantic::Symbol Visitor::visitVarDecl (const syntax::VarDecl & stdecl, const std::string & comments) {
	    auto decl = VarDecl::init (stdecl.getLocation (), comments, stdecl.getDecorators (), stdecl.getType (), stdecl.getValue (), this-> _isWeak);
	    auto symbols = getReferent ().getLocal (stdecl.getLocation ().getStr ());
	    if (symbols.size () != 0) {
		auto note = Ymir::Error::createNote (symbols [0].getName ());
		Ymir::Error::occurAndNote (stdecl.getLocation (), note, Ymir::ExternalError::get (Ymir::SHADOWING_DECL), stdecl.getLocation ().getStr ());
	    }

	    getReferent ().insert (decl);	
	    return decl;
	}
    
	semantic::Symbol Visitor::visitGlobal (const syntax::Global & stglob) {
	    return visit (syntax::ExpressionWrapper::init (stglob.getLocation (), stglob.getComments (), stglob.getContent ()));	
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
	    auto dir = State::instance ().getCorePath ();
	    auto entries = dirEntries (dir, "core");
	    for (auto & it : entries) {
		if (__imported__.find (it.first) == __imported__.end ()) {
		    auto file_path = it.first + ".yr";
			
		    if (Ymir::file_exists (file_path)) {
			// We add a fake module, to prevent infinite import loops
			__imported__.emplace (it.first);
			std::list <Ymir::Error::ErrorMsg> errors;
			try {
			    auto synt_module = syntax::Visitor::init (file_path).visitModGlobal ();
			    declarator::Visitor::init ().visit (synt_module);
			} catch (Error::ErrorList list) {
			    
			    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			} 
			
			if (errors.size () != 0) 
			    throw Error::ErrorList {errors};
		    }
		}
		
		getReferent ().use (it.second, Symbol::getModuleByPath (it.second));		
	    }
	}

	semantic::Symbol Visitor::visitImport (const syntax::Import & imp) {
	    auto path = Path {imp.getModule ().getStr (), "::"};
	    bool success = false;

	    if (__imported__ .find (path.toString ()) == __imported__.end ()) {
		auto file_path = imp.getPath () + ".yr";		
		if (Ymir::file_exists (file_path)) {
		    success = true;
		    // We add a fake module, to prevent infinite import loops
		    __imported__.emplace (path.toString ());
		    std::list <Ymir::Error::ErrorMsg> errors;
		    try {
			auto synt_module = syntax::Visitor::init (file_path).visitModGlobal ();
			bool isTrusted = path.startWith (Path {CoreNames::get (STD_MODULE), "::"}) || path.startWith (Path {CoreNames::get (CORE_MODULE), "::"}) || path.startWith ({CoreNames::get (ETC_MODULE), "::"});
			declarator::Visitor::init (isTrusted).visit (synt_module);
		    } catch (Error::ErrorList list) {
			
			errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    }		    
		    
		    if (errors.size () != 0) {
			auto note = Ymir::Error::createNote (imp.getModule (), ExternalError::get (IN_IMPORT));
			errors.back ().addNote (note);
			throw Error::ErrorList {errors};
		    }
		} 
	    } else success = true;
	    
	    if (!success) {
		int j = 0;
		for (auto & it : State::instance ().getIncludeDirs ()) {
		    path = Path::build (it, Path {imp.getModule ().getStr (), "::"}.toString ());
		    if (__imported__.find (path.toString ()) == __imported__.end ()) {
			auto file_path = path.toString () + ".yr";
			
			if (Ymir::file_exists (file_path)) {
			    success = true;
			    // We add a fake module, to prevent infinite import loops
			    __imported__.emplace (path.toString ());
			    std::list <Ymir::Error::ErrorMsg> errors;
			    try {
				auto synt_module = syntax::Visitor::init (file_path).visitModGlobal ();
				bool isTrusted = path.startWith (Path {CoreNames::get (STD_MODULE), "::"}) || path.startWith (Path {CoreNames::get (CORE_MODULE), "::"}) || path.startWith ({CoreNames::get (ETC_MODULE), "::"});
				declarator::Visitor::init (isTrusted).visit (synt_module);
			    } catch (Error::ErrorList list) {				
				errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
			    } 
			    
			    if (errors.size () != 0) {
				auto note = Ymir::Error::createNote (imp.getModule (), ExternalError::get (IN_IMPORT));
				errors.back ().addNote (note);
				throw Error::ErrorList {errors};
			    }
			    break;			    
			}
		    } else success = true;
		    j += 1;
		}

		if (!success) {
		    auto path = (Path {imp.getModule ().getStr (), "::"}).toString () + ".yr";
		    Error::occur (imp.getModule (), ExternalError::get (NO_SUCH_FILE), path);
		}
	    }

	    getReferent ().use (imp.getModule ().getStr () , Symbol::getModuleByPath (imp.getModule ().getStr ()));
	    
	    return Symbol::empty ();	    		    
	}	
	
	semantic::Symbol Visitor::visitTemplate (const syntax::Template & tep) {
	    std::vector <lexing::Word> used;
	    for (auto par : tep.getParams ()) {
		match (par) {
		    of (syntax::VariadicVar, vr) {
			for (auto & use : used) {
			    if (use.getStr () == vr.getLocation ().getStr ()) {
				Error::occur (vr.getLocation (), ExternalError::get (SHADOWING_DECL), use.getStr ());
			    }
			}
			used.push_back (vr.getLocation ());
		    } elof (syntax::OfVar, vr) {
			for (auto & use : used) {
			    if (use.getStr () == vr.getLocation ().getStr ()) {
				Error::occur (vr.getLocation (), ExternalError::get (SHADOWING_DECL), use.getStr ());
			    }
			}
			used.push_back (vr.getLocation ());
		    } elof (syntax::VarDecl, vr) {
			for (auto & use : used) {
			    if (use.getStr () == vr.getLocation ().getStr ()) {
				       Error::occur (vr.getLocation (), ExternalError::get (SHADOWING_DECL), use.getStr ());
			    }
			}
			used.push_back (vr.getLocation ());
		    } elof(syntax::Var, vr) {
			for (auto & use : used) {
			    if (use.getStr () == vr.getLocation ().getStr ()) {
				Error::occur (vr.getLocation (), ExternalError::get (SHADOWING_DECL), use.getStr ());
			    }
			}
			used.push_back (vr.getLocation ());
		    } fo;	       
		}		
	    }
	    
	    auto sym = Template::init (tep.getLocation (), tep.getComments (), tep.getParams (), tep.getContent (), tep.getTest (), tep.getParams (), this-> _isWeak);
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

	void Visitor::purge () {
	    __imported__.clear ();
	}
	
    }
    
}
