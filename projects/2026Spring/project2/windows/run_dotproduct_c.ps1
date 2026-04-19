param(
    [int]$Runs = 5
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ($Runs -lt 1) {
    throw "Runs must be >= 1"
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptDir

$srcFile = 'dotproduct.c'
$binFile = 'dotproduct_opt.exe'
$levels = @('O0', 'O1', 'O2', 'O3', 'Ofast')

if (-not (Test-Path $srcFile)) {
    throw "Source file not found: $srcFile"
}

function Get-CCompiler {
    foreach ($name in @('clang', 'gcc', 'cc')) {
        $cmd = Get-Command $name -ErrorAction SilentlyContinue
        if ($cmd) {
            return $cmd.Source
        }
    }
    throw "No C compiler found. Install LLVM/clang or MinGW-w64 gcc and add to PATH."
}

function Normalize-CResult {
    param(
        [string]$RawCapture,
        [string]$OutCsv
    )

    $rows = Import-Csv -Path $RawCapture
    if (-not $rows -or -not ($rows[0].PSObject.Properties.Name -contains 'avg_ns')) {
        throw "Unexpected C output format in $RawCapture"
    }

    $lines = @('lang,type,N,avg_ns')
    foreach ($r in $rows) {
        $avg = [double]::Parse($r.avg_ns, [System.Globalization.CultureInfo]::InvariantCulture)
        $nVal = [int64]::Parse($r.N, [System.Globalization.CultureInfo]::InvariantCulture)
        $lines += ('{0},{1},{2},{3:F12}' -f $r.lang, $r.type, $nVal, $avg)
    }

    Set-Content -Path $OutCsv -Value $lines -Encoding utf8
}

function Build-Stats {
    param(
        [string[]]$RunFiles,
        [string]$MeanFile,
        [string]$MedianFile
    )

    $allRows = foreach ($f in $RunFiles) {
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

    Set-Content -Path $MeanFile -Value $meanLines -Encoding utf8
    Set-Content -Path $MedianFile -Value $medianLines -Encoding utf8
}

$compiler = Get-CCompiler
Write-Host "[INFO] Using compiler: $compiler"

foreach ($level in $levels) {
    Write-Host "[INFO] Building with -$level ..."
    & $compiler $srcFile '-std=c11' '-Wall' '-Wextra' "-$level" '-o' $binFile
    if ($LASTEXITCODE -ne 0) {
        throw "Compile failed for -$level"
    }

    if ($Runs -eq 1) {
        $outFile = "results_c($level).csv"
        $rawCapture = "results_c($level)_raw.tmp"

        Write-Host "[INFO] Running benchmark -> $outFile"
        & ".\$binFile" | Set-Content -Path $rawCapture -Encoding utf8
        Normalize-CResult -RawCapture $rawCapture -OutCsv $outFile
        Remove-Item -Force $rawCapture
    }
    else {
        Get-ChildItem -Path . -Filter "results_c($level)_run*.csv" -ErrorAction SilentlyContinue | Remove-Item -Force

        $runFiles = @()
        foreach ($r in 1..$Runs) {
            $outFile = "results_c($level)_run$r.csv"
            $rawCapture = "results_c($level)_raw_run$r.tmp"

            Write-Host "[INFO] Running benchmark ($level, run $r/$Runs) -> $outFile"
            & ".\$binFile" | Set-Content -Path $rawCapture -Encoding utf8
            Normalize-CResult -RawCapture $rawCapture -OutCsv $outFile
            Remove-Item -Force $rawCapture
            $runFiles += $outFile
        }

        $meanFile = "results_c($level)_mean.csv"
        $medianFile = "results_c($level)_median.csv"
        Build-Stats -RunFiles $runFiles -MeanFile $meanFile -MedianFile $medianFile

        Copy-Item -Force "results_c($level)_run$Runs.csv" "results_c($level).csv"
    }
}

Write-Host "[DONE] Generated C files:"
Get-ChildItem -Name 'results_c(*).csv' | Sort-Object
Get-ChildItem -Name 'results_c(*)_mean.csv' | Sort-Object
Get-ChildItem -Name 'results_c(*)_median.csv' | Sort-Object
