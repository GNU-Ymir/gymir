#include <ymir/documentation/Visitor.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/string.hh>
#include <algorithm>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/syntax/declaration/_.hh>

namespace documentation {

    using namespace semantic;
    
    Visitor::Visitor (validator::Visitor & context):
	_context (context)
    {}

    Visitor Visitor::init (validator::Visitor & context) {
	return Visitor (context);
    }

    std::string Visitor::dump (const semantic::Symbol & sym) {
	Ymir::OutBuffer buf;
	if (!sym.isWeak ()) {
	    match (sym) {
		of (semantic::Module, mod, {
			this-> _context.pushReferent (sym, "dump::module");
			buf.write (this-> dumpModule (mod));
			this-> _context.popReferent ("dump::module");
		    }
		    )
		else of (semantic::Function, func, {
			this-> _context.pushReferent (sym, "dump::function");
			buf.write (this-> dumpFunction (func));
			this-> _context.popReferent ("dump::function");
		    }
		    )
		else of (semantic::VarDecl, decl, {
			this-> _context.pushReferent (sym, "dump::vardecl");
			buf.write (this-> dumpVarDecl (decl));
			this-> _context.popReferent ("dump::vardecl");			    
		    }
		    )
	        else of (semantic::Alias, al, {
		       this-> _context.pushReferent (sym, "dump::alias");
		       buf.write (this-> dumpAlias (al));
		       this-> _context.popReferent ("dump::alias");
		   }
		   )
		else of (semantic::Struct, str, {
			this-> _context.pushReferent (sym, "dump::struct");
			buf.write (this-> dumpStruct (str));
			this-> _context.popReferent ("dump::struct");
		    }
		    )
		else of (semantic::TemplateSolution, sol ATTRIBUTE_UNUSED, {})
		else of (semantic::Enum, en, {
			this-> _context.pushReferent (sym, "dump::enum");
			buf.write (this-> dumpEnum (en));
			this-> _context.popReferent ("dump::enum");
		    }
		    )
		else of (semantic::Class, cl, {
			this-> _context.pushReferent (sym, "dump::class");
			buf.write (this-> dumpClass (cl));
			this-> _context.popReferent ("dump::class");
		    }
		    )
	        else of (semantic::Trait, tr, {
			this-> _context.pushReferent (sym, "dump::trait");
			buf.write (this-> dumpTrait (tr));
			this-> _context.popReferent ("dump::trait");
		    }
		    )
		else of (semantic::Template, tm, {
			this-> _context.pushReferent (sym, "dump::template");
			buf.write (this-> dumpTemplate (tm));
			this-> _context.popReferent ("dump::template");
		    }
		    )
		else of (semantic::Macro, x, {
			this-> _context.pushReferent (sym, "dump::macro");
			buf.write (this-> dumpMacro (x));
			this-> _context.popReferent ("dump::macro");
		    }
		    )
		else of (semantic::ModRef, x ATTRIBUTE_UNUSED, {})
		else {
		    // Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
		}
	    }
	}
	return buf.str ();
    }

    std::string Visitor::dumpUnvalidated (const semantic::Symbol & sym) {
	match (sym) {
	    of (semantic::Function, fn, {
		    return this-> dumpFunctionUnvalidated (fn.getContent (), sym.isPublic (), false);
		}
		);	    
	}
	Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	return "";
    }

    std::string Visitor::dumpUnvalidated (const syntax::Declaration & decl, bool pub, bool prot) {
	match (decl) {
	    of (syntax::DeclBlock, bl, {
		    return this-> dumpDeclBlockUnvalidated (bl, pub, prot);
		}
		);
	    of (syntax::CondBlock, cd, {
		    return this-> dumpCondBlockUnvalidated (cd, pub, prot);
		}
		);
	    of (syntax::Class, cl, {
		    return this-> dumpClassUnvalidated (cl, pub, prot);
		}
		);
	    of (syntax::Enum, en, {
		    return this-> dumpEnumUnvalidated (en, pub, prot);
		}
		);
	    of (syntax::ExternBlock, ex, {
		    return this-> dumpExternBlockUnvalidated (ex, pub, prot);
		}
		);
	    of (syntax::Constructor, cst ATTRIBUTE_UNUSED, {return "";});
	    of (syntax::Function, fn, {
		    return this-> dumpFunctionUnvalidated (fn, pub, prot);
		}
		);
	    of (syntax::Global, gl, {
		    return this-> dumpGlobalUnvalidated (gl, pub, prot);
		}
		);
	    of (syntax::Import, imp ATTRIBUTE_UNUSED, {return "";});
	    of (syntax::Mixin, mx ATTRIBUTE_UNUSED, {return"";});
	    of (syntax::Module, mod, {
		    return this-> dumpModuleUnvalidated (mod, pub, prot);
		}
		);
	    of (syntax::Struct, str, {
		    return this-> dumpStructUnvalidated (str, pub, prot);
		}
		);
	    of (syntax::Template, tmp, {
		    return this-> dumpTemplateUnvalidated (tmp, pub, prot);
		}
		);
	    of (syntax::Trait, tr, {
		    return this-> dumpTraitUnvalidated (tr, pub, prot);
		}
		);
	    of (syntax::Use, u ATTRIBUTE_UNUSED, {return "";});
	    
	}
	Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	return "";
    }

