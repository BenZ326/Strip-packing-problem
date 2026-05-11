from dataclasses import dataclass
from typing import Optional

from matplotlib.axes import Axes
from matplotlib.figure import Figure

Item = tuple[int, int]
Coordinate = tuple[int, int]


@dataclass
class spp_result:
    """Result container returned by :func:`pack`.

    Attributes:
        placements: Mapping from input item index to bottom-left coordinate ``(x, y)``.
        items: Original input ``(width, height)`` list in input order.
        bin_width: Width of the strip/bin used for packing.
        state: Solver state, either ``"completed"`` or ``"timeout"``.
        height: Packed strip height when completed, otherwise ``None``.
    """

    placements: dict[int, Coordinate]
    items: list[Item]
    bin_width: int
    state: str
    height: Optional[int]


def pack(
    items: list[Item],
    bin_width: int,
    branch_and_bound: bool,
    benders: bool,
    timeout: int,
) -> spp_result:
    """Solve a 2D strip-packing instance.

    Args:
        items: List of ``(width, height)`` tuples.
        bin_width: Width of the strip/bin.
        branch_and_bound: Enable branch-and-bound phase.
        benders: Enable combinatorial Benders phase.
        timeout: Time budget in seconds. ``0`` returns immediate timeout.

    Returns:
        A :class:`spp_result` containing placements, state, and height.
    """
    ...


def plot_pack(result: spp_result) -> tuple[Figure, Axes]:
    """Visualize a completed packing solution with Matplotlib.

    Args:
        result: A completed :class:`spp_result`.

    Returns:
        ``(figure, axes)`` used for the plot.
    """
    ...


__all__: list[str]
