#ifndef APP_HPP
#define APP_HPP

#include "Crypto.hpp"
#include "Entry.hpp"
#include "FileManeger.hpp"
#include "Vault.hpp"
#include "TerminalEchoGuard.hpp"
#include <cstring>

class App
{
private:
    SecureBuffer _masterPassword;
    SecureBuffer _checkPassword;
    Crypto      _crypto;
    Vault       _vault;
    FileManeger _fileManeger;
    std::string _option;

    void    parseArgs(int argc, char *argv[]);
    void    add(void);
    void    show(void);
    void    del(void);
    //void    erasePassword(void);
    void    readHiddenInput(SecureBuffer& pass, std::string prompt);
    bool    checkPassword(void);

public:
    App(void);
    ~App();

    void    run(int argc, char *argv[]);
};


#endif