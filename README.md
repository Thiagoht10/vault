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
- require password confirmation when creating a new vault;
- hide and confirm credential passwords while they are being entered;
- keep the master password, credential fields, and serialized vault text in secure buffers;
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

The program prompts for the master password without displaying it while it is being entered. When creating a new vault, the password must be typed twice and both entries must match. After initialization, the program shows the menu:

```text
1. add
2. show
3. delete
0. exit
```

Option `2` first lists only the available indexes and service names. The full credential, including username and password, is printed only after selecting a specific index.

If the specified file already exists, it is read and decrypted with the master password. If it does not exist, a new vault is created when the confirmed master password is accepted and the program is closed using option `0`. A missing, empty, or mismatched confirmation causes the `bad password` error.

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

`Vault::serialize()` writes to a `SecureBuffer`. It calculates the required size, reserves the buffer, and appends the fields directly to it. `Vault::deserialize()` traverses the same buffer by position, without creating a complete copy in a `stringstream` or using `substr()` for each field.

`FileManeger::writeEncrypted()` writes the encrypted file and then replaces its permissions with owner-only read/write access. On POSIX-style systems this corresponds to mode `0600`, so the vault file is not left readable by group or other users after a normal save.

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

### Secure buffers and clearing

The project uses `SecureBuffer` for the master password, credential passwords, and the serialized plaintext produced before encryption or after decryption. `SecureBuffer` owns its memory, cannot be copied, supports move semantics, and overwrites its allocated bytes with `sodium_memzero()` before releasing them.

`SecureBuffer` tracks two different sizes: the number of bytes currently used and the allocated capacity. Binary data must always be written with an explicit length through methods such as `assign(data, length)` and `append(data, length)`. The class no longer provides an overload that accepts a raw `unsigned char*` without a size, because that would require searching for `'\0'` to guess the length.

The buffer still keeps a trailing `'\0'` after the used bytes so text-oriented helpers can work when the content is textual. `c_data()` exists for display or APIs that require a `const char*`, and `append(const char*)` is intended for trusted C-string literals such as serialization labels. Cryptographic operations and sensitive byte copies use `data()` together with `size()`.

`SecureString` wraps `SecureBuffer` for text fields such as service and username. This keeps the credential fields move-only and gives them the same explicit erase behavior used by the password buffers, while still allowing textual input and display through controlled accessors.

The master password is cleared by the `SecureBuffer` destructor when `App` is destroyed. Credential fields are cleared by `SecureBuffer` through `SecureString` and the password buffer when an `Entry` is erased, removed, overwritten, or destroyed by the `std::vector`.

Derived keys use the private `Crypto::SecureKey` class, which contains a fixed-size buffer, cannot be copied, and calls `sodium_memzero()` in its destructor. This ensures that the key is erased both during normal execution and when an exception occurs.

Password comparisons currently use normal byte-by-byte comparisons. They are sufficient for the current local CLI flow, but they are not constant-time. If this project is hardened further, sensitive buffer comparisons should use a constant-time primitive such as `sodium_memcmp()`.

### Reading passwords

`App::readHiddenInput()` uses `TerminalEchoGuard` to temporarily disable terminal echo before reading passwords. The guard saves the original settings and restores them in its destructor, including when reading throws an exception.

This protection is applied to the master password requested during initialization, to the confirmation prompt used when creating a new vault, and to credential passwords added through option `1`. Credential passwords must also be typed twice and both entries must match. Service and username remain visible while they are being typed.

### Moving entries

`Entry` and `Vault` cannot be copied. An entry is transferred to the `Vault` using move semantics:

```text
temporary Entry -> std::move -> std::vector<Entry>
```

The move constructor and move assignment operator are `noexcept`, allowing the vector to move entries during reallocations instead of copying the credentials. Before an entry is removed or overwritten, its secure fields are explicitly erased.

## Project structure

```text
.
├── includes/
│   ├── App.hpp
│   ├── Crypto.hpp
│   ├── Entry.hpp
│   ├── FileManeger.hpp
│   ├── SecureBuffer.hpp
│   ├── SecureString.hpp
│   ├── TerminalEchoGuard.hpp
│   └── Vault.hpp
├── src/
│   ├── App.cpp
│   ├── Crypto.cpp
│   ├── Entry.cpp
│   ├── FileManeger.cpp
│   ├── SecureBuffer.cpp
│   ├── SecureString.cpp
│   ├── TerminalEchoGuard.cpp
│   └── Vault.cpp
├── main.cpp
└── Makefile
```

- `App`: handles arguments, the master password, and the interactive menu;
- `Entry`: represents a credential using secure storage for service, username, and password;
- `Vault`: stores credentials and converts between objects and text;
- `Crypto`: derives the key, encrypts, and decrypts;
- `FileManeger`: reads and writes the vault's binary format;
- `SecureBuffer`: owns sensitive byte buffers and erases them with `sodium_memzero()`;
- `SecureString`: stores text fields on top of `SecureBuffer`;
- `TerminalEchoGuard`: temporarily disables terminal echo and restores its configuration through RAII.

## Security limitations

- selecting a credential through the `show` option still prints that password as readable text;
- file permissions are applied after writing the encrypted data, so a stronger implementation would create the file with restrictive permissions from the start;
- vault writes are not atomic, so an interruption during `FileManeger::writeEncrypted()` can leave a corrupted file;
- the binary reader must treat malformed files carefully because size fields are stored in the file and are used to allocate buffers;
- sensitive data does not yet use memory protected by `sodium_malloc()` or `sodium_mlock()` and may therefore be affected by swap or core dumps;
- sensitive buffer comparisons are not constant-time yet;
- the binary format uses the machine's native types and representation, so it is not portable across all architectures;
- there is no protection against repeated password attempts.

Use this project as an educational implementation, not as a replacement for an audited password manager.
