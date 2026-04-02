#!/usr/bin/env bash
# Release Gate — deterministic release decision
# Usage: ./release-gate.sh [path/to/binary]
# Fix 8: accepts binary path as argument (not hardcoded to build/kvstore)
set -euo pipefail

echo "═══════════════════════════════════"
echo "  RELEASE GATE — Decision Engine"
echo "═══════════════════════════════════"

DECISION="ALLOW"
WARNINGS=0

# Check 1: All CI jobs passed (implicit — this runs in needs:)
echo "[OK] All upstream CI jobs passed"

# Check 2: Version tag exists
VERSION=$(grep -v 'cmake_minimum_required' CMakeLists.txt \
    | sed -n 's/.*VERSION \([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\).*/\1/p' \
    | head -1)
if [ -z "$VERSION" ]; then
    echo "[FAIL] No version found in CMakeLists.txt"
    DECISION="BLOCK"
else
    echo "[OK] Version: $VERSION"
    if [ -n "${GITHUB_OUTPUT:-}" ]; then
        echo "version=$VERSION" >> "$GITHUB_OUTPUT"
    fi
fi

# Check 3: No TODO/FIXME in critical paths
TODOS=$(grep -rn 'TODO\|FIXME\|HACK\|XXX' src/ || true)
if [ -n "$TODOS" ]; then
    COUNT=$(echo "$TODOS" | wc -l)
    echo "[WARN] Found $COUNT TODO/FIXME markers in src/"
    WARNINGS=$((WARNINGS + 1))
fi

# Check 4: Binary size sanity check
# Binary path passed as $1, defaults to build/kvstore for local use
BINARY="${1:-build/kvstore}"
if [ -f "$BINARY" ]; then
    SIZE=$(stat --printf="%s" "$BINARY" 2>/dev/null || echo "0")
    MAX_SIZE=$((10 * 1024 * 1024))  # 10 MB limit
    if [ "$SIZE" -gt "$MAX_SIZE" ]; then
        echo "[FAIL] Binary too large: ${SIZE} bytes (max: ${MAX_SIZE})"
        DECISION="BLOCK"
    else
        echo "[OK] Binary size: ${SIZE} bytes"
    fi
else
    echo "[WARN] Binary not found at '$BINARY' — skipping size check"
    WARNINGS=$((WARNINGS + 1))
fi

echo ""
echo "───────────────────────────────────"
echo "  Warnings:  $WARNINGS"
echo "  Decision:  $DECISION"
echo "───────────────────────────────────"

if [ "$DECISION" = "BLOCK" ]; then
    echo "::error::Release BLOCKED"
    exit 1
elif [ "$WARNINGS" -gt 0 ]; then
    echo "::warning::Release ALLOWED with $WARNINGS warning(s)"
fi
