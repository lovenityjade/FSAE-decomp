[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('Check', 'Build')]
    [string]$Action,

    [Parameter(Mandatory = $true)]
    [string]$Workspace,

    [Parameter(Mandatory = $true)]
    [string]$ToolchainRoot,

    [Parameter(Mandatory = $true)]
    [string]$EnvironmentScript,

    [string]$Mwccarm,
    [string]$Mwldarm,
    [string]$BuildCommandBase64
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
Set-StrictMode -Version 2.0

if (Test-Path -LiteralPath $EnvironmentScript -PathType Leaf) {
    . $EnvironmentScript
}

function Resolve-Compiler {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [string]$ExplicitPath,
        [Parameter(Mandatory = $true)][string]$EnvironmentName
    )

    $candidates = New-Object System.Collections.Generic.List[string]
    if (-not [string]::IsNullOrWhiteSpace($ExplicitPath)) {
        $candidates.Add($ExplicitPath)
    }
    $environmentValue = [Environment]::GetEnvironmentVariable($EnvironmentName, 'Process')
    if (-not [string]::IsNullOrWhiteSpace($environmentValue)) {
        $candidates.Add($environmentValue)
    }
    $command = Get-Command ($Name + '.exe') -CommandType Application -ErrorAction SilentlyContinue
    if ($null -ne $command) {
        $candidates.Add($command.Source)
    }
    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    if (Test-Path -LiteralPath $ToolchainRoot -PathType Container) {
        # Do not recursively scan every TwlSDK data file. Installed compilers
        # belong in a clearly named private toolchain directory.
        $compilerRoots = Get-ChildItem -LiteralPath $ToolchainRoot -Directory `
            -ErrorAction SilentlyContinue | Where-Object {
                $_.Name -match '(?i)(codewarrior|metrowerks|compiler|^cw)'
            }
        foreach ($compilerRoot in $compilerRoots) {
            $match = Get-ChildItem -LiteralPath $compilerRoot.FullName -Filter ($Name + '.exe') `
                -File -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($null -ne $match) {
                return $match.FullName
            }
        }
    }
    throw "Unable to find $Name.exe. Install it outside the workspace or set `$env:$EnvironmentName."
}

function Get-CompilerVersion {
    param([Parameter(Mandatory = $true)][string]$Path)

    $output = @(& $Path -version 2>&1 | ForEach-Object { $_.ToString() })
    $exitCode = $LASTEXITCODE
    $text = ($output -join "`n").Trim()
    if ($exitCode -ne 0) {
        throw "Version probe failed for $Path (exit $exitCode): $text"
    }
    if ([string]::IsNullOrWhiteSpace($text)) {
        $text = (Get-Item -LiteralPath $Path).VersionInfo.FileVersion
    }
    if ([string]::IsNullOrWhiteSpace($text)) {
        throw "No version information returned by $Path"
    }
    return $text
}

if (-not (Test-Path -LiteralPath $Workspace -PathType Container)) {
    throw "Remote workspace does not exist; synchronize it first: $Workspace"
}

$mwccPath = Resolve-Compiler -Name 'mwccarm' -ExplicitPath $Mwccarm -EnvironmentName 'MWCCARM'
$mwldPath = Resolve-Compiler -Name 'mwldarm' -ExplicitPath $Mwldarm -EnvironmentName 'MWLDARM'

$workspacePrefix = (Resolve-Path -LiteralPath $Workspace).Path.TrimEnd('\') + '\'
foreach ($compiler in @($mwccPath, $mwldPath)) {
    if ($compiler.StartsWith($workspacePrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Compiler must remain private and outside the synchronized workspace: $compiler"
    }
}

$report = [ordered]@{
    status = 'ready'
    action = $Action.ToLowerInvariant()
    workspace = (Resolve-Path -LiteralPath $Workspace).Path
    environment_script_loaded = (Test-Path -LiteralPath $EnvironmentScript -PathType Leaf)
    compilers = [ordered]@{
        mwccarm = [ordered]@{
            path = $mwccPath
            version = Get-CompilerVersion -Path $mwccPath
        }
        mwldarm = [ordered]@{
            path = $mwldPath
            version = Get-CompilerVersion -Path $mwldPath
        }
    }
}

if ($Action -eq 'Build') {
    if ([string]::IsNullOrWhiteSpace($BuildCommandBase64)) {
        throw 'Build requires a base64-encoded command.'
    }
    try {
        $buildCommand = [Text.Encoding]::UTF8.GetString(
            [Convert]::FromBase64String($BuildCommandBase64)
        )
    }
    catch {
        throw "Invalid build command encoding: $($_.Exception.Message)"
    }
    if ([string]::IsNullOrWhiteSpace($buildCommand)) {
        throw 'Build command may not be empty.'
    }

    $env:MWCCARM = $mwccPath
    $env:MWLDARM = $mwldPath
    $compilerDirectories = @(
        (Split-Path -Parent $mwccPath),
        (Split-Path -Parent $mwldPath)
    ) | Select-Object -Unique
    $env:PATH = (($compilerDirectories -join ';') + ';' + $env:PATH)

    Push-Location -LiteralPath $Workspace
    try {
        & $env:ComSpec /d /s /c $buildCommand
        $buildExitCode = $LASTEXITCODE
    }
    finally {
        Pop-Location
    }
    $report.build = [ordered]@{
        command = $buildCommand
        exit_code = $buildExitCode
    }
    if ($buildExitCode -ne 0) {
        $report.status = 'build-failed'
    }
}

$report | ConvertTo-Json -Depth 5 -Compress
if (($Action -eq 'Build') -and ($buildExitCode -ne 0)) {
    exit $buildExitCode
}
