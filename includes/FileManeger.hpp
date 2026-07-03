#ifndef FILE_MANEGER_HPP
#define FILE_MANEGER_HPP

#include <cstddef>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Crypto.hpp"

static const std::size_t MAX_VAULT_SIZE = 10 * 1024 * 1024;

class FileManeger
{
private:
    std::string _path;

    void    createTempFile(const std::filesystem::path& path) const;
    void    openFile(std::ofstream& file, const std::filesystem::path& path) const;
    void    syncFile(const std::filesystem::path& path) const;
    void    syncDirectory(const std::filesystem::path& path) const;

public:
    FileManeger(void);
    FileManeger(const std::string pathname);
    FileManeger(const FileManeger& other);
    FileManeger&    operator=(const FileManeger& other);
    ~FileManeger();

    EncryptedData	readEncrypted(void) const;
    void            writeEncrypted(const EncryptedData& data);
    bool            ifExist(void) const;
    void            setPath(const std::string pathname);
};


#endif
