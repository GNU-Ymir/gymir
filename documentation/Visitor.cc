#include <ymir/documentation/Visitor.hh>
#include <ymir/semantic/validator/FunctionVisitor.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/string.hh>
#include <algorithm>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/syntax/declaration/_.hh>
#include <ymir/documentation/String.hh>
#include <ymir/documentation/Array.hh>
#include <ymir/documentation/Dict.hh>
#include <ymir/global/Core.hh>
#include <ymir/global/State.hh>
#include <ymir/utils/Path.hh>


namespace documentation {

    using namespace semantic;
    using namespace json;
    using namespace generator;
    
    Visitor::Visitor (validator::Visitor & context):
	_context (context)
    {}

    Visitor Visitor::init (validator::Visitor & context) {
	return Visitor (context);
    }

    JsonValue Visitor::dumpDependency (semantic::Symbol & sym) {
	std::vector <JsonValue> deps;
	auto list = sym.getAllReachableUsed ();
	for (auto & it : list) {
	    deps.push_back (JsonString::init (it.getRealName ()));
	}

	std::map <std::string, JsonValue> ret;
	ret ["dependencies"] = JsonArray::init (deps);
	return JsonDict::init (ret);
    }

    std::string Visitor::dumpGCCDependency (semantic::Symbol & sym) {
	Ymir::OutBuffer buf;
	auto list = sym.getAllReachableUsed ();
	int i = 0;
	for (auto & it : global::State::instance ().getGCCDepTargets ()) {
	    if (i != 0) buf.write (" ");
	    i += 1;
	    buf.write (it);
	}

	buf.write (": ", sym.getName ().getFilename ());
	
	for (auto & it : list) {
	    Ymir::Path name (it.getRealName (), "::");
	    bool isStd = false;
	    if (name.getFiles ()[0] == global::CoreNames::get (global::CORE_MODULE) ||
		name.getFiles ()[0] == global::CoreNames::get (global::STD_MODULE) ||
		name.getFiles () [0] == global::CoreNames::get (global::ETC_MODULE)) {
		isStd = true;
	    }
	    
	    if (!global::State::instance ().isGCCDepSkipActive () || !isStd) {
		buf.write (" \\\n", it.getName ().getFilename ());
	    }
	}

	buf.writeln ("");
	return buf.str ();
    }    

