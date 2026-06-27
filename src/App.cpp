#include "App.hpp"

#include <utility>

App::App(void) {}

App::~App() 
{
    //erasePassword();
}

void    App::parseArgs(int argc, char *argv[])
{
    if (argc < 2)
        throw std::runtime_error("wrong number argument");

    _fileManeger.setPath(argv[1]);
}

void    App::add(void)
{
    Entry           entry;
    SecureBuffer    password;
    SecureBuffer    checkPass;
    SecureString    tmp;
    bool            cancel = false;

    std::cout << "\n" << "------------------" << "\n" << std::endl;
    std::cout << "please, insert datas:" << std::endl;
    std::cout << "type /cancel to cancel\n" << std::endl;

    while(1)
    {
        std::cout << "service: \n" << "> ";
        tmp.erase();
        tmp.readBytes();
        if(tmp.empty())
        {
            std::cout << "can't have empyt inputs\n" << std::endl;
            continue ;
        }
        if(tmp == "/cancel")
        {
            cancel = true;
            break;
        }
        entry.setService(tmp.data());
        tmp.erase();
        break;
    }

    while (1)
    {
        if (cancel)
            break;
        std::cout << "username: \n" << "> ";
        tmp.erase();
        tmp.readBytes();
        if (tmp.empty())
        {
            std::cout << "can't have empyt inputs\n" << std::endl;
            continue ;
        }
        if(tmp == "/cancel")
        {
            cancel = true;
            break;
        }
        entry.setUsername(tmp.data());
        tmp.erase();
        break;
    }
    
	while (1)
	{
        if (cancel)
            break;
        readHiddenInput(password, "password:");
        if(password == "/cancel")
        {
            cancel = true;
            break;
        }
        readHiddenInput(checkPass, "\nplease, confirm your password");
        if(checkPass == "/cancel")
        {
            cancel = true;
            break;
        }
        if(password != checkPass)
        {
            std::cout << "\npassword not match" << std::endl;
            cancel = true;
            break;
        }
        if (password.empty())
        {
            std::cout << "can't have empyt inputs\n" << std::endl;
            continue ;
        }
        entry.setPassword(password.data(), password.size());
        password.erase();
        checkPass.erase();
        break;
	}
    if (!cancel)
        _vault.addEntry(std::move(entry));
}

void    App::show(void)
{
    _vault.printAll();
}

void    App::del(void)
{
    std::string input;
    size_t      index;
    std::stringstream   ss;

    while (1)
    {
        std::cout << "please, select an index" << std::endl;
        std::cout << "type /cancel to cancel\n" << std::endl;

        if (!std::getline(std::cin, input))
            return;
        if(input == "/cancel")
            return;
        ss << input;
        for (int i = 0; input[i]; i++)
        {
            if(input[i] < '0' || input[i] > '9')
            {
                std::cout << "invalid index\n" << std::endl;
                continue;
            }
        }
        ss >> index;
        _vault.removeEntry(index);
        break;
    }
}

void App::readHiddenInput(SecureBuffer& pass, std::string prompt)
{
    std::cout << prompt << "\n" << "> ";

    TerminalEchoGuard guard;

    pass.readBytes();
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
    EncryptedData data;
    SecureBuffer plaintext;

    parseArgs(argc, argv);
    if (_fileManeger.ifExist())
    {
        readHiddenInput(_masterPassword, "please, put your password");
        data = _fileManeger.readEncrypted();
        plaintext = _crypto.decrypt(data, _masterPassword);
        _vault.deserialize(plaintext);
        plaintext.erase();
    }
    else
    {
        readHiddenInput(_masterPassword, "please, put your password");
        readHiddenInput(_checkPassword, "\nplease, confirm your password");
        if (!checkPassword())
        {
            _checkPassword.erase();
            throw std::runtime_error("bad password");
        }
        _checkPassword.erase();
    }

    while(1)
    {
        std::cout << "\n" << "------------------" << "\n" << std::endl;
        std::cout << "\nselect one option\n" << std::endl;
        std::cout << "1. add" << std::endl;
        std::cout << "2. show" << std::endl;
        std::cout << "3. delete" << std::endl;
        std::cout << "4. exit\n" << "\n> ";

        if (!std::getline(std::cin, _option))
        {
            throw std::runtime_error("failure in insert of dates");
        }
        
        if(_option == "1")
            add();
        else if(_option == "2")
            show();
        else if(_option == "3")
            del();
        else if(_option == "4")
            break;
        else
            std::cout << "option not found" << std::endl;
    }
    _vault.serialize(plaintext);
    data = _crypto.encrypt(plaintext, _masterPassword);
    plaintext.erase();
    _fileManeger.writeEncrypted(data);
}
