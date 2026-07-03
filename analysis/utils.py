import numpy as np
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D


def report_metrics(arr: np.ndarray, name: str = "Report") -> None:
    # Safety check for empty arrays
    if arr.size == 0:
        print(f"--- {name} ---")
        print("No data found.\n")
        return

    percentiles = np.percentile(arr, [25, 50, 75, 90, 99, 99.9, 99.99])
    print(f"=== {name} ===")
    print(f"{'Count':<10}: {arr.size:,}")
    print(f"{'Min':<10}: {np.min(arr):,.3f}")
    print(f"{'Max':<10}: {np.max(arr):,.3f}")
    print(f"{'Mean':<10}: {np.mean(arr):,.3f}")
    print(f"{'Std Dev':<10}: {np.std(arr):,.3f}")
    print(f"{'P50 (Med)':<10}: {percentiles[1]:,.3f}")
    print(f"{'P90':<10}: {percentiles[3]:,.3f}")
    print(f"{'P99':<10}: {percentiles[4]:,.3f}")
    print(f"{'P99.9':<10}: {percentiles[5]:,.3f}")
    print(f"{'P99.99':<10}: {percentiles[6]:,.3f}")
    print("=" * (8 + len(name)) + "\n")


def generate_percentiles(last: float) -> np.ndarray:
    """
    Generate a percentile array with increasing resolution as it approaches `last`.

    Resolution tiers (fixed breakpoints at 90 and 99):
      [0, 90) → step 1
      [90, 99) → step 0.1
      [99, 99.9) → step 0.01
      [99.9, last) → step 0.001
      + [last] appended
    """
    breakpoints = [(0, 90, 1), (90, 99, 0.1), (99, 99.9, 0.01), (99.9, 100, 0.001)]

    segments = []
    for start, end, step in breakpoints:
        if last <= start:
            break
        segments.append(np.arange(start, min(end, last), step))

    segments.append([last])
    return np.unique(np.concatenate(segments))


def plot_percentiles(
        *series: tuple[np.ndarray, str],
        percentiles: np.ndarray,
        xlabel: str = "Percentile",
        ylabel: str = "Delay / Delta (µs)",
        title: str = "Latency vs Percentile",
) -> None:
    last = percentiles[-1]

    # Fixed reference percentiles — skip those beyond last, deduplicate
    fixed_refs: list[float] = []
    seen: set[float] = set()
    pivot_percentiles = [50.0, 99.0, 99.9, 99.99]
    if abs(last - pivot_percentiles[-1]) > 0.1:
        pivot_percentiles.append(float(last))

    for p in pivot_percentiles:
        if p <= last and p not in seen:
            fixed_refs.append(p)
            seen.add(p)

    # Bright neon palette — legible on dark backgrounds
    PCT_STYLES = {
        50.0:  {"color": "#00FF7F", "name": "50th"},     # spring green
        99.0:  {"color": "red", "name": "99th"},     # deep sky blue
        99.9:  {"color": "#FF69B4", "name": "99.9th"},   # hot pink
        99.99: {"color": "#FF4500", "name": "99.99th"},  # orange red
    }
    LAST_COLOR = "#FFD700"  # gold — fallback if last isn't in PCT_STYLES

    fig, ax = plt.subplots(figsize=(14, 6))

    # ── Series lines + mean verticals ─────────────────────────────────────
    mean_handles: list[Line2D] = []

    for i, (latencies, label) in enumerate(series):
        ax.plot(percentiles, np.percentile(latencies, percentiles), linewidth=2, label=label)

        mean_val = np.mean(latencies)
        mean_pct = np.mean(latencies <= mean_val) * 100
        if mean_pct <= last:
            ax.axvline(mean_pct, linestyle=":", alpha=0.7, linewidth=1.5)
            mean_handles.append(Line2D(
                [0], [0], linestyle=":", linewidth=1.5,
                label=f"Mean — {label}: {mean_val:,.2f} µs",
            ))

    # ── Percentile reference verticals ────────────────────────────────────
    ref_handles: list[Line2D] = []

    for p in fixed_refs:
        style = PCT_STYLES.get(p, {"color": LAST_COLOR, "name": f"{p:g}th"})
        c, name = style["color"], style["name"]

        ax.axvline(p, linestyle="--", color=c, alpha=0.85, linewidth=1)

        vals = [np.percentile(data, p) for data, _ in series]
        val_str = (
            f"{vals[0]:,.2f} µs"
            if len(vals) == 1
            else "  |  ".join(f"{v:,.2f}" for v in vals) + " µs"
        )
        ref_handles.append(Line2D(
            [0], [0], linestyle="--", color=c, linewidth=1.5,
            label=f"{name} %ile:  {val_str}",
        ))

    # ── Formatting & legend ───────────────────────────────────────────────
    ax.set_xlabel(xlabel, fontsize=12)
    ax.set_ylabel(ylabel, fontsize=12)
    ax.set_title(title, fontsize=14)
    ax.grid(True, alpha=0.3)

    series_handles, _ = ax.get_legend_handles_labels()
    ax.legend(
        handles=series_handles + mean_handles + ref_handles,
        loc="upper left",
        fontsize=9,
        framealpha=0.85,
    )

    plt.tight_layout()
    plt.show()