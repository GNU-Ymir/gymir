#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/utils/Match.hh>
#include <ymir/semantic/validator/UtfVisitor.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {

    namespace generator {
	
	using namespace Ymir;

	std::string Mangler::YMIR_PREFIX = "_Y";
	std::string Mangler::YMIR_FUNCTION = "F";
	std::string Mangler::YMIR_TEST = "T";
	std::string Mangler::YMIR_FUNCTION_RET = "Z";
	std::string Mangler::YMIR_VAR = "V";
	std::string Mangler::YMIR_CST = "CST";
	std::string Mangler::YMIR_VTABLE = "VT";
	std::string Mangler::YMIR_TYPEINFO = "TI";
	
	Mangler::Mangler (bool fail) :
	    _fail (fail)
	{}

	Mangler Mangler::init (bool fail) {
	    return Mangler (fail);
	}

	std::string Mangler::getYmirPrefix () {
	    return YMIR_PREFIX;
	}
	
	std::string Mangler::mangle (const generator::Generator & gen) const {
	    match (gen) {
		s_of (Frame, fr) return mangleFrame (fr);
		s_of (Test, ts) return mangleTest (ts);
		s_of (FrameProto, proto) return mangleFrameProto (proto);
		s_of (ConstructorProto, proto) return mangleConstructorProto (proto);
		s_of (GlobalVar, v) return mangleGlobalVar (v);
		s_of (BoolValue, bl) return mangleBoolV (bl);
		s_of (CharValue, c) return mangleCharV (c);
		s_of (Fixed, f) return mangleFixed (f);
		s_of (FloatValue, fl) return mangleFloatV (fl);
		s_of (TupleValue, tl) return mangleTupleV (tl);
		s_of (Addresser, addr) return mangleAddrV (addr);
		s_of (StringValue, s) return mangleStringV (s);
		s_of (Aliaser, a) return mangle (a.getWho ());
	    }

	    return mangleType (gen, true);
	}

	std::string Mangler::mangleType (const Generator & gen, bool fatherMut) const {
	    std::string result = "";
	    match (gen) {
		of (Array, ar) result = mangleArrayT (ar);
		elof (Bool, b) result = mangleBoolT (b);
		elof (Char, c) result = mangleCharT (c);
		elof (Float, f) result = mangleFloatT (f);
		elof (Integer, i) result = mangleIntegerT (i);
		elof (Slice, sl) result = mangleSliceT (sl);
		elof (Tuple, tl) result = mangleTupleT (tl);
		elof (Void, v) result = mangleVoidT (v);
		elof (StructRef, r) result = mangleStructRef (r);
		elof (ClassRef, r) result = mangleClassRef (r);
		elof (EnumRef, r) result = mangleEnumRef (r);
		elof (Range, r) result = mangleRangeT (r);
		elof (Pointer, p) result = manglePointerT (p);
		elof (ClassPtr, p) result = mangleClassPointerT (p);
		elof (FuncPtr, f) result = mangleFuncPtrT (f);
		elof (Delegate, d) result = mangleDelegateT (d);
		elof (Option, o) result = mangleOptionT (o);
		elof_u (NoneType) return "";
		elof_u (Closure) return ""; // Closure does not impact the name of the func, as it is only a lambda, and its name is already uniq
		fo;
	    }

	    if (result == "" && this-> _fail) {
		println (gen.prettyString ());
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    }	    
	    else if (result != "") {
		if (fatherMut && gen.to <Type> ().isMutable ()) 
		    result = "x" + result;
		if (gen.to <Type> ().isRef ())
		    result = "R" + result;
	    }
	    
	    return result;	    
	}
	
	std::string Mangler::mangleFrame (const Frame & fr) const {
	    if (fr.getManglingStyle () == Frame::ManglingStyle::Y) {
		auto name = fr.getMangledName ();
		std::vector <std::string> splits = split (name, "::");

		OutBuffer buf;
		buf.write (Mangler::YMIR_PREFIX);
		for (auto & it : splits) buf.write ((uint) it.length (), it);
		
		buf.write (Mangler::YMIR_FUNCTION);

		for (auto & p : fr.getParams ()) {
		    buf.write (mangle (p.to <ParamVar> ().getType ()));
		}
		
		buf.write (Mangler::YMIR_FUNCTION_RET, mangle (fr.getType ()));
		return buf.str ();
	    } else {
		auto name = fr.getName ();
		std::vector <std::string> splits = split (name, "::");
		return splits.back ();
	    }
	}

	std::string Mangler::mangleTest (const Test & ts) const {
	    auto name = ts.getName ();
	    std::vector <std::string> splits = split (name, "::");

	    OutBuffer buf;
	    buf.write (Mangler::YMIR_PREFIX);
	    for (auto & it : splits) buf.write ((uint) it.length (), it);
		
	    buf.write (Mangler::YMIR_TEST);
	    buf.write (Mangler::YMIR_FUNCTION_RET, "v");
	    return buf.str ();
	}
	
	std::string Mangler::mangleFrameProto (const FrameProto & proto) const {
	    if (proto.getManglingStyle () == Frame::ManglingStyle::Y) {
		auto name = proto.getMangledName ();
		std::vector <std::string> splits = split (name, "::");
	    
		OutBuffer buf;
		buf.write (Mangler::YMIR_PREFIX);
		for (auto & it : splits) buf.write ((uint) it.length (), it);
		buf.write (Mangler::YMIR_FUNCTION);
		
		auto cl = proto.clone ();
		if (cl.is <MethodProto> ()) {
		    auto type = Type::init (cl.to <MethodProto> ().getClassType ().to <Type> (), cl.to <MethodProto> ().isMutable ());
		    buf.write (mangle (type));
		}
		
		for (auto & p : proto.getParameters ())
		    buf.write (mangle (p.to <ProtoVar> ().getType ()));
			
		buf.write (Mangler::YMIR_FUNCTION_RET, mangle (proto.getReturnType ()));
		return buf.str ();
	    } else {
		auto name = proto.getName ();
		std::vector <std::string> splits = split (name, "::");
		return splits.back ();
	    }
	}

	std::string Mangler::mangleConstructorProto (const ConstructorProto & proto) const {
	    auto name = proto.getMangledName ();
	    std::vector <std::string> splits = split (name, "::");

	    OutBuffer buf;
	    buf.write (Mangler::YMIR_PREFIX);
	    for (auto & it : splits) buf.write ((uint) it.length (), it);
	    buf.write (Mangler::YMIR_FUNCTION);

	    // a construct proto take the return type as first argument, but it is hidden
	    buf.write (mangle (proto.getReturnType ())); 
	    for (auto & p : proto.getParameters ())
		buf.write (mangle (p.to <ProtoVar> ().getType ()));

	    buf.write (Mangler::YMIR_FUNCTION_RET, mangle (proto.getReturnType ()));
	    return buf.str ();
	}
	
	std::string Mangler::mangleGlobalVar (const GlobalVar & v) const {
	    if (v.getExternalLanguage () == "") {
		auto name = v.getName ();
		std::vector <std::string> splits = split (name, "::");

		OutBuffer buf;
		buf.write (Mangler::YMIR_PREFIX);
		for (auto & it : splits) buf.write ((uint) it.length (), it);
		buf.write (Mangler::YMIR_VAR);

		buf.write (mangle (v.getType ()));
		return buf.str ();
	    } else {
		auto name = v.getName ();
		std::vector <std::string> splits = split (name, "::");
		return splits.back ();
	    }
	}

	std::string Mangler::mangleGlobalConstant (const GlobalConstant & v) const {
	    auto name = v.getName ();
	    std::vector <std::string> splits = split (name, "::");

	    OutBuffer buf;
	    buf.write (Mangler::YMIR_PREFIX);
	    for (auto & it : splits) buf.write ((uint) it.length (), it);
	    buf.write (Mangler::YMIR_CST);

	    buf.write (mangle (v.getType ()));
	    return buf.str ();
	}

	
	std::string Mangler::mangleBoolV (const BoolValue & b) const {
	    if (b.getValue ()) return "b1";
	    else return "b0";
	}

	std::string Mangler::mangleCharV (const CharValue & c) const {
	    OutBuffer buf;
	    buf.write (c.getValue ());
	    return buf.str ();
	}
	
	std::string Mangler::mangleStringV (const StringValue & v) const {
	    // Mangling string consist in convert it in utf8 and write it in hex mode
	    // By default string
	    OutBuffer buf;
	    if (v.getType ().to <Type> ().getInners ()[0].to <Char> ().getSize () == 32) {		
		auto res = validator::UtfVisitor::utf32_to_utf8 (v.getValue ());	    
		for (auto & it : res) {
		    buf.write ((uint8_t) it);
		}
	    } else {
		for (auto & it : v.getValue ()) {
		    buf.write ((uint8_t) it);
		}
	    }
	    
	    return format ("A%_%_%", (uint) buf.str().length (),
			   v.getType ().to <Type> ().getInners ()[0].to <Char> ().getSize (), buf.str ());
	}
	
	std::string Mangler::mangleFixed (const Fixed & f) const {
	    OutBuffer buf;
	    if (f.getType ().to <Integer> ().isSigned ())
		buf.write (f.getUI ().i);
	    else buf.write (f.getUI ().u);
	    return buf.str ();
	}

	std::string Mangler::mangleFloatV (const FloatValue & fl) const {
	    return fl.getValue ();
	}

	std::string Mangler::mangleTupleV (const TupleValue & tl) const {
	    OutBuffer buf, res;
	    for (auto & it : tl.getContent ()) {
		auto mn = mangle (it);
		buf.write ((uint) mn.length (), mn);
	    }
	    
	    auto inner = buf.str ();
	    res.write ("T", (uint) inner.length (), inner);
	    return res.str ();
	}

	std::string Mangler::mangleAddrV (const Addresser & addr) const {
	    auto proto = addr.getWho ();
	    return mangle (proto);
	}
	
	std::string Mangler::mangleArrayT (const Array & arr) const {
	    OutBuffer buf;
	    buf.write ("A");
	    buf.write (arr.getSize ());
	    buf.write (mangleType (arr.getInners () [0], arr.isMutable ()));
	    return buf.str ();
	}

	std::string Mangler::mangleBoolT (const Bool &) const {
	    return "b";
	}

	std::string Mangler::mangleCharT (const Char & c) const {
	    return c.computeTypeName (false, false, true);
	}
       
	std::string Mangler::mangleFloatT (const Float & f) const {
	    return f.computeTypeName (false, false, true);
	}

	std::string Mangler::mangleIntegerT (const Integer & i) const {
	    return i.computeTypeName (false, false, true);
	}

	std::string Mangler::mangleSliceT (const Slice & s) const {
	    OutBuffer buf;
	    buf.write ("S");
	    buf.write (mangleType (s.getInners () [0], s.isMutable ()));
	    return buf.str ();
	}

	std::string Mangler::mangleTupleT (const Tuple & tu) const {
	    OutBuffer buf, res;
	    for (auto & it : tu.getInners ())
		buf.write (mangleType (it, tu.isMutable ()));

	    auto inner = buf.str ();
	    res.write ("T", (uint) inner.length (), inner);
	    
	    return res.str ();
	}

	std::string Mangler::mangleVoidT (const Void &) const {
	    return "v";
	}

	std::string Mangler::mangleStructRef (const StructRef & ref) const {
	    Ymir::OutBuffer buf;
	    auto splits = split (ref.getMangledName (), "::");
	    for (auto & it : splits) buf.write ((uint) it.length (), it);
	    return buf.str ();
	}

	std::string Mangler::mangleClassRef (const ClassRef & ref) const {
	    Ymir::OutBuffer buf;
	    auto splits = split (ref.getMangledName (), "::");
	    for (auto & it : splits) buf.write ((uint) it.length (), it);
	    return buf.str ();
	}

	std::string Mangler::mangleTypeInfo (const ClassRef & ref) const {
	    auto buf = mangleClassRef (ref);
	    return Mangler::YMIR_PREFIX + buf + Mangler::YMIR_TYPEINFO;
	}
	
	std::string Mangler::mangleVtable (const ClassRef & ref) const {
	    auto buf = mangleClassRef (ref);
	    return Mangler::YMIR_PREFIX + buf + Mangler::YMIR_VTABLE;
	}
	
	std::string Mangler::mangleEnumRef (const EnumRef & ref) const {
	    Ymir::OutBuffer buf;
	    auto splits = split (ref.getMangledName (), "::");
	    for (auto & it : splits) buf.write ((uint) it.length (), it);
	    return buf.str ();
	}

	std::string Mangler::mangleRangeT (const Range & range) const {
	    auto res = mangleType (range.getInners () [0], range.isMutable ());
	    return format ("R%%", (uint) res.length (), res);	    
	}

	std::string Mangler::manglePointerT (const Pointer & ptr) const {
	    auto res = mangleType (ptr.getInners () [0], ptr.isMutable ());
	    return format ("P%%", (uint) res.length (), res);	    
	}

	std::string Mangler::mangleOptionT (const Option & o) const {
	    auto res = mangleType (o.getInners () [0], o.isMutable ());
	    return format ("O%%", (uint) res.length (), res);	    
	}
	
	std::string Mangler::mangleClassPointerT (const ClassPtr & ptr) const {
	    auto res = mangleType (ptr.getInners () [0], ptr.isMutable ());
	    return format ("P%%", (uint) res.length (), res);	    
	}

	std::string Mangler::mangleFuncPtrT (const FuncPtr & ptr) const {
	    Ymir::OutBuffer buf;
	    for (auto & it : ptr.getInners ()) {		
		buf.write (format ("%", mangleType (it, false)));
	    }
	    return format ("FP%%", (uint) buf.str ().length (), buf.str ());
	}

	std::string Mangler::mangleDelegateT (const Delegate & ptr) const {
	    Ymir::OutBuffer buf;
	    for (auto & it : ptr.getInners ()) {
		if (it.is <Type> ()) {
		    buf.write (format ("%", mangleType (it, false)));
		} else if (it.is <FrameProto> ()) {
		    buf.write (format ("%", mangleFrameProto (it.to <FrameProto> ())));
		}
	    }
	    return format ("DG%%", (uint) buf.str ().length (), buf.str ());
	}
	
	std::string Mangler::mangleClosureT (const Closure &) const {
	    return ""; 
	}

	std::string Mangler::manglePath (const std::string & path) const {
	    std::vector <std::string> splits = split (path, "::");
	    OutBuffer buf;
	    for (auto & it : splits) buf.write ((uint) it.length (), it);
	    return buf.str ();
	}
	
	std::vector <std::string> Mangler::split (const std::string & str, const std::string & delim) const {
	    auto aux = str;
	    std::vector <std::string> result;
	    size_t pos = 0;
	    while ((pos = aux.find (delim)) != std::string::npos) {
		result.push_back (aux.substr (0, pos));
		aux = aux.substr ((uint) (pos + delim.length ()));
	    }
	    
	    if (aux != "") result.push_back (aux);
	    return result;
	}

	
    }
    
}
