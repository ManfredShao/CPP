param(
    [int]$Runs = 5
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptDir

Write-Host "[STEP] Run C benchmark (O0/O1/O2/O3/Ofast), runs=$Runs"
& powershell -ExecutionPolicy Bypass -File .\run_dotproduct_c.ps1 -Runs $Runs
if ($LASTEXITCODE -ne 0) {
    throw 'C benchmark failed.'
}

Write-Host "[STEP] Run Java benchmark, runs=$Runs"
& powershell -ExecutionPolicy Bypass -File .\run_dotproduct_java.ps1 -Runs $Runs
if ($LASTEXITCODE -ne 0) {
    throw 'Java benchmark failed.'
}

Write-Host '[DONE] All benchmarks completed.'
