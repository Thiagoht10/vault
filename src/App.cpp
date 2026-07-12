#include "App.hpp"
#include "IUserInterface.hpp"

#include <utility>

volatile sig_atomic_t App::_signalReceived = 0;

App::App(IUserInterface& ui)
    :_ui(ui) {}

App::~App()
{
    _fileManeger.closeLockFile();
}

void    App::parseArgs(int argc, char *argv[])
{
    if (argc < 2)
        throw std::runtime_error("wrong number argument");

    _fileManeger.setPath(argv[1]);
    _fileManeger.openLockFile();
}

IUserInterface::InputResult App::add(void)
{
    Entry                       entry;
    IUserInterface::InputOutcome outcome;

    outcome = _ui.askNewEntry(entry);
    receiveMessage(outcome.message);
    if (outcome.result != IUserInterface::INPUT_OK)
        return outcome.result;
    
    _vault.addEntry(std::move(entry));
    _message.set("credential added", Message::SUCCESS);
    return IUserInterface::INPUT_OK;
}

IUserInterface::InputResult App::show(void)
{
    size_t                      index;
    IUserInterface::InputOutcome outcome;

    if (_vault.size() == 0)
    {
        _message.set("vault is empty", Message::INFO);
        return IUserInterface::INPUT_OK;
    }
    _ui.showEntryList(_vault);
    outcome = _ui.askEntryIndex(index, _vault);
    receiveMessage(outcome.message);
    if (outcome.result != IUserInterface::INPUT_OK)
        return outcome.result;
    return _ui.showEntryTemporarily(_vault.getEntry(index));
}

IUserInterface::InputResult App::del(void)
{
    size_t                          index;
    IUserInterface::InputOutcome    outcome;
    IUserInterface::ConfirmationInput confirmation;

    _ui.showEntryList(_vault);
    outcome = _ui.askEntryIndex(index, _vault);
    receiveMessage(outcome.message);
    if (outcome.result != IUserInterface::INPUT_OK)
        return outcome.result;
    
    confirmation = _ui.askConfirmation(_vault.getEntry(index));
    if (confirmation.result != IUserInterface::INPUT_OK)
        return confirmation.result;
    if (confirmation.confirmed)
    {
        if (!_vault.removeEntry(index))
        {
            _message.set("invalid input", Message::ERROR);
            return IUserInterface::INPUT_INVALID;
        }
        _message.set("credential removed", Message::SUCCESS);
    }
    return IUserInterface::INPUT_OK;
}

IUserInterface::InputResult App::edit(void)
{
    size_t                              index;
    IUserInterface::InputOutcome        outcome;
    SecureBuffer                        password;
    SecureBuffer                        username;

    _ui.showEntryList(_vault);
    outcome = _ui.askEntryIndex(index, _vault);
    receiveMessage(outcome.message);
    if (outcome.result != IUserInterface::INPUT_OK)
        return outcome.result;

    outcome = _ui.askEditEntry(password, username);
    receiveMessage(outcome.message);
    if (outcome.result != IUserInterface::INPUT_OK)
        return outcome.result;

    if (!username.empty())
    {
        if (!_vault.editUsername(index, std::move(username)))
            return IUserInterface::INPUT_INVALID;
    }

    if (!_vault.editPassword(index, std::move(password)))
        return IUserInterface::INPUT_INVALID;

    _message.set("credential updated", Message::SUCCESS);
    password.erase();
    username.erase();
    return IUserInterface::INPUT_OK;
}

IUserInterface::InputResult App::changeMasterPassword(void)
{
    SecureBuffer    password;
    SecureBuffer    confirmPassword;
    IUserInterface::InputResult result;

    result = _ui.askPassWord(password, "put your actual master password");
    if (result != IUserInterface::INPUT_OK)
        return result;

    if (password != _masterPassword)
    {
        _message.set("wrong password", Message::ERROR);
        return IUserInterface::INPUT_INVALID;
    }
    
    result = _ui.askPassWord(password, "put your new master password");
    if (result != IUserInterface::INPUT_OK)
        return result;

    if (!checkPolicyPassword(password))
        return IUserInterface::INPUT_INVALID;
    
    result = _ui.askPassWord(confirmPassword, "confirm your new password");
    if (result != IUserInterface::INPUT_OK)
        return result;

    if (password != confirmPassword)
    {
        _message.set("passwords do not match", Message::ERROR);
        return IUserInterface::INPUT_INVALID;
    }

    _masterPassword = std::move(password);
    _message.set("master password updated", Message::SUCCESS);
    confirmPassword.erase();
    return IUserInterface::INPUT_OK;
}

void    App::receiveMessage(const Message& message)
{
    if (!message.empty())
        _message = message;
}

bool    App::checkMatchPassword(void)
{
    if (_masterPassword != _checkPassword)
        return false;

    return true;
}