    std::string Visitor::dumpModule (const semantic::Module & mod) {
	Ymir::OutBuffer buf;
	if (!mod.isExtern ()) {
	    buf.writeln ("{");
	    buf.writeln ("\t\"type\" : \"module\",");
	    buf.writefln ("\t\"name\" : \"%\",", mod.getRealName ());
	    buf.writefln ("\t\"loc_file\" : \"%\",", mod.getName ().locFile);
	    buf.writefln ("\t\"loc_line\" : %,", mod.getName ().line);
	    buf.writefln ("\t\"loc_col\" : %,", mod.getName ().column);
	    buf.writefln ("\t\"doc\" : \"%\",", mod.getComments ());
	    if (mod.isPublic ()) buf.writeln ("\t\"protection\" : \"public\",");
	    else buf.writeln ("\t\"protection\" : \"private\",");

	    buf.writeln ("\t\"childs\" : [");
	    int i = 0;
	    for (auto & it : mod.getAllLocal ()) {
		if (i != 0) buf.writeln ("\t,");
		auto text = Ymir::entab (this-> dump (it), "\t");
		if (text != "") {
		    buf.writeln (text);
		    i += 1;
		}
	    }
	    buf.writeln ("\t]");
	    buf.writeln ("}");
	}
	return buf.str ();
    }    

