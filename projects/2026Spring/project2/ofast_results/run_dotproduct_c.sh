#!/usr/bin/env zsh

set -euo pipefail
setopt nonomatch

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

SRC_FILE="dotproduct.c"
BIN_FILE="dotproduct_opt"
RUNS="${1:-1}"

if ! [[ "$RUNS" =~ '^[0-9]+$' ]] || [[ "$RUNS" -lt 1 ]]; then
  echo "Usage: ./run_dotproduct_opt.sh [runs>=1]" >&2
  exit 1
fi

if [[ ! -f "$SRC_FILE" ]]; then
  echo "Error: $SRC_FILE not found in $SCRIPT_DIR" >&2
  exit 1
fi

normalize_c_run_csv() {
  local raw_capture="$1"
  local out_csv="$2"

  if [[ -s "$raw_capture" ]] && head -n 1 "$raw_capture" | grep -q '^lang,type,N,reps,total_ns,avg_ns,checksum'; then
    awk -F, '
      FNR == 1 { print "lang,type,N,avg_ns"; next }
      NF >= 6 {
        printf "%s,%s,%s,%.12f\n", $1, $2, $3, $6;
      }
    ' "$raw_capture" > "$out_csv"
    return 0
  fi

  local legacy_csv=""
  if [[ -f "results_c.csv" ]]; then
    legacy_csv="results_c.csv"
  elif [[ -f "c_results.csv" ]]; then
    legacy_csv="c_results.csv"
  fi

  if [[ -n "$legacy_csv" ]] && head -n 1 "$legacy_csv" | grep -q '^lang,type,length,avg_time_s'; then
    awk -F, '
      FNR == 1 { print "lang,type,N,avg_ns"; next }
      NF >= 4 {
        printf "%s,%s,%s,%.12f\n", $1, $2, $3, ($4 * 1000000000.0);
      }
    ' "$legacy_csv" > "$out_csv"
    return 0
  fi

  echo "Error: unable to parse benchmark output for C run" >&2
  return 1
}

for level in Ofast; do
  echo "[INFO] Building with -${level} ..."
  cc "$SRC_FILE" -std=c11 -Wall -Wextra -"$level" -o "$BIN_FILE"

  if [[ "$RUNS" -eq 1 ]]; then
    out_file="results_c(${level}).csv"
    echo "[INFO] Running benchmark -> $out_file"
    rm -f "results_c.csv" "c_results.csv"
    raw_capture="results_c(${level})_raw.tmp"
    ./$BIN_FILE > "$raw_capture"
    normalize_c_run_csv "$raw_capture" "$out_file"
    rm -f "$raw_capture"
  else
    rm -f "results_c(${level})_run"*.csv

    run_files=()
    for r in $(seq 1 "$RUNS"); do
      out_file="results_c(${level})_run${r}.csv"
      echo "[INFO] Running benchmark (${level}, run ${r}/${RUNS}) -> $out_file"
      rm -f "results_c.csv" "c_results.csv"
      raw_capture="results_c(${level})_raw_run${r}.tmp"
      ./$BIN_FILE > "$raw_capture"
      normalize_c_run_csv "$raw_capture" "$out_file"
      rm -f "$raw_capture"
      run_files+=("$out_file")
    done

    mean_file="results_c(${level})_mean.csv"
    median_file="results_c(${level})_median.csv"
    raw_file="results_c(${level})_raw.tmp"
    mean_tmp="${mean_file}.tmp"
    median_tmp="${median_file}.tmp"

    awk -F, '
      FNR == 1 { next }
      {
        printf "%s,%s,%s,%.12f\n", $1, $2, $3, $4;
      }
    ' "${run_files[@]}" | sort -t, -k1,1 -k2,2 -k3,3n -k4,4n > "$raw_file"

    awk -F, '
      {
        key = $1 FS $2 FS $3;
        sum[key] += $4;
        cnt[key] += 1;
      }
      END {
        for (k in sum) {
          printf "%s,%.6f,%d\n", k, sum[k] / cnt[k], cnt[k];
        }
      }
    ' "$raw_file" | sort -t, -k2,2 -k3,3n > "$mean_tmp"

    {
      echo "lang,type,N,mean_avg_ns,runs"
      cat "$mean_tmp"
    } > "$mean_file"

    awk -F, '
      function emit_median(    med) {
        if (n == 0) return;
        if (n % 2 == 1) {
          med = vals[(n + 1) / 2];
        } else {
          med = (vals[n / 2] + vals[n / 2 + 1]) / 2.0;
        }
        printf "%s,%.6f,%d\n", curr, med, n;
      }
      {
        key = $1 FS $2 FS $3;
        val = $4 + 0.0;
        if (NR == 1) {
          curr = key;
          n = 0;
        }
        if (key != curr) {
          emit_median();
          delete vals;
          n = 0;
          curr = key;
        }
        vals[++n] = val;
      }
      END {
        emit_median();
      }
    ' "$raw_file" | sort -t, -k2,2 -k3,3n > "$median_tmp"

    {
      echo "lang,type,N,median_avg_ns,runs"
      cat "$median_tmp"
    } > "$median_file"

    rm -f "$raw_file" "$mean_tmp" "$median_tmp"

    cp "results_c(${level})_run${RUNS}.csv" "results_c(${level}).csv"
  fi
done

echo "[DONE] Generated files:"
if [[ "$RUNS" -eq 1 ]]; then
  ls -1 "results_c(Ofast).csv"
else
  ls -1 \
    "results_c(Ofast).csv" \
    "results_c(Ofast)_mean.csv" \
    "results_c(Ofast)_median.csv"
fi

