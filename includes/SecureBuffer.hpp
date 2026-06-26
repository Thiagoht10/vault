#ifndef SECURE_BUFFER_HPP
#define SECURE_BUFFER_HPP

#include <cstddef>
#include <stdexcept>
#include <sodium.h>
#include <iostream>
#include <string>
#include <utility>

class SecureBuffer
{
private:
	unsigned char*  _buffer;
	std::size_t     _sizeUsed;
	std::size_t     _capacity;
	std::size_t     _increaseFactor;

    void    increaseSpace(void);
    void    createSpace(std::size_t value);
    std::size_t size(const unsigned char* data) const;
    unsigned char* copy(const unsigned char* data);
    void    eraseBuffer(unsigned char* data);
    void    clear(unsigned char* data, std::size_t length);

public:
    SecureBuffer(void);
    SecureBuffer(const SecureBuffer& other) = delete;
    SecureBuffer&   operator=(const SecureBuffer& other) = delete;
    SecureBuffer(SecureBuffer&& other) noexcept;
    SecureBuffer&   operator=(SecureBuffer&& other) noexcept;
    ~SecureBuffer();

	std::size_t             size(void) const;
	void                    erase(void);
	const char*             c_data(void) const;
	const unsigned char*    data(void) const;
	unsigned char*          data(void);
	void                    readBytes(void);
	void                    assign(const unsigned char* data);
	void                    assign(const unsigned char* data, std::size_t length);
	void                    assign(const std::string& data);
	void                    swap(SecureBuffer& other) noexcept;
	void                    reserve(size_t value);
	void                    resize(size_t value);
	bool                    empty(void) const;
	int                     compare(size_t position, size_t lenght, const char* data) const;
	size_t                  find(char character, size_t position) const;
	size_t                  max_size(void) const;
	void                    append(const char* str);
	void                    append(const char* str, std::size_t length);

};


#endif
