#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/pack/FrameProto.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/value/Value.hh>
#include <ymir/semantic/types/_.hh>
#include <string>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Array.hh>
#include <ymir/ast/Var.hh>
#include <ymir/syntax/Keys.hh>


namespace Mangler {
    using namespace Ymir;
    using namespace semantic;
    
    std::string mangle_file (std::string & in) {
	Ymir::OutBuffer ss;
	for (auto it : in) {
	    if (it == '/') ss.write ('.');
	    else ss.write ((char) it);
	}
	return ss.str ();
    }

    std::string replace (std::string ph, std::vector <char> from, std::vector <char> to) {
	OutBuffer buf;
	for (ulong i = 0 ; i < ph.length () ; i++) {
	    bool towrite = true;
	    for (ulong j = 0 ; j < from.size () ; j++) {
		if (ph [i] == from [j]) {
		    buf.write (to [j]);
		    towrite = false;
		    break;
		} 
	    }
	    if (towrite) buf.write (ph [i]);
	}
	return buf.str ();
    }
    
    std::string mangle_var (std::string name) {
	auto res = replace (name, {'(', ')', ',', '\''}, {'N', 'N', 'U', 'G'});
	OutBuffer fin;
	for (auto it : res) {
	    if ((it < 'a' || it > 'z') &&
		(it < 'A' || it > 'Z') &&
		(it < '0' || it > '9') &&
		(it != '_')
	    ) {
		fin.write((short) it);
	    } else fin.write (it);
	}
	
	OutBuffer ss;
	ss.write (fin.str ().length (), replace (fin.str (), {'!'}, {'T'}));
	return ss.str ();
    }
    
    std::string mangle_type (std::string name) {
	auto res = replace (name, {'(', ')', ',', '\''}, {'N', 'N', 'U', 'G'});
	OutBuffer fin;
	for (auto it : res) {
	    if ((it < 'a' || it > 'z') &&
		(it < 'A' || it > 'Z') &&
		(it < '0' || it > '9') &&
		(it != '_')
	    ) {
		fin.write ((short) it);
	    } else fin.write (it);
	}
	return fin.str ();
    }

    std::string mangle_type_CPP (semantic::InfoType type) {
	if (auto fixed = type-> to<IFixedInfo> ()) {
	    if (fixed-> type () == FixedConst::UBYTE) return "h";
	    if (fixed-> type () == FixedConst::BYTE) return "c";
	    if (fixed-> type () == FixedConst::SHORT) return "s";
	    if (fixed-> type () == FixedConst::USHORT) return "t";
	    if (fixed-> type () == FixedConst::INT) return "i";
	    if (fixed-> type () == FixedConst::UINT) return "j";
	    if (fixed-> type () == FixedConst::LONG) return "l";
	    if (fixed-> type () == FixedConst::ULONG) return "m";
	} else if (auto flt = type-> to <IFloatInfo> ()) {
	    if (flt-> type () == FloatConst::FLOAT) return "f";
	    if (flt-> type () == FloatConst::DOUBLE) return "d";
	} else if (type-> to <IBoolInfo> ()) { return "b"; 
	} else if (type-> to<ICharInfo> ()) { return "c";
	} else if (auto arr = type-> to<IArrayInfo> ()) {
	    auto content = mangle_type_CPP (arr-> content());
	    if (!arr-> isStatic ()) {
		return std::string ("mP") + content;
	    } else return "P" + content;
	} else if (auto ref = type-> to <IRefInfo> ()) {
	    auto content = mangle_type_CPP (ref-> content());
	    return std::string ("R") + content;
	} else if (auto en = type-> to <IEnumInfo> ()) {
	    return mangle_var (en-> name ());
	} else if (auto st = type-> to <IStructInfo> ()) {
	    return mangle_var (st-> getName ());
	} else if (auto tu = type-> to <ITupleInfo> ()) {
	    Ymir::OutBuffer buf;
	    for (auto it : tu-> getParams ())
		buf.write (mangle_type_CPP (it));
	    return buf.str ();
	} else if (auto rng = type-> to <IRangeInfo> ()) {
	    auto content = mangle_type_CPP (rng-> content ());
	    return content + content;
	} 
	Ymir::Error::assert ("TODO ", type-> typeString ());
	return "";    
    }
    
    std::string mangle_namespace (std::string name) {
	OutBuffer ss;
	while (true) {
	    auto index = name.find (".");
	    if (index != name.npos) {
		auto curr = mangle_var (name.substr (0, index));
		name = name.substr (index + 1, name.length () - (index + 1));
		ss.write (curr);
	    } else {
		ss.write (mangle_var (name));
		break;
	    }
	}
	return ss.str ();
    }

    std::string mangle_namespace_CPP (std::string name) {
	OutBuffer ss;
	ss.write ("N");
	while (true) {
	    auto index = name.find (".");
	    if (index != name.npos) {
		auto curr = mangle_var (name.substr (0, index));
		name = name.substr (index + 1, name.length () - (index + 1));
		ss.write (curr);
	    } else {
		ss.write (mangle_var (name));
		break;
	    }
	}
	return ss.str ();
    }
    
