#pragma once

#include "Frame.hh"
#include "Namespace.hh"

#include <string>
#include <ymir/utils/Array.hh>

namespace syntax {
    class IProto;
    typedef IProto* Proto;
}
    
namespace semantic {

    class IFrame;
    typedef IFrame* Frame;

    class IFrameProto;
    typedef IFrameProto* FrameProto;
    
    class IExternFrame : public IFrame {

	std::string _name;
	::syntax::Proto _proto;
	std::string _from;
	std::string _externSpace;
	FrameProto _fr;

	static std::vector <IExternFrame*> __extFrames__;
	
    public:

	IExternFrame (Namespace space, std::string from, ::syntax::Proto func);

	IExternFrame (Namespace space, ::syntax::Function func);

	ApplicationScore isApplicable  (::syntax::ParamList params) override;

	ApplicationScore isApplicable  (const std::vector <InfoType> & params) override;
		
	FrameProto validate () override ;

	FrameProto validate (::syntax::ParamList) override;
	
	FrameProto validate (const std::vector<InfoType> &) override;

	static std::vector <IExternFrame*> frames () {
	    return __extFrames__;
	}

	static void clear () {
	    __extFrames__.clear ();
	}

	Word ident () override;

	std::string from ();

	std::string& externSpace ();
	
	FrameProto proto ();

	std::string name ();

	std::string getName () override;
	
	std::string toString () override;
	
	bool isVariadic () const override;

	static const char* id () {
	    return "IExternFrame";
	}

	virtual const char* getId ();
	
    private:

	FrameProto validateFunc ();
	
	ApplicationScore isApplicableVariadic  (const std::vector <InfoType> & params) ;
	
    };

    typedef IExternFrame* ExternFrame;
    
}
