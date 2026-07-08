#ifndef FILE_MANEGER_HPP
#define FILE_MANEGER_HPP

#include <cstddef>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <sys/file.h>
#include "Crypto.hpp"

static const std::size_t MAX_VAULT_SIZE = 10 * 1024 * 1024;

class FileManeger
{
private:
    std::string _path;
    int         _fd;
    int         _tmpFd;
    int         _lockFd;

    void    syncFile(int fd) const;
    void    syncDirectory(const std::filesystem::path& path) const;
    void    inOpen(void);
    void    outOpen(std::string& path);
    void    closeFile(int *fd);
    void    readFull(void *buffer, std::size_t size);
    void    writeFull(const void *buffer, std::size_t size);

public:
    FileManeger(void);
    FileManeger(const std::string pathname);
    FileManeger(const FileManeger& other) = delete;
    FileManeger&    operator=(const FileManeger& other) = delete;
    ~FileManeger();

    EncryptedData	readEncrypted(void);
    void            writeEncrypted(const EncryptedData& data);
    bool            ifExist(void) const;
    void            setPath(const std::string pathname);
    void            openLockFile(void);
    void            closeLockFile(void);
};


#endif
