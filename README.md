# Vault

A terminal-based password manager written in C++. Credentials are kept in memory while the program is running and saved to a binary file encrypted with the [libsodium](https://doc.libsodium.org/) library.

> This is an educational project. Before using it to store real credentials, read the [Security limitations](#security-limitations) section.

## Features

- add a credential with a service, username, and password;
- list stored credential indexes and service names;
- reveal a selected credential temporarily in the terminal;
- delete a credential by index;
- load an existing encrypted vault;
- save the vault automatically when exiting through the menu;
- preserve completed changes when input is interrupted after an action;
- retry password input up to three times when opening or creating a vault;
- detect an incorrect password or corrupted file during decryption;
- reject truncated or malformed encrypted vault files before using incomplete metadata;
- reject serialized plaintext data that does not match the expected format;
- prevent two cooperative `vault` processes from opening the same vault file at the same time;
- enforce a basic master-password policy when creating a new vault;
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

The program prompts for the master password without displaying it while it is being entered. When creating a new vault, the password must pass the basic password policy, then be typed a second time for confirmation. After initialization, the program shows the menu:

```text
1. add
2. show
3. delete
0. exit
```

Option `2` lists the available indexes and service names, asks which entry should be shown, and then displays the selected credential temporarily with the password masked. The user can choose whether to reveal the password. The terminal UI uses the terminal alternate screen for this view: pressing Enter after a reveal returns to the previous screen so the credential is not left visible in the normal terminal scrollback.

Option `3` lists the available indexes and service names, then asks which entry should be deleted. Typing `/cancel` returns to the main menu without deleting anything.

If the specified file already exists, it is read and decrypted with the master password. The program allows up to three password attempts. A different password, or changes to the encrypted bytes, causes retry messages first and then the `wrong password or corrupted file` error after the final failed attempt.

If the file does not exist, a new vault is created after a confirmed master password is accepted. The master password must be between 15 and 64 bytes and must not match the small built-in list of common passwords. The password and confirmation must match. Invalid input can be retried up to three times; after the final failed attempt, the program exits with `bad password`.

If terminal input is interrupted, the current unfinished input operation is cancelled. A partially filled new credential is not added to the vault. If a previous action has already completed, the application leaves the main loop and persists the current vault state before exiting.

When a vault is opened or created, the program also opens a lock file next to it using the same path plus `.lock`. For example:

```text
my_vault.vault
my_vault.vault.lock
```

The `.lock` file is used with `flock()` to reject a second cooperative `vault` process while the first one is still running. The file can remain on disk after the program exits; the active lock is tied to the open file descriptor, not to the mere existence of the `.lock` file.

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

`FileManeger::readEncrypted()` and `FileManeger::writeEncrypted()` use POSIX file descriptors for the encrypted vault format. Reads go through `readFull()`, which keeps reading until the requested field is complete and rejects truncated files before the field is used. Writes go through `writeFull()`, which keeps writing until the requested buffer is complete or reports an error.

`FileManeger::writeEncrypted()` writes encrypted data to a temporary file next to the target vault. The temporary file uses the target path plus `.tmp` and is created with POSIX `open()` using `O_CREAT | O_EXCL` and mode `0600`, so it is owner-only from creation and an existing temporary file is not overwritten. After the encrypted bytes are fully written, the file descriptor is synchronized with `fsync()`, closed, moved into place with `std::filesystem::rename()`, and the parent directory is synchronized so the rename is persisted more reliably after crashes or power loss.

### File locking

`FileManeger::openLockFile()` creates or opens a lock file using the vault path plus `.lock`, then applies an exclusive non-blocking `flock()` lock to that file. The lock file is opened with mode `0600`, and the file descriptor stays open while the application is using the vault. If another cooperative `vault` process already holds the lock, the second process exits with a lock error instead of reading or writing the same vault concurrently.

`FileManeger::closeLockFile()` releases the lock and closes the lock file descriptor when the application exits. The `.lock` file itself is not removed automatically, because removing lock files can create races between processes. A leftover `.lock` file is normal and does not mean the vault is still locked.

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

The project uses `crypto_pwhash` with libsodium's `MODERATE` operation and memory limits for key derivation, and `crypto_secretbox_easy`/`crypto_secretbox_open_easy` for authenticated encryption.

When reading an encrypted file, `FileManeger::readEncrypted()` validates the binary metadata before variable-sized encrypted blocks are allocated. It accepts only vault format version `1`, the password-hashing parameters currently written by `Crypto::encrypt()`, the expected salt and nonce sizes, and a ciphertext size between the secretbox MAC size and `MAX_VAULT_SIZE` (`10 MiB`). Truncated files and malformed metadata are rejected before the program tries to derive a key or allocate the ciphertext buffer.

### Secure buffers and clearing

The project uses `SecureBuffer` for the master password, credential passwords, and the serialized plaintext produced before encryption or after decryption. `SecureBuffer` owns its memory, cannot be copied, supports move semantics, allocates its storage with `sodium_malloc()`, tries to lock it with `sodium_mlock()`, overwrites it with `sodium_memzero()`, and releases it with `sodium_free()`.

`SecureBuffer` tracks two different sizes: the number of bytes currently used and the allocated capacity. Binary data must always be written with an explicit length through methods such as `assign(data, length)` and `append(data, length)`. The class no longer provides an overload that accepts a raw `unsigned char*` without a size, because that would require searching for `'\0'` to guess the length.

The buffer still keeps a trailing `'\0'` after the used bytes so text-oriented helpers can work when the content is textual. `c_data()` exists for display or APIs that require a `const char*`, and `append(const char*)` is intended for trusted C-string literals such as serialization labels. Cryptographic operations and sensitive byte copies use `data()` together with `size()`.

`SecureString` wraps `SecureBuffer` for text fields such as service and username. This keeps the credential fields move-only and gives them the same explicit erase behavior used by the password buffers, while still allowing textual input and display through controlled accessors.

The master password is cleared by the `SecureBuffer` destructor when `App` is destroyed. Credential fields are cleared by `SecureBuffer` through `SecureString` and the password buffer when an `Entry` is erased, removed, overwritten, or destroyed by the `std::vector`.

Derived keys use the private `Crypto::SecureKey` class, which stores the key bytes in a `SecureBuffer`, cannot be copied, and therefore uses the same protected allocation and explicit clearing path as the other sensitive buffers.

Sensitive buffer comparisons use `sodium_memcmp()` through `SecureBuffer::operator==()`. Password confirmation paths compare `SecureBuffer` values instead of manually checking byte by byte.

### Master password policy

`PasswordPolicy` is applied when creating a new vault. It rejects empty passwords, passwords shorter than 15 bytes, passwords longer than 64 bytes, passwords that exactly match a small built-in list of common weak values such as `password`, `123456`, `qwerty`, `admin`, and `senha123`, passwords with 3 repeated characters in a row, and passwords with a 4-character ascending or descending sequence.

The policy is intentionally basic. It does not require character-composition rules such as uppercase letters, numbers, or symbols, and it does not estimate entropy or check leaked-password databases. Existing vaults are still opened by authentication against the encrypted file, and stored credential passwords are not forced through this master-password policy.

### Reading passwords

`ConsoleUI::readHiddenInput()` uses `TerminalEchoGuard` to temporarily disable terminal echo before reading passwords. The guard saves the original settings and restores them in its destructor, including when reading throws an exception.

This protection is applied to the master password requested during initialization, to the confirmation prompt used when creating a new vault, and to credential passwords added through option `1`. Credential passwords must also be typed twice and both entries must match. Service and username remain visible while they are being typed.

### Showing credentials

`ConsoleUI::showEntryTemporarily()` displays a selected entry on the terminal alternate screen with the password masked, then asks whether the password should be revealed. This is a visual protection: when the user presses Enter after a reveal, the program leaves the alternate screen and returns to the previous menu view. In normal terminal use, this avoids leaving the revealed username and password in the main scrollback.

Credential fields are written to the terminal with explicit byte lengths from the stored `Entry`, instead of relying on C-string termination. This avoids truncating a stored field at the first `'\0'` byte when it is intentionally revealed.

This does not make terminal output a secure storage channel. Once a password is written to the terminal, the terminal emulator, screen capture tools, session recorders, or other external components may still have seen it. The reveal flow should therefore be treated as temporary display, not as guaranteed secure deletion from the terminal.

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
│   ├── ConsoleUI.hpp
│   ├── Crypto.hpp
│   ├── Entry.hpp
│   ├── FileManeger.hpp
│   ├── IUserInterface.hpp
│   ├── PasswordPolicy.hpp
│   ├── SecureBuffer.hpp
│   ├── SecureString.hpp
│   ├── TerminalEchoGuard.hpp
│   └── Vault.hpp
├── src/
│   ├── App.cpp
│   ├── ConsoleUI.cpp
│   ├── Crypto.cpp
│   ├── Entry.cpp
│   ├── FileManeger.cpp
│   ├── PasswordPolicy.cpp
│   ├── SecureBuffer.cpp
│   ├── SecureString.cpp
│   ├── TerminalEchoGuard.cpp
│   └── Vault.cpp
├── main.cpp
└── Makefile
```

- `App`: handles arguments, the master password, vault loading/saving, and the application flow;
- `IUserInterface`: defines the UI operations used by `App`;
- `ConsoleUI`: implements the current terminal prompts, hidden password input, menus, list rendering, and temporary credential reveal;
- `Entry`: represents a credential using secure storage for service, username, and password;
- `Vault`: stores credentials, exposes read-only access for UI rendering, and converts between objects and serialized text;
- `Crypto`: derives the key, encrypts, and decrypts;
- `FileManeger`: reads and writes the vault's binary format;
- `PasswordPolicy`: validates the master password when creating a new vault;
- `SecureBuffer`: owns sensitive byte buffers and erases them with `sodium_memzero()`;
- `SecureString`: stores text fields on top of `SecureBuffer`;
- `TerminalEchoGuard`: temporarily disables terminal echo and restores its configuration through RAII.

## Security limitations

These points are security boundaries of the current implementation:

- service names are shown as readable text in the terminal when listing entries, so service names should not be treated as hidden metadata;
- option `2` can reveal usernames and passwords in the terminal; the alternate screen hides them from the normal scrollback in typical terminal use, but it cannot guarantee secure deletion from the terminal emulator or external capture tools;
- file locking uses advisory `flock()` semantics, so it prevents concurrent access only between cooperative processes that follow the same lock protocol;
- `SecureBuffer` uses `sodium_malloc()` and `sodium_mlock()`, but this cannot protect secrets after they are intentionally written to the terminal or copied by external system components;
- the master-password policy is basic and local-only, so protection against offline brute-force still depends heavily on the master password strength plus the `crypto_pwhash` cost.

## Operational notes

These points are expected behavior or format constraints, not security vulnerabilities:

- lock files use the vault path plus `.lock` and may remain on disk after the program exits; this is expected and does not mean the vault is still locked;
- vault writes use a fixed `.tmp` path while saving; if a write fails before `rename()`, the `.tmp` file may remain and must be removed before the next save attempt;
- encrypted file metadata is validated before variable-sized allocations, but the binary format stores native integer representations and is intentionally tied to the current format version and KDF settings;
- the binary format uses the machine's native types and representation, so it is not portable across all architectures.

Use this project as an educational implementation, not as a replacement for an audited password manager.
