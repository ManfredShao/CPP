param(
    [int]$Runs = 5,
    [string]$JavaOpts = '-Xms1g -Xmx4g -server'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ($Runs -lt 1) {
    throw "Runs must be >= 1"
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptDir

$srcFile = 'Dotproduct.java'
$className = 'Dotproduct'

if (-not (Test-Path $srcFile)) {
    throw "Source file not found: $srcFile"
}

if (-not (Get-Command javac -ErrorAction SilentlyContinue)) {
    throw 'javac not found. Install JDK 17+ and add to PATH.'
}
if (-not (Get-Command java -ErrorAction SilentlyContinue)) {
    throw 'java not found. Install JDK 17+ and add to PATH.'
}

$javaArgList = @()
if ($JavaOpts.Trim().Length -gt 0) {
    $javaArgList = $JavaOpts.Split(' ', [System.StringSplitOptions]::RemoveEmptyEntries)
}

Write-Host "[INFO] Compiling $srcFile ..."
& javac $srcFile
if ($LASTEXITCODE -ne 0) {
    throw 'javac failed.'
}

if ($Runs -eq 1) {
    $outFile = 'results_java.csv'
    Write-Host "[INFO] Running benchmark -> $outFile"
    & java @javaArgList $className | Set-Content -Path $outFile -Encoding utf8
    Write-Host '[DONE] Generated Java files:'
    Get-ChildItem -Name 'results_java*.csv' | Sort-Object
    exit 0
}

Get-ChildItem -Path . -Filter 'results_java_run*.csv' -ErrorAction SilentlyContinue | Remove-Item -Force

$runFiles = @()
foreach ($r in 1..$Runs) {
    $outFile = "results_java_run$r.csv"
    Write-Host "[INFO] Running benchmark (run $r/$Runs) -> $outFile"
    & java @javaArgList $className | Set-Content -Path $outFile -Encoding utf8
    $runFiles += $outFile
}

$allRows = foreach ($f in $runFiles) {
    Import-Csv -Path $f | ForEach-Object {
        [pscustomobject]@{
            lang   = $_.lang
            type   = $_.type
            N      = [int64]::Parse($_.N, [System.Globalization.CultureInfo]::InvariantCulture)
            avg_ns = [double]::Parse($_.avg_ns, [System.Globalization.CultureInfo]::InvariantCulture)
        }
    }
}

$groups = $allRows | Group-Object -Property lang, type, N

$meanLines = @('lang,type,N,mean_avg_ns,runs')
$medianLines = @('lang,type,N,median_avg_ns,runs')

foreach ($g in ($groups | Sort-Object { $_.Group[0].type }, { $_.Group[0].N })) {
    $first = $g.Group[0]
    $vals = $g.Group.avg_ns | Sort-Object
    $count = $vals.Count

    $mean = ($vals | Measure-Object -Average).Average

    if ($count % 2 -eq 1) {
        $median = $vals[[int](($count - 1) / 2)]
    }
    else {
        $left = $vals[[int]($count / 2) - 1]
        $right = $vals[[int]($count / 2)]
        $median = ($left + $right) / 2.0
    }

    $meanLines += ('{0},{1},{2},{3:F6},{4}' -f $first.lang, $first.type, $first.N, $mean, $count)
    $medianLines += ('{0},{1},{2},{3:F6},{4}' -f $first.lang, $first.type, $first.N, $median, $count)
}

Set-Content -Path 'results_java_mean.csv' -Value $meanLines -Encoding utf8
Set-Content -Path 'results_java_median.csv' -Value $medianLines -Encoding utf8
Copy-Item -Force "results_java_run$Runs.csv" 'results_java.csv'

Write-Host '[DONE] Generated Java files:'
Get-ChildItem -Name 'results_java*.csv' | Sort-Object
