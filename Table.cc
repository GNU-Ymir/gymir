#include "semantic/pack/Table.hh"

namespace semantic {

    Table Table::__instance__;

    Table::Table () :
	_templateScope (""),
	_space (""),
	_programSpace ("")	
    {}

    void Table::enterBlock () {
    }

    void Table::quitBlock () {
    }

    void Table::setCurrentSpace (Namespace space, std::string name) {
    }

    void Table::resetCurrentSpace (Namespace space) {
    }

    void Table::addCall (Word sym) {
    }

    void Table::addGlobal (syntax::Global gl) {
    }

    std::vector <syntax::Global> Table::globalVars () {
    }

    void Table::enterFrame (Namespace space, std::string name, bool internal) {
    }

    void Table::quitFrame () {
    }

    Namespace Table::space () {
    }

    Namespace Table::globalNamespace () {
    }

    Namespace& Table::programNamespace () {
    }

    Namespace& Table::templateNamespace () {
    }

    void Table::insert (Symbol info) {
    }

    void Table::addStaticInit (syntax::Instruction exp) {
    }

    std::vector <syntax::Instruction> Table::staticInits () {
    }

    void Table::purge () {
    }

    Symbol Table::get (std::string name) {
    }

    std::vector <Symbol> Table::getAll (std::string name) {
    }

    Symbol Table::getLocal (std::string name) {
    }

    Symbol Table::local (std::string name) {
    }

    bool Table::sameFrame (Symbol sym) {
    }

    FrameReturnInfo& Table::retInfo () {
    }

    Module Table::addModule (Namespace space) {
    }

    void Table::addForeignModule (Namespace space) {
    }

    std::vector <Module> getAllMod (Namespace space) {
    }

    bool Table::isModule (Namespace space) {
    }

    void Table::openModuleForSpace (Namespace from, Namespace to) {
    }

    void Table::closeModuleForSpace (Namespace from, Namespace to) {
    }

    std::vector <Namespace> Table::modules () {
    }

    std::vector <Namespace> Table::modulesAndForeigns () {
    }

    bool Table::moduleExists (Namespace name) {
    }

    ulong Table::nbRecursive () {
    }
    
}
