#!/bin/bash
# Copyright 2025, Ludwig V. <https://github.com/ludwig-v>
# Thanks to Zubastic <https://github.com/Zubastic> for the work around UPX obfuscated binaries to extract AES keys

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License at <http://www.gnu.org/licenses/> for
# more details.

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# Check if OpenSSL is installed
if ! command -v openssl &> /dev/null; then
    echo "Error: OpenSSL is not installed. Please install it and try again."
    exit 1
fi

# Check for input arguments
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <encrypt|decrypt> <input_file> <output_file>"
    exit 1
fi

# Mode selection
MODE="$1"
INPUT_FILE="$2"
OUTPUT_FILE="$3"
TEMP_ENC_FILE="temp_encrypted.bin"

# AES key and IV
AES_KEY="8e15c895KBP6ClJv"
AES_IV="8e15c895KBP6ClJv"
KEY_HEX=$(printf "%s" "$AES_KEY" | od -A n -t x1 | tr -d ' ')
IV_HEX=$(printf "%s" "$AES_IV" | od -A n -t x1 | tr -d ' ')

# Verify input file exists
if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: Input file '$INPUT_FILE' does not exist."
    exit 1
fi

# Get file size
if [[ "$OSTYPE" == "darwin"* ]]; then
    FILESIZE=$(stat -f %z "$INPUT_FILE")
else
    FILESIZE=$(stat -c%s "$INPUT_FILE")
fi
BLOCK_SIZE=16
REMAINDER=$(( FILESIZE % BLOCK_SIZE ))

if [ $REMAINDER -eq 0 ]; then
    TRUNCATED_SIZE=$FILESIZE
    LAST_BLOCK=""
else
    TRUNCATED_SIZE=$(( FILESIZE - REMAINDER ))
    LAST_BLOCK=$(tail -c $REMAINDER "$INPUT_FILE" | base64)
fi

if [ "$MODE" == "decrypt" ]; then
    # Extract the fully encrypted part
    head -c $TRUNCATED_SIZE "$INPUT_FILE" > "$TEMP_ENC_FILE"
    
    # Decrypt using OpenSSL with -nopad option
    openssl enc -d -aes-128-cbc -nopad -K "$KEY_HEX" -iv "$IV_HEX" -in "$TEMP_ENC_FILE" -out "$OUTPUT_FILE"
    
    # Append the unencrypted last block if necessary
    if [ $REMAINDER -ne 0 ]; then
        echo "$LAST_BLOCK" | base64 -d >> "$OUTPUT_FILE"
    fi
    echo "Decryption completed. Output saved to $OUTPUT_FILE"

elif [ "$MODE" == "encrypt" ]; then
    # Extract the fully encrypted part
    head -c $TRUNCATED_SIZE "$INPUT_FILE" > "$TEMP_ENC_FILE"
    
    # Encrypt using OpenSSL with -nopad option
    openssl enc -e -aes-128-cbc -nopad -K "$KEY_HEX" -iv "$IV_HEX" -in "$TEMP_ENC_FILE" -out "$OUTPUT_FILE"
    
    # Append the unencrypted last block if necessary
    if [ $REMAINDER -ne 0 ]; then
        echo "$LAST_BLOCK" | base64 -d >> "$OUTPUT_FILE"
    fi
    echo "Encryption completed. Output saved to $OUTPUT_FILE"
else
    echo "Error: Invalid mode. Use 'encrypt' or 'decrypt'"
    exit 1
fi

# Cleanup
rm -f "$TEMP_ENC_FILE"