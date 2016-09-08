#pragma once

#include "config.h"
#include "system.h"
#include "input.h"

#include "Token.hh"
#include <vector>
#include <map>

namespace Lexical {

  /**
     Cette classe permet d'analyser un fichier et de renvoyer le Token associe a chaque element
     Elle est l'element principale de l'analyse lexical
   */
  struct Lexer {

    Lexer (const char * filename, FILE * input);

    TokenPtr next ();

    void rewind (unsigned long nb = 1);

    void setSkip (std::vector<std::string> skip);

    void setComments (std::vector <std::pair<std::string, std::string> > comments);

    void setComments (bool on);
    
    ~Lexer ();

  private:

    TokenPtr get ();
    
    location_t getCurrentLocation ();

    TokenId isComment (TokenPtr);

    bool isSkip (TokenPtr);

    std::string readln ();
    
  private:

    static const int maxColumnHint = 80;
    int currentLine, currentColumn;    
    bool commentOn = true;
    std::vector <std::string> skip;
    std::vector <std::pair <std::string, std::string> > comments;
    std::vector <TokenPtr> read;
    unsigned long currentWord;
    FILE * input;

    const struct line_map * line_map;
    
  };
  
};
