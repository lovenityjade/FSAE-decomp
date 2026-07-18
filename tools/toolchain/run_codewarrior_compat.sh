#!/usr/bin/env bash
set -euo pipefail

# Run the licensed legacy CodeWarrior ARM command-line tools without changing
# the host clock. All proprietary inputs stay outside the repository.
: "${CW_TOOL:?set CW_TOOL to mwccarm.exe, mwldarm.exe, or mwasmarm.exe}"
: "${CW_LICENSE_FILE:?set CW_LICENSE_FILE to your legally supplied license.dat}"
: "${FAKETIME_LIB:?set FAKETIME_LIB to a compatible libfaketime.so.1}"

if [[ ! -f "$CW_TOOL" ]]; then
    echo "CodeWarrior compatibility error: tool not found: $CW_TOOL" >&2
    exit 2
fi
if [[ ! -f "$CW_LICENSE_FILE" ]]; then
    echo "CodeWarrior compatibility error: license not found: $CW_LICENSE_FILE" >&2
    exit 2
fi
if [[ ! -f "$FAKETIME_LIB" ]]; then
    echo "CodeWarrior compatibility error: libfaketime not found: $FAKETIME_LIB" >&2
    exit 2
fi
if ! command -v wine >/dev/null 2>&1; then
    echo "CodeWarrior compatibility error: wine is not installed" >&2
    exit 2
fi
if ! command -v winepath >/dev/null 2>&1; then
    echo "CodeWarrior compatibility error: winepath is not installed" >&2
    exit 2
fi

export WINEPREFIX="${WINEPREFIX:-$PWD/build/toolchain/wineprefix}"
mkdir -p "$WINEPREFIX"

license_windows="$(WINEDEBUG=-all winepath -w "$CW_LICENSE_FILE")"
export LM_LICENSE_FILE="$license_windows"
export LD_PRELOAD="$FAKETIME_LIB${LD_PRELOAD:+:$LD_PRELOAD}"
export FAKETIME="${CW_COMPAT_DATE:-@2015-03-01 12:00:00}"
export FAKETIME_DONT_RESET=1
export FAKETIME_DONT_FAKE_MONOTONIC=1
export WINEDEBUG="${WINEDEBUG:--all}"

exec wine "$CW_TOOL" "$@"
