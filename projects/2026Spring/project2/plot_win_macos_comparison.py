from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

BASE_DIR = Path(__file__).resolve().parent
WIN_DIR = BASE_DIR / "windows"
OUT_DIR = BASE_DIR / "output_plots" / "win_macos"
OUT_DIR.mkdir(parents=True, exist_ok=True)

C_LEVELS = ["O0", "O1", "O2", "O3", "Ofast"]
SERIES_ORDER = [f"C-{level}" for level in C_LEVELS] + ["Java"]
FOCUS_SERIES = ["C-Ofast", "Java"]
TYPE_ORDER = ["schar", "short", "int", "float", "double"]
TYPE_LABEL = {
    "schar": "signed_char",
    "short": "short",
    "int": "int",
    "float": "float",
    "double": "double",
}
SERIES_COLOR = {
    "C-O0": "#1f77b4",
    "C-O1": "#2ca02c",
    "C-O2": "#ff7f0e",
    "C-O3": "#9467bd",
    "C-Ofast": "#17becf",
    "Java": "#d62728",
}


def _load_platform(platform_dir: Path, platform_name: str) -> pd.DataFrame:
    metric_col = "median_avg_ns"
    frames = []

    for level in C_LEVELS:
        if level == "Ofast":
            local = platform_dir / f"results_c({level})_median.csv"
            nested = platform_dir / "ofast_results" / f"results_c({level})_median.csv"
            c_file = local if local.exists() else nested
        else:
            c_file = platform_dir / f"results_c({level})_median.csv"

        frame = pd.read_csv(c_file)
        frame = frame.rename(columns={metric_col: "avg_ns"})
        frame["series"] = f"C-{level}"
        frame["platform"] = platform_name
        frames.append(frame[["type", "N", "avg_ns", "series", "platform"]])

    java_file = platform_dir / "results_java_median.csv"
    java_frame = pd.read_csv(java_file)
    java_frame = java_frame.rename(columns={metric_col: "avg_ns"})
    java_frame["series"] = "Java"
    java_frame["platform"] = platform_name
    frames.append(java_frame[["type", "N", "avg_ns", "series", "platform"]])

    out = pd.concat(frames, ignore_index=True)
    out["N"] = out["N"].astype(int)
    out["avg_ns"] = out["avg_ns"].astype(float)
    return out


def build_comparison_table(mac_df: pd.DataFrame, win_df: pd.DataFrame) -> pd.DataFrame:
    mac = mac_df.rename(columns={"avg_ns": "mac_avg_ns"}).drop(columns=["platform"])
    win = win_df.rename(columns={"avg_ns": "win_avg_ns"}).drop(columns=["platform"])

    merged = pd.merge(mac, win, on=["type", "N", "series"], how="inner")
    merged["win_over_mac"] = merged["win_avg_ns"] / merged["mac_avg_ns"]
    merged["mac_over_win"] = merged["mac_avg_ns"] / merged["win_avg_ns"]

    type_rank = {t: i for i, t in enumerate(TYPE_ORDER)}
    series_rank = {s: i for i, s in enumerate(SERIES_ORDER)}
    merged = merged.sort_values(
        by=["type", "series", "N"],
        key=lambda col: col.map(type_rank) if col.name == "type" else (col.map(series_rank) if col.name == "series" else col),
    )
    return merged


