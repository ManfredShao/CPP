Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptDir

$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) {
    $python = Get-Command py -ErrorAction SilentlyContinue
}
if (-not $python) {
    throw 'Python not found. Install Python 3 and add to PATH.'
}

if ($python.Name -eq 'py.exe' -or $python.Name -eq 'py') {
    & py -3 .\plot_dotproduct_comparison.py
}
else {
    & python .\plot_dotproduct_comparison.py
}

if ($LASTEXITCODE -ne 0) {
    throw 'Plot script failed.'
}

Write-Host '[DONE] Plot and tables generated in .\output_plots\'
