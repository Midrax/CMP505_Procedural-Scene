param(
    [ValidateSet('x64-windows', 'x86-windows')]
    [string]$Triplet = 'x64-windows'
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$manifest = Get-Content (Join-Path $projectRoot 'vcpkg.json') -Raw | ConvertFrom-Json
$baseline = $manifest.'builtin-baseline'
$vcpkgRoot = Join-Path $projectRoot 'build\vcpkg'
$platform = if ($Triplet -eq 'x86-windows') { 'Win32' } else { 'x64' }

function Invoke-Checked {
    param([string]$Program, [string[]]$Arguments)
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Program exited with code $LASTEXITCODE"
    }
}

Push-Location $projectRoot
try {
    if (!(Test-Path (Join-Path $vcpkgRoot '.git'))) {
        Invoke-Checked git @('clone', '--no-checkout', '--filter=blob:none',
            'https://github.com/microsoft/vcpkg.git', $vcpkgRoot)
        Invoke-Checked git @('-C', $vcpkgRoot, 'fetch', '--depth', '1', 'origin', $baseline)
        Invoke-Checked git @('-C', $vcpkgRoot, 'checkout', '--detach', $baseline)
    }

    $actualBaseline = & git -C $vcpkgRoot rev-parse HEAD
    if ($LASTEXITCODE -ne 0 -or $actualBaseline -ne $baseline) {
        throw 'The local build/vcpkg checkout differs from the pinned baseline. Use a matching checkout before continuing.'
    }

    if (!(Test-Path (Join-Path $vcpkgRoot 'vcpkg.exe'))) {
        Invoke-Checked (Join-Path $vcpkgRoot 'bootstrap-vcpkg.bat') @('-disableMetrics')
    }

    Invoke-Checked (Join-Path $vcpkgRoot 'vcpkg.exe') @(
        'install', '--triplet', $Triplet,
        "--x-install-root=build\dependencies\$platform",
        '--x-buildtrees-root=build\vcpkg-buildtrees',
        '--x-packages-root=build\vcpkg-packages',
        '--downloads-root=build\vcpkg-downloads'
    )
}
finally {
    Pop-Location
}
