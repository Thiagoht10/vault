#include "Crypto.hpp"

Crypto::Crypto()
{
	if (sodium_init() < 0)
		throw std::runtime_error("libsodium initialization failed");
}

Crypto::~Crypto() {}

std::string Crypto::generateSalt(void)
{
    std::string salt;

    salt.resize(crypto_pwhash_SALTBYTES);
    randombytes_buf(&salt[0], salt.size());

    return salt;
}

std::string Crypto::generateNonce(void)
{
    std::string nonce;

    nonce.resize(crypto_secretbox_NONCEBYTES);
    randombytes_buf(&nonce[0], nonce.size());

    return nonce;
}

std::string	Crypto::deriveKey(const std::string& masterPassword,
		const std::string& salt, unsigned long long opsLimit,
		std::size_t memLimit, int algorithm) const
{
	std::string	key;

	key.resize(crypto_secretbox_KEYBYTES);
	if (crypto_pwhash(
			reinterpret_cast<unsigned char*>(&key[0]),
			key.size(),
			masterPassword.c_str(),
			masterPassword.size(),
			reinterpret_cast<const unsigned char*>(salt.data()),
			opsLimit,
			memLimit,
			algorithm) != 0)
	{
		throw std::runtime_error("failed to derive key");
	}
	return key;
}

EncryptedData	Crypto::encrypt(const std::string& plaintext,
	    const std::string& masterPassword)
{
	EncryptedData	data;
	std::string		key;

	data.version = 1;
	data.algorithm = crypto_pwhash_ALG_DEFAULT;
	data.opsLimit = crypto_pwhash_OPSLIMIT_INTERACTIVE;
	data.memLimit = crypto_pwhash_MEMLIMIT_INTERACTIVE;

	data.salt = generateSalt();
	data.nonce = generateNonce();

	key = deriveKey(masterPassword,
			data.salt,
			data.opsLimit,
			data.memLimit,
			data.algorithm);

	data.ciphertext.resize(plaintext.size() + crypto_secretbox_MACBYTES);

	if (crypto_secretbox_easy(
			reinterpret_cast<unsigned char*>(&data.ciphertext[0]),
			reinterpret_cast<const unsigned char*>(plaintext.data()),
			plaintext.size(),
			reinterpret_cast<const unsigned char*>(data.nonce.data()),
			reinterpret_cast<const unsigned char*>(key.data())) != 0)
	{
		sodium_memzero(&key[0], key.size());
		throw std::runtime_error("failed to encrypt data");
	}

	sodium_memzero(&key[0], key.size());
	return data;
}

std::string	Crypto::decrypt(const EncryptedData& data,
	    const std::string& masterPassword)
{
	std::string	key;
	std::string	plaintext;

	if (data.salt.size() != crypto_pwhash_SALTBYTES)
		throw std::runtime_error("invalid salt size");

	if (data.nonce.size() != crypto_secretbox_NONCEBYTES)
		throw std::runtime_error("invalid nonce size");

	if (data.ciphertext.size() < crypto_secretbox_MACBYTES)
		throw std::runtime_error("invalid ciphertext size");

	key = deriveKey(masterPassword,
			data.salt,
			data.opsLimit,
			data.memLimit,
			data.algorithm);

	plaintext.resize(data.ciphertext.size() - crypto_secretbox_MACBYTES);

	if (crypto_secretbox_open_easy(
			reinterpret_cast<unsigned char*>(&plaintext[0]),
			reinterpret_cast<const unsigned char*>(data.ciphertext.data()),
			data.ciphertext.size(),
			reinterpret_cast<const unsigned char*>(data.nonce.data()),
			reinterpret_cast<const unsigned char*>(key.data())) != 0)
	{
		sodium_memzero(&key[0], key.size());
		throw std::runtime_error("wrong password or corrupted file");
	}

	sodium_memzero(&key[0], key.size());
	return plaintext;
}
