# Vault

Gerenciador de senhas executado no terminal e escrito em C++. As credenciais são mantidas em memória durante a execução e salvas em um arquivo binário criptografado com a biblioteca [libsodium](https://doc.libsodium.org/).

> Este é um projeto de estudo. Antes de utilizá-lo para armazenar credenciais reais, consulte a seção [Limitações de segurança](#limitações-de-segurança).

## Funcionalidades

- adicionar uma credencial com serviço, usuário e senha;
- listar as credenciais armazenadas;
- remover uma credencial pelo índice;
- carregar um cofre criptografado existente;
- salvar o cofre automaticamente ao encerrar pelo menu;
- detectar senha incorreta ou arquivo corrompido durante a descriptografia.

## Requisitos

- compilador C++ com suporte a `std::filesystem`;
- `make`;
- libsodium e seus arquivos de desenvolvimento.

Em distribuições baseadas em Debian ou Ubuntu, as dependências podem ser instaladas com:

```sh
sudo apt install build-essential libsodium-dev
```

## Compilação

```sh
make
```

O comando gera o executável `vault` na raiz do projeto.

Outros alvos disponíveis:

```sh
make clean   # remove os arquivos objeto
make fclean  # remove os objetos e o executável
make re      # recompila todo o projeto
```

## Uso

Passe como argumento o caminho do arquivo que armazenará o cofre:

```sh
./vault meu_cofre.vault
```

O programa solicita a senha mestra e apresenta o menu:

```text
1. add
2. show
3. delete
4. exit
```

Se o arquivo informado já existir, ele será lido e descriptografado com a senha mestra. Se não existir, um cofre novo será criado quando o programa for encerrado pela opção `4`.

O mesmo arquivo deve ser aberto posteriormente com a mesma senha. Uma senha diferente — ou alterações nos bytes criptografados — causa o erro `wrong password or corrupted file`.

## Como funciona

Ao abrir um cofre, os dados percorrem este fluxo:

```text
arquivo binário
    -> FileManeger::readEncrypted()
    -> EncryptedData
    -> Crypto::decrypt()
    -> texto serializado
    -> Vault::deserialize()
    -> lista de Entry
```

Ao encerrar, o processo é invertido:

```text
lista de Entry
    -> Vault::serialize()
    -> texto serializado
    -> Crypto::encrypt()
    -> EncryptedData
    -> FileManeger::writeEncrypted()
    -> arquivo binário
```

### Estrutura `EncryptedData`

`EncryptedData` agrupa o conteúdo criptografado e os parâmetros necessários para recuperá-lo:

| Campo | Finalidade |
| --- | --- |
| `version` | identifica a versão do formato |
| `algorithm` | algoritmo usado para derivar a chave |
| `opsLimit` | custo computacional da derivação da chave |
| `memLimit` | memória usada na derivação da chave |
| `salt` | valor aleatório usado com a senha mestra |
| `nonce` | valor único usado na operação criptográfica |
| `ciphertext` | credenciais criptografadas e autenticadas |

O `salt` e o `nonce` não são senhas e podem ser armazenados no arquivo. A senha mestra não é gravada. Ela é usada com o `salt` para derivar temporariamente a chave de criptografia.

O projeto utiliza `crypto_pwhash` para derivação da chave e `crypto_secretbox_easy`/`crypto_secretbox_open_easy` para criptografia autenticada.

### Limpeza de dados sensíveis

As chaves derivadas usadas durante a criptografia e a descriptografia são sobrescritas com `sodium_memzero` assim que deixam de ser necessárias.

Depois que o cofre é salvo com sucesso, o programa também chama `_masterPassword.clear()`. Isso deixa a `std::string` vazia, impedindo seu uso posterior pelo programa, mas não garante que os bytes anteriores sejam imediatamente sobrescritos na memória. Se ocorrer uma exceção antes do final do salvamento, essa chamada não é alcançada.

## Organização do projeto

```text
.
├── includes/
│   ├── App.hpp
│   ├── Crypto.hpp
│   ├── Entry.hpp
│   ├── FileManeger.hpp
│   └── Vault.hpp
├── src/
│   ├── App.cpp
│   ├── Crypto.cpp
│   ├── Entry.cpp
│   ├── FileManeger.cpp
│   └── Vault.cpp
├── main.cpp
└── Makefile
```

- `App`: controla argumentos, senha mestra e menu interativo;
- `Entry`: representa uma credencial;
- `Vault`: mantém as credenciais e converte entre objetos e texto;
- `Crypto`: deriva a chave, criptografa e descriptografa;
- `FileManeger`: lê e escreve o formato binário do cofre.

## Limitações de segurança

- a senha mestra é exibida enquanto é digitada no terminal;
- a opção `show` imprime todas as senhas em texto legível;
- as chaves derivadas são apagadas com `sodium_memzero`, mas credenciais, texto serializado e senha mestra ainda passam por objetos `std::string` sem garantia de sobrescrita segura;
- `_masterPassword.clear()` é executado somente após o salvamento normal; caminhos de erro ou exceção podem encerrar o programa antes dessa limpeza;
- o formato binário utiliza tipos e representação nativos da máquina, portanto ainda não é portátil entre todas as arquiteturas;
- não existe confirmação da senha ao criar um cofre novo;
- não há bloqueio contra tentativas repetidas de senha.

Use o projeto como implementação educacional, não como substituto de um gerenciador de senhas auditado.
