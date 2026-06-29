#include "App.hpp"
#include "IUserInterface.hpp"

#include <utility>

App::App(IUserInterface& ui)
    :_ui(ui) {}

void    App::parseArgs(int argc, char *argv[])
{
    if (argc < 2)
        throw std::runtime_error("wrong number argument");

    _fileManeger.setPath(argv[1]);
}

void    App::add(void)
{
    Entry           entry;
    
    if (!_ui.askNewEntry(entry))
        return ;
    
    _vault.addEntry(std::move(entry));
}

void    App::show(void)
{
    _ui.showEntryList(_vault);
}

void    App::del(void)
{
    std::string input;
    size_t      index;

    _ui.showEntryList(_vault);
    if (!_ui.askEntryIndex(index, _vault))
        return;
    
    if (!_vault.removeEntry(index))
    {
        _ui.showError("invalid index");
        return;
    }
}

bool    App::checkPassword(void)
{
    if(_masterPassword.empty() || _checkPassword.empty())
        return false;

    if(_masterPassword.size() != _checkPassword.size())
        return false;

    for (size_t i = 0; i < _masterPassword.size(); i++)
    {
        if(_masterPassword.data()[i] != _checkPassword.data()[i])
            return false;
    }
    return true;
}

void    App::run(int argc, char *argv[])
{
    EncryptedData   data;
    SecureBuffer    plaintext;
    IUserInterface::MenuAction  menu;

    parseArgs(argc, argv);
    if (_fileManeger.ifExist())
    {
        bool    unlocked = false;
        data = _fileManeger.readEncrypted();

        for (size_t attempt = 0; attempt < 3 && !unlocked; attempt++)
        {
            _masterPassword.erase();
            plaintext.erase();

            _ui.askPassWord(_masterPassword, "please, put your password");
            if(_crypto.decrypt(plaintext, data, _masterPassword))
            {
                _vault.deserialize(plaintext);
                plaintext.erase();
                unlocked = true;
            }
            else
            {
                if (attempt + 1 == 3)
                    throw std::runtime_error("wrong password or corrupted file");

                _ui.showError("wrong password, try again");
            }
        }
    }
    else
    {
        bool    unlocked = false;

        for (size_t attempt = 0; attempt < 3 && !unlocked; attempt++)
        {
            _masterPassword.erase();
            _checkPassword.erase();

            _ui.askPassWord(_masterPassword, "please, put your password");
            _ui.askPassWord(_checkPassword, "\nplease, confirm your password");
            if (checkPassword())
                unlocked = true;
            else
                std::cerr << "\nbad password, try again\n" << std::endl;
            
            if (attempt + 1 == 3 && !unlocked)
            {
                _checkPassword.erase();
                _checkPassword.erase();
                throw std::runtime_error("bad password");
            }
        }
    }

    while(1)
    {
        menu = _ui.askMainMenuAction();        
        if(menu == IUserInterface::ACTION_ADD)
            add();
        else if(menu == IUserInterface::ACTION_SHOW)
            show();
        else if(menu == IUserInterface::ACTION_DELETE)
            del();
        else if(menu == IUserInterface::ACTION_EXIT)
            break;
        else
            std::cerr << "option not found" << std::endl;
    }
    _vault.serialize(plaintext);
    data = _crypto.encrypt(plaintext, _masterPassword);
    plaintext.erase();
    _fileManeger.writeEncrypted(data);
}
