# CodeWarrior ARM compatibility runner

The target compiler is Freescale CodeWarrior ARM 4.0 build 1051 with linker
2.0 build 99. The supplied historical license is valid only within its
original development period, so a normal 2026 invocation fails before code
generation. Changing the workstation clock would disturb unrelated services
and is not acceptable.

`tools/toolchain/run_codewarrior_compat.sh` instead gives only the Wine process
a running clock that starts in 2015. It does not edit a license, patch an
executable, change the host clock, or place any proprietary file in this
repository. The caller must supply the original tool and license paths.

Example environment (paths are intentionally local placeholders):

```sh
export CW_TOOL=/private/codewarrior/ARM_Tools/Command_Line_Tools/mwccarm.exe
export CW_LICENSE_FILE=/private/codewarrior/license.dat
export FAKETIME_LIB=/private/lib/libfaketime.so.1
export WINEPREFIX="$PWD/build/toolchain/wineprefix"

tools/toolchain/run_codewarrior_compat.sh \
  -c tools/windows/tests/fixtures/smoke.c \
  -o "$PWD/build/toolchain/smoke.o"
```

The verified local smoke test produced a 32-bit ARM relocatable ELF object.
Two independent invocations produced the same 512-byte object with SHA-256
`f03eb7056b078493e6a1e26383a71644763d063b673821788dc4911f7d23de1e`.
This hash is evidence for the public smoke fixture only; it is not a game
matching metric.