    std::string Visitor::dumpModuleUnvalidated (const syntax::Module & mod, bool pub, bool prot) {
	Ymir::OutBuffer buf;
	buf.writeln ("{");
	buf.writeln ("\t\"type\" : \"module\",");
	buf.writefln ("\t\"name\" : \"%\",", mod.getLocation ().str);
	buf.writefln ("\t\"loc_file\" : \"%\",", mod.getLocation ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", mod.getLocation ().line);
	buf.writefln ("\t\"loc_col\" : %,", mod.getLocation ().column);
	buf.writefln ("\t\"doc\" : \"%\",", mod.getComments ());
	if (pub) buf.writeln ("\t\"protection\" : \"public\",");
	else if (prot) buf.writeln ("\t\"protection\" : \"prot\",");
	else buf.writeln ("\t\"protection\" : \"private\",");
	
	buf.writeln ("\t\"childs\" : [");
	int i = 0;
	for (auto & it : mod.getDeclarations ()) {
	    if (i != 0) buf.writeln ("\t,");
	    auto text = Ymir::entab (this-> dumpUnvalidated (it, false, false), "\t");
	    if (text != "") {
		buf.writeln (text);
		i += 1;
	    }
	}
	buf.writeln ("\t]");
	buf.writeln ("}");	
	return buf.str ();
    }

    std::string Visitor::dumpDeclBlockUnvalidated (const syntax::DeclBlock & dl, bool, bool) {
	Ymir::OutBuffer buf;
	for (auto & it : dl.getDeclarations ()) {
	    auto text = this-> dumpUnvalidated (it, dl.isPublic (), dl.isProt ());
	    if (text != "") {
		buf.writefln ("%,", text);
	    }
	}
	return buf.str ();
    }


    std::string Visitor::dumpExternBlockUnvalidated (const syntax::ExternBlock & mod, bool pub, bool prot) {
	Ymir::OutBuffer buf;
	buf.writeln ("{");
	buf.writeln ("\t\"type\" : \"extern\",");
	buf.writefln ("\t\"name\" : \"%\",", mod.getLocation ().str);
	buf.writefln ("\t\"loc_file\" : \"%\",", mod.getLocation ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", mod.getLocation ().line);
	buf.writefln ("\t\"loc_col\" : %,", mod.getLocation ().column);
	buf.writefln ("\t\"doc\" : \"%\",", mod.getComments ());
	if (pub) buf.writeln ("\t\"protection\" : \"public\",");
	else if (prot) buf.writeln ("\t\"protection\" : \"prot\",");
	else buf.writeln ("\t\"protection\" : \"private\",");

	buf.writeln ("\t\"from\" : \"%\",", mod.getFrom ().str);
	buf.writeln ("\t\"space\" : \"%\",", mod.getSpace ().str);
	
	buf.writeln ("\t\"childs\" : [");
	auto text = Ymir::entab (this-> dumpUnvalidated (mod.getDeclaration (), false, false), "\t");
	if (text != "") {
	    buf.writeln (text);
	}
	
	buf.writeln ("\t]");
	buf.writeln ("}");	
	return buf.str ();
    }

    std::string Visitor::dumpCondBlockUnvalidated (const syntax::CondBlock & mod, bool pub, bool prot) {
	Ymir::OutBuffer buf;
	buf.writeln ("{");
	buf.writeln ("\t\"type\" : \"extern\",");
	buf.writefln ("\t\"name\" : \"%\",", mod.getLocation ().str);
	buf.writefln ("\t\"loc_file\" : \"%\",", mod.getLocation ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", mod.getLocation ().line);
	buf.writefln ("\t\"loc_col\" : %,", mod.getLocation ().column);
	buf.writefln ("\t\"doc\" : \"%\",", mod.getComments ());
	if (pub) buf.writeln ("\t\"protection\" : \"public\",");
	else if (prot) buf.writeln ("\t\"protection\" : \"prot\",");
	else buf.writeln ("\t\"protection\" : \"private\",");
	buf.writefln ("\t\"test\" : \"%\",", mod.getTest ().prettyString ());
	if (!mod.getElse ().isEmpty ()) {
	    auto text = this-> dumpUnvalidated (mod.getElse (), pub, prot);
	    if (text != "")
		buf.writefln ("\t\"else\" : {%\n\t},", text);
	}
	
	buf.writeln ("\t\"childs\" : [");
	int i = 0;
	for (auto & it : mod.getDeclarations ()) {
	    if (i != 0) buf.writeln ("\t,");
	    auto text = Ymir::entab (this-> dumpUnvalidated (it, false, false), "\t");
	    if (text != "") {
		buf.writeln (text);
		i += 1;
	    }
	}
	buf.writeln ("\t]");
	buf.writeln ("}");
	return buf.str ();
    }
    
    std::string Visitor::dumpFunction (const semantic::Function & func) {
	Ymir::OutBuffer buf;
	auto proto = this-> _context.validateFunctionProto (func);
	buf.writeln ("{");
	buf.writeln ("\t\"type\" : \"function\",");	
	buf.writefln ("\t\"name\" : \"%\",", func.getRealName ());
	buf.writefln ("\t\"loc_file\" : \"%\",", func.getContent ().getLocation ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", func.getContent ().getLocation ().line);
	buf.writefln ("\t\"loc_col\" : %,", func.getContent ().getLocation ().column);
	buf.writefln ("\t\"doc\" : \"%\",", func.getComments ());
	if (func.isPublic ()) buf.writeln ("\t\"protection\" : \"public\",");
	else buf.writeln ("\t\"protection\" : \"private\",");

	buf.writeln ("\t\"attributes\" : [");
	int i = 0;
	for (auto & it : func.getContent ().getCustomAttributes ()) {
	    if (i != 0) buf.write (",");
	    buf.writef ("\"%\"", it.str);
	    i += 1;
	}
	
	buf.writeln ("],\n\t\"params\" : [");
	i = 0;
	for (auto & it : proto.to<generator::FrameProto> ().getParameters ()) {
	    if (i == 0) buf.writeln ("\t{");
	    else buf.writeln ("\t,{");
	    buf.writefln ("\t\t\"name\" : \"%\",", it.to <generator::ProtoVar> ().getLocation ().str);
	    buf.writefln ("\t\t\"type\" : \"%\",", it.to <generator::Value> ().getType ().prettyString ());
	    buf.writefln ("\t\t\"mut\" : \"%\",", it.to <generator::ProtoVar> ().isMutable ()? "true" : "false");
	    if (!it.to <generator::ProtoVar> ().getValue ().isEmpty ())
		buf.writefln ("\t\t\"value\" : \"%\"", it.to<generator::ProtoVar> ().getValue ().prettyString ());
	    buf.writeln ("\t}");
	    i += 1;
	}	
	buf.writefln ("\t],\n\t\"ret_type\" : \"%\",", proto.to <generator::FrameProto> ().getReturnType ().prettyString ());
	buf.writeln ("\t\"throwers\" : [");
	i = 0;
	for (auto & it : proto.getThrowers ()) {
	    if (i != 0) buf.write (",");
	    buf.writef ("\"%\"", it.prettyString ());
	    i += 1;
	}
	buf.writeln ("\t]");	
	buf.writeln ("}");
	return buf.str ();
    }

    std::string Visitor::dumpFunctionUnvalidated (const syntax::Function & func, bool pub, bool prot) {
	Ymir::OutBuffer buf;
	auto decl = func;
	buf.writeln ("{");
	buf.writeln ("\t\"type\" : \"function\",");	
	buf.writefln ("\t\"name\" : \"%\",", func.getLocation ().str);
	buf.writefln ("\t\"loc_file\" : \"%\",", func.getLocation ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", func.getLocation ().line);
	buf.writefln ("\t\"loc_col\" : %,", func.getLocation ().column);
	buf.writefln ("\t\"doc\" : \"%\",", func.getComments ());
	if (pub) buf.writeln ("\t\"protection\" : \"public\",");
	else if (prot) buf.writeln ("\t\"protection\" : \"prot\",");
	else buf.writeln ("\t\"protection\" : \"private\",");
	
	buf.writeln ("\t\"params\" : [");
	int i = 0;
	for (auto & it : decl.getPrototype ().getParameters ()) {
	    if (i == 0) buf.writeln ("\t{");
	    else buf.writeln ("\t,{");
	    buf.writefln ("\t\t\"name\" : \"%\",", it.to <syntax::VarDecl> ().getLocation ().str);
	    buf.writefln ("\t\t\"type\" : \"%\",", it.to <syntax::VarDecl> ().getType ().prettyString ());
	    buf.writefln ("\t\t\"mut\" : \"%\",", it.to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT)? "true" : "false");
	    buf.writeln ("\t}");
	    i += 1;
	}	
	buf.writefln ("\t],\n\t\"ret_type\" : \"%\"," ,decl.getPrototype ().getType ().prettyString ());
	auto attrs = decl.getCustomAttributes ();
	buf.writeln ("\t\"throwers\" : [");
	i = 0;
	for (auto & it : decl.getThrowers ()) {
	    if (i != 0) buf.write (",");
	    buf.writef ("\"%\"", it.prettyString ());
	    i += 1;
	}
	buf.writeln ("\t]");
	buf.writeln ("}");
	return buf.str ();
    }
    
    std::string Visitor::dumpVarDecl (const semantic::VarDecl & decl) {
	Ymir::OutBuffer buf;
	auto gen = decl.getGenerator ();
	buf.writeln ("{");
	buf.writeln ("\"type\" : \"var\",");
	buf.writefln ("\"name\" : \"%\",", gen.to <generator::GlobalVar> ().getLocation ().str);
	buf.writefln ("\"mut\" : \"%\",", gen.to <generator::GlobalVar> ().isMutable () ? "true" : "false");
	buf.writefln ("\"type\" : \"%\",", gen.to <generator::GlobalVar> ().getType ().prettyString ());
	buf.writefln ("\t\"loc_file\" : \"%\",", gen.getLocation ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", gen.getLocation ().line);
	buf.writefln ("\t\"loc_col\" : %,", gen.getLocation ().column);
	buf.writef ("\t\"doc\" : \"%\"", decl.getComments ());
	if (decl.isPublic ()) buf.writeln ("\t\"protection\" : \"public\",");
	else buf.writeln ("\t\"protection\" : \"private\",");
	
	if (!gen.to <generator::GlobalVar> ().getValue ().isEmpty ()){
	    buf.writef (",\n\t\"value\" : \"%\"", gen.to <generator::GlobalVar> ().getValue ().prettyString ());
	}
	buf.writeln ("\n}");
	return buf.str ();
    }

    std::string Visitor::dumpGlobalUnvalidated (const syntax::Global & gv, bool pub, bool prot) {
	Ymir::OutBuffer buf;
	auto vdecl = gv.getContent ().to <syntax::VarDecl> ();
	buf.writeln ("{");
	buf.writeln ("\"type\" : \"var\",");
	buf.writefln ("\"name\" : \"%\",", gv.getLocation ().str);
	buf.writefln ("\"mut\" : \"%\",", vdecl.hasDecorator (syntax::Decorator::MUT)? "true" : "false");
	buf.writefln ("\"type\" : \"%\",", vdecl.getType ().prettyString ());
	buf.writefln ("\t\"loc_file\" : \"%\",", gv.getLocation ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", gv.getLocation ().line);
	buf.writefln ("\t\"loc_col\" : %,", gv.getLocation ().column);
	buf.writef ("\t\"doc\" : \"%\"", gv.getComments ());
	if (pub) buf.writeln ("\t\"protection\" : \"public\",");
	else if (prot) buf.writeln ("\t\"protection\" : \"prot\",");
	else buf.writeln ("\t\"protection\" : \"private\",");
	
	if (!vdecl.getValue ().isEmpty ()){
	    buf.writef (",\n\t\"value\" : \"%\"", vdecl.getValue ().prettyString ());
	}
	buf.writeln ("\n}");
	return buf.str ();
    }
        
    std::string Visitor::dumpAlias (const semantic::Alias & al) {
	Ymir::OutBuffer buf;
	auto gen = al.getGenerator ();
	buf.writeln ("{");
	buf.writeln ("\"type\" : \"alias\",");
	buf.writefln ("\"name\" : \"%\",", al.getRealName ());
	buf.writefln ("\t\"loc_file\" : \"%\",", al.getName ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", al.getName ().line);
	buf.writefln ("\t\"loc_col\" : %,", al.getName ().column);
	buf.writefln ("\t\"doc\" : \"%\",", al.getComments ());
	buf.writefln ("\t\"value\" : \"%\"", gen.prettyString ());
	if (al.isPublic ()) buf.writeln ("\t\"protection\" : \"public\",");
	else buf.writeln ("\t\"protection\" : \"private\",");
	
	buf.writeln ("}");
	return buf.str ();
    }

    std::string Visitor::dumpStruct (const semantic::Struct & str) {
	Ymir::OutBuffer buf;
	auto gen = str.getGenerator ();
	buf.writeln ("{");
	buf.writeln ("\"type\" : \"struct\",");
	buf.writefln ("\"name\" : \"%\",", str.getRealName ());
	buf.writefln ("\t\"loc_file\" : \"%\",", str.getName ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", str.getName ().line);
	buf.writefln ("\t\"loc_col\" : %,", str.getName ().column);
	buf.writefln ("\t\"doc\" : \"%\",", str.getComments ());
	if (str.isPublic ()) buf.writeln ("\t\"protection\" : \"public\",");
	else buf.writeln ("\t\"protection\" : \"private\",");
		
	buf.writeln ("\t\"childs\" : [");
	int i = 0;
	for (auto & it : gen.to<generator::Struct> ().getFields ()) {
	    if (i == 0) buf.writeln ("\t{");
	    else buf.writeln ("\t,{");
	    buf.writefln ("\t\t\"name\" : \"%\",", it.to <generator::VarDecl> ().getName ());
	    buf.writefln ("\t\t\"type\" : \"%\",", it.to <generator::VarDecl> ().getVarType ().prettyString ());
	    buf.writefln ("\t\t\"mut\" : \"%\"", it.to <generator::VarDecl> ().isMutable () ? "true" : "false");    
	    if (!it.to <generator::VarDecl> ().getVarValue ().isEmpty ())
		buf.writefln ("\t\t,\"value\" : \"%\"", it.to<generator::VarDecl> ().getVarValue ().prettyString ());
	    buf.writeln ("\t}");
	    i += 1;
	}
	buf.writeln ("\t]\n}");
	return buf.str ();
    }

    std::string Visitor::dumpStructUnvalidated (const syntax::Struct & str, bool pub, bool prot) {
	Ymir::OutBuffer buf;
	buf.writeln ("{");
	buf.writeln ("\"type\" : \"struct\",");
	buf.writefln ("\"name\" : \"%\",", str.getLocation ().str);
	buf.writefln ("\t\"loc_file\" : \"%\",", str.getLocation ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", str.getLocation ().line);
	buf.writefln ("\t\"loc_col\" : %,", str.getLocation ().column);
	buf.writefln ("\t\"doc\" : \"%\",", str.getComments ());
	if (pub) buf.writeln ("\t\"protection\" : \"public\",");
	else if (prot) buf.writeln ("\t\"protection\" : \"prot\",");
	else buf.writeln ("\t\"protection\" : \"private\",");

	buf.writeln ("\t\"attributes\" : [");
	int i = 0;
	for (auto & it : str.getCustomAttributes ()) {
	    if (i != 0) buf.write (",");
	    buf.writef ("\"%\"", it.str);
	    i += 1;
	}
	
	buf.writeln ("],\n\t\"childs\" : [");
	i = 0;
	for (auto & it : str.getDeclarations ()) {
	    if (i == 0) buf.writeln ("\t{");
	    else buf.writeln ("\t,{");
	    buf.writefln ("\t\t\"name\" : \"%\",", it.to <syntax::VarDecl> ().getName ());
	    buf.writefln ("\t\t\"type\" : \"%\",", it.to <syntax::VarDecl> ().getType ().prettyString ());
	    buf.writefln ("\t\t\"mut\" : \"%\"", it.to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT) ? "true" : "false");    
	    if (!it.to <syntax::VarDecl> ().getValue ().isEmpty ())
		buf.writefln ("\t\t,\"value\" : \"%\"", it.to<syntax::VarDecl> ().getValue ().prettyString ());
	    buf.writeln ("\t}");
	    i += 1;
	}
	buf.writeln ("\t]\n}");
	return buf.str ();
	
    }

    std::string Visitor::dumpEnum (const semantic::Enum & en) {
	Ymir::OutBuffer buf;
	auto gen = en.getGenerator ();
	buf.writeln ("{");
	buf.writeln ("\"type\" : \"enum\",");
	buf.writefln ("\"name\" : \"%\",", en.getRealName ());
	buf.writefln ("\t\"loc_file\" : \"%\",", en.getName ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", en.getName ().line);
	buf.writefln ("\t\"loc_col\" : %,", en.getName ().column);
	buf.writefln ("\t\"doc\" : \"%\",", en.getComments ());
	if (en.isPublic ()) buf.writeln ("\t\"protection\" : \"public\",");
	else buf.writeln ("\t\"protection\" : \"private\",");
	
	buf.writefln ("\t\"type\" : \"%\",", gen.to <semantic::generator::Enum> ().getType ().prettyString ());

	buf.writeln ("\t\"childs\" : [");
	int i = 0;
        for (auto & it : gen.to <semantic::generator::Enum> ().getFields ()) {
	    if (i == 0) buf.writeln ("\t{");
	    else buf.writeln ("\t,{");
	    buf.writefln ("\t\t\"name\" : \"%\"", it.to <generator::VarDecl> ().getName ());
	    if (!it.to <generator::VarDecl> ().getVarValue ().isEmpty ())
		buf.writefln ("\t\t\"value\" : \"%\"", it.to<generator::VarDecl> ().getVarValue ().prettyString ());
	    buf.writeln ("\t}");
	    i += 1;
        }
	buf.writeln ("\t]\n}");
	return buf.str ();
    }

    std::string Visitor::dumpEnumUnvalidated (const syntax::Enum & en, bool pub, bool prot) {
	Ymir::OutBuffer buf;
	buf.writeln ("{");
	buf.writeln ("\"type\" : \"enum\",");
	buf.writefln ("\"name\" : \"%\",", en.getLocation ().str);
	buf.writefln ("\t\"loc_file\" : \"%\",", en.getLocation ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", en.getLocation ().line);
	buf.writefln ("\t\"loc_col\" : %,", en.getLocation ().column);
	buf.writefln ("\t\"doc\" : \"%\",", en.getComments ());
	if (pub) buf.writeln ("\t\"protection\" : \"public\",");
	else if (prot) buf.writeln ("\t\"protection\" : \"prot\",");
	else buf.writeln ("\t\"protection\" : \"private\",");

	buf.writefln ("\t\"type\" : \"%\",", en.getType ().prettyString ());
	
	buf.writeln ("\t\"childs\" : [");
	int i = 0;
	for (auto & it : en.getValues ()) {
	    if (i == 0) buf.writeln ("\t{");
	    else buf.writeln ("\t,{");
	    buf.writefln ("\t\t\"name\" : \"%\",", it.to <syntax::VarDecl> ().getName ());
	    if (!it.to <syntax::VarDecl> ().getValue ().isEmpty ())
		buf.writefln ("\t\t,\"value\" : \"%\"", it.to<syntax::VarDecl> ().getValue ().prettyString ());
	    buf.writeln ("\t}");
	    i += 1;
	}
	buf.writeln ("\t]\n}");
	return buf.str ();
    }
    
    std::string Visitor::dumpClass (const semantic::Class & cl) {
	Ymir::OutBuffer buf;
	auto gen = cl.getGenerator ();
	buf.writeln ("{");
	buf.writeln ("\"type\" : \"class\",");
	buf.writefln ("\"name\" : \"%\",", cl.getRealName ());
	buf.writefln ("\t\"loc_file\" : \"%\",", cl.getName ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", cl.getName ().line);
	buf.writefln ("\t\"loc_col\" : %,", cl.getName ().column);
	buf.writefln ("\t\"doc\" : \"%\",", cl.getComments ());
	if (cl.isPublic ()) buf.writeln ("\t\"protection\" : \"public\",");
	else buf.writeln ("\t\"protection\" : \"private\",");
	
	auto ancestor = gen.to <generator::Class> ().getClassRef ().to <generator::ClassRef> ().getAncestor ();
	if (!ancestor.isEmpty ())
	    buf.writefln ("\t\"ancestor\" : \"%\",", ancestor.prettyString ());
	
	buf.writeln ("\t\"fields\" : [");
	int i = 0;
	for (auto & it : gen.to <semantic::generator::Class> ().getFields ()) {
	    if (i == 0) buf.writeln ("\t{");
	    else buf.writeln ("\t,{");
	    
	    buf.writefln ("\t\t\"name\" : \"%\",", it.to <generator::VarDecl> ().getName ());
	    buf.writefln ("\t\t\"type\" : \"%\",", it.to <generator::VarDecl> ().getVarType ().prettyString ());
	    buf.writefln ("\t\t\"mut\" : \"%\",", it.to <generator::VarDecl> ().isMutable () ? "true" : "false");
	    if (cl.isMarkedPrivate (it.to <generator::VarDecl> ().getName ()))
		buf.writefln ("\t\t\"protection\" : \"%\"", "private");
	    else if (cl.isMarkedProtected (it.to <generator::VarDecl> ().getName ()))
		buf.writefln ("\t\t\"protection\" : \"%\"", "prot");
	    else
		buf.writefln ("\t\t\"protection\" : \"%\"", "public");
		
	    if (!it.to <generator::VarDecl> ().getVarValue ().isEmpty ())
		buf.writefln ("\t\t,\"value\" : \"%\"", it.to<generator::VarDecl> ().getVarValue ().prettyString ());
	    buf.writeln ("\t}");
	    i += 1;
	}
	buf.writeln ("\t],\n\t\"constructors\" : [");
	i = 0;
	for (auto & it : cl.getAllInner ()) { // Dump constructors
	    match (it) {
		of (semantic::Constructor, cst ATTRIBUTE_UNUSED, {
			auto proto = this-> _context.validateConstructorProto (it);
			if (i == 0) buf.writeln ("\t{");
			else buf.writeln ("\t,{");
			buf.writefln ("\t\t\"loc_file\" : \"%\",", it.getName ().locFile);
			buf.writefln ("\t\t\"loc_line\" : %,", it.getName ().line);
			buf.writefln ("\t\t\"loc_col\" : %,", it.getName ().column);
			buf.writefln ("\t\t\"doc\" : \"%\",", it.getComments ());
			if (it.isProtected ())
			    buf.writefln ("\t\t\"protection\" : \"%\",", "prot");
			else if (it.isPublic ())
			    buf.writefln ("\t\t\"protection\" : \"%\",", "public");
			else
			    buf.writefln ("\t\t\"protection\" : \"%\",", "private");
		
			buf.writeln ("\t\t\"params\" : [");
			int j = 0;
			for (auto & it : proto.to <generator::ConstructorProto> ().getParameters ()) {
			    if (j == 0) buf.writeln ("\t{");
			    else buf.writeln ("\t\t,{");
			    buf.writefln ("\t\t\t\"name\" : \"%\",", it.to <generator::ProtoVar> ().getLocation ().str);
			    buf.writefln ("\t\t\t\"type\" : \"%\",", it.to <generator::Value> ().getType ().prettyString ());
			    buf.writefln ("\t\t\t\"mut\" : \"%\",", it.to <generator::ProtoVar> ().isMutable ()? "true" : "false");
			    if (!it.to <generator::ProtoVar> ().getValue ().isEmpty ())
				buf.writefln ("\t\t\t\"value\" : \"%\"", it.to<generator::ProtoVar> ().getValue ().prettyString ());
			    buf.writeln ("\t\t}");
			    j += 1;
			}			
			buf.writeln ("\t\t]\t}");
			i += 1;
		    }		    
		    );
	    }
	}
	
	buf.writeln ("\t],\n\t\"methods\": [");
	i = 0;	    
	for (auto & it : gen.to <semantic::generator::Class> ().getVtable ()) {
	    if (i != 0) buf.writeln ("\t,");
	    buf.writeln (Ymir::entab (this-> dumpMethodProto (it.to <generator::MethodProto> (), gen.to <semantic::generator::Class> ().getProtectionVtable ()[i]), "\t"));
	    i += 1;
	}

	buf.writeln ("\t],\n\t\"impl\": [");
	i = 0;
	for (auto & it : cl.getAllInner ()) {
	    match (it) {
		of (semantic::Impl, im ATTRIBUTE_UNUSED, {
			if (i == 0) buf.writeln ("\t{");
			else buf.writeln ("\t,{");
			buf.writefln ("\t\t\"loc_file\" : \"%\",", it.getName ().locFile);
			buf.writefln ("\t\t\"loc_line\" : %,", it.getName ().line);
			buf.writefln ("\t\t\"loc_col\" : %,", it.getName ().column);
			buf.writefln ("\t\t\"doc\" : \"%\",", it.getComments ());
			buf.writefln ("\t\t\"type\" : \"%\"", this-> _context.validateType (im.getTrait ()).prettyString ());
			buf.writeln ("}");
		    }
		    );
	    }
	}
	
	buf.writeln ("\t]\n}");
	return buf.str ();
    }


    std::string Visitor::dumpClassUnvalidated (const syntax::Class & s_cl, bool pub, bool prot) {
	auto visit = declarator::Visitor::init ();
	visit.setWeak ();
	auto symcl = visit.visitClass (s_cl);
	auto & cl = symcl.to <semantic::Class> ();
	Ymir::OutBuffer buf;
	
	buf.writeln ("{");
	buf.writeln ("\"type\" : \"class\",");
	buf.writefln ("\"name\" : \"%\",", cl.getRealName ());
	buf.writefln ("\t\"loc_file\" : \"%\",", cl.getName ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", cl.getName ().line);
	buf.writefln ("\t\"loc_col\" : %,", cl.getName ().column);
	buf.writefln ("\t\"doc\" : \"%\",", cl.getComments ());
	if (pub) buf.writeln ("\t\"protection\" : \"public\",");
	else if (prot) buf.writeln ("\t\"protection\" : \"prot\",");
	else buf.writeln ("\t\"protection\" : \"private\",");
	
	auto ancestor = s_cl.getAncestor ();
	if (!ancestor.isEmpty ())
	    buf.writefln ("\t\"ancestor\" : \"%\",", ancestor.prettyString ());
	
	buf.writeln ("\t\"fields\" : [");
	int i = 0;
	for (auto & it : cl.getFields ()) {
	    if (i == 0) buf.writeln ("\t{");
	    else buf.writeln ("\t,{");
	    
	    buf.writefln ("\t\t\"name\" : \"%\",", it.to <syntax::VarDecl> ().getName ());
	    buf.writefln ("\t\t\"type\" : \"%\",", it.to <syntax::VarDecl> ().getType ().prettyString ());
	    buf.writefln ("\t\t\"mut\" : \"%\",", it.to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT) ? "true" : "false");
	    if (cl.isMarkedPrivate (it.to <syntax::VarDecl> ().getName ().str))
		buf.writefln ("\t\t\"protection\" : \"%\"", "private");
	    else if (cl.isMarkedProtected (it.to <syntax::VarDecl> ().getName ().str))
		buf.writefln ("\t\t\"protection\" : \"%\"", "prot");
	    else
		buf.writefln ("\t\t\"protection\" : \"%\"", "public");
		
	    buf.writeln ("\t}");
	    i += 1;
	}
	buf.writeln ("\t],\n\t\"constructors\" : [");
	i = 0;
	for (auto & it : cl.getAllInner ()) { // Dump constructors
	    match (it) {
		of (semantic::Constructor, cst ATTRIBUTE_UNUSED, {
			if (i == 0) buf.writeln ("\t{");
			else buf.writeln ("\t,{");
			buf.writefln ("\t\t\"loc_file\" : \"%\",", it.getName ().locFile);
			buf.writefln ("\t\t\"loc_line\" : %,", it.getName ().line);
			buf.writefln ("\t\t\"loc_col\" : %,", it.getName ().column);
			buf.writefln ("\t\t\"doc\" : \"%\",", it.getComments ());
			if (it.isProtected ())
			    buf.writefln ("\t\t\"protection\" : \"%\",", "prot");
			else if (it.isPublic ())
			    buf.writefln ("\t\t\"protection\" : \"%\",", "public");
			else
			    buf.writefln ("\t\t\"protection\" : \"%\",", "private");
		
			buf.writeln ("\t\t\"params\" : [");
			int j = 0;
			for (auto & it : cst.getContent ().getPrototype ().getParameters ()) {
			    if (j == 0) buf.writeln ("\t{");
			    else buf.writeln ("\t\t,{");
			    buf.writefln ("\t\t\t\"name\" : \"%\",", it.to <syntax::VarDecl> ().getLocation ().str);
			    buf.writefln ("\t\t\t\"type\" : \"%\",", it.to <syntax::VarDecl> ().getType ().prettyString ());
			    buf.writefln ("\t\t\"mut\" : \"%\",", it.to <syntax::VarDecl> ().hasDecorator (syntax::Decorator::MUT)? "true" : "false");
			    buf.writeln ("\t\t}");
			    j += 1;
			}			
			buf.writeln ("\t\t]\t}");
			i += 1;
		    }		    
		    );
	    }
	}
	
	buf.writeln ("\t],\n\t\"methods\": [");
	i = 0;	    
	for (auto & it : cl.getAllInner ()) {
	    match (it) {
		of (semantic::Function, fn ATTRIBUTE_UNUSED, {
			if (i != 0) buf.writeln ("\t,");
			buf.writeln (Ymir::entab (this-> dumpUnvalidated (it), "\t"));				     
			i += 1;
		    }
		    );
	    }
	}

	buf.writeln ("\t],\n\t\"impl\": [");
	i = 0;
	for (auto & it : cl.getAllInner ()) {
	    match (it) {
		of (semantic::Impl, im ATTRIBUTE_UNUSED, {
			if (i == 0) buf.writeln ("\t{");
			else buf.writeln ("\t,{");
			buf.writefln ("\t\t\"loc_file\" : \"%\",", it.getName ().locFile);
			buf.writefln ("\t\t\"loc_line\" : %,", it.getName ().line);
			buf.writefln ("\t\t\"loc_col\" : %,", it.getName ().column);
			buf.writefln ("\t\t\"doc\" : \"%\",", it.getComments ());
			buf.writefln ("\t\t\"type\" : \"%\"", this-> _context.validateType (im.getTrait ()).prettyString ());
			buf.writeln ("}");
		    }
		    );
	    }
	}
	
	buf.writeln ("\t]\n}");
	
	return buf.str ();
    }


    std::string Visitor::dumpMethodProto (const generator::MethodProto & proto, const semantic::generator::Class::MethodProtection & prot) {
	Ymir::OutBuffer buf;
	buf.writeln ("{");
	buf.writeln ("\t\"type\" : \"method\",");	
	buf.writefln ("\t\"name\" : \"%\",", proto.getLocation ().str);
	buf.writefln ("\t\"loc_file\" : \"%\",", proto.getLocation ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", proto.getLocation ().line);
	buf.writefln ("\t\"loc_col\" : %,", proto.getLocation ().column);
	buf.writefln ("\t\"doc\" : \"%\",", proto.getComments ());
	buf.writeln ("\t\"params\" : [");
	int i = 0;
	for (auto & it : proto.getParameters ()) {
	    if (i == 0) buf.writeln ("\t{");
	    else buf.writeln ("\t,{");
	    buf.writefln ("\t\t\"name\" : \"%\",", it.to <generator::ProtoVar> ().getLocation ().str);
	    buf.writefln ("\t\t\"type\" : \"%\",", it.to <generator::Value> ().getType ().prettyString ());
	    buf.writefln ("\t\t\"mut\" : \"%\",", it.to <generator::ProtoVar> ().isMutable ()? "true" : "false");
	    if (!it.to <generator::ProtoVar> ().getValue ().isEmpty ())
		buf.writefln ("\t\t\"value\" : \"%\"", it.to<generator::ProtoVar> ().getValue ().prettyString ());
	    buf.writeln ("\t}");
	    i += 1;
	}
	buf.writeln ("\t],");
	buf.writefln ("\t\"ret_type\" : \"%\"", proto.getReturnType ().prettyString ());
	if (proto.isEmptyFrame ()) {
	    buf.writeln ("\t,\"pure\" : \"true\"");
	}
	
	if (proto.isFinal ()) {
	    buf.writeln ("\t,\"final\" : \"true\"");
	}
	
	if (proto.isMutable ()) {
	    buf.writeln ("\t,\"mutable\" : \"true\"");
	}

	if (prot == semantic::generator::Class::MethodProtection::PRV_PARENT) {
	    buf.writeln ("\t,\"prot\" : \"private_parent\"");
	} else if (prot == semantic::generator::Class::MethodProtection::PRV) {
	    buf.writeln ("\t,\"prot\" : \"private\"");
	} else if (prot == semantic::generator::Class::MethodProtection::PROT) {
	    buf.writeln ("\t,\"prot\" : \"prot\"");
	} else {
	    buf.writeln ("\t,\"prot\" : \"public\"");
	}

	buf.writeln ("}");
	return buf.str ();
    }

    std::string Visitor::dumpTrait (const semantic::Trait & tr) {
	Ymir::OutBuffer buf;
	buf.writeln ("{");
	buf.writeln ("\t\"type\" : \"trait\",");
	buf.writefln ("\t\"name\" : \"%\",", tr.getRealName ());
	buf.writefln ("\t\"loc_file\" : \"%\",", tr.getName ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", tr.getName ().line);
	buf.writefln ("\t\"loc_col\" : %,", tr.getName  ().column);
	buf.writefln ("\t\"doc\" : \"%\",", tr.getComments ());
	if (tr.isPublic ()) buf.writeln ("\t\"protection\" : \"public\",");
	else buf.writeln ("\t\"protection\" : \"private\",");

	buf.writeln ("\t\"childs\" : [");
	int i = 0;
	for (auto & it : tr.getAllInner ()) {
	    if (i != 0) buf.writeln ("\t,");
	    auto text = Ymir::entab (this-> dumpUnvalidated (it), "\t");
	    if (text != "") {
		buf.writeln (text);
		i += 1;
	    }
	}
	buf.writeln ("\t]");
	buf.writeln ("}");
	return buf.str ();
    }


    std::string Visitor::dumpTraitUnvalidated (const syntax::Trait & tr, bool pub, bool prot) {
	return "";
    }
    
    std::string Visitor::dumpTemplate (const semantic::Template & tm) {
	Ymir::OutBuffer buf;
	buf.writeln ("{");
	buf.writeln ("\t\"type\" : \"template\",");
	buf.writefln ("\t\"name\" : \"%\",", tm.getRealName ());
	buf.writefln ("\t\"loc_file\" : \"%\",", tm.getName ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", tm.getName ().line);
	buf.writefln ("\t\"loc_col\" : %,", tm.getName  ().column);
	buf.writefln ("\t\"doc\" : \"%\",", tm.getComments ());
	if (tm.isPublic ()) buf.writeln ("\t\"protection\" : \"public\",");
	else buf.writeln ("\t\"protection\" : \"private\",");

	if (!tm.getTest ().isEmpty ()) {
	    buf.writefln ("\t\"test\" : \"%\",", tm.getTest ().prettyString ());
	}
	
	int i = 0;
	buf.write ("\t\"params\" : [");
	for (auto & it : tm.getParams ()) {
	    if (i != 0) buf.write (", ");
	    buf.writef ("\"%\"", it.prettyString ());
	    i += 1;
	}
	
	buf.writeln ("]\n\t,\"childs\" : [");
	auto text = Ymir::entab (this-> dumpUnvalidated (tm.getDeclaration ()), "\t");
	if (text != "") {
	    buf.writeln (text);	
	}
	buf.writeln ("\t]");
	buf.writeln ("}");
	return buf.str ();
    }

    std::string Visitor::dumpTemplateUnvalidated (const syntax::Template & tl, bool pub, bool prot) {
	return "";
    }

    std::string Visitor::dumpMacro (const semantic::Macro & m) {}

    
    
}

