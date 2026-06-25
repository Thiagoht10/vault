#ifndef APP_HPP
#define APP_HPP

#include "Crypto.hpp"
#include "Entry.hpp"
#include "FileManeger.hpp"
#include "Vault.hpp"
#include "TerminalEchoGuard.hpp"

class App
{
private:
    std::string _masterPassword;
    std::string _checkPassword;
    Crypto      _crypto;
    Vault       _vault;
    FileManeger _fileManeger;
    std::string _option;

    void    parseArgs(int argc, char *argv[]);
    void    add(void);
    void    show(void);
    void    del(void);
    void    erasePassword(void);
    std::string readHiddenInput(std::string prompt);
    bool    checkPassword(const std::string& pass1, const std::string& pass2);

public:
    App(void);
    ~App();

    void    run(int argc, char *argv[]);
};


#endif