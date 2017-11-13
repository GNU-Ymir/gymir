#pragma once

#include "Frame.hh"
#include "FrameProto.hh"
#include "Namespace.hh"

#include <string>
#include <vector>

namespace semantic {

    namespace syntax {
	class IProto;
	typedef IProto* Proto;
    }
    
    class IExternFrame : public IFrame {

	std::string _name;
	syntax::Proto _proto;
	std::string _from;	
	FrameProto _fr;

	static std::vector <IExternFrame*> __extFrames__;
	
    public:

	IExternFrame (Namespace space, std::string from, syntax::Proto func);

	IExternFrame (Namespace space, syntax::Function func);

	//ApplicationScore isApplicable  (ParamList params) override;

	FrameProto validate () override ;

	FrameProto validate (syntax::ParamList) override;

	static std::vector <IExternFrame*> frames () {
	    return __extFrames__;
	}

	static void clear () {
	    __extFrames__.clear ();
	}

	Word ident () override;

	std::string from ();

	FrameProto proto ();

	std::string name ();

	bool isVariadic () const override;
	
    private:

	FrameProto validateFunc ();
	
	//ApplicationScore isApplicableVariadic  (ParamList params) ;
	
    };

    typedef IExternFrame* ExternFrame;
    
}
