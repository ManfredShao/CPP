from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

BASE_DIR = Path(__file__).resolve().parent
OUT_DIR = BASE_DIR / "output_plots" / "showcase"
OUT_DIR.mkdir(parents=True, exist_ok=True)

N_TARGET = 100_000
TYPE_ORDER = ["float", "double", "int", "short", "signed_char"]
TYPE_LABEL = {
    "float": "float",
    "double": "double",
    "int": "int",
    "short": "short",
    "signed_char": "char",
}

SERIES = ["C-O3", "C-Ofast", "C-Native", "Java-Normal", "Java-Vector"]
COLORS = {
    "C-O3": "#9467bd",
    "C-Ofast": "#17becf",
    "C-Native": "#8c564b",
    "Java-Normal": "#d62728",
    "Java-Vector": "#1f77b4",
}


def _load_platform_frames(platform: str) -> dict[str, pd.DataFrame]:
    if platform == "mac":
        base = BASE_DIR
        return {
            "C-O3": pd.read_csv(base / "results_c(O3)_median.csv"),
            "C-Ofast": pd.read_csv(base / "ofast_results" / "results_c(Ofast)_median.csv"),
            "C-Native": pd.read_csv(base / "ofast_native_results" / "results_c(Ofast_native)_median.csv"),
            "Java-Normal": pd.read_csv(base / "results_java_median.csv"),
            "Java-Vector": pd.read_csv(base / "results_java_vector_median.csv"),
        }

    if platform == "windows":
        base = BASE_DIR / "windows"
        return {
            "C-O3": pd.read_csv(base / "results_c(O3)_median.csv"),
            "C-Ofast": pd.read_csv(base / "results_c(Ofast)_median.csv"),
            "C-Native": pd.read_csv(base / "results_c(Ofast_native)_median.csv"),
            "Java-Normal": pd.read_csv(base / "results_java_median.csv"),
            "Java-Vector": pd.read_csv(base / "results_java_vector_median.csv"),
        }

    raise ValueError(f"Unknown platform: {platform}")


def _extract_at_n(df: pd.DataFrame, n: int) -> pd.Series:
    subset = df[df["N"].astype(int) == n].copy()
    subset["type"] = subset["type"].astype(str)
    subset["median_avg_ns"] = subset["median_avg_ns"].astype(float)
    return subset.set_index("type")["median_avg_ns"]


def make_bar_chart(platform: str, title_platform: str) -> pd.DataFrame:
    frames = _load_platform_frames(platform)

    rows = []
    for series in SERIES:
        series_at_n = _extract_at_n(frames[series], N_TARGET)
        for dtype in TYPE_ORDER:
            rows.append(
                {
                    "platform": title_platform,
                    "series": series,
                    "type": dtype,
                    "label": TYPE_LABEL[dtype],
                    "median_avg_ns": float(series_at_n.loc[dtype]),
                }
            )

    bar_df = pd.DataFrame(rows)

    fig, ax = plt.subplots(figsize=(12.8, 6.2))
    x = np.arange(len(TYPE_ORDER))
    width = 0.15

    for idx, series in enumerate(SERIES):
        s = bar_df[bar_df["series"] == series].set_index("type").loc[TYPE_ORDER]
        offset = (idx - 2) * width
        ax.bar(
            x + offset,
            s["median_avg_ns"].values,
            width=width,
            color=COLORS[series],
            label=series,
            alpha=0.9,
        )

    ax.set_xticks(x)
    ax.set_xticklabels([TYPE_LABEL[t] for t in TYPE_ORDER])
    ax.set_yscale("log")
    ax.set_ylabel("median avg_ns @ N=100000 (log scale)")
    ax.set_title(f"{title_platform}: Peak Throughput Snapshot @ N=100000")
    ax.grid(True, axis="y", linestyle="--", alpha=0.35)
    ax.legend(ncol=3, frameon=False)

    fig.tight_layout()
    out_png = OUT_DIR / f"bar_peak_snapshot_{platform}.png"
    fig.savefig(out_png, dpi=240, bbox_inches="tight")
    plt.close(fig)

    out_csv = OUT_DIR / f"bar_peak_snapshot_{platform}.csv"
    bar_df.to_csv(out_csv, index=False)
    return bar_df


def make_mac_crossover_chart() -> pd.DataFrame:
    ofast = pd.read_csv(BASE_DIR / "ofast_results" / "results_c(Ofast)_median.csv")
    native = pd.read_csv(BASE_DIR / "ofast_native_results" / "results_c(Ofast_native)_median.csv")

    ofast_d = ofast[ofast["type"] == "double"][["N", "median_avg_ns"]].copy()
    native_d = native[native["type"] == "double"][["N", "median_avg_ns"]].copy()

    ofast_d["N"] = ofast_d["N"].astype(int)
    native_d["N"] = native_d["N"].astype(int)
    ofast_d["median_avg_ns"] = ofast_d["median_avg_ns"].astype(float)
    native_d["median_avg_ns"] = native_d["median_avg_ns"].astype(float)

    merged = pd.merge(
        ofast_d.rename(columns={"median_avg_ns": "ofast_ns"}),
        native_d.rename(columns={"median_avg_ns": "native_ns"}),
        on="N",
        how="inner",
    ).sort_values("N")

    merged["native_minus_ofast"] = merged["native_ns"] - merged["ofast_ns"]

    fig, ax = plt.subplots(figsize=(10.8, 6.0))
    ax.plot(merged["N"], merged["ofast_ns"], marker="o", linewidth=2.2, color=COLORS["C-Ofast"], label="C-Ofast")
    ax.plot(merged["N"], merged["native_ns"], marker="s", linewidth=2.2, color=COLORS["C-Native"], label="C-Native")

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("N (log scale)")
    ax.set_ylabel("median avg_ns (log scale)")
    ax.set_title("macOS Double: C-Native vs C-Ofast (Crossover View)")
    ax.grid(True, linestyle="--", alpha=0.35)
    ax.legend(frameon=False)

    sign = np.sign(merged["native_minus_ofast"].values)
    crossover_idx = np.where(sign[:-1] * sign[1:] < 0)[0]
    for idx in crossover_idx:
        n_left = merged.iloc[idx]["N"]
        n_right = merged.iloc[idx + 1]["N"]
        ax.axvspan(n_left, n_right, color="#cccccc", alpha=0.2)

    fig.tight_layout()
    out_png = OUT_DIR / "line_mac_double_native_vs_ofast.png"
    fig.savefig(out_png, dpi=240, bbox_inches="tight")
    plt.close(fig)

    out_csv = OUT_DIR / "line_mac_double_native_vs_ofast.csv"
    merged.to_csv(out_csv, index=False)
    return merged


def main():
    make_bar_chart("mac", "macOS")
    make_bar_chart("windows", "Windows")
    make_mac_crossover_chart()
    print(f"Done. Outputs in: {OUT_DIR}")


if __name__ == "__main__":
    main()
