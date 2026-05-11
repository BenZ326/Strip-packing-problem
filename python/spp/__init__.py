from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple

from ._spp_native import pack_native


@dataclass
class spp_result:
    placements: Dict[int, Tuple[int, int]]
    items: List[Tuple[int, int]]
    bin_width: int
    state: str
    height: Optional[int]


def pack(
    items: List[Tuple[int, int]],
    bin_width: int,
    branch_and_bound: bool,
    benders: bool,
    timeout: int,
) -> spp_result:
    native = pack_native(
        items=items,
        bin_width=bin_width,
        branch_and_bound=branch_and_bound,
        benders=benders,
        timeout=timeout,
    )
    return spp_result(
        placements=dict(native["placements"]),
        items=list(items),
        bin_width=bin_width,
        state=str(native["state"]),
        height=None if native["height"] is None else int(native["height"]),
    )


def plot_pack(result: spp_result):
    import matplotlib.pyplot as plt
    from matplotlib.patches import Rectangle

    if result.state != "completed":
        raise ValueError("plot_pack requires a completed solution")

    if result.height is not None:
        strip_height = result.height
    else:
        strip_height = 0
        for idx, (x, y) in result.placements.items():
            _, h = result.items[idx]
            strip_height = max(strip_height, y + h)

    fig, ax = plt.subplots()
    for idx, (x, y) in result.placements.items():
        w, h = result.items[idx]
        rect = Rectangle((x, y), w, h, fill=False, linewidth=1.5)
        ax.add_patch(rect)
        ax.text(x + w / 2.0, y + h / 2.0, str(idx), ha="center", va="center", fontsize=8)

    ax.set_xlim(0, result.bin_width)
    ax.set_ylim(0, max(strip_height, 1))
    ax.set_aspect("equal", adjustable="box")
    ax.set_title(f"Strip Packing ({result.state})")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    plt.tight_layout()
    return fig, ax


__all__ = ["spp_result", "pack", "plot_pack"]
