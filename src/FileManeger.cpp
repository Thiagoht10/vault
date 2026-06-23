#include "FileManeger.hpp"

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

	file.read(reinterpret_cast<char*>(&saltSize),
		sizeof(saltSize));
	data.salt.resize(saltSize);
	file.read(&data.salt[0], saltSize);

	file.read(reinterpret_cast<char*>(&nonceSize),
		sizeof(nonceSize));
	data.nonce.resize(nonceSize);
	file.read(&data.nonce[0], nonceSize);

	file.read(reinterpret_cast<char*>(&cipherSize),
		sizeof(cipherSize));
	data.ciphertext.resize(cipherSize);
	file.read(&data.ciphertext[0], cipherSize);

	if (!file)
		throw std::runtime_error("corrupted vault file");

	return (data);
}

void	FileManeger::writeEncrypted(const EncryptedData& data)
{
	std::ofstream	file(_path, std::ios::binary);
	const char		magic[5] = {'V', 'A', 'U', 'L', 'T'};
	std::size_t		saltSize;
	std::size_t		nonceSize;
	std::size_t		cipherSize;

	if (!file)
		throw std::runtime_error("could not open file");

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
}

bool    FileManeger::ifExist(void) const
{
    return std::filesystem::exists(_path);
}