def plot_by_type(comparison_df: pd.DataFrame):
    present_types = list(dict.fromkeys(comparison_df["type"].tolist()))
    ordered_types = [t for t in TYPE_ORDER if t in present_types]
    ordered_types += [t for t in present_types if t not in ordered_types]

    for dtype in ordered_types:
        data = comparison_df[comparison_df["type"] == dtype]

        fig, axes = plt.subplots(1, 2, figsize=(17, 6.4), sharex=True)
        ax_abs, ax_ratio = axes

        for series in FOCUS_SERIES:
            sd = data[data["series"] == series].sort_values("N")
            if sd.empty:
                continue

            color = SERIES_COLOR.get(series)
            ax_abs.plot(
                sd["N"],
                sd["mac_avg_ns"],
                color=color,
                marker="o",
                linewidth=2.0,
                markersize=4.8,
                linestyle="-",
                label=f"{series} (macOS)",
            )
            ax_abs.plot(
                sd["N"],
                sd["win_avg_ns"],
                color=color,
                marker="s",
                linewidth=1.8,
                markersize=4.4,
                linestyle="--",
                label=f"{series} (Windows)",
            )

            ax_ratio.plot(
                sd["N"],
                sd["win_over_mac"],
                color=color,
                marker="o",
                linewidth=2.0,
                markersize=4.8,
                label=series,
            )

        ax_abs.set_xscale("log")
        ax_abs.set_yscale("log")
        ax_abs.set_xlabel("N (log scale)")
        ax_abs.set_ylabel("median avg_ns (log scale)")
        ax_abs.set_title("Absolute Time")
        ax_abs.grid(True, linestyle="--", alpha=0.35)

        ax_ratio.set_xscale("log")
        ax_ratio.axhline(1.0, color="black", linestyle=":", linewidth=1.2)
        ax_ratio.set_xlabel("N (log scale)")
        ax_ratio.set_ylabel("Windows / macOS")
        ax_ratio.set_title("Relative Ratio")
        ax_ratio.grid(True, linestyle="--", alpha=0.35)

        fig.suptitle(
            f"Windows vs macOS Dot Product (Median, C-Ofast + Java) - {TYPE_LABEL.get(dtype, dtype)}",
            fontsize=14,
        )

        handles1, labels1 = ax_abs.get_legend_handles_labels()
        handles2, labels2 = ax_ratio.get_legend_handles_labels()
        seen = set()
        merged_handles = []
        merged_labels = []
        for h, l in list(zip(handles1, labels1)) + list(zip(handles2, labels2)):
            if l in seen:
                continue
            seen.add(l)
            merged_handles.append(h)
            merged_labels.append(l)

        fig.legend(
            merged_handles,
            merged_labels,
            loc="upper center",
            ncol=3,
            frameon=False,
            bbox_to_anchor=(0.5, 1.07),
            fontsize=9,
        )

        fig.tight_layout()
        out_path = OUT_DIR / f"win_macos_compare_median_{TYPE_LABEL.get(dtype, dtype)}.png"
        fig.savefig(out_path, dpi=240, bbox_inches="tight")
        plt.close(fig)


def export_tables(comparison_df: pd.DataFrame):
    csv_path = OUT_DIR / "win_macos_ratio_median.csv"
    comparison_df.to_csv(csv_path, index=False)

    pivot = comparison_df.pivot_table(
        index=["type", "N"],
        columns="series",
        values="win_over_mac",
        aggfunc="first",
    ).reset_index()

    col_order = ["type", "N"] + FOCUS_SERIES
    for c in col_order:
        if c not in pivot.columns:
            pivot[c] = pd.NA
    pivot = pivot[col_order]

    type_rank = {t: i for i, t in enumerate(TYPE_ORDER)}
    pivot = pivot.sort_values(
        by=["type", "N"],
        key=lambda col: col.map(type_rank) if col.name == "type" else col,
    )

    pivot_csv = OUT_DIR / "win_macos_ratio_pivot_median.csv"
    pivot.to_csv(pivot_csv, index=False)

    md_path = OUT_DIR / "win_macos_ratio_table.md"
    with md_path.open("w", encoding="utf-8") as f:
        f.write("# Windows / macOS 性能比（Median）\n\n")
        f.write("- 比值定义：`Windows / macOS`。\n")
        f.write("- 小于 1 表示 Windows 更快；大于 1 表示 macOS 更快。\n\n")
        f.write(pivot.to_markdown(index=False, floatfmt=".4f"))
        f.write("\n")


def main():
    mac_df = _load_platform(BASE_DIR, "macOS")
    win_df = _load_platform(WIN_DIR, "Windows")

    comparison_df = build_comparison_table(mac_df, win_df)
    comparison_df = comparison_df[comparison_df["series"].isin(FOCUS_SERIES)].copy()
    plot_by_type(comparison_df)
    export_tables(comparison_df)

    print(f"Done. Outputs in: {OUT_DIR}")


if __name__ == "__main__":
    main()
