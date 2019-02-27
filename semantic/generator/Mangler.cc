#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/utils/Match.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {

    namespace generator {
	
	using namespace Ymir;

	std::string Mangler::YMIR_PREFIX = "_Y";
	std::string Mangler::YMIR_FUNCTION = "F";
	std::string Mangler::YMIR_FUNCTION_RET = "Z";
	std::string Mangler::YMIR_VAR = "V";
	
	Mangler::Mangler () {}

	Mangler Mangler::init () {
	    return Mangler ();
	}

	std::string Mangler::mangle (const generator::Generator & gen) const {
	    match (gen) {
		of (Frame, fr, return mangleFrame (fr));
		of (GlobalVar, v, return mangleGlobalVar (v));
		of (BoolValue, bl, return mangleBoolV (bl));
		of (CharValue, c, return mangleCharV (c));
		of (Fixed, f, return mangleFixed (f));
		of (FloatValue, fl, return mangleFloatV (fl));
		of (TupleValue, tl, return mangleTupleV (tl));
	    }
	    
	    return mangleType (gen, true);
	}

	std::string Mangler::mangleType (const Generator & gen, bool fatherMut) const {
	    std::string result = "";
	    match (gen) {
		of (Array, ar, result = mangleArrayT (ar));
		of (Bool, b, result = mangleBoolT (b));
		of (Char, c, result = mangleCharT (c));
		of (Float, f, result = mangleFloatT (f));
		of (Integer, i, result = mangleIntegerT (i));
		of (Slice, sl, result = mangleSliceT (sl));
		of (Tuple, tl, result = mangleTupleT (tl));
		of (Void, v, result = mangleVoidT (v));
	    }
	    
	    if (result == "")
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");

	    else if (fatherMut && gen.to <Type> ().isMutable ()) 
		result = "x" + result;	    
	    
	    return result;	    
	}
	
	std::string Mangler::mangleFrame (const Frame & fr) const {
	    auto name = fr.getName ();
	    std::vector <std::string> splits = split (name, "::");

	    OutBuffer buf;
	    buf.write (Mangler::YMIR_PREFIX);
	    for (auto & it : splits) buf.write (it.length (), it);
	    buf.write (Mangler::YMIR_FUNCTION);

	    for (auto & p : fr.getParams ()) buf.write (mangle (p.to <ParamVar> ().getType ()));
	    buf.write (Mangler::YMIR_FUNCTION_RET, mangle (fr.getType ()));
	    return buf.str ();
	}

	std::string Mangler::mangleFrameProto (const FrameProto & proto) const {
	    auto name = proto.getName ();
	    std::vector <std::string> splits = split (name, "::");
	    
	    OutBuffer buf;
	    buf.write (Mangler::YMIR_PREFIX);
	    for (auto & it : splits) buf.write (it.length (), it);
	    buf.write (Mangler::YMIR_FUNCTION);

	    for (auto & p : proto.getParameters ()) buf.write (mangle (p.to <ProtoVar> ().getType ()));
	    buf.write (Mangler::YMIR_FUNCTION_RET, mangle (proto.getReturnType ()));
	    return buf.str ();
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

	std::string Mangler::mangleBoolV (const BoolValue & b) const {
	    if (b.getValue ()) return "b1";
	    else return "b0";
	}

	std::string Mangler::mangleCharV (const CharValue & c) const {
	    OutBuffer buf;
	    buf.write (c.getValue ());
	    return buf.str ();
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
	    return c.getTypeName ();
	}

	std::string Mangler::mangleFloatT (const Float & f) const {
	    return f.getTypeName ();
	}

	std::string Mangler::mangleIntegerT (const Integer & i) const {
	    return i.getTypeName ();
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