    std::string mangle_global (std::string name) {
	auto res = mangle_namespace (name);
	for (int i = 0 ; i < (int) res.length () ; i++) {
	    if (res [i] < '0' || res [i] > '9')
		return res.substr (i);
	}
	return res;
    }

    std::string mangle_function_C (std::string & name, semantic::FrameProto) {
	return name;
    }

    std::string mangle_function_C (std::string & name, semantic::FinalFrame) {
	return name;
    }

    std::string mangle_function_CPP (std::string & name, semantic::FrameProto proto) {
	auto space = proto-> externLangSpace ();
	if (space != "") {
	    Ymir::OutBuffer buf;
	    buf.write ("_Z", mangle_namespace_CPP (space));
	    buf.write (mangle_var (name), "E");
	    for (auto it : proto-> vars ()) {
		buf.write (mangle_type_CPP (it-> info-> type));
	    }
	    return buf.str ();
	} else {
	    Ymir::OutBuffer buf;
	    buf.write ("_Z", mangle_var (name));
	    for (auto it : proto-> vars ()) {
		buf.write (mangle_type_CPP (it-> info-> type));
	    }
	    return buf.str ();
	}
    }
    
    std::string mangle_function_D (std::string &, semantic::FrameProto) {
	Ymir::Error::assert ("TODO");
	return "";
    }
    
    std::string mangle_function (std::string& name, ::semantic::FrameProto frame) {
	if (name == Keys::MAIN) {
	    if (!Options::instance ().isStandalone ())
		return "_Y" + name;
	    else return name;
	}

	if (frame-> externLang () == Keys::CLANG) return mangle_function_C (name, frame);
	else if (frame-> externLang () == Keys::CPPLANG) return mangle_function_CPP (name, frame);
	else if (frame-> externLang () == Keys::DLANG) return mangle_function_D (name, frame);
	
	auto space = frame-> space ().toString ();
	OutBuffer ss;
	ss.write ("_Y", mangle_namespace (space), mangle_namespace (name), "F");
	for (auto it : frame-> tmps ()) {
	    if (it-> info) {
		if (it-> info-> isImmutable ()) {
		    ss.write ("N", mangle_var (it-> info-> value ()-> toString ()));		    
		} else if (auto tu = it-> info-> type-> to <ITupleInfo> ()) {
		    if (tu-> isFake ())
			for (auto it : tu-> getParams ())
			    ss.write ("N", it-> value ()-> toString ());
		    else ss.write ("N", mangle_type (it-> info-> simpleTypeString ()));
		} else ss.write ("N", mangle_type (it-> info-> simpleTypeString ()));
	    }
	}
	
	for (auto it : frame-> vars ()) {
	    ss.write (mangle_type (it-> info-> simpleTypeString ()));
	}
	
	ss.write ("Z", mangle_type (frame-> type ()-> simpleTypeString ()));
	return ss.str ();
    }

    std::string mangle_function (std::string & name, ::semantic::FinalFrame frame) {
	if (name == Keys::MAIN) {
	    if (!Options::instance ().isStandalone ())
		return "_Y" + name;
	    else return name;
	}

	if (frame-> externLang () == Keys::CLANG) return mangle_function_C (name, frame);
	auto space = frame-> space ().toString ();
	OutBuffer ss;
	ss.write ("_Y", mangle_namespace (space), mangle_namespace (name), "F");
	for (auto it : frame-> tmps ()) {
	    if (it-> info) {
		if (it-> info-> isImmutable ()) ss.write ("N", mangle_var (it-> info-> value ()-> toString ()));
		else if (auto tu = it-> info-> type-> to <ITupleInfo> ()) {
		    if (tu-> isFake ())
			for (auto it : tu-> getParams ())
			    ss.write ("N", it-> value ()-> toString ());
		    else ss.write ("N", mangle_type (it-> info-> simpleTypeString ()));
		} else ss.write ("N", mangle_type (it-> info-> simpleTypeString ()));
	    }
	}
	
	for (auto it : frame-> vars ()) {
	    ss.write (mangle_type (it-> info-> simpleTypeString ()));
	}
	ss.write ("Z", mangle_type (frame-> type ()-> simpleTypeString ()));
	return ss.str ();
    }
    
    std::string mangle_functionv (std::string& name, ::semantic::FrameProto frame) {
	auto space = frame-> space ().toString ();
	OutBuffer ss;
	ss.write  ("_Y", mangle_namespace (space), mangle_namespace (name), "VF");
	for (auto it : frame-> tmps ()) {
	    if (it-> info-> isImmutable ()) ss.write ("N", mangle_var (it-> info-> value ()-> toString ()));
	    else if (auto tu = it-> info-> type-> to <ITupleInfo> ()) {
		if (tu-> isFake ())
		    for (auto it : tu-> getParams ())
			ss.write ("N", it-> value ()-> toString ());
		else ss.write ("N", mangle_type (it-> info-> simpleTypeString ()));
	    } else ss.write ("N", mangle_type (it-> info-> simpleTypeString ()));
	}
	
	for (auto it : frame-> vars ()) {
	    ss.write (mangle_type (it-> info-> simpleTypeString ()));
	}
	ss.write ("Z", mangle_type (frame-> type ()-> simpleTypeString ()));
	return ss.str ();
    }

}
