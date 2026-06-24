# Vault

Gerenciador de senhas executado no terminal e escrito em C++. As credenciais são mantidas em memória durante a execução e salvas em um arquivo binário criptografado com a biblioteca [libsodium](https://doc.libsodium.org/).

> Este é um projeto de estudo. Antes de utilizá-lo para armazenar credenciais reais, consulte a seção [Limitações de segurança](#limitações-de-segurança).

## Funcionalidades

- adicionar uma credencial com serviço, usuário e senha;
- listar as credenciais armazenadas;
- remover uma credencial pelo índice;
- carregar um cofre criptografado existente;
- salvar o cofre automaticamente ao encerrar pelo menu;
- detectar senha incorreta ou arquivo corrompido durante a descriptografia;
- rejeitar dados serializados que não estejam no formato esperado;
- ocultar a senha mestra durante a digitação;
- sobrescrever buffers sensíveis conhecidos antes de liberá-los.

## Requisitos

- compilador C++ com suporte a `std::filesystem`;
- terminal compatível com POSIX `termios`;
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

O programa solicita a senha mestra sem exibi-la durante a digitação e apresenta o menu:

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

`Vault::serialize()` calcula o tamanho necessário, reserva o buffer uma única vez e acrescenta os campos diretamente nele. `Vault::deserialize()` percorre esse mesmo buffer por posições, sem criar uma cópia completa em `stringstream` nem usar `substr()` para cada campo.

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

O projeto centraliza a limpeza de strings em `secureErase()`. Essa função usa `sodium_memzero()` para sobrescrever os bytes atuais e depois chama `clear()` para deixar a string logicamente vazia.

`SecureEraseGuard` aplica essa limpeza por RAII: seu destrutor é executado ao final do escopo, inclusive durante o tratamento de exceções. Ele protege os dados temporários coletados ao adicionar uma entrada e o texto completo produzido pela descriptografia e serialização.

A senha mestra é limpa no destrutor de `App`. Cada credencial é limpa por `Entry::~Entry()`, chamado automaticamente pelo `std::vector` quando uma entrada é removida ou quando o `Vault` é destruído.

As chaves derivadas usam a classe privada `Crypto::SecureKey`, que contém um buffer fixo, não pode ser copiada e executa `sodium_memzero()` no destrutor. Assim, a chave é apagada tanto no fluxo normal quanto se ocorrer uma exceção.

### Leitura da senha mestra

`App::readHiddenInput()` usa `TerminalEchoGuard` para desativar temporariamente o eco do terminal antes de ler a senha mestra. O guard salva as configurações originais e as restaura em seu destrutor, inclusive quando a leitura lança uma exceção.

Essa proteção é aplicada à senha mestra solicitada na inicialização. Os demais campos inseridos pelo menu continuam visíveis durante a digitação.

### Movimentação das entradas

`Entry` e `Vault` não podem ser copiados. Uma entrada é transferida para o `Vault` com semântica de movimento:

```text
Entry temporário -> std::move -> std::vector<Entry>
```

O construtor e o operador de movimento são `noexcept`, permitindo que o vetor mova entradas durante realocações em vez de copiar as credenciais. Antes de remover ou sobrescrever uma entrada, seus campos são apagados com `secureErase()`.

## Organização do projeto

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

- `App`: controla argumentos, senha mestra e menu interativo;
- `Entry`: representa uma credencial;
- `Vault`: mantém as credenciais e converte entre objetos e texto;
- `Crypto`: deriva a chave, criptografa e descriptografa;
- `FileManeger`: lê e escreve o formato binário do cofre.
- `SecureMemory`: fornece limpeza com `sodium_memzero()` e proteção RAII para strings sensíveis.
- `TerminalEchoGuard`: desativa temporariamente o eco do terminal e restaura sua configuração por RAII.

## Limitações de segurança

- as senhas das credenciais adicionadas pelo menu ainda são exibidas durante a digitação;
- a opção `show` imprime todas as senhas em texto legível;
- senha mestra, credenciais e texto serializado ainda utilizam `std::string`; `sodium_memzero()` apaga o buffer atual, mas não recupera buffers antigos que uma realocação da string possa ter abandonado;
- os dados sensíveis ainda não usam memória protegida por `sodium_malloc()` ou `sodium_mlock()`, podendo ser afetados por swap ou core dumps;
- o formato binário utiliza tipos e representação nativos da máquina, portanto ainda não é portátil entre todas as arquiteturas;
- não existe confirmação da senha ao criar um cofre novo;
- não há bloqueio contra tentativas repetidas de senha.

Use o projeto como implementação educacional, não como substituto de um gerenciador de senhas auditado.
