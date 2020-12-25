#include <ymir/documentation/Visitor.hh>
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
		else of (semantic::TemplateSolution, sol ATTRIBUTE_UNUSED, {
		    }
		    )
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
	    of (syntax::Function, fn, {
		    return this-> dumpFunctionUnvalidated (fn, pub, prot);
		}
		);
	    of (syntax::Module, mod, {
		    return this-> dumpModuleUnvalidated (mod, pub, prot);
		}
		);
	    of (syntax::DeclBlock, bl, {
		    return this-> dumpDeclBlockUnvalidated (bl, pub, prot);
		}
		);
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
	    buf.writefln ("\t\"private\" : \"%\",", mod.isPublic ()? "false" : "true");
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
	buf.writefln ("\t\"private\" : \"%\",", func.isPublic ()? "false" : "true");
	buf.writeln ("\t\"params\" : [");
	int i = 0;
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
	buf.writefln ("\t\"private\" : \"%\",", decl.isPublic ()? "false" : "true");
	if (!gen.to <generator::GlobalVar> ().getValue ().isEmpty ()){
	    buf.writef (",\n\t\"value\" : \"%\"", gen.to <generator::GlobalVar> ().getValue ().prettyString ());
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
	buf.writefln ("\t\"private\" : \"%\",", al.isPublic ()? "false" : "true");
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
	buf.writefln ("\t\"private\" : \"%\",", str.isPublic ()? "false" : "true");
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
	buf.writefln ("\t\"type\" : \"%\",", gen.to <semantic::generator::Enum> ().getType ().prettyString ());
	buf.writefln ("\t\"private\" : \"%\",", en.isPublic ()? "false" : "true");
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
	buf.writefln ("\t\"private\" : \"%\",", cl.isPublic ()? "false" : "true");
	
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
	buf.writefln ("\t\"private\" : \"%\",", tr.isPublic ()? "false" : "true");
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
    
    std::string Visitor::dumpTemplate (const semantic::Template & tm) {
	Ymir::OutBuffer buf;
	buf.writeln ("{");
	buf.writeln ("\t\"type\" : \"template\",");
	buf.writefln ("\t\"name\" : \"%\",", tm.getRealName ());
	buf.writefln ("\t\"loc_file\" : \"%\",", tm.getName ().locFile);
	buf.writefln ("\t\"loc_line\" : %,", tm.getName ().line);
	buf.writefln ("\t\"loc_col\" : %,", tm.getName  ().column);
	buf.writefln ("\t\"doc\" : \"%\",", tm.getComments ());
	buf.writefln ("\t\"private\" : \"%\",", tm.isPublic ()? "false" : "true");
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

    
    
}

