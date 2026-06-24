# Vault

A terminal-based password manager written in C++. Credentials are kept in memory while the program is running and saved to a binary file encrypted with the [libsodium](https://doc.libsodium.org/) library.

> This is an educational project. Before using it to store real credentials, read the [Security limitations](#security-limitations) section.

## Features

- add a credential with a service, username, and password;
- list stored credentials;
- delete a credential by index;
- load an existing encrypted vault;
- save the vault automatically when exiting through the menu;
- detect an incorrect password or corrupted file during decryption;
- reject serialized data that does not match the expected format;
- hide the master password while it is being entered;
- overwrite known sensitive buffers before releasing them.

## Requirements

- a C++ compiler with `std::filesystem` support;
- a terminal compatible with POSIX `termios`;
- `make`;
- libsodium and its development files.

On Debian- or Ubuntu-based distributions, the dependencies can be installed with:

```sh
sudo apt install build-essential libsodium-dev
```

## Building

```sh
make
```

This command generates the `vault` executable in the project root.

Other available targets:

```sh
make clean   # remove object files
make fclean  # remove object files and the executable
make re      # rebuild the entire project
```

## Usage

Pass the path of the file that will store the vault as an argument:

```sh
./vault my_vault.vault
```

The program prompts for the master password without displaying it while it is being entered, then shows the menu:

```text
1. add
2. show
3. delete
4. exit
```

If the specified file already exists, it is read and decrypted with the master password. If it does not exist, a new vault is created when the program is closed using option `4`.

The same file must later be opened with the same password. A different password—or changes to the encrypted bytes—causes the `wrong password or corrupted file` error.

## How it works

When a vault is opened, the data follows this flow:

```text
binary file
    -> FileManeger::readEncrypted()
    -> EncryptedData
    -> Crypto::decrypt()
    -> serialized text
    -> Vault::deserialize()
    -> list of Entry objects
```

When the program exits, the process is reversed:

```text
list of Entry objects
    -> Vault::serialize()
    -> serialized text
    -> Crypto::encrypt()
    -> EncryptedData
    -> FileManeger::writeEncrypted()
    -> binary file
```

`Vault::serialize()` calculates the required size, allocates the buffer only once, and appends the fields directly to it. `Vault::deserialize()` traverses the same buffer by position, without creating a complete copy in a `stringstream` or using `substr()` for each field.

### `EncryptedData` structure

`EncryptedData` groups the encrypted content and the parameters required to recover it:

| Field | Purpose |
| --- | --- |
| `version` | identifies the format version |
| `algorithm` | algorithm used to derive the key |
| `opsLimit` | computational cost of key derivation |
| `memLimit` | memory used during key derivation |
| `salt` | random value used with the master password |
| `nonce` | unique value used in the cryptographic operation |
| `ciphertext` | encrypted and authenticated credentials |

The `salt` and `nonce` are not passwords and can be stored in the file. The master password is not written to disk. It is used with the `salt` to derive the encryption key temporarily.

The project uses `crypto_pwhash` for key derivation and `crypto_secretbox_easy`/`crypto_secretbox_open_easy` for authenticated encryption.

### Clearing sensitive data

The project centralizes string clearing in `secureErase()`. This function uses `sodium_memzero()` to overwrite the current bytes and then calls `clear()` to leave the string logically empty.

`SecureEraseGuard` applies this clearing through RAII: its destructor runs when the scope ends, including during exception handling. It protects the temporary data collected when an entry is added and the complete text produced by decryption and serialization.

The master password is cleared in the `App` destructor. Each credential is cleared by `Entry::~Entry()`, which is called automatically by the `std::vector` when an entry is removed or the `Vault` is destroyed.

Derived keys use the private `Crypto::SecureKey` class, which contains a fixed-size buffer, cannot be copied, and calls `sodium_memzero()` in its destructor. This ensures that the key is erased both during normal execution and when an exception occurs.

### Reading the master password

`App::readHiddenInput()` uses `TerminalEchoGuard` to temporarily disable terminal echo before reading the master password. The guard saves the original settings and restores them in its destructor, including when reading throws an exception.

This protection is applied to the master password requested during initialization. The other fields entered through the menu remain visible while they are being typed.

### Moving entries

`Entry` and `Vault` cannot be copied. An entry is transferred to the `Vault` using move semantics:

```text
temporary Entry -> std::move -> std::vector<Entry>
```

The move constructor and move assignment operator are `noexcept`, allowing the vector to move entries during reallocations instead of copying the credentials. Before an entry is removed or overwritten, its fields are erased with `secureErase()`.

## Project structure

```text
.
├── includes/
│   ├── App.hpp
│   ├── Crypto.hpp
│   ├── Entry.hpp
│   ├── FileManeger.hpp
│   ├── SecureMemory.hpp
│   ├── TerminalEchoGuard.hpp
│   └── Vault.hpp
├── src/
│   ├── App.cpp
│   ├── Crypto.cpp
│   ├── Entry.cpp
│   ├── FileManeger.cpp
│   ├── SecureMemory.cpp
│   ├── TerminalEchoGuard.cpp
│   └── Vault.cpp
├── main.cpp
└── Makefile
```

- `App`: handles arguments, the master password, and the interactive menu;
- `Entry`: represents a credential;
- `Vault`: stores credentials and converts between objects and text;
- `Crypto`: derives the key, encrypts, and decrypts;
- `FileManeger`: reads and writes the vault's binary format;
- `SecureMemory`: provides clearing with `sodium_memzero()` and RAII protection for sensitive strings;
- `TerminalEchoGuard`: temporarily disables terminal echo and restores its configuration through RAII.

## Security limitations

- passwords for credentials added through the menu are still displayed while they are being entered;
- the `show` option prints all passwords as readable text;
- the master password, credentials, and serialized text still use `std::string`; `sodium_memzero()` erases the current buffer but cannot recover old buffers that a string reallocation may have abandoned;
- sensitive data does not yet use memory protected by `sodium_malloc()` or `sodium_mlock()` and may therefore be affected by swap or core dumps;
- the binary format uses the machine's native types and representation, so it is not portable across all architectures;
- there is no password confirmation when creating a new vault;
- there is no protection against repeated password attempts.

Use this project as an educational implementation, not as a replacement for an audited password manager.
