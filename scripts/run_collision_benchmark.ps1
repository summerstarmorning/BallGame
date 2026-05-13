param(
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot "build_vs"
$outputPath = Join-Path $repoRoot "docs\performance_benchmark_results.json"

Push-Location $repoRoot
try {
    cmake --build $buildDir --config $Configuration --target collision_benchmark
    & (Join-Path $buildDir "$Configuration\collision_benchmark.exe") --output $outputPath
}
finally {
    Pop-Location
}