bool    App::checkPolicyPassword(const SecureBuffer& pass) const
{
    PasswordPolicy::Result result = _policy.checkPasswordPolity(pass);

    switch (result)
    {
        case PasswordPolicy::PW_OK:
            return true;
        case PasswordPolicy::PW_EMPTY:
            _ui.showError("password cannot be empty");
            return false;
        case PasswordPolicy::PW_TOO_COMMON:
            _ui.showError("password is too common");
            return false;
        case PasswordPolicy::PW_TOO_SHORT:
            _ui.showError("password must be at least 15 characters");
            return false;
        case PasswordPolicy::PW_TOO_LONG:
            _ui.showError("password must be at most 64 characters");
            return false;
        case PasswordPolicy::PW_REPEAT_CHAR:
            _ui.showError("password cannot contain 3 repeated characters in a row");
            return false;
        case PasswordPolicy::PW_SEQUENCE_CHAR:
            _ui.showError("password cannot contain a 4-character ascending or descending sequence");
            return false;
    }
    return false;
}

void    App::signalHandler(int sig)
{
    _signalReceived = sig;
}

void    App::setupSignal(void)
{
    struct sigaction    sa;

    sa.sa_handler = App::signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

bool    App::shouldStop(void)
{
    return (_signalReceived != 0);
}

bool    App::openVault(EncryptedData& data, SecureBuffer& plaintext)
{
    IUserInterface::InputResult passwordInput;
    
    bool    unlocked = false;
    data = _fileManeger.readEncrypted();

    for (size_t attempt = 0; attempt < 3 && !unlocked; attempt++)
    {
        _masterPassword.erase();
        plaintext.erase();
        passwordInput = _ui.askPassWord(_masterPassword,
                "please, put your password");
        if (passwordInput == IUserInterface::INPUT_INTERRUPTED
                || shouldStop())
            return false;
        if (_crypto.decrypt(plaintext, data, _masterPassword))
        {
            _vault.deserialize(plaintext);
            plaintext.erase();
            unlocked = true;
        }
        else
        {
            if (attempt + 1 == 3)
            {
                _masterPassword.erase();
                _checkPassword.erase();
                throw std::runtime_error("wrong password or corrupted file");
            }
            _ui.showError("wrong password, try again");
        }
    }
    return true;
}

bool    App::createVault(void)
{
    IUserInterface::InputResult passwordInput;
    bool    unlocked = false;

    for (size_t attempt = 0; attempt < 3 && !unlocked; attempt++)
    {
        _masterPassword.erase();
        _checkPassword.erase();

        passwordInput = _ui.askPassWord(_masterPassword,
                "please, put your password");
        if(passwordInput == IUserInterface::INPUT_INTERRUPTED
                || shouldStop())
            return false;

        passwordInput = _ui.askPassWord(_checkPassword,
                "\nplease, confirm your password");
        if(passwordInput == IUserInterface::INPUT_INTERRUPTED
                || shouldStop())
            return false;
        
        if (checkPolicyPassword(_masterPassword))
        {
            if (checkMatchPassword())
                unlocked = true;
            else
                _ui.showError("bad password, try again");
        }
        
        if (attempt + 1 == 3 && !unlocked)
        {
            _masterPassword.erase();
            _checkPassword.erase();
            throw std::runtime_error("bad password");
        }
    }
    return true;
}

void    App::run(int argc, char *argv[])
{
    EncryptedData   data;
    SecureBuffer    plaintext;
    IUserInterface::MenuInput   input;
    IUserInterface::InputResult actionResult;

    setupSignal();
    parseArgs(argc, argv);
    
    if (_fileManeger.ifExist())
    {
        if (!openVault(data, plaintext))
            return;
    }
    else
    {
        if (!createVault())
            return;
    }

    while(!shouldStop())
    {
        actionResult = IUserInterface::INPUT_OK;
        input = _ui.askMainMenuAction(_message);
        _message.clear();
        if (input.result == IUserInterface::INPUT_INTERRUPTED)
            break;
        if(input.action == IUserInterface::ACTION_ADD)
            actionResult = add();
        else if(input.action == IUserInterface::ACTION_SHOW)
            actionResult = show();
        else if(input.action == IUserInterface::ACTION_DELETE)
            actionResult = del();
        else if (input.action == IUserInterface::ACTION_EDIT)
            actionResult = edit();
        else if(input.action == IUserInterface::ACTION_CHANGE_PASSWORD)
            actionResult = changeMasterPassword();
        else if(input.action == IUserInterface::ACTION_EXIT)
            break;
        else
        {
            if(!shouldStop())
                _message.set("option not found", Message::ERROR);
        }
        if (actionResult == IUserInterface::INPUT_INTERRUPTED)
            break;
    }
    _vault.serialize(plaintext);
    data = _crypto.encrypt(plaintext, _masterPassword);
    plaintext.erase();
    _fileManeger.writeEncrypted(data);
}
