#ifndef FILE_MANEGER_HPP
#define FILE_MANEGER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Crypto.hpp"

class FileManeger
{
private:
    std::string _path;

public:
    FileManeger(void);
    FileManeger(const std::string pathname);
    FileManeger(const FileManeger& other);
    FileManeger&    operator=(const FileManeger& other);
    ~FileManeger();

    EncryptedData	readEncrypted(void) const;
    void    writeEncrypted(const EncryptedData& data);
    bool    ifExist(void) const;
    void    setPath(const std::string pathname);
};


#endif