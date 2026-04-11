param(
    [int]$Runs = 5
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptDir

Write-Host "[STEP] Run C benchmark (-Ofast -march=native), runs=$Runs"
& powershell -ExecutionPolicy Bypass -File .\run_dotproduct_c_native.ps1 -Runs $Runs
if ($LASTEXITCODE -ne 0) {
    throw 'Native C benchmark failed.'
}

Write-Host "[STEP] Run Java Vector benchmark, runs=$Runs"
& powershell -ExecutionPolicy Bypass -File .\run_dotproduct_java_vector.ps1 -Runs $Runs
if ($LASTEXITCODE -ne 0) {
    throw 'Java Vector benchmark failed.'
}

Write-Host '[DONE] Native + Java Vector benchmarks completed.'
