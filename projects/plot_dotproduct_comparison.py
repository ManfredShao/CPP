from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

BASE_DIR = Path(__file__).resolve().parent
OUT_DIR = BASE_DIR / "output_plots"
OUT_DIR.mkdir(parents=True, exist_ok=True)

C_LEVELS = ["O0", "O1", "O2", "O3", "Ofast", "Ofast_native"]
TYPE_ORDER = ["schar", "short", "int", "float", "double"]
TYPE_LABEL = {
    "schar": "signed_char",
    "short": "short",
    "int": "int",
    "float": "float",
    "double": "double",
}


def load_metric_data(metric: str):
    if metric not in {"mean", "median"}:
        raise ValueError("metric must be 'mean' or 'median'")

    metric_col = f"{metric}_avg_ns"

    c_frames = []
    for level in C_LEVELS:
        if level == "Ofast":
            file_path = BASE_DIR / "ofast_results" / f"results_c({level})_{metric}.csv"
        elif level == "Ofast_native":
            file_path = BASE_DIR / "ofast_native_results" / f"results_c({level})_{metric}.csv"
        else:
            file_path = BASE_DIR / f"results_c({level})_{metric}.csv"
        frame = pd.read_csv(file_path)
        frame = frame.rename(columns={metric_col: "avg_ns"})
        frame["series"] = f"C-{level}"
        c_frames.append(frame[["type", "N", "avg_ns", "series"]])

    java_file = BASE_DIR / f"results_java_{metric}.csv"
    java_frame = pd.read_csv(java_file)
    java_frame = java_frame.rename(columns={metric_col: "avg_ns"})
    java_frame["series"] = "Java"
    java_frame = java_frame[["type", "N", "avg_ns", "series"]]

    combined = pd.concat(c_frames + [java_frame], ignore_index=True)
    combined["N"] = combined["N"].astype(int)
    combined["avg_ns"] = combined["avg_ns"].astype(float)
    return combined


def plot_by_type(median_df: pd.DataFrame):
    series_order = [f"C-{level}" for level in C_LEVELS] + ["Java"]
    colors = {
        "C-O0": "#1f77b4",
        "C-O1": "#2ca02c",
        "C-O2": "#ff7f0e",
        "C-O3": "#9467bd",
        "C-Ofast": "#17becf",
        "C-Ofast_native": "#8c564b",
        "Java": "#d62728",
    }

    present_types = sorted(set(median_df["type"]))
    ordered_types = [t for t in TYPE_ORDER if t in present_types]
    ordered_types += [t for t in present_types if t not in ordered_types]

    for dtype in ordered_types:
        fig, axes = plt.subplots(1, 2, figsize=(16, 6.2), sharex=True)
        subset = median_df[median_df["type"] == dtype]

        ax_full = axes[0]
        for series in series_order:
            series_data = subset[subset["series"] == series].sort_values("N")
            if series_data.empty:
                continue
            ax_full.plot(
                series_data["N"],
                series_data["avg_ns"],
                marker="o",
                markersize=5.5,
                linewidth=2.2,
                label=series,
                color=colors.get(series),
            )

        ax_full.set_xscale("log")
        ax_full.set_yscale("log")
        ax_full.set_xlabel("N (log scale)")
        ax_full.set_ylabel("median avg_ns (log scale)")
        ax_full.set_title("Median (All Series)")
        ax_full.grid(True, linestyle="--", alpha=0.35)

        ax_zoom = axes[1]
        zoom_series_order = ["C-O1", "C-O2", "C-O3", "C-Ofast", "C-Ofast_native", "Java"]
        for series in zoom_series_order:
            series_data = subset[subset["series"] == series].sort_values("N")
            if series_data.empty:
                continue
            ax_zoom.plot(
                series_data["N"],
                series_data["avg_ns"],
                marker="o",
                markersize=5.5,
                linewidth=2.2,
                label=series,
                color=colors.get(series),
            )

        ax_zoom.set_xscale("log")
        ax_zoom.set_xlabel("N (log scale)")
        ax_zoom.set_ylabel("median avg_ns")
        ax_zoom.set_title("Zoom View (C-O1/O2/O3/Ofast/Ofast_native vs Java)")
        ax_zoom.grid(True, linestyle="--", alpha=0.35)

        fig.suptitle(
            f"Dot Product Time Comparison (Median) - {TYPE_LABEL.get(dtype, dtype)}",
            fontsize=14,
        )
        handles, labels = ax_full.get_legend_handles_labels()
        zoom_handles, zoom_labels = ax_zoom.get_legend_handles_labels()
        fig.legend(
            handles + [h for h, l in zip(zoom_handles, zoom_labels) if l not in labels],
            labels + [l for l in zoom_labels if l not in labels],
            loc="upper center",
            ncol=7,
            frameon=False,
            bbox_to_anchor=(0.5, 1.04),
        )
        fig.tight_layout()

        out_file = OUT_DIR / f"dot_time_compare_median_{dtype}.png"
        fig.savefig(out_file, dpi=240, bbox_inches="tight")
        plt.close(fig)


def build_per_element_tables(mean_df: pd.DataFrame, median_df: pd.DataFrame):
    def make_table(df: pd.DataFrame, metric: str):
        table = df.copy()
        table["ns_per_element"] = table["avg_ns"] / table["N"]
        pivot = table.pivot_table(
            index=["type", "N"],
            columns="series",
            values="ns_per_element",
            aggfunc="first",
        ).reset_index()

        col_order = ["type", "N"] + [f"C-{level}" for level in C_LEVELS] + ["Java"]
        for c in col_order:
            if c not in pivot.columns:
                pivot[c] = pd.NA
        pivot = pivot[col_order]

        pivot = pivot.sort_values(
            by=["type", "N"],
            key=lambda col: col.map({t: i for i, t in enumerate(TYPE_ORDER)}) if col.name == "type" else col,
        )

        csv_path = OUT_DIR / f"ns_per_element_{metric}.csv"
        pivot.to_csv(csv_path, index=False)
        return pivot

    mean_table = make_table(mean_df, "mean")
    median_table = make_table(median_df, "median")

    md_path = OUT_DIR / "ns_per_element_table.md"
    with md_path.open("w", encoding="utf-8") as f:
        f.write("# 每元素耗时（avg_ns / N）\n\n")
        f.write("## Mean\n\n")
        f.write(mean_table.to_markdown(index=False, floatfmt=".6f"))
        f.write("\n\n## Median\n\n")
        f.write(median_table.to_markdown(index=False, floatfmt=".6f"))
        f.write("\n")


def main():
    median_df = load_metric_data("median")

    plot_by_type(median_df)

    mean_df = load_metric_data("mean")
    build_per_element_tables(mean_df, median_df)

    print(f"Done. Outputs in: {OUT_DIR}")


if __name__ == "__main__":
    main()