    JsonValue Visitor::dump (const semantic::Symbol & sym) {
	JsonValue val (JsonValue::empty ());
	if (!sym.isWeak ()) {
	    match (sym) {
		of (semantic::Module, mod) {
		    this-> _context.pushReferent (sym, "dump::module");
		    val = this-> dumpModule (mod);
		    this-> _context.popReferent ("dump::module");
		} elof_u (semantic::Function) {
		    this-> _context.pushReferent (sym, "dump::function");
		    val = this-> dumpFunction (sym);
		    this-> _context.popReferent ("dump::function");
		} elof (semantic::VarDecl, decl) {
		    this-> _context.pushReferent (sym, "dump::vardecl");
		    val = this-> dumpVarDecl (decl);
		    this-> _context.popReferent ("dump::vardecl");			    
		} elof (semantic::Aka, al) {
		    this-> _context.pushReferent (sym, "dump::alias");
		    val = this-> dumpAka (al);
		    this-> _context.popReferent ("dump::alias");
		} elof (semantic::Struct, str) {
		    this-> _context.pushReferent (sym, "dump::struct");
		    val = this-> dumpStruct (str);
		    this-> _context.popReferent ("dump::struct");
		} elof_u (semantic::TemplateSolution) {
		} elof (semantic::Enum, en) {
		    this-> _context.pushReferent (sym, "dump::enum");
		    val = this-> dumpEnum (en);
		    this-> _context.popReferent ("dump::enum");
		} elof (semantic::Class, cl) {
		    this-> _context.pushReferent (sym, "dump::class");
		    val = this-> dumpClass (cl);
		    this-> _context.popReferent ("dump::class");
		} elof (semantic::Trait, tr) {
		    this-> _context.pushReferent (sym, "dump::trait");
		    val = this-> dumpTrait (tr);
		    this-> _context.popReferent ("dump::trait");
		} elof (semantic::Template, tm) {
		    this-> _context.pushReferent (sym, "dump::template");
		    val = this-> dumpTemplate (tm);
		    this-> _context.popReferent ("dump::template");
		} elof (semantic::Macro, x) {
		    this-> _context.pushReferent (sym, "dump::macro");
		    val = this-> dumpMacro (x);
		    this-> _context.popReferent ("dump::macro");
		} elof_u (semantic::ModRef) {
		} elfo {
		    // Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		}
	    }
	}
	return val;
    }

    JsonValue Visitor::dumpUnvalidated (const semantic::Symbol & sym) {
	match (sym) {
	    of (semantic::Function, fn) {
		return this-> dumpFunctionUnvalidated (fn.getContent (), sym.isPublic (), false);
	    } fo;
	}
	Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	return JsonValue::empty ();
    }

    JsonValue Visitor::dumpUnvalidated (const syntax::Declaration & decl, bool pub, bool prot) {
	match (decl) {
	    of (syntax::DeclBlock, bl) {
		return this-> dumpDeclBlockUnvalidated (bl, pub, prot);
	    } elof (syntax::CondBlock, cd) {
		return this-> dumpCondBlockUnvalidated (cd, pub, prot);
	    } elof (syntax::Class, cl) {
		return this-> dumpClassUnvalidated (cl, pub, prot);
	    } elof (syntax::Enum, en) {
		return this-> dumpEnumUnvalidated (en, pub, prot);
	    } elof (syntax::ExternBlock, ex) {
		return this-> dumpExternBlockUnvalidated (ex, pub, prot);
	    } elof_u (syntax::Constructor) {
		return JsonValue::empty ();
	    } elof (syntax::Function, fn) {
		return this-> dumpFunctionUnvalidated (fn, pub, prot);
	    } elof (syntax::Global, gl) {
		return this-> dumpGlobalUnvalidated (gl, pub, prot);
	    } elof_u (syntax::Import) {
		return JsonValue::empty ();
	    } elof_u (syntax::Mixin) {
		return JsonValue::empty ();
	    } elof (syntax::Module, mod) {
		return this-> dumpModuleUnvalidated (mod, pub, prot);
	    } elof (syntax::Struct, str) {
		return this-> dumpStructUnvalidated (str, pub, prot);
	    } elof (syntax::Template, tmp) {
		return this-> dumpTemplateUnvalidated (tmp, pub, prot);
	    } elof (syntax::Trait, tr) {
		return this-> dumpTraitUnvalidated (tr, pub, prot);
	    } elof_u (syntax::Use) {
		return JsonValue::empty ();
	    } elof (syntax::Macro, m) {
		return this-> dumpMacroUnvalidated (m, pub, prot);		
	    } elof (syntax::Aka, a) {
		return this-> dumpAkaUnvalidated (a, pub, prot);
	    } fo;
	}
	Ymir::OutBuffer buf;
	decl.treePrint (buf, 0);
	println (buf.str ());
    
	Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	return JsonValue::empty ();
    }

    void Visitor::dumpStandard (const semantic::ISymbol & mod, std::map <std::string, JsonValue> & val) {
	val ["name"] = JsonString::init (mod.getRealName ().getValue ());
	val ["loc_file"] = JsonString::init (mod.getName ().getFilename ());
	val ["loc_line"] = JsonString::init (mod.getName ().getLine ());
	val ["loc_col"] = JsonString::init (mod.getName ().getColumn ());
	val ["doc"] = JsonString::init (mod.getComments ());
	
	if (mod.isPublic ()) val ["protection"] = JsonString::init ("pub");
	else if (mod.isProtected ()) val ["protection"] = JsonString::init ("prot");
	else val ["protection"] = JsonString::init ("prv");
    }    

    void Visitor::dumpStandard (const syntax::IDeclaration & mod, bool pub, bool prot, std::map <std::string, json::JsonValue> & val) {
	val ["name"] = JsonString::init (mod.getLocation ().getStr ());
	val ["loc_file"] = JsonString::init (mod.getLocation ().getFilename ());
	val ["loc_line"] = JsonString::init (mod.getLocation ().getLine ());
	val ["loc_col"] = JsonString::init (mod.getLocation ().getColumn ());
	val ["doc"] = JsonString::init (mod.getComments ());
	
	if (pub) val ["protection"] = JsonString::init ("pub");
	else if (prot) val ["protection"] = JsonString::init ("prot");
	else val ["protection"] = JsonString::init ("prv");
    }
    
    JsonValue Visitor::dumpModule (const semantic::Module & mod) {
	if (!mod.isExtern ()) {
	    std::map <std::string, JsonValue> val;
	    val ["type"] = JsonString::init ("module");
	    this-> dumpStandard (mod, val);
	    
	    std::vector <JsonValue> childs;
	    for (auto & it : mod.getAllLocal ()) {
		if (!it.is <semantic::Module> () || !it.to <semantic::Module> ().isGlobal ()) {		
		    auto ch = this-> dump (it);
		    if (!ch.isEmpty ()) {
			childs.push_back (ch);
		    }
		}
	    }

	    val ["childs"] = JsonArray::init (childs);
	    return JsonDict::init (val);
	}
	return JsonValue::empty ();
    }    

    JsonValue Visitor::dumpModuleUnvalidated (const syntax::Module & mod, bool pub, bool prot) {	
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("module");
	this-> dumpStandard (mod, pub, prot, val);

	std::vector <JsonValue> childs;
	for (auto & it : mod.getDeclarations ()) {	    
	    auto ch = this-> dumpUnvalidated (it, false, false);
	    if (!ch.isEmpty ())
		childs.push_back (ch);
	}
	
	val ["childs"] = JsonArray::init (childs);	
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpDeclBlockUnvalidated (const syntax::DeclBlock & dl, bool pub, bool prot) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("block");
	this-> dumpStandard (dl, pub, prot, val);

	std::vector <JsonValue> childs;
	for (auto & it : dl.getDeclarations ()) {
	    auto ch = this-> dumpUnvalidated (it, dl.isPublic (), dl.isProt ());
	    if (!ch.isEmpty ())
		childs.push_back (ch);
	}
	
	val ["childs"] = JsonArray::init (childs);	
	return JsonDict::init (val);
    }


    JsonValue Visitor::dumpExternBlockUnvalidated (const syntax::ExternBlock & mod, bool pub, bool prot) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("extern");
	this-> dumpStandard (mod, pub, prot, val);

	val ["from"] = JsonString::init (mod.getFrom ().getStr ());
	val ["space"] = JsonString::init (mod.getSpace ().getStr ());

	std::vector <JsonValue> childs;
	auto ch = this-> dumpUnvalidated (mod.getDeclaration (), pub, prot);
	if (!ch.isEmpty ())
	    childs.push_back (ch);
	
	val ["childs"] = JsonArray::init (childs);
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpCondBlockUnvalidated (const syntax::CondBlock & mod, bool pub, bool prot) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("condition");
	this-> dumpStandard (mod, pub, prot, val);
	if (!mod.getTest ().isEmpty ())
	    val ["test"] = JsonString::init (mod.getTest ().prettyString ());
	
	if (!mod.getElse ().isEmpty ()) {
	    auto ch = this-> dumpUnvalidated (mod.getElse (), pub, prot);
	    if (!ch.isEmpty ())
		val ["else"] = ch;
	}

	
	std::vector <JsonValue> childs;
	for (auto & it : mod.getDeclarations ()) {
	    auto ch = this-> dumpUnvalidated (it, pub, prot);
	    if (!ch.isEmpty ())
		childs.push_back (ch);
	}
	
	val ["childs"] = JsonArray::init (childs);
	return JsonDict::init (val);
    }
    
    JsonValue Visitor::dumpFunction (const semantic::Symbol & fSym) {
	std::map <std::string, JsonValue> val;
	auto & func = fSym.to <semantic::Function> ();
	auto proto = semantic::validator::FunctionVisitor::init (this-> _context).validateFunctionProto (fSym);
	val ["type"] = JsonString::init ("function");
	this-> dumpStandard (func, val);
	
	std::vector <JsonValue> attrs;
	for (auto & it : func.getContent ().getCustomAttributes ()) {
	    attrs.push_back (JsonString::init (it.getStr ()));
	}
	val ["attributes"] = JsonArray::init (attrs);

	std::vector <JsonValue> params;
	for (auto & it : proto.to<generator::FrameProto> ().getParameters ()) {
	    std::map <std::string, JsonValue> param;
	    param ["name"] = JsonString::init (it.to <generator::ProtoVar> ().getLocation ().getStr ());
	    param ["type"] = dumpType (it.to <generator::Value> ().getType ());
	    param ["dmut"] = JsonString::init (it.to<generator::ProtoVar> ().getType ().to<Type> ().isDeeplyMutable () ? "true" : "false");
	    param ["mut"] = JsonString::init (it.to <generator::ProtoVar> ().isMutable ()? "true" : "false");
	    param ["ref"] = JsonString::init (it.to <generator::Value> ().getType ().to <Type> ().isRef () ? "true" : "false");
	    
	    if (!it.to <generator::ProtoVar> ().getValue ().isEmpty ()) {
		param ["value"] = JsonString::init (Ymir::replace (it.to<generator::ProtoVar> ().getValue ().prettyString (), "\\", "\\\\"));
	    }
	    
	    params.push_back (JsonDict::init (param));
	}
	val ["params"] = JsonArray::init (params);
	val ["ret_type"] = dumpType (proto.to <generator::FrameProto> ().getReturnType ());
	
	
	std::vector <JsonValue> throwers;
	for (auto & it : proto.getThrowers ()) {
	    throwers.push_back (dumpType (it));
	}
	
	val ["throwers"] = JsonArray::init (throwers);	
	return JsonDict::init (val);
    }
    
