#ifndef APP_HPP
#define APP_HPP

#define _POSIX_C_SOURCE
#include "Crypto.hpp"
#include "Entry.hpp"
#include "FileManeger.hpp"
#include "Vault.hpp"
#include "TerminalEchoGuard.hpp"
#include "IUserInterface.hpp"
#include "Message.hpp"
#include "PasswordPolicy.hpp"
#include <cstring>
#include <signal.h>

class App
{
private:
    static volatile sig_atomic_t _signalReceived;

    IUserInterface& _ui;
    Crypto          _crypto;
    SecureBuffer    _masterPassword;
    SecureBuffer    _checkPassword;
    Vault           _vault;
    FileManeger     _fileManeger;
    PasswordPolicy  _policy;
    Message         _message;

    void parseArgs(int argc, char *argv[]);
    IUserInterface::InputResult add(void);
    IUserInterface::InputResult show(void);
    IUserInterface::InputResult del(void);
    IUserInterface::InputResult edit(void);
    IUserInterface::InputResult changeMasterPassword(void);
    void receiveMessage(const Message& message);
    bool checkMatchPassword(void);
    bool checkPolicyPassword(const SecureBuffer &pass) const;

    static void signalHandler(int sig);
    void    setupSignal(void);
    bool    shouldStop(void);
    bool    openVault(EncryptedData &data, SecureBuffer &plaintext);
    bool    createVault(void);

public:
    App(IUserInterface &ui);
    ~App();

    void run(int argc, char *argv[]);
};

#endif
