#!/bin/bash

TEST_DIR="./test_conf"
TEST_VALID_DIR="$TEST_DIR/valid"
TEST_INVALID_DIR="$TEST_DIR/invalid"
EXECUTABLE="./webserv"

echo "🚀 Avvio test configurazione..."
echo
echo "✅✅✅ Testando configurazioni valide... ✅✅✅"
echo
for testfile in $TEST_VALID_DIR/*; do
    echo "🔍 Testando $testfile..."
    $EXECUTABLE $testfile
    echo "--------------------------------------"
done

echo

echo "❌❌❌ Testando configurazioni non valide... ❌❌❌"
echo
echo "🔍 Testando file non esistente..."
$EXECUTABLE "file_non_esistente.conf"
echo "--------------------------------------"
for testfile in $TEST_INVALID_DIR/*; do
    echo "🔍 Testando $testfile..."
    $EXECUTABLE $testfile
    echo "--------------------------------------"
done

echo
echo "Test completati!"
