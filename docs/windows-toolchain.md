# Windows CodeWarrior host

The matching build will run on the private Windows host named
`sekailink-windows`. The repository contains only orchestration code. Nintendo
SDKs, ROMs, BIOS dumps, installers, compilers, libraries, and generated builds
must remain outside the synchronized workspace and outside Git.

## Private host layout

The canonical root is `D:\FourSwordAnniversary`:

```text
D:\FourSwordAnniversary\
  workspace\                  Public, disposable mirror of this repository
  private\
    environment.ps1          Optional private environment variables
    sdk-archives\            Original SDK archives; never synchronized
    rom\                     Optional ROM/BIOS storage; never synchronized
  toolchains\
    codewarrior-installer\   Private installer media
    cw-admin\                Current private CodeWarrior installation image
    codewarrior-arm\         Alternative explicit compiler location
    twlsdk-base\             Private base SDK
    twlsdk-5.5-p4\           Private working SDK
    twlsdk-patch5\           Private patch reference
    twlsystem\               Private TwlSystem tree
```

`workspace` may contain an ignored `build\` directory for generated output.
The synchronizer preserves files created only on the host by default. An
explicit cleanup can delete stale, non-excluded files. All irreplaceable
material belongs in `private\` or `toolchains\`, which are siblings of
`workspace`, not children.

An optional private `D:\FourSwordAnniversary\private\environment.ps1` can select
installed tools without putting paths into the repository:

```powershell
$env:MWCCARM = 'D:\FourSwordAnniversary\toolchains\codewarrior-arm\bin\mwccarm.exe'
$env:MWLDARM = 'D:\FourSwordAnniversary\toolchains\codewarrior-arm\bin\mwldarm.exe'
$env:TWLSDK_ROOT = 'D:\FourSwordAnniversary\toolchains\twlsdk-5.5-p4\TwlSDK'
$env:TWLSYSTEM_ROOT = 'D:\FourSwordAnniversary\toolchains\twlsystem\TwlSystem-2_2_0-20090805\TwlSystem'
```

The remote helper loads this file when present. Otherwise it checks explicit
CLI paths, the `MWCCARM`/`MWLDARM` process environment, `PATH`, then searches
private toolchain directories whose names identify CodeWarrior, Metrowerks, or
a compiler under `D:\FourSwordAnniversary\toolchains`. A compiler found inside
`workspace` is rejected so private binaries cannot become part of the mirror.

The verified host currently resolves CodeWarrior below
`toolchains\cw-admin\program files\Freescale\CW for NINTENDO DSi V1.6 SP2`.
`check.sh` reports `mwccarm` 4.0 build 1051 and `mwldarm` 2.0 build 99. These
locations are discovered dynamically and are not embedded in the public scripts.

Presence and `-version` probes do not prove that the private license permits
compilation. The current smoke compile reaches `mwccarm` but FLEXlm rejects the
`Win32_Plugins_ARM` feature as expired on 31 March 2013 (`-10,32`). A valid,
privately managed CodeWarrior license configuration is therefore a prerequisite
before matching builds can proceed. No license material or workaround belongs in
this repository.

## Synchronize public sources

Preview the exact changes without writing:

```sh
tools/windows/sync.sh --dry-run
```

Apply them:

```sh
tools/windows/sync.sh
```

The command uses content checksums and is idempotent. It creates the workspace
when needed, synchronizes to `sekailink-windows:/d/FourSwordAnniversary/workspace/`,
and preserves remote-only files. Use `--delete-stale` only when an exact mirror
is intentionally required; deletion remains delayed until a successful transfer.

Exclusions cover Git metadata, every `private`, `toolchains`, `build`, `out`,
and cache directory; ROM/SRL/TAD files; BIOS/firmware dumps; Nintendo/TwlSDK
archives; and Windows executables/installers. Excluded remote files are never
deleted because the command deliberately does not use `--delete-excluded`.

## Verify CodeWarrior

After synchronization:

```sh
tools/windows/check.sh
```

The remote PowerShell helper must locate both `mwccarm.exe` and `mwldarm.exe`,
run each with `-version`, and return their absolute paths and version output as
JSON. Missing tools, failed version probes, a missing workspace, or compilers
inside the workspace produce a non-zero exit status.

Explicit private paths can be tested without changing `environment.ps1`:

```sh
tools/windows/check.sh \
  --mwccarm 'D:\FourSwordAnniversary\toolchains\codewarrior-arm\bin\mwccarm.exe' \
  --mwldarm 'D:\FourSwordAnniversary\toolchains\codewarrior-arm\bin\mwldarm.exe'
```

## Execute a remote build

Pass the repository's build entry point as one `cmd.exe` command:

```sh
tools/windows/build.sh --command 'python tools\build.py'
```

The command is UTF-8/base64 encoded locally, decoded by the remote helper, and
executed with `cmd.exe /d /s /c` from the workspace. Before execution the helper
repeats both compiler/version checks, exports `MWCCARM` and `MWLDARM`, and adds
their directories to `PATH`. It returns the build exit code and a JSON summary;
there is no path that runs the build after a failed toolchain check.

A minimal compiler proof is included and writes only to ignored remote build
output:

```sh
tools/windows/build.sh --command 'python tools\windows\smoke_build.py'
```

It compiles the public `smoke.c` fixture with the verified private `mwccarm`,
checks that a non-empty object was produced under `build\windows-toolchain-smoke`,
and prints its size and SHA-256. The object is excluded from synchronization and
is never versioned. Until the private FLEXlm prerequisite above is resolved, this
command intentionally returns the compiler's non-zero exit status and the remote
wrapper reports `build-failed`.

## Tests

The unit tests contain no SSH connection or proprietary fixture:

```sh
python3 -m unittest discover -s tools/windows/tests -v
```

They cover Windows-to-MSYS path conversion, mandatory rsync exclusions,
deletion behavior, encoded PowerShell launchers, build-command encoding, and
host/path validation.
