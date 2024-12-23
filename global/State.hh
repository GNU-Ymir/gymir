#pragma once

#include <vector>
#include <string>
#include <set>
#include <cstdint>

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

	std::string __includeInPrefix__ = "include/ymir/";
	
	std::string _corePath;
	
	std::string _prefixPath;

	std::string _executable;

	std::string _ouputDir = "./";
	
	bool _isDebug = false;

	bool _isVerbose = false;

	bool _isStandalone = false;

	bool _isDumpDoc = false;

	bool _isTrusted = true;

	bool _includeTest = false;

	bool _enableReflect = true;
	
	bool _isDumpDependency = false;

	// Option -M[M]D
	bool _depFilename;

	// Option -M
	bool _deps;	

	// Option -MMD
	bool _depSkip;
	
	// Option -MF <arg>
	std::string _depFilenameUser;

	// Option -M[QT] <arg>
	std::vector<std::string> _depTargets;
	
	// -MP
	bool _depPhony;

	// The number of byte of the size type (arch dependent)
	uint32_t _sizeOfSize;
	
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
       * \returns: the list of active versions
       */
      const std::vector <std::string> &getActiveVersions () const;
      
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
	 * \brief If stand, do not include core files and linkage
	 */
	void activateStandalone (bool stand);

	/**
	 * \brief If act, dump the documentation
	 */
	void activateDocDumping (bool act);

	/**
	 * \brief If act, dump the dependency
	 */
	void activateDependencyDumping (bool act);
	
	/**
	 * \brief If act, the compiler while compile trusted blocks
	 */
	void activeTrusted (bool act);

	/**
	 * \brief If act, the compiler include testing
	 */
	void activateIncludeTesting (bool act);

	/**
	 * If act, the compiler include reflection symbols
	 */
	void activateReflection (bool act);
	
	/**
	 * \brief tell if the debug mode is active (option -g or -ggdb)
	 */
	bool isDebugActive () const;
	
	/**
	 * \return tell if the verbose mode is active (option --verbose)
	 */
	bool isVerboseActive () const;

	/**
	 * \return tell if we need to include core files
	 */
	bool isStandalone () const;

	/**
	 * \return tell if we need to dump the doc
	 */
	bool isDocDumpingActive () const;

	/**
	 * \return tell if we nned to dump the dependencies
	 */
	bool isDependencyDumpingActive () const;

	/**
	 * \return the option -M[M]... are actives
	 */
	bool isGCCDependencyActive () const;

	/**
	 * Option -MM
	 */
	bool isGCCDepSkipActive () const;

	/**
	 * Option -MD
	 */
	bool isGCCDepFilenameActive () const;

	/**
	 * Option -MF
	 */
	const std::string& getGCCDepFilenameUser () const;

	/**
	 * Option -M[QT]
	 */
	const std::vector<std::string>& getGCCDepTargets () const;

	/**
	 * Option -MP
	 */
	bool getGCCDepPhony () const ;
	
	/**
	 * \brief False, if the option --no-trusted is activated, true otherwise
	 */
	bool isTrustedActive () const;

	/**
	 * \brief true if the option -funittest is active
	 */
	bool isIncludeTesting () const;

	/**
	 * \brief true if the option --enable-reflect is active
	 */
	bool isEnableReflect () const;

	/**
	 * \brief Change the size of the size type
	 */
	void setSizeType (uint32_t size);

	/**
	 * \brief Get the size of the size type
	 */
	uint32_t getSizeType () const;	
	
	/**
	 * \brief Set the prefix path dir
	 * \brief set the core dir, and std include path
	 */
	void setPrefix (const std::string & path);

	/**
	 * \brief Set the executable path
	 * \change the prefix path, Cf setPrefix
	 */
	void setExecutable (const std::string & executable);

	/**
	 * Option -M
	 */
	void activateGCCDeps ();

	/**
	 * Option -MM
	 */
	void activateGCCDepSkip ();
	
	/**
	 * Set the dependency filename
	 * Option -M[M]D
	 */
	void activateGCCDepFilename ();

	/**
	 * Option -MF <arg>
	 */
	void setGCCDepFilenameUser (const std::string & depFilename);

	/**
	 * Add a dependency target
	 */
	void addGCCDepTarget (const std::string & target, bool quoted);

	/**
	 * Option -MP
	 */
	void setGCCDepPhony (bool);
	
	/**
	 * Change the output directory (used for documentation dumping)
	 */
	void setOutputDir (const std::string & dir);
	
	/**
	 * \return the prefix path
	 */
	const std::string & getPrefix () const;

	/**
	 * \return the path of the core directory
	 */
	const std::string & getCorePath () const;
	
	/**
	 * \return the list of include directories
	 */
	const std::set <std::string> & getIncludeDirs () const ;

	/**
	 * @return the output directory
	 */
	const std::string & getOutputDir () const;
	

    };
   
}
