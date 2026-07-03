#ifndef APP_HPP
#define APP_HPP

#include "Crypto.hpp"
#include "Entry.hpp"
#include "FileManeger.hpp"
#include "Vault.hpp"
#include "TerminalEchoGuard.hpp"
#include "IUserInterface.hpp"
#include <cstring>

class App
{
private:
    IUserInterface& _ui;
    Crypto          _crypto;
    SecureBuffer    _masterPassword;
    SecureBuffer    _checkPassword;
    Vault           _vault;
    FileManeger     _fileManeger;
    std::string     _message;

    void    parseArgs(int argc, char *argv[]);
    void    add(void);
    void    show(void);
    void    del(void);
    bool    checkPassword(void);

public:
    App(IUserInterface& ui);
    ~App() = default;

    void    run(int argc, char *argv[]);
};


#endif