    JsonValue Visitor::dumpFunctionUnvalidated (const syntax::Function & func, bool pub, bool prot) {
	std::map <std::string, JsonValue> val;
	auto decl = func;
	val ["type"] = JsonString::init ("function");
	this-> dumpStandard (func, pub, prot, val);

	std::vector <JsonValue> attrs;
	for (auto & it : decl.getCustomAttributes ()) {
	    attrs.push_back (JsonString::init (it.getStr ()));
	}
	val ["attributes"] = JsonArray::init (attrs);
	
	std::vector <JsonValue> params;
	for (auto & it : decl.getPrototype ().getParameters ()) {
	    std::map <std::string, JsonValue> param;
	    param ["name"] = JsonString::init (it.to <syntax::VarDecl> ().getLocation ().getStr ());
	    param ["type"] = dumpType (it.to <syntax::VarDecl> ().getType ());
	    param ["ref"] = JsonString::init (it.to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::REF) ? "true" : "false");
	    param ["dmut"] = JsonString::init (it.to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::DMUT) ? "true" : "false");
	    param ["mut"] = JsonString::init ((it.to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT)) ? "true" : "false");
	    if (!it.to <syntax::VarDecl> ().getValue ().isEmpty ()) {
		param ["value"] = JsonString::init (Ymir::replace (it.to <syntax::VarDecl> ().getValue ().prettyString (), "\\", "\\\\"));
	    }
	    params.push_back (JsonDict::init (param));
	}
	
	val ["params"] = JsonArray::init (params);
	val ["ret_type"] = dumpType (decl.getPrototype ().getType ());

	std::vector <JsonValue> throwers;
	for (auto & it : decl.getThrowers ()) {
	    throwers.push_back (dumpType (it));
	}
	
	val ["throwers"] = JsonArray::init (throwers);	
	return JsonDict::init (val);	
    }

    
    JsonValue Visitor::dumpVarDecl (const semantic::VarDecl & decl) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("var");
	this-> dumpStandard (decl, val);
	auto gen = decl.getGenerator ();

	val ["mut"] = JsonString::init (gen.to <generator::GlobalVar> ().isMutable () ? "true" : "false");
	val ["var_type"] = dumpType (gen.to <generator::GlobalVar> ().getType ());
		
	if (!gen.to <generator::GlobalVar> ().getValue ().isEmpty ()){
	    val ["value"] = JsonString::init (Ymir::replace (gen.to <generator::GlobalVar> ().getValue ().prettyString (), "\\", "\\\\"));
	}
	
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpGlobalUnvalidated (const syntax::Global & gv, bool pub, bool prot) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("var");
	this-> dumpStandard (gv, pub, prot, val);
	auto & vdecl = gv.getContent ().to <syntax::VarDecl> ();

	val ["dmut"] =  JsonString::init (vdecl.hasDecorator (syntax::Decorator::DMUT) ? "true" : "false");
	val ["mut"] = JsonString::init ((vdecl.hasDecorator (syntax::Decorator::MUT)) ? "true" : "false");
	val ["var_type"] = dumpType (vdecl.getType ());
		
	if (!vdecl.getValue ().isEmpty ()){
	    val ["value"] = JsonString::init (Ymir::replace (vdecl.getValue ().prettyString (), "\\", "\\\\"));
	}

	return JsonDict::init (val);
    }
        
    JsonValue Visitor::dumpAka (const semantic::Aka & al) {
	std::map <std::string, JsonValue> val;
	auto gen = al.getGenerator ();
	val ["type"] = JsonString::init ("aka");
	this-> dumpStandard (al, val);
	
	val ["value"] = JsonString::init (Ymir::replace (gen.prettyString (), "\\", "\\\\"));
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpAkaUnvalidated (const syntax::Aka & al, bool pub, bool prot) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("aka");
	this-> dumpStandard (al, pub, prot, val);
	
	val ["value"] = JsonString::init (Ymir::replace (al.getValue ().prettyString (), "\\", "\\\\"));
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpStruct (const semantic::Struct & str) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("struct");
	this-> dumpStandard (str, val);

	auto gen = str.getGenerator ();
	
	std::vector <JsonValue> attrs;
	if (str.isUnion ()) attrs.push_back (JsonString::init (Keys::UNION));
	if (str.isPacked ()) attrs.push_back (JsonString::init (Keys::PACKED));
	val ["attributes"] = JsonArray::init (attrs);

	std::vector <JsonValue> childs;
	const std::vector <std::string> & field_coms = str.getFieldComments ();
	
	int i = 0;
	for (auto & it : gen.to<generator::Struct> ().getFields ()) {
	    std::map <std::string, JsonValue> field;
	    field ["name"] = JsonString::init (it.to <generator::VarDecl> ().getName ());
	    field ["type"] = dumpType (it.to <generator::VarDecl> ().getVarType ());
	    field ["dmut"] = JsonString::init (it.to <generator::VarDecl> ().getType ().to<generator::Type> ().isDeeplyMutable () ? "true" : "false");
	    field ["mut"] = JsonString::init (it.to <generator::VarDecl> ().isMutable () ? "true" : "false");
	    field ["doc"] = JsonString::init (field_coms [i]);
	    
	    i += 1;
	    
	    if (!it.to <generator::VarDecl> ().getVarValue ().isEmpty ()) {
		field ["value"] = JsonString::init (Ymir::replace (it.to<generator::VarDecl> ().getVarValue ().prettyString (), "\\", "\\\\"));
	    }
	    childs.push_back (JsonDict::init (field));
	}
	
	val ["fields"] = JsonArray::init (childs);
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpStructUnvalidated (const syntax::Struct & str, bool pub, bool prot) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("struct");
	
	this-> dumpStandard (str, pub, prot, val);

	std::vector <JsonValue> attrs;	
	for (auto & it : str.getCustomAttributes ()) {
	    attrs.push_back (JsonString::init (it.getStr ()));
	}
	val ["attributes"] = JsonArray::init (attrs);
	
	std::vector <JsonValue> childs;
	const std::vector <std::string> & field_coms = str.getDeclComments ();
	int i = 0;
	for (auto & it : str.getDeclarations ()) {
	    std::map <std::string, JsonValue> field;
	    field ["name"] = JsonString::init (it.to <syntax::VarDecl> ().getName ().getStr ());
	    field ["type"] = dumpType (it.to <syntax::VarDecl> ().getType ());
	    field ["dmut"] = JsonString::init (it.to<syntax::VarDecl> ().hasDecorator (syntax::Decorator::DMUT) ? "true" : "false");
	    field ["mut"] = JsonString::init ((it.to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT)) ? "true" : "false");
	    field ["doc"] = JsonString::init (field_coms [i]);
	    i += 1;
	    
	    if (!it.to <syntax::VarDecl> ().getValue ().isEmpty ()) {
		field ["value"] = JsonString::init (Ymir::replace (it.to<syntax::VarDecl> ().getValue ().prettyString (), "\\", "\\\\"));
	    }
	    childs.push_back (JsonDict::init (field));
	}
	
	val ["fields"] = JsonArray::init (childs);
	return JsonDict::init (val);
	
    }

    JsonValue Visitor::dumpEnum (const semantic::Enum & en) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("enum");
	this-> dumpStandard (en, val);
	auto gen = en.getGenerator ();
	
	val ["en_type"] = dumpType (gen.to <semantic::generator::Enum> ().getType ());
	
	std::vector <JsonValue> childs;
	const std::vector <std::string> & field_coms = en.getFieldComments ();
	int i = 0;
	
        for (auto & it : gen.to <semantic::generator::Enum> ().getFields ()) {
	    std::map <std::string, JsonValue> param;
	    param ["name"] = JsonString::init (it.to <generator::VarDecl> ().getName ());
	    param ["doc"] = JsonString::init (field_coms [i]);
	    i += 1;
	    
	    if (!it.to <generator::VarDecl> ().getVarValue ().isEmpty ())
	    param ["value"] = JsonString::init (Ymir::replace (it.to<generator::VarDecl> ().getVarValue ().prettyString (), "\\", "\\\\"));
	    childs.push_back (JsonDict::init (param));
        }
	
	val ["fields"] = JsonArray::init (childs);
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpEnumUnvalidated (const syntax::Enum & en, bool pub, bool prot) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("enum");
	this-> dumpStandard (en, pub, prot, val);
	val ["en_type"] = dumpType (en.getType ());

	
	std::vector <JsonValue> childs;
	const std::vector <std::string> & field_coms = en.getFieldComments ();
	int i = 0;
	for (auto & it : en.getValues ()) {
	    std::map <std::string, JsonValue> param;
	    param ["name"] = JsonString::init (it.to <syntax::VarDecl> ().getName ().getStr ());
	    param ["doc"] = JsonString::init (field_coms [i]);
	    i += 1;
	    
	    if (!it.to <syntax::VarDecl> ().getValue ().isEmpty ()) {
		param ["value"] = JsonString::init (Ymir::replace (it.to<syntax::VarDecl> ().getValue ().prettyString (), "\\", "\\\\"));
	    }
	    childs.push_back (JsonDict::init (param));
	}
	
	val ["fields"] = JsonArray::init (childs);
	return JsonDict::init (val);
    }
    
    JsonValue Visitor::dumpClass (const semantic::Class & cl) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("class");
	this-> dumpStandard (cl, val);

	auto gen = cl.getGenerator ();
	
	auto ancestor = gen.to <generator::Class> ().getClassRef ().to <generator::ClassRef> ().getAncestor ();
	if (!ancestor.isEmpty ()) {
	    val ["ancestor"] = dumpType (ancestor);
	}

	if (cl.isAbs ()) val ["abstract"] = JsonString::init ("true");
	else val ["abstract"] = JsonString::init ("false");
	if (cl.isFinal ()) val ["final"] = JsonString::init ("true");
	else val ["final"] = JsonString::init ("false");	
	
	std::vector <JsonValue> fields;
	for (auto & it : gen.to <semantic::generator::Class> ().getLocalFields ()) {
	    std::map <std::string, JsonValue> field;
	    field ["name"] = JsonString::init (it.to <generator::VarDecl> ().getName ());
	    field ["type"] = dumpType (it.to <generator::VarDecl> ().getVarType ());
	    field ["dmut"] = JsonString::init (it.to<generator::VarDecl> ().getType ().to<Type> ().isDeeplyMutable () ? "true" : "false");
	    field ["mut"] = JsonString::init (it.to <generator::VarDecl> ().isMutable () ? "true" : "false");
	    field ["doc"] = JsonString::init (cl.getFieldComments (it.to <generator::VarDecl> ().getName ()));
	    
	    if (cl.isMarkedPrivate (it.to <generator::VarDecl> ().getName ()))
		field ["protection"] = JsonString::init ("prv");
	    else if (cl.isMarkedProtected (it.to <generator::VarDecl> ().getName ()))
		field ["protection"] = JsonString::init ("prot");
	    else field ["protection"] = JsonString::init ("pub");
		
	    if (!it.to <generator::VarDecl> ().getVarValue ().isEmpty ()) {
		field ["value"] = JsonString::init (Ymir::replace (it.to<generator::VarDecl> ().getVarValue ().prettyString (), "\\", "\\\\"));
	    }
	    
	    fields.push_back (JsonDict::init (field));
	}
	
	val ["fields"] = JsonArray::init (fields);
	
	std::vector <JsonValue> asserts;
	auto & assertions = cl.getAssertions ();
	auto & assertComments = cl.getAssertionComments ();
	for (auto it : Ymir::r (0, assertions.size ())) {
	    std::map <std::string, JsonValue> a;
	    a ["test"] = JsonString::init (assertions [it].to <syntax::Assert> ().getTest ().prettyString ());
	    a ["msg"]  = JsonString::init (assertions [it].to <syntax::Assert> ().getMsg ().prettyString ());
	    a ["doc"]  = JsonString::init (assertComments [it]);
					  
	    asserts.push_back (JsonDict::init (a));
	}
	
	val ["asserts"] = JsonArray::init (asserts);
	
	std::vector <JsonValue> cstrs;
	std::vector <JsonValue> methods;
	std::vector <JsonValue> impls;
	std::vector <JsonValue> templates;
	
	for (auto & it : cl.getAllInner ()) { // Dump constructors
	    std::map <std::string, JsonValue> def;
	    if (it.is <semantic::Constructor> ()) {
		auto proto = semantic::validator::FunctionVisitor::init (this-> _context).validateConstructorProto (it);
		def ["type"] = JsonString::init ("cstr");
		this-> dumpStandard (it.to <semantic::Constructor> (), def);
		
		std::vector <JsonValue> params;
					       
		for (auto & it : proto.to <generator::ConstructorProto> ().getParameters ()) {
		    std::map <std::string, JsonValue> param;
		    param ["name"] = JsonString::init (it.to <generator::ProtoVar> ().getLocation ().getStr ());
		    param ["type"] = dumpType (it.to <generator::Value> ().getType ());
		    param ["ref"] = JsonString::init (it.to <generator::ProtoVar> ().getType ().to<Type> ().isRef ()? "true" : "false");
		    param ["dmut"] = JsonString::init (it.to<generator::Value> ().getType ().to<Type> ().isDeeplyMutable () ? "true" : "false");
		    param ["mut"] = JsonString::init (it.to <generator::ProtoVar> ().isMutable ()? "true" : "false");
		    if (!it.to <generator::ProtoVar> ().getValue ().isEmpty ()) {
			param ["value"] = JsonString::init (Ymir::replace (it.to<generator::ProtoVar> ().getValue ().prettyString (), "\\", "\\\\"));
		    }


		    params.push_back (JsonDict::init (param));
		}
		def ["params"] = JsonArray::init (params);
		cstrs.push_back (JsonDict::init (def));
	    } else if (it.is <semantic::Impl> ()) {	    
		def ["type"] = JsonString::init ("impl");
		this-> dumpStandard (it.to <semantic::Impl> (), def);
		def ["trait"] = JsonString::init (this-> _context.validateType (it.to <semantic::Impl> ().getTrait ()).prettyString ());
		std::vector <JsonValue> implMethods;
		for (auto & jt : it.to <semantic::Impl> ().getAllInner ()) {
		    if (jt.is <semantic::Function> ()) {
			auto prot = semantic::generator::Class::MethodProtection::PRV;
			if (jt.isPublic ()) prot = semantic::generator::Class::MethodProtection::PUB;
			if (jt.isProtected ()) prot = semantic::generator::Class::MethodProtection::PROT;

			auto proto = semantic::validator::FunctionVisitor::init (this-> _context).validateMethodProto (jt.to <semantic::Function> (), cl.getGenerator ().to<generator::Class> ().getClassRef (), Generator::empty (), false);
			auto m = this-> dumpMethodProto (proto.to <generator::MethodProto> (), prot, jt.to <semantic::Function> ().isOver ());
			if (!m.isEmpty ()) implMethods.push_back (m);	
		    }
		}
		
		def ["childs"] = JsonArray::init (implMethods);		
		impls.push_back (JsonDict::init (def));
	    } else if (it.is <semantic::Function> ()) {
		auto prot = semantic::generator::Class::MethodProtection::PRV;
		if (it.isPublic ()) prot = semantic::generator::Class::MethodProtection::PUB;
		if (it.isProtected ()) prot = semantic::generator::Class::MethodProtection::PROT;

		auto proto = semantic::validator::FunctionVisitor::init (this-> _context).validateMethodProto (it.to <semantic::Function> (), cl.getGenerator ().to<generator::Class> ().getClassRef (), Generator::empty (), false);
		auto m = this-> dumpMethodProto (proto.to <generator::MethodProto> (), prot, it.to <semantic::Function> ().isOver ());
		if (!m.isEmpty ()) methods.push_back (m);	
	    } else if (it.is<semantic::Template> ()) {
		templates.push_back (this-> dumpTemplate (it.to<semantic::Template> ()));
	    }
	}	

	val ["cstrs"] = JsonArray::init (cstrs);
	val ["impls"] = JsonArray::init (impls);
	val ["methods"] = JsonArray::init (methods);
	val ["templates"] = JsonArray::init (templates);
	return JsonDict::init (val);
    }

    void Visitor::dumpInnerClassUnvalidated (std::vector <JsonValue> & fields, std::vector <JsonValue> & cstrs, std::vector<JsonValue> & methods, std::vector <JsonValue> & others, const  std::vector <syntax::Declaration> & decls, bool prv, bool prot, bool pub) {
	for (auto & jt : decls) {	    
	    if  (jt.is <syntax::ExpressionWrapper> ()) {
		auto wrap = jt.to <syntax::ExpressionWrapper> ();
		if (wrap.getContent ().is <syntax::VarDecl> ()) {
		    auto de = wrap.getContent ().to <syntax::VarDecl> ();		
		    std::map <std::string, JsonValue> param;
		    param ["name"] = JsonString::init (de.getName ().getStr ());
		    param ["type"] = dumpType (de.getType ());
		    param ["dmut"] = JsonString::init (de.hasDecorator (syntax::Decorator::DMUT) ? "true" : "false");
		    param ["mut"] = JsonString::init ((de.hasDecorator (syntax::Decorator::MUT)) ? "true" : "false");
		    param ["doc"] = JsonString::init (wrap.getComments ());
				    	
		    if (prv)
			param ["protection"] = JsonString::init ("prv");
		    else if (prot) {
			param ["protection"] = JsonString::init ("prot");
		    } else
			param ["protection"] = JsonString::init ("pub");
		    fields.push_back (JsonDict::init (param));
		} else if (wrap.getContent ().is <syntax::Set> ()) {
		    for (auto it : wrap.getContent ().to <syntax::Set> ().getContent ()) {
			auto de = it.to <syntax::VarDecl> ();
			std::map <std::string, JsonValue> param;
			param ["name"] = JsonString::init (de.getName ().getStr ());
			param ["type"] = dumpType (de.getType ());
			param ["dmut"] = JsonString::init (de.hasDecorator (syntax::Decorator::DMUT) ? "true" : "false");
			param ["mut"] = JsonString::init ((de.hasDecorator (syntax::Decorator::MUT)) ? "true" : "false");
			param ["doc"] = JsonString::init (wrap.getComments ());
				    	
			if (prv)
			    param ["protection"] = JsonString::init ("prv");
			else if (prot) {
			    param ["protection"] = JsonString::init ("prot");
			} else
			    param ["protection"] = JsonString::init ("pub");
			fields.push_back (JsonDict::init (param));

		    }
		} else if (wrap.getContent ().is <syntax::Assert> ()) {
		    std::map <std::string, JsonValue> a;
		    a ["test"] = JsonString::init (wrap.getContent ().to <syntax::Assert> ().getTest ().prettyString ());
		    a ["msg"]  = JsonString::init (wrap.getContent ().to <syntax::Assert> ().getMsg ().prettyString ());
		    a ["doc"]  = JsonString::init (wrap.getComments ());
		} else {
		    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");			
		}		
	    } else if (jt.is <syntax::DeclBlock> ()) {
		auto dc = jt.to <syntax::DeclBlock> ();
		this-> dumpInnerClassUnvalidated (fields, cstrs, methods, others, dc.getDeclarations (), dc.isPrivate (), dc.isProt (), dc.isPublic ());
	    } else if (jt.is <syntax::Constructor> ()) {
		cstrs.push_back (this-> dumpConstructorUnvalidated (jt.to <syntax::Constructor> (), prv, prot, pub));
	    } else if (jt.is <syntax::Function> ()) {
		methods.push_back (this-> dumpFunctionUnvalidated (jt.to <syntax::Function> (), pub, prot));
	    } else if (jt.is <syntax::CondBlock> ()) {
		auto dc = jt.to <syntax::CondBlock> ();
		std::map <std::string, JsonValue> val;
		std::map <std::string, JsonValue> r_val;
		
		val ["type"] = JsonString::init ("template_condition");
		val ["condition"] = JsonString::init (dc.getTest ().prettyString ());
		this-> dumpStandard (dc, pub, prot, val);
		std::vector <JsonValue> l_fields;
		std::vector <JsonValue> l_cstrs;
		std::vector <JsonValue> l_methods;
		std::vector <JsonValue> l_others;
		this-> dumpInnerClassUnvalidated (l_fields, l_cstrs, l_methods, l_others, dc.getDeclarations (), prv, prot, pub);
		val ["cstrs"] = JsonArray::init (l_cstrs);
		val ["methods"] = JsonArray::init (l_methods);
		val ["fields"] = JsonArray::init (l_fields);
		val ["others"] = JsonArray::init (l_others);

		if (!dc.getElse ().isEmpty ()) {
		    std::vector <JsonValue> r_fields;
		    std::vector <JsonValue> r_cstrs;
		    std::vector <JsonValue> r_methods;
		    std::vector <JsonValue> r_others;
		    
		    this-> dumpInnerClassUnvalidated (r_fields, r_cstrs, r_methods, r_others, {dc.getElse ()}, prv, prot, pub);
		    if (r_others.size () != 0) {
			val ["else"] = r_others [0];
		    } else val ["else"] = JsonDict::init (r_val);
		} else {		
		    val ["else"] = JsonDict::init (r_val);
		}
		others.push_back (JsonDict::init (val));
	    } else if (jt.is <syntax::Mixin> ()) {
		std::map <std::string, JsonValue> val;
		val ["type"] = JsonString::init ("impl");
		this-> dumpStandard (jt.to <syntax::Mixin> (), pub, prot, val);
		val ["trait"] = JsonString::init (jt.to <syntax::Mixin> ().getMixin ().prettyString ());
		others.push_back (JsonDict::init (val));
	    } else {
		others.push_back (this-> dumpUnvalidated (jt, pub, prot));
	    } 
	}	
    }


    JsonValue Visitor::dumpClassUnvalidated (const syntax::Class & s_cl, bool pub, bool prot) {	
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("template_class");
	this-> dumpStandard (s_cl, pub, prot, val);
	
	auto ancestor = s_cl.getAncestor ();
	if (!ancestor.isEmpty ()) {
	    val ["ancestor"] = JsonString::init (ancestor.prettyString ());
	}

	std::vector <JsonValue> fields;
	std::vector <JsonValue> cstrs;
	std::vector <JsonValue> methods;
	std::vector <JsonValue> others;

	this-> dumpInnerClassUnvalidated (fields, cstrs, methods, others, s_cl.getDeclarations (), false, true, false);
	
	val ["methods"] = JsonArray::init (methods);
	val ["fields"] = JsonArray::init (fields);
	val ["cstrs"] = JsonArray::init (cstrs);
	val ["others"] = JsonArray::init (others);
	
	return JsonDict::init (val);
    }


    JsonValue Visitor::dumpMethodProto (const generator::MethodProto & proto, const semantic::generator::Class::MethodProtection & prot, bool isOver) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("method");
	val ["over"] = JsonString::init (isOver ? "true" : "false");
	val ["name"] = JsonString::init (proto.getLocation ().getStr ());
	val ["loc_file"] = JsonString::init (proto.getLocation ().getFilename ());
	val ["loc_line"] = JsonString::init (proto.getLocation ().getLine ());
	val ["loc_col"] = JsonString::init (proto.getLocation ().getColumn ());
	val ["doc"] = JsonString::init (proto.getComments ());
	
	std::vector <JsonValue> params;
	for (auto & it : proto.getParameters ()) {
	    std::map <std::string, JsonValue> param;
	    param ["name"] = JsonString::init (it.to <generator::ProtoVar> ().getLocation ().getStr ());
	    param ["type"] = dumpType (it.to <generator::Value> ().getType ());
	    param ["ref"] = JsonString::init (it.to <generator::ProtoVar> ().getType ().to<Type> ().isRef () ? "true" : "false");
	    param ["dmut"] = JsonString::init (it.to<generator::Value> ().getType ().to<Type> ().isDeeplyMutable () ? "true" : "false");
	    param ["mut"] = JsonString::init (it.to <generator::ProtoVar> ().isMutable ()? "true" : "false");
	    
	    if (!it.to <generator::ProtoVar> ().getValue ().isEmpty ()) {
		param ["value"] = JsonString::init (Ymir::replace (it.to<generator::ProtoVar> ().getValue ().prettyString (), "\\", "\\\\"));
	    }
	    params.push_back (JsonDict::init (param));
	}
	val ["params"] = JsonArray::init (params);
	val ["ret_type"] = dumpType (proto.getReturnType ());
	
	std::vector<JsonValue> attrs;
	if (proto.isEmptyFrame ()) attrs.push_back (JsonString::init ("virtual"));
	if (proto.isFinal ()) attrs.push_back (JsonString::init ("final"));
	if (proto.isMutable ()) attrs.push_back (JsonString::init ("mut"));
	val ["attributes"] = JsonArray::init (attrs);
	
	
	if (prot == semantic::generator::Class::MethodProtection::PRV_PARENT) {
	    val ["protection"] = JsonString::init ("prv_parent");
	} else if (prot == semantic::generator::Class::MethodProtection::PRV) {
	    val ["protection"] = JsonString::init ("prv");
	} else if (prot == semantic::generator::Class::MethodProtection::PROT) {
	    val ["protection"] = JsonString::init ("prot");
	} else {
	    val ["protection"] = JsonString::init ("pub");
	}
	
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpConstructorUnvalidated (const syntax::Constructor & decl, bool prv, bool prot, bool pub) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("cstrs");
	this-> dumpStandard (decl, pub, prot, val);

	std::vector <JsonValue> params;
	for (auto & it : decl.getPrototype ().getParameters ()) {
	    std::map <std::string, JsonValue> param;
	    param ["name"] = JsonString::init (it.to <syntax::VarDecl> ().getLocation ().getStr ());
	    param ["type"] = dumpType (it.to <syntax::VarDecl> ().getType ());
	    param ["ref"] = JsonString::init (it.to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::REF)? "true" : "false");
	    param ["dmut"] = JsonString::init (it.to<syntax::VarDecl> ().hasDecorator (syntax::Decorator::DMUT) ? "true" : "false");
	    param ["mut"] = JsonString::init ((it.to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT)) ? "true" : "false");
	    if (!it.to <syntax::VarDecl> ().getValue ().isEmpty ()) {
		param ["value"] = JsonString::init (Ymir::replace (it.to <syntax::VarDecl> ().getValue ().prettyString (), "\\", "\\\\"));
	    }
	    params.push_back (JsonDict::init (param));
	}

	val ["param"] = JsonArray::init (params);

	std::vector <JsonValue> throwers;
	for (auto & it : decl.getThrowers ()) {
	    throwers.push_back (dumpType (it));
	}

	val ["throwers"] = JsonArray::init (throwers);	
	return JsonDict::init (val);	
    }    

    JsonValue Visitor::dumpTrait (const semantic::Trait & tr) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("trait");
	this-> dumpStandard (tr, val);
	
	std::vector <JsonValue> childs;
	for (auto & it : tr.getAllInner ()) {
	    auto ch = this-> dumpUnvalidated (it);
	    if (!ch.isEmpty ())
		childs.push_back (ch);	    
	}
	
	val ["childs"] = JsonArray::init (childs);
	return JsonDict::init (val);
    }


    JsonValue Visitor::dumpTraitUnvalidated (const syntax::Trait & s_tr, bool pub, bool prot) {
	auto visit = declarator::Visitor::init ();
	visit.setWeak ();
	auto symcl = visit.visitTrait (s_tr);
	auto & tr = symcl.to <semantic::Trait> ();

	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("trait");
	this-> dumpStandard (s_tr, pub, prot, val);
	
	std::vector <JsonValue> childs;
	for (auto & it : tr.getAllInner ()) {
	    auto ch = this-> dumpUnvalidated (it);
	    if (!ch.isEmpty ())
		childs.push_back (ch);	    
	}
	
	val ["childs"] = JsonArray::init (childs);
	return JsonDict::init (val);	
    }
    
    JsonValue Visitor::dumpTemplate (const semantic::Template & tm) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("template");
	this-> dumpStandard (tm, val);
	
	if (!tm.getTest ().isEmpty ()) {
	    val ["test"] = JsonString::init (tm.getTest ().prettyString ());
	}
	
	std::vector <JsonValue> params;
	for (auto & it : tm.getParams ()) {
	    params.push_back (JsonString::init (it.prettyString ()));
	}

	val ["params"] = JsonArray::init (params);

	auto ch = this-> dumpUnvalidated (tm.getDeclaration ());
	if (!ch.isEmpty ())
	    val ["child"] = ch;
	
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpTemplateUnvalidated (const syntax::Template & tm, bool pub, bool prot) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("template");
	this-> dumpStandard (tm, pub, prot, val);
	
	if (!tm.getTest ().isEmpty ()) {
	    val ["test"] = JsonString::init (tm.getTest ().prettyString ());
	}
	
	std::vector <JsonValue> params;
	for (auto & it : tm.getParams ()) {
	    params.push_back (JsonString::init (it.prettyString ()));
	}
	val ["params"] = JsonArray::init (params);
	
	auto ch = this-> dumpUnvalidated (tm.getContent (), pub, prot);
	if (!ch.isEmpty ())
	    val ["child"] = ch;
	
	return JsonDict::init (val);	
    }

    JsonValue Visitor::dumpMacro (const semantic::Macro & m) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("macro");
	this-> dumpStandard (m, val);
	
	std::vector <JsonValue> cstrs;
	std::vector <JsonValue> rules;
	for (auto & it : m.getAllInner ()) {
	    if (it.is <semantic::MacroConstructor> ()) {
		auto & mc = it.to <semantic::MacroConstructor> ().getContent ().to<syntax::MacroConstructor> ();
		std::map <std::string, JsonValue> content;
		this-> dumpStandard (it.to <semantic::MacroConstructor> (), content);
		content ["rule"] = JsonString::init (mc.getRule ().prettyString ());
		std::vector <JsonValue> skips;
		for (auto & jt : mc.getSkips ()) {
		    skips.push_back (JsonString::init (jt.prettyString ()));
		}
		
		content ["skips"] = JsonArray::init (skips);
		cstrs.push_back (JsonDict::init (content));
	    } else if (it.is <semantic::MacroRule> ()) {
		auto & mc = it.to <semantic::MacroRule> ().getContent ().to<syntax::MacroRule> ();
		std::map <std::string, JsonValue> content;
		this-> dumpStandard (it.to <semantic::MacroRule> (), content);
		content ["rule"] = JsonString::init (mc.getRule ().prettyString ());
		std::vector <JsonValue> skips;
		for (auto & jt : mc.getSkips ()) {
		    skips.push_back (JsonString::init (jt.prettyString ()));
		}
		
		content ["skips"] = JsonArray::init (skips);
		rules.push_back (JsonDict::init (content));
	    }
	}

	val ["cstrs"] = JsonArray::init (cstrs);
	val ["rules"] = JsonArray::init (rules);
	
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpMacroUnvalidated (const syntax::Macro & s_m, bool pub, bool prot) {
	auto visit = declarator::Visitor::init ();
	visit.setWeak ();
	auto symcl = visit.visitMacro (s_m);
	auto & m = symcl.to <semantic::Macro> ();
	
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("macro");
	this-> dumpStandard (s_m, pub, prot, val);
	
	std::vector <JsonValue> cstrs;
	std::vector <JsonValue> rules;
	for (auto & it : m.getAllInner ()) {
	    if (it.is <semantic::MacroConstructor> ()) {
		auto & mc = it.to <semantic::MacroConstructor> ().getContent ().to<syntax::MacroConstructor> ();
		std::map <std::string, JsonValue> content;
		this-> dumpStandard (it.to <semantic::MacroConstructor> (), content);
		content ["rule"] = JsonString::init (mc.getRule ().prettyString ());
		std::vector <JsonValue> skips;
		for (auto & jt : mc.getSkips ()) {
		    skips.push_back (JsonString::init (jt.prettyString ()));
		}
			
		content ["rule"] = JsonArray::init (skips);
		cstrs.push_back (JsonDict::init (content));
	    } else if (it.is <semantic::MacroRule> ()) {		
		auto & mc = it.to <semantic::MacroRule> ().getContent ().to<syntax::MacroRule> ();
		std::map <std::string, JsonValue> content;
		this-> dumpStandard (it.to <semantic::MacroRule> (), content);
		content ["rule"] = JsonString::init (mc.getRule ().prettyString ());
		std::vector <JsonValue> skips;
		for (auto & jt : mc.getSkips ()) {
		    skips.push_back (JsonString::init (jt.prettyString ()));
		}
		
		content ["rule"] = JsonArray::init (skips);
		rules.push_back (JsonDict::init (content));
	    }
	}

	val ["cstrs"] = JsonArray::init (cstrs);
	val ["rules"] = JsonArray::init (rules);
	
	return JsonDict::init (val);
    }

    JsonValue Visitor::dumpType (const generator::Generator & type, bool forceMut) {	
	std::map <std::string, JsonValue> val;
	if (type.is <generator::Type> ()) {
	    if (!type.to <Type> ().getProxy ().isEmpty ()) {
		return dumpType (type.to <Type> ().getProxy (), type.to <generator::Type> ().isMutable ());
	    }
	    val ["mut"] = JsonString::init (forceMut || type.to <generator::Type> ().isMutable () ? "true" : "false");	    
	} else val ["mut"] = JsonString::init ("false");
	match (type) {
	    of (Integer, i) {
		val ["type"] = JsonString::init ("int");
		val ["name"] = JsonString::init (i.computeTypeName (false));
	    }
	    elof (Void, v) {
		val ["type"] = JsonString::init ("void");
		val ["name"] = JsonString::init (v.computeTypeName (false));
	    }	    
	    elof (Bool, b) {
		val ["type"] = JsonString::init ("bool");
		val ["name"] = JsonString::init (b.computeTypeName (false));
	    }
	    elof (Float, f) {
		val ["type"] = JsonString::init ("float");
		val ["name"] = JsonString::init (f.computeTypeName (false));
	    }
	    elof (Char, c) {
		val ["type"] = JsonString::init ("char");
		val ["name"] = JsonString::init (c.computeTypeName (false));
	    }
	    elof (Array, ar) {
		val ["type"] = JsonString::init ("array");
		val ["size"] = JsonString::init (Ymir::format ("%", ar.getSize ()));
		val ["childs"] = JsonArray::init ({dumpType (ar.getInners ()[0])});
	    }
	    elof (Slice, sl) {
		val ["type"] = JsonString::init ("slice");
		val ["childs"] = JsonArray::init ({dumpType (sl.getInners ()[0])});
	    }
	    elof (Tuple, tl) {
		val ["type"] = JsonString::init ("tuple");
		std::vector <JsonValue> childs;
		for (auto & it : tl.getInners ()) {
		    childs.push_back (dumpType (it));
		}
		val ["childs"] = JsonArray::init (childs);
	    }
	    elof (StructRef, r) {
		val ["type"] = JsonString::init ("struct");
		val ["name"] = JsonString::init (r.computeTypeName (false));
	    }
	    elof (EnumRef, e) {
		val ["type"] = JsonString::init ("enum");
		val ["name"] = JsonString::init (e.computeTypeName (false));
	    }
	    elof (ClassRef, c) {
		val ["type"] = JsonString::init ("class");
		val ["name"] = JsonString::init (c.computeTypeName (false));
	    }
	    elof (Pointer, p) {
		val ["type"] = JsonString::init ("pointer");
		val ["childs"] = JsonArray::init ({dumpType (p.getInners ()[0])});
	    }
	    elof (ClassPtr, p) {
		val ["type"] = JsonString::init ("class_pointer");
		val ["childs"] = JsonArray::init ({dumpType (p.getInners ()[0])});
	    }
	    elof (Range, r) {
		val ["type"] = JsonString::init ("range");
		val ["childs"] = JsonArray::init ({dumpType (r.getInners ()[0])});
	    }
	    elof (FuncPtr, fn) {
		val ["type"] = JsonString::init ("fn_pointer");
		std::vector <JsonValue> childs;
		for (auto & it : fn.getParamTypes ()) {
		    childs.push_back (dumpType (it));
		}
		val ["childs"] = JsonArray::init (childs);
		val ["ret_type"] = dumpType (fn.getReturnType ());
	    }
	    elof (Closure, c) {
		val ["type"] = JsonString::init ("closure");
		std::vector <JsonValue> childs;
		for (auto & it : c.getInners ()) {
		    childs.push_back (dumpType (it));
		}
		val ["childs"] = JsonArray::init (childs);
	    }
	    elof (Delegate, d) {
		val ["type"] = JsonString::init ("dg_pointer");
		std::vector <JsonValue> childs;
		if (d.getInners () [0].is <FrameProto> ()) {
		    std::map <std::string, JsonValue> val2;
		    auto & proto = d.getInners ()[0];
		    auto params = proto.to <FrameProto> ().getParameters ();
		    auto ret = proto.to <FrameProto> ().getReturnType ();
		    std::vector <Generator> paramTypes;
		    for (auto & it : params) {
			childs.push_back (dumpType (it.to <generator::ProtoVar> ().getType ()));
		    }
		    val2 ["type"] = JsonString::init ("fn_pointer");
		    val2 ["childs"] = JsonArray::init (childs);
		    val2 ["ret_type"] = dumpType (ret);
		    val ["childs"] = JsonArray::init ({JsonDict::init (val2)});
		} else {
		    val ["childs"] = JsonArray::init ({dumpType (d.getInners () [0])});
		}
	    }
	    elof (Option, o) {
		val ["type"] = JsonString::init ("option");
		val ["childs"] = JsonArray::init ({dumpType (o.getInners ()[0])});
	    } elfo {
		val ["type"] = JsonString::init ("unknown");
		val ["name"] = JsonString::init (type.prettyString ());		    
	    }
	}	
	return JsonDict::init (val);	
    }

    JsonValue Visitor::dumpType (const syntax::Expression & type) {
	std::map <std::string, JsonValue> val;
	val ["type"] = JsonString::init ("unknown");
	val ["name"] = JsonString::init (type.prettyString ());
	return JsonDict::init (val);	
    }

    
    
    
}

