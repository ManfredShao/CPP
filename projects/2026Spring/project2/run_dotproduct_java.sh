#!/usr/bin/env zsh

set -euo pipefail
setopt nonomatch

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

SRC_FILE="Dotproduct.java"
CLASS_NAME="Dotproduct"
RUNS="${1:-1}"
JAVA_OPTS=("-Xmx4g" "-Xms1g" "-server")

if ! [[ "$RUNS" =~ '^[0-9]+$' ]] || [[ "$RUNS" -lt 1 ]]; then
  echo "Usage: ./run_dotproduct_java.sh [runs>=1]" >&2
  exit 1
fi

if [[ ! -f "$SRC_FILE" ]]; then
  echo "Error: $SRC_FILE not found in $SCRIPT_DIR" >&2
  exit 1
fi

echo "[INFO] Compiling $SRC_FILE ..."
javac "$SRC_FILE"

if [[ "$RUNS" -eq 1 ]]; then
  out_file="results_java.csv"
  echo "[INFO] Running benchmark -> $out_file"
  java "${JAVA_OPTS[@]}" "$CLASS_NAME" > "$out_file"

  echo "[DONE] Generated files:"
  ls -1 "$out_file"
  exit 0
fi

rm -f "results_java_run"*.csv

run_files=()
for r in $(seq 1 "$RUNS"); do
  out_file="results_java_run${r}.csv"
  echo "[INFO] Running benchmark (run ${r}/${RUNS}) -> $out_file"
  java "${JAVA_OPTS[@]}" "$CLASS_NAME" > "$out_file"
  run_files+=("$out_file")
done

mean_file="results_java_mean.csv"
median_file="results_java_median.csv"
raw_file="results_java_raw.tmp"
mean_tmp="${mean_file}.tmp"
median_tmp="${median_file}.tmp"

awk -F, '
  FNR == 1 { next }
  {
    printf "%s,%s,%s,%.12f\n", $1, $2, $3, $6;
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

cp "results_java_run${RUNS}.csv" "results_java.csv"

echo "[DONE] Generated files:"
ls -1 "results_java.csv" "results_java_mean.csv" "results_java_median.csv"
