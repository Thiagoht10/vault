#!/bin/bash

BIN="./vault"
OUT="out.txt"
VAULT="vault.test"
MASTER="correct-password1"

make re > /dev/null

printf "%s\n%s\n0\n" "$MASTER" "$MASTER" \
    | script -q -c "$BIN $VAULT" /dev/null > "$OUT"

if [ -f "$VAULT" ]; then
    echo "vault created [OK]"

    MODE=$(stat -c "%a" "$VAULT")

    if [ "$MODE" = "600" ]; then
        echo "vault permition [OK]"
    else
        echo "vault permition [NOK]"
    fi
else
    echo "vault created [NOK]"
fi

### test wrong master password ###

rm -rf "$OUT" "$VAULT"

### test password pattern warnings ###

REPEATED="secure-passwooord1"
printf "%s\n%s\n0\n" "$REPEATED" "$REPEATED" \
    | script -q -c "$BIN $VAULT" /dev/null > "$OUT"

REPEATED_CLEAR=$(grep -Fabo $'\033[2J\033[3J\033[H' "$OUT" \
    | head -n 1 | cut -d: -f1)
REPEATED_WARNING=$(grep -Fabo \
    'warning: password contains 3 repeated characters in a row' "$OUT" \
    | head -n 1 | cut -d: -f1)
REPEATED_MENU=$(grep -Fabo 'select one option' "$OUT" \
    | head -n 1 | cut -d: -f1)

if [ -f "$VAULT" ] \
    && grep -Fq $'\033[33mwarning: password contains 3 repeated characters in a row' "$OUT" \
    && [ "$REPEATED_CLEAR" -lt "$REPEATED_WARNING" ] \
    && [ "$REPEATED_WARNING" -lt "$REPEATED_MENU" ]; then
    echo "repeated characters warning [OK]"
else
    echo "repeated characters warning [NOK]"
fi

rm -rf "$OUT" "$VAULT"

SEQUENCE="safe-abcd-password"
printf "%s\n%s\n0\n" "$SEQUENCE" "$SEQUENCE" \
    | script -q -c "$BIN $VAULT" /dev/null > "$OUT"

SEQUENCE_CLEAR=$(grep -Fabo $'\033[2J\033[3J\033[H' "$OUT" \
    | head -n 1 | cut -d: -f1)
SEQUENCE_WARNING=$(grep -Fabo \
    'warning: password contains a 4-character ascending or descending sequence' \
    "$OUT" | head -n 1 | cut -d: -f1)
SEQUENCE_MENU=$(grep -Fabo 'select one option' "$OUT" \
    | head -n 1 | cut -d: -f1)

if [ -f "$VAULT" ] \
    && grep -Fq $'\033[33mwarning: password contains a 4-character ascending or descending sequence' "$OUT" \
    && [ "$SEQUENCE_CLEAR" -lt "$SEQUENCE_WARNING" ] \
    && [ "$SEQUENCE_WARNING" -lt "$SEQUENCE_MENU" ]; then
    echo "sequence warning [OK]"
else
    echo "sequence warning [NOK]"
fi

rm -rf "$OUT" "$VAULT"

printf "%s\nwrong-password-1\n%s\nwrong-password-2\n%s\nwrong-password-3\n" \
    "$MASTER" "$MASTER" "$MASTER" \
    | script -q -c "$BIN $VAULT" /dev/null > "$OUT"

if grep -q "bad password" "$OUT"; then
    echo "wrong confirm password [OK]"
else
    echo "wrong confirm password [NOK]"
fi

rm -rf "$OUT" "$VAULT"

### test adding credentials ###

printf "%s\n%s\n1\ngoogle\nthiago\n123\n123\n0\n" "$MASTER" "$MASTER" \
    | script -q -c "$BIN $VAULT" /dev/null > "$OUT"

printf "%s\n2\n0\n0\n0\n" "$MASTER" \
    | script -q -c "$BIN $VAULT" /dev/null > "$OUT"

if grep -q "service: google" "$OUT"; then
    echo "insert credential [OK]"
else
    echo "insert credential [NOK]"
fi

rm -rf "$OUT" "$VAULT"

make fclean > /dev/null
