#include "Crypto.hpp"

Crypto::Crypto()
{
	if (sodium_init() < 0)
		throw std::runtime_error("libsodium initialization failed");
}

Crypto::~Crypto() {}

Crypto::SecureKey::SecureKey(void)
	: _bytes() {}

Crypto::SecureKey::~SecureKey()
{
	sodium_memzero(_bytes.data(), _bytes.size());
}

unsigned char*	Crypto::SecureKey::data(void)
{
	return _bytes.data();
}

std::size_t	Crypto::SecureKey::size(void) const
{
	return _bytes.size();
}

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

void	Crypto::deriveKey(SecureKey& key,
				const SecureBuffer& masterPassword,
				const std::string& salt, unsigned long long opsLimit,
				std::size_t memLimit, int algorithm) const
{
	if (crypto_pwhash(
			key.data(),
			key.size(),
			masterPassword.c_data(),
			masterPassword.size(),
			reinterpret_cast<const unsigned char*>(salt.data()),
			opsLimit,
			memLimit,
			algorithm) != 0)
	{
		throw std::runtime_error("failed to derive key");
	}
}

EncryptedData	Crypto::encrypt(const SecureBuffer& plaintext,
		    const SecureBuffer& masterPassword)
{
	EncryptedData	data;
	SecureKey		key;

	data.version = 1;
	data.algorithm = crypto_pwhash_ALG_DEFAULT;
	data.opsLimit = crypto_pwhash_OPSLIMIT_INTERACTIVE;
	data.memLimit = crypto_pwhash_MEMLIMIT_INTERACTIVE;

	data.salt = generateSalt();
	data.nonce = generateNonce();

	deriveKey(key,
			masterPassword,
			data.salt,
			data.opsLimit,
			data.memLimit,
			data.algorithm);

	data.ciphertext.resize(plaintext.size() + crypto_secretbox_MACBYTES);

	if (crypto_secretbox_easy(
				reinterpret_cast<unsigned char*>(&data.ciphertext[0]),
				plaintext.data(),
				plaintext.size(),
			reinterpret_cast<const unsigned char*>(data.nonce.data()),
			key.data()) != 0)
	{
		throw std::runtime_error("failed to encrypt data");
	}

	return data;
}

SecureBuffer	Crypto::decrypt(const EncryptedData& data,
		    const SecureBuffer& masterPassword)
{
	SecureKey		key;
	SecureBuffer	plaintext;

	if (data.salt.size() != crypto_pwhash_SALTBYTES)
		throw std::runtime_error("invalid salt size");

	if (data.nonce.size() != crypto_secretbox_NONCEBYTES)
		throw std::runtime_error("invalid nonce size");

	if (data.ciphertext.size() < crypto_secretbox_MACBYTES)
		throw std::runtime_error("invalid ciphertext size");

	deriveKey(key,
			masterPassword,
			data.salt,
			data.opsLimit,
			data.memLimit,
			data.algorithm);

	plaintext.resize(data.ciphertext.size() - crypto_secretbox_MACBYTES);

	if (crypto_secretbox_open_easy(
			reinterpret_cast<unsigned char*>(plaintext.data()),
			reinterpret_cast<const unsigned char*>(data.ciphertext.data()),
			data.ciphertext.size(),
			reinterpret_cast<const unsigned char*>(data.nonce.data()),
			key.data()) != 0)
	{
		throw std::runtime_error("wrong password or corrupted file");
	}

	return plaintext;
}
