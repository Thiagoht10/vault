#include "FileManeger.hpp"

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

FileManeger::FileManeger()
	: _fd(-1), _tmpFd(-1) {}

FileManeger::FileManeger(const std::string pathname)
	    :_path(pathname), _fd(-1), _tmpFd(-1) {}

FileManeger::~FileManeger()
{
	closeFile(&_fd);
}

EncryptedData	FileManeger::readEncrypted(void)
{
	EncryptedData	data;
	char			magic[5];
	std::size_t		saltSize;
	std::size_t		nonceSize;
	std::size_t		cipherSize;

	inOpen();

	readFull(magic, 5);
	if (std::string(magic, 5) != "VAULT")
		throw std::runtime_error("invalid vault file");

	readFull(reinterpret_cast<char*>(&data.version),
		sizeof(data.version));

	readFull(reinterpret_cast<char*>(&data.algorithm),
		sizeof(data.algorithm));

	readFull(reinterpret_cast<char*>(&data.opsLimit),
		sizeof(data.opsLimit));

	readFull(reinterpret_cast<char*>(&data.memLimit),
		sizeof(data.memLimit));

	if (data.version != 1)
		throw std::runtime_error("unsupported vault version");
	if (data.algorithm != crypto_pwhash_ALG_DEFAULT)
		throw std::runtime_error("unsupported pwhash algorithm");
	if (data.opsLimit != crypto_pwhash_OPSLIMIT_MODERATE)
		throw std::runtime_error("invalid ops limit");
	if (data.memLimit != crypto_pwhash_MEMLIMIT_MODERATE)
		throw std::runtime_error("invalid mem limit");

	readFull(reinterpret_cast<char*>(&saltSize),
		sizeof(saltSize));
	if (saltSize != crypto_pwhash_SALTBYTES)
		throw std::runtime_error("invalid salt size");
	data.salt.resize(saltSize);
	readFull(&data.salt[0], saltSize);

	readFull(reinterpret_cast<char*>(&nonceSize),
		sizeof(nonceSize));
	if (nonceSize != crypto_secretbox_NONCEBYTES)
		throw std::runtime_error("invalid nonce size");
	data.nonce.resize(nonceSize);
	readFull(&data.nonce[0], nonceSize);

	readFull(reinterpret_cast<char*>(&cipherSize),
		sizeof(cipherSize));
	if (cipherSize < crypto_secretbox_MACBYTES || cipherSize > MAX_VAULT_SIZE)
		throw std::runtime_error("invalid ciphertext size");
	data.ciphertext.resize(cipherSize);
	readFull(&data.ciphertext[0], cipherSize);

	return (data);
}

void	FileManeger::writeEncrypted(const EncryptedData& data)
{
	std::string		finalPath(_path);
	std::string		tmpPath = finalPath;
	const char		magic[5] = {'V', 'A', 'U', 'L', 'T'};
	std::size_t		saltSize;
	std::size_t		nonceSize;
	std::size_t		cipherSize;

	tmpPath += ".tmp";

	outOpen(tmpPath);

	saltSize = data.salt.size();
	nonceSize = data.nonce.size();
	cipherSize = data.ciphertext.size();

	writeFull(magic, 5);

	writeFull(reinterpret_cast<const char*>(&data.version),
		sizeof(data.version));

	writeFull(reinterpret_cast<const char*>(&data.algorithm),
		sizeof(data.algorithm));

	writeFull(reinterpret_cast<const char*>(&data.opsLimit),
		sizeof(data.opsLimit));

	writeFull(reinterpret_cast<const char*>(&data.memLimit),
		sizeof(data.memLimit));

	writeFull(reinterpret_cast<const char*>(&saltSize),
		sizeof(saltSize));
	writeFull(data.salt.data(), saltSize);

	writeFull(reinterpret_cast<const char*>(&nonceSize),
		sizeof(nonceSize));
	writeFull(data.nonce.data(), nonceSize);

	writeFull(reinterpret_cast<const char*>(&cipherSize),
		sizeof(cipherSize));
	writeFull(data.ciphertext.data(), cipherSize);

	syncFile(_tmpFd);
	closeFile(&_tmpFd);

	std::filesystem::rename(tmpPath, finalPath);
	syncDirectory(finalPath);
}

bool    FileManeger::ifExist(void) const
{
    return std::filesystem::exists(_path);
}

void	FileManeger::setPath(const std::string pathname)
{
	_path = pathname;
}

void	FileManeger::readFull(void *buffer, std::size_t size)
{
	char		*current;
	std::size_t	total;
	ssize_t		bytesRead;

	current = static_cast<char*>(buffer);
	total = 0;
	while (total < size)
	{
		bytesRead = read(_fd, current + total, size - total);
		if (bytesRead == 0)
			throw std::runtime_error("truncated vault file");
		if (bytesRead == -1)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("could not read file");
		}
		total += static_cast<std::size_t>(bytesRead);
	}
}

void	FileManeger::writeFull(const void *buffer, std::size_t size)
{
	const char	*current;
	std::size_t	total;
	ssize_t		bytesWritten;

	current = static_cast<const char*>(buffer);
	total = 0;
	while (total < size)
	{
		bytesWritten = write(_tmpFd, current + total, size - total);
		if (bytesWritten == -1)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("could not write file");
		}
		if (bytesWritten == 0)
			throw std::runtime_error("could not write file");
		total += static_cast<std::size_t>(bytesWritten);
	}
}

void	FileManeger::syncFile(int fd) const
{
	if (fd == -1)
		throw std::runtime_error("could not open file for sync");
	if (fsync(fd) == -1)
		throw std::runtime_error("could not sync file");
}

void	FileManeger::syncDirectory(const std::filesystem::path& path) const
{
	std::filesystem::path	dir = path.parent_path();
	int						fd;

	if (dir.empty())
		dir = ".";
	fd = open(dir.c_str(), O_RDONLY | O_DIRECTORY);
	if (fd == -1)
		throw std::runtime_error("could not open directory for sync");
	if (fsync(fd) == -1)
	{
		close(fd);
		throw std::runtime_error("could not sync directory");
	}
	close(fd);
}

void	FileManeger::inOpen(void)
{
	_fd = open(_path.c_str(), O_RDONLY);
	if (_fd == -1)
		throw std::runtime_error("could not open file");
}

void	FileManeger::outOpen(std::string& path)
{
	_tmpFd = open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);

	if (_tmpFd == -1)
	{
		if (errno == EEXIST)
			throw std::runtime_error("temporary file already exists");
		else
			throw std::runtime_error("could not open file");
	}
}

void	FileManeger::closeFile(int *fd)
{
	if (*fd != -1)
	{
		close(*fd);
		*fd = -1;
	}
}
