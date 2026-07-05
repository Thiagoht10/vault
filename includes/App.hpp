#ifndef APP_HPP
#define APP_HPP

#define _POSIX_C_SOURCE
#include "Crypto.hpp"
#include "Entry.hpp"
#include "FileManeger.hpp"
#include "Vault.hpp"
#include "TerminalEchoGuard.hpp"
#include "IUserInterface.hpp"
#include <cstring>
#include <signal.h>

class App
{
private:
    static volatile sig_atomic_t    _signalReceived;

    IUserInterface& _ui;
    Crypto          _crypto;
    SecureBuffer    _masterPassword;
    SecureBuffer    _checkPassword;
    Vault           _vault;
    FileManeger     _fileManeger;
    std::string     _message;

    void    parseArgs(int argc, char *argv[]);
    IUserInterface::InputResult add(void);
    IUserInterface::InputResult show(void);
    IUserInterface::InputResult del(void);
    bool    checkPassword(void);

    static void     signalHandler(int sig);
    void            setupSignal(void);
    bool            shouldStop(void);

public:
    App(IUserInterface& ui);
    ~App() = default;

    void    run(int argc, char *argv[]);
};


#endif
