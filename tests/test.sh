#!/bin/bash

BIN="./vault"
OUT="out.txt"
VAULT="vault.test"

printf "pass\npass\n0\n" | script -q -c "$BIN $VAULT" /dev/null > "$OUT"

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

printf "password\nwrong\n" | script -q -c "$BIN $VAULT" /dev/null > "$OUT"

if grep -q "bad password" "$OUT"; then
    echo "wrong confirm password [OK]"
else
    echo "wrong confirm password [NOK]"
fi

rm -rf "$OUT" "$VAULT"

### test adding credentials ###

printf "pass\npass\n1\ngoogle\nthiago\n123\n123\n0\n" | script -q -c "$BIN $VAULT" /dev/null > "$OUT"

printf "pass\n2\n0\n0\n" | script -q -c "$BIN $VAULT" /dev/null > "$OUT"

if grep -q "Service: google" "$OUT"; then
    echo "insert credential [OK]"
else
    echo "insert credential [NOK]"
fi

rm -rf "$OUT" "$VAULT"
