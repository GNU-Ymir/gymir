#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/utils/Match.hh>
#include <ymir/semantic/validator/UtfVisitor.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {

    namespace generator {
	
	using namespace Ymir;

	std::string Mangler::YMIR_PREFIX = "_Y";
	std::string Mangler::YMIR_FUNCTION = "F";
	std::string Mangler::YMIR_FUNCTION_RET = "Z";
	std::string Mangler::YMIR_VAR = "V";
	std::string Mangler::YMIR_CST = "CST";
	
	Mangler::Mangler () {}

	Mangler Mangler::init () {
	    return Mangler ();
	}

	std::string Mangler::getYmirPrefix () {
	    return YMIR_PREFIX;
	}
	
	std::string Mangler::mangle (const generator::Generator & gen) const {
	    match (gen) {
		of (Frame, fr, return mangleFrame (fr));
		of (FrameProto, proto, return mangleFrameProto (proto));
		of (GlobalVar, v, return mangleGlobalVar (v));
		of (BoolValue, bl, return mangleBoolV (bl));
		of (CharValue, c, return mangleCharV (c));
		of (Fixed, f, return mangleFixed (f));
		of (FloatValue, fl, return mangleFloatV (fl));
		of (TupleValue, tl, return mangleTupleV (tl));
		of (Addresser, addr, return mangleAddrV (addr));
		of (StringValue, s, return mangleStringV (s));
		of (Aliaser, a, return mangle (a.getWho ()));
	    }
	    
	    return mangleType (gen, true);
	}

	std::string Mangler::mangleType (const Generator & gen, bool fatherMut) const {
	    std::string result = "";
	    match (gen) {
		of (Array, ar, result = mangleArrayT (ar))
		else of (Bool, b, result = mangleBoolT (b))
		else of (Char, c, result = mangleCharT (c))
		else of (Float, f, result = mangleFloatT (f))
		else of (Integer, i, result = mangleIntegerT (i))
		else of (Slice, sl, result = mangleSliceT (sl))
		else of (Tuple, tl, result = mangleTupleT (tl))
		else of (Void, v, result = mangleVoidT (v))
		else of (StructRef, r, result = mangleStructRef (r))
		else of (EnumRef, r, result = mangleEnumRef (r))
		else of (Range, r, result = mangleRangeT (r))
		else of (Pointer, p, result = manglePointerT (p))
		else of (FuncPtr, f, result = mangleFuncPtrT (f))
		else of (Closure, c ATTRIBUTE_UNUSED, return ""); // Closure does not impact the name of the func, as it is only a lambda, and its name is already uniq
	    }

	    if (result == "") {
		println (gen.to <Type> ().prettyString ());
		 Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    }
	    
	    else {
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
		for (auto & it : splits) buf.write (it.length (), it);
		buf.write (Mangler::YMIR_FUNCTION);

		for (auto & p : fr.getParams ()) buf.write (mangle (p.to <ParamVar> ().getType ()));
		buf.write (Mangler::YMIR_FUNCTION_RET, mangle (fr.getType ()));
		return buf.str ();
	    } else {
		auto name = fr.getName ();
		std::vector <std::string> splits = split (name, "::");
		return splits.back ();
	    }
	}

	std::string Mangler::mangleFrameProto (const FrameProto & proto) const {
	    if (proto.getManglingStyle () == Frame::ManglingStyle::Y) {
		auto name = proto.getMangledName ();
		std::vector <std::string> splits = split (name, "::");
	    
		OutBuffer buf;
		buf.write (Mangler::YMIR_PREFIX);
		for (auto & it : splits) buf.write (it.length (), it);
		buf.write (Mangler::YMIR_FUNCTION);

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
	
	std::string Mangler::mangleGlobalVar (const GlobalVar & v) const {
	    auto name = v.getName ();
	    std::vector <std::string> splits = split (name, "::");

	    OutBuffer buf;
	    buf.write (Mangler::YMIR_PREFIX);
	    for (auto & it : splits) buf.write (it.length (), it);
	    buf.write (Mangler::YMIR_VAR);

	    buf.write (mangle (v.getType ()));
	    return buf.str ();
	}

	std::string Mangler::mangleGlobalConstant (const GlobalConstant & v) const {
	    auto name = v.getName ();
	    std::vector <std::string> splits = split (name, "::");

	    OutBuffer buf;
	    buf.write (Mangler::YMIR_PREFIX);
	    for (auto & it : splits) buf.write (it.length (), it);
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
	    
	    return format ("A%_%_%", buf.str().length (),
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
		buf.write (mn.length (), mn);
	    }
	    
	    auto inner = buf.str ();
	    res.write ("T", inner.length (), inner);
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
	    return c.getTypeName (false, false);
	}

	std::string Mangler::mangleFloatT (const Float & f) const {
	    return f.getTypeName (false, false);
	}

	std::string Mangler::mangleIntegerT (const Integer & i) const {
	    return i.getTypeName (false, false);
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
	    res.write ("T", inner.length (), inner);
	    return res.str ();
	}

	std::string Mangler::mangleVoidT (const Void &) const {
	    return "v";
	}

	std::string Mangler::mangleStructRef (const StructRef & ref) const {
	    Ymir::OutBuffer buf;
	    auto splits = split (ref.getMangledName (), "::");
	    for (auto & it : splits) buf.write (it.length (), it);
	    return buf.str ();
	}

	std::string Mangler::mangleEnumRef (const EnumRef & ref) const {
	    Ymir::OutBuffer buf;
	    auto splits = split (ref.getMangledName (), "::");
	    for (auto & it : splits) buf.write (it.length (), it);
	    return buf.str ();
	}

	std::string Mangler::mangleRangeT (const Range & range) const {
	    auto res = mangleType (range.getInners () [0], range.isMutable ());
	    return format ("R%%", res.length (), res);	    
	}

	std::string Mangler::manglePointerT (const Pointer & ptr) const {
	    auto res = mangleType (ptr.getInners () [0], ptr.isMutable ());
	    return format ("P%%", res.length (), res);	    
	}

	std::string Mangler::mangleFuncPtrT (const FuncPtr & ptr) const {
	    Ymir::OutBuffer buf;
	    for (auto & it : ptr.getInners ()) {		
		buf.write (format ("%", mangleType (it, false)));
	    }
	    return format ("FP%%", buf.str ().length (), buf.str ());
	}

	std::string Mangler::mangleClosureT (const Closure &) const {
	    return ""; 
	}
	
	std::vector <std::string> Mangler::split (const std::string & str, const std::string & delim) const {
	    auto aux = str;
	    std::vector <std::string> result;
	    size_t pos = 0;
	    while ((pos = aux.find (delim)) != std::string::npos) {
		result.push_back (aux.substr (0, pos));
		aux = aux.substr (pos + delim.length ());
	    }
	    
	    if (aux != "") result.push_back (aux);
	    return result;
	}

	
    }
    
}
