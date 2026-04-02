#!/usr/bin/env bash
# Checks C source files for known-unsafe functions.
# Fix 4: exits with code 1 on any finding (hard gate, not warning-only).
# Based on Microsoft SDL Banned Functions list.
set -euo pipefail

TARGET_DIR="${1:-.}"

# Functions that must not appear in new code
BANNED=(
    "gets"       # Buffer overflow — removed in C11
    "sprintf"    # Use snprintf instead
    "vsprintf"   # Use vsnprintf instead
    "strcpy"     # Use strncpy or strlcpy
    "strcat"     # Use strncat or strlcat
    "scanf"      # Format string risks
)

FOUND=0
for func in "${BANNED[@]}"; do
    MATCHES=$(grep -rn --include='*.c' --include='*.h' \
        -w "$func" "$TARGET_DIR" \
        | grep -v '^\s*//' \
        | grep -v '^\s*\*' \
        | grep -v 'check-banned' || true)

    if [ -n "$MATCHES" ]; then
        echo "ERROR: Banned function '$func' found:"
        echo "$MATCHES" | head -5
        echo ""
        FOUND=$((FOUND + 1))
    fi
done

if [ "$FOUND" -gt 0 ]; then
    echo "::error::Found $FOUND banned function(s). Replace with safe alternatives before merging."
    exit 1
else
    echo "OK: No banned functions found"
fi
