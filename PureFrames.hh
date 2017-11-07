#pragma once

#include <vector>
#include "Frame.hh"

namespace Semantic {


    struct PureFrame : Frame {

	PureFrame (Syntax::Function * fun, const std::string & space)
	    : Frame (fun, space)
	{}

	void validate ();	

	void verifyReturn ();

	void setMainParams (std::vector<Syntax::VarPtr> &vars);
    };

    struct FinalFrame : Frame {

	FinalFrame (Syntax::Function * fun, const std::string & space)
	    : Frame (fun, space)
	{}

	void finalize ();
	
    };
    
    struct PureFrames {

	static std::vector<PureFrame> & allPure() {
	    return frames;
	}

	static std::vector <FinalFrame> & allFinal () {
	    return final_frames;
	}
	
	static void insertPure (PureFrame fr) {
	    frames.push_back (fr);
	}

	static void insertFinal (FinalFrame fr) {
	    final_frames.push_back (fr);
	}
	
    private:

	static std::vector <FinalFrame> final_frames;
	static std::vector <PureFrame> frames;
	
    };

    
}
