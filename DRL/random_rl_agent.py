"""Random RL baseline for strip packing with skyline-based actions.

This module provides:
1. A simple strip packing RL environment.
2. A random-action agent.
3. A visualization utility for packed rectangles in the strip.
"""

from __future__ import annotations

from dataclasses import dataclass
import random
from typing import List, Sequence, Tuple


Item = Tuple[int, int]  # (width, height)


@dataclass(frozen=True)
class Placement:
    """A placed rectangle in the strip."""

    item_index: int
    x: int
    y: int
    width: int
    height: int


class StripPackingEnv:
    """Skyline-based strip packing environment.

    State:
      - skyline vector
      - remaining items (ordered)
      - bin width

    Action:
      - x-coordinate for placing the next item, restricted to skyline anchor points

    Reward:
      - Terminal reward is -final_height; otherwise 0
    """

    def __init__(self, bin_width: int, items: Sequence[Item]) -> None:
        if bin_width <= 0:
            raise ValueError("bin_width must be positive")
        if not items:
            raise ValueError("items must be non-empty")
        for idx, (w, h) in enumerate(items):
            if w <= 0 or h <= 0:
                raise ValueError(f"item {idx} has non-positive dimension: {(w, h)}")
            if w > bin_width:
                raise ValueError(f"item {idx} width {w} exceeds bin width {bin_width}")

        self.bin_width = int(bin_width)
        self.items: List[Item] = [(int(w), int(h)) for w, h in items]
        self.reset()

    def reset(self) -> Tuple[Tuple[int, ...], Tuple[Item, ...], int]:
        self.skyline = [0 for _ in range(self.bin_width)]
        self.current_index = 0
        self.placements: List[Placement] = []
        return self.state

    @property
    def state(self) -> Tuple[Tuple[int, ...], Tuple[Item, ...], int]:
        remaining = tuple(self.items[self.current_index :])
        return tuple(self.skyline), remaining, self.bin_width

    @property
    def done(self) -> bool:
        return self.current_index >= len(self.items)

    @property
    def final_height(self) -> int:
        return max(self.skyline) if self.skyline else 0

    def available_actions(self) -> List[int]:
        if self.done:
            return []
        next_w, _ = self.items[self.current_index]
        if next_w > self.bin_width:
            return []

        # Skyline anchors are x=0 and skyline breakpoints where height changes.
        anchors = [0]
        for x in range(1, self.bin_width):
            if self.skyline[x] != self.skyline[x - 1]:
                anchors.append(x)

        valid = [x for x in anchors if x + next_w <= self.bin_width]
        return valid

    def step(
        self, action_x: int
    ) -> Tuple[Tuple[Tuple[int, ...], Tuple[Item, ...], int], float, bool, dict]:
        if self.done:
            raise RuntimeError("episode is done; call reset()")

        actions = self.available_actions()
        if action_x not in actions:
            raise ValueError(f"invalid action {action_x}; valid actions are {actions}")

        w, h = self.items[self.current_index]
        y = max(self.skyline[action_x : action_x + w])
        new_height = y + h
        for x in range(action_x, action_x + w):
            self.skyline[x] = new_height

        placement = Placement(
            item_index=self.current_index,
            x=action_x,
            y=y,
            width=w,
            height=h,
        )
        self.placements.append(placement)
        self.current_index += 1

        done = self.done
        reward = float(-self.final_height) if done else 0.0
        info = {"placement": placement, "height": self.final_height}
        return self.state, reward, done, info


class RandomPlacementAgent:
    """Uniform-random action baseline."""

    def __init__(self, seed: int | None = None) -> None:
        self._rng = random.Random(seed)

    def select_action(self, available_actions: Sequence[int]) -> int:
        if not available_actions:
            raise RuntimeError("no available actions")
        return self._rng.choice(list(available_actions))

    def run_episode(self, env: StripPackingEnv) -> Tuple[float, List[Placement], int]:
        env.reset()
        total_reward = 0.0

        while not env.done:
            action = self.select_action(env.available_actions())
            _, reward, _, _ = env.step(action)
            total_reward += reward

        return total_reward, list(env.placements), env.final_height


def visualize_packing(
    bin_width: int,
    placements: Sequence[Placement],
    *,
    title: str = "Strip Packing",
    save_path: str | None = None,
    show: bool = True,
) -> None:
    """Visualize rectangle placements in the strip/bin."""

    try:
        import matplotlib.pyplot as plt
        from matplotlib.patches import Rectangle
    except ImportError as exc:
        raise ImportError(
            "matplotlib is required for visualization. Install with: pip install matplotlib"
        ) from exc

    if bin_width <= 0:
        raise ValueError("bin_width must be positive")

    final_height = 0
    for p in placements:
        final_height = max(final_height, p.y + p.height)

    fig_h = max(4.0, min(12.0, final_height / 4.0 + 1.0))
    fig, ax = plt.subplots(figsize=(10, fig_h))
    cmap = plt.get_cmap("tab20")

    for idx, p in enumerate(placements):
        color = cmap(idx % 20)
        rect = Rectangle((p.x, p.y), p.width, p.height, facecolor=color, edgecolor="black")
        ax.add_patch(rect)
        ax.text(
            p.x + p.width / 2.0,
            p.y + p.height / 2.0,
            str(p.item_index),
            ha="center",
            va="center",
            fontsize=8,
            color="black",
        )

    ax.set_xlim(0, bin_width)
    ax.set_ylim(0, max(1, final_height))
    ax.set_aspect("equal", adjustable="box")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_title(title)
    ax.grid(True, linestyle="--", linewidth=0.5, alpha=0.5)

    if save_path:
        fig.savefig(save_path, dpi=180, bbox_inches="tight")
    if show:
        plt.show()
    else:
        plt.close(fig)


if __name__ == "__main__":
    # Example usage: python DRL/random_rl_agent.py
    
    example_items: List[Item] = [(3, 2), (4, 3), (2, 5), (1, 2), (5, 2), (2, 1)]
    env = StripPackingEnv(bin_width=10, items=example_items)
    agent = RandomPlacementAgent(seed=113)

    reward, placements, height = agent.run_episode(env)
    print(f"Episode done. Reward={reward:.1f}, final height={height}")
    for p in placements:
        print(f"item#{p.item_index}: x={p.x}, y={p.y}, w={p.width}, h={p.height}")

    visualize_packing(env.bin_width, placements, title=f"Random packing (height={height})")
