#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <array>
#include <sodium.h>
#include <stdexcept>
#include <string>

struct EncryptedData
{
    int                 version;
    int                 algorithm;
    unsigned long long  opsLimit;
    std::size_t         memLimit;
    std::string         salt;
    std::string         nonce;
    std::string         ciphertext;
};

class Crypto
{
private:
    class SecureKey
    {
    private:
        std::array<unsigned char, crypto_secretbox_KEYBYTES> _bytes;

        SecureKey(const SecureKey& other);
        SecureKey&  operator=(const SecureKey& other);

    public:
        SecureKey(void);
        ~SecureKey();

        unsigned char*  data(void);
        std::size_t     size(void) const;
    };

    std::string generateSalt(void);
    std::string generateNonce(void);
    void	deriveKey(SecureKey& key, const std::string& masterPassword,
            const std::string& salt, unsigned long long opsLimit,
            std::size_t memLimit, int algorithm) const;

public:
    Crypto(void);
    ~Crypto();

    EncryptedData	encrypt(const std::string& plaintext,
	        const std::string& masterPassword);
    std::string	decrypt(const EncryptedData& data,
	        const std::string& masterPassword);
};


#endif
