#include "FileManeger.hpp"

#include <fcntl.h>
#include <unistd.h>

FileManeger::FileManeger() {}

FileManeger::FileManeger(const std::string pathname) 
    :_path(pathname) {}

FileManeger::FileManeger(const FileManeger& other)
    :_path(other._path) {}

FileManeger&    FileManeger::operator=(const FileManeger& other)
{
    if(this != &other)
        _path = other._path;
    
    return *this;
}

FileManeger::~FileManeger() {}

EncryptedData	FileManeger::readEncrypted(void) const
{
	EncryptedData	data;
	std::ifstream	file(_path, std::ios::binary);
	char			magic[5];
	std::size_t		saltSize;
	std::size_t		nonceSize;
	std::size_t		cipherSize;

	if (!file)
		throw std::runtime_error("could not open file");

	file.read(magic, 5);
	if (!file || std::string(magic, 5) != "VAULT")
		throw std::runtime_error("invalid vault file");

	file.read(reinterpret_cast<char*>(&data.version),
		sizeof(data.version));

	file.read(reinterpret_cast<char*>(&data.algorithm),
		sizeof(data.algorithm));

	file.read(reinterpret_cast<char*>(&data.opsLimit),
		sizeof(data.opsLimit));

	file.read(reinterpret_cast<char*>(&data.memLimit),
		sizeof(data.memLimit));
	if (!file)
		throw std::runtime_error("corrupted vault file");
	if (data.version != 1)
		throw std::runtime_error("unsupported vault version");
	if (data.algorithm != crypto_pwhash_ALG_DEFAULT)
		throw std::runtime_error("unsupported pwhash algorithm");
	if (data.opsLimit != crypto_pwhash_OPSLIMIT_MODERATE)
		throw std::runtime_error("invalid ops limit");
	if (data.memLimit != crypto_pwhash_MEMLIMIT_MODERATE)
		throw std::runtime_error("invalid mem limit");

	file.read(reinterpret_cast<char*>(&saltSize),
		sizeof(saltSize));
	if (!file)
		throw std::runtime_error("corrupted vault file");
	if (saltSize != crypto_pwhash_SALTBYTES)
		throw std::runtime_error("invalid salt size");
	data.salt.resize(saltSize);
	file.read(&data.salt[0], saltSize);

	file.read(reinterpret_cast<char*>(&nonceSize),
		sizeof(nonceSize));
	if (!file)
		throw std::runtime_error("corrupted vault file");
	if (nonceSize != crypto_secretbox_NONCEBYTES)
		throw std::runtime_error("invalid nonce size");
	data.nonce.resize(nonceSize);
	file.read(&data.nonce[0], nonceSize);

	file.read(reinterpret_cast<char*>(&cipherSize),
		sizeof(cipherSize));
	if (!file)
		throw std::runtime_error("corrupted vault file");
	if (cipherSize < crypto_secretbox_MACBYTES || cipherSize > MAX_VAULT_SIZE)
		throw std::runtime_error("invalid ciphertext size");
	data.ciphertext.resize(cipherSize);
	file.read(&data.ciphertext[0], cipherSize);

	if (!file)
		throw std::runtime_error("corrupted vault file");

	return (data);
}

void	FileManeger::writeEncrypted(const EncryptedData& data)
{
	std::filesystem::path	finalPath(_path);
	std::filesystem::path	tmpPath = finalPath;
	std::ofstream			file;
	const char		magic[5] = {'V', 'A', 'U', 'L', 'T'};
	std::size_t		saltSize;
	std::size_t		nonceSize;
	std::size_t		cipherSize;

	tmpPath += ".tmp";
	createTempFile(tmpPath);

	openFile(file, tmpPath);

	saltSize = data.salt.size();
	nonceSize = data.nonce.size();
	cipherSize = data.ciphertext.size();

	file.write(magic, 5);

	file.write(reinterpret_cast<const char*>(&data.version),
		sizeof(data.version));

	file.write(reinterpret_cast<const char*>(&data.algorithm),
		sizeof(data.algorithm));

	file.write(reinterpret_cast<const char*>(&data.opsLimit),
		sizeof(data.opsLimit));

	file.write(reinterpret_cast<const char*>(&data.memLimit),
		sizeof(data.memLimit));

	file.write(reinterpret_cast<const char*>(&saltSize),
		sizeof(saltSize));
	file.write(data.salt.data(), saltSize);

	file.write(reinterpret_cast<const char*>(&nonceSize),
		sizeof(nonceSize));
	file.write(data.nonce.data(), nonceSize);

	file.write(reinterpret_cast<const char*>(&cipherSize),
		sizeof(cipherSize));
	file.write(data.ciphertext.data(), cipherSize);

	file.close();
	if (!file)
		throw std::runtime_error("could not write file");
	syncFile(tmpPath);

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

void	FileManeger::createTempFile(const std::filesystem::path& path) const
{
	int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);

	if (fd == -1)
	{
		if (errno == EEXIST)
			throw std::runtime_error("temporary file already exists");
		else
			throw std::runtime_error("could not open file");
	}
	close(fd);
}

void	FileManeger::openFile(std::ofstream& file,
			const std::filesystem::path& path) const
{
	file.open(path, std::ios::binary | std::ios::trunc);

	if (!file)
		throw std::runtime_error("could not open file");

	std::filesystem::permissions(path,
			std::filesystem::perms::owner_write |
			std::filesystem::perms::owner_read,
			std::filesystem::perm_options::replace);
}

void	FileManeger::syncFile(const std::filesystem::path& path) const
{
	int	fd = open(path.c_str(), O_RDONLY);

	if (fd == -1)
		throw std::runtime_error("could not open file for sync");
	if (fsync(fd) == -1)
	{
		close(fd);
		throw std::runtime_error("could not sync file");
	}
	close(fd);
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
