#pragma once

#include <vector>
#include <string>
#include <set>

namespace global {

    /**
     * This class enumerate the global state of the compilation 
     * For example the prefix of the included std
     */
    class State {

	/** The list of activated version 
	 * + -fversion switch
	 * + or via syntax 'version = X'
	 */
	std::vector <std::string> _activeVersion;

	std::set <std::string> _includeDir;

	bool _isDebug = false;

	bool _isVerbose = false;
	
    private :

	State ();

	State (const State & st);
	
	State & operator= (const State & st);
	
    public : 

	/**
	 * \return the only available instance of State
	 */
	static State & instance ();

	/**
	 * \brief Check if the version is active
	 * \param version the version to check
	 */
	bool isVersionActive (const std::string & version);

	/**
	 * \brief Activate a new version controller
	 * \param version the name of the version controller
	 */
	void activateVersion (const std::string & version);	

	/**
	 * \brief Add include dir path to the include sir possible includes -I options
	 * \param dir the directory to include
	 */
	void addIncludeDir (const std::string & dir);

	/**
	 * \brief If debug, activate the debug mode, desactivate it otherwise
	 */
	void activateDebug (bool debug);

	/**
	 * \brief If verb, activate the verbose mode, desactivate it otherwise
	 */
	void activateVerbose (bool verb);

	/**
	 * \brief tell if the debug mode is active (option -g or -ggdb)
	 */
	bool isDebugActive () const;
	
	/**
	 * \return tell if the verbose mode is active (option --verbose)
	 */
	bool isVerboseActive () const;

	/**
	 * \return the list of include directories
	 */
	const std::set <std::string> & getIncludeDirs () const ;
	
    };
   
}
