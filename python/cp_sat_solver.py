from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, Iterable, List, Optional, Tuple

from ortools.sat.python import cp_model

Item = Tuple[int, int]
Coordinate = Tuple[int, int]


@dataclass(frozen=True)
class CPSATResult:
    status: str
    height: Optional[int]
    placements: Dict[int, Coordinate]
    objective_bound: Optional[float]
    wall_time: float


class CPSATSolver:
    """Direct OR-Tools CP-SAT model for the 2D strip packing problem.

    This implements the mathematical-logic model from Section 2.2 of
    "Combinatorial Benders' Cuts for the Strip Packing Problem": choose one
    horizontal position for each item, optimize the strip height, and enforce
    per-column vertical no-overlap with optional intervals.
    """

    def __init__(
        self,
        items: Iterable[Item],
        strip_width: int,
        *,
        upper_bound: Optional[int] = None,
        use_normal_patterns: bool = True,
    ) -> None:
        self.items = list(items)
        self.strip_width = strip_width
        self.upper_bound = upper_bound
        self.use_normal_patterns = use_normal_patterns
        self._validate_input()

    def solve(self, time_limit_seconds: Optional[float] = None) -> CPSATResult:
        if time_limit_seconds == 0:
            return CPSATResult("timeout", None, {}, None, 0.0)

        height_ub = self.upper_bound or sum(height for _, height in self.items)
        model = cp_model.CpModel()

        z = model.new_int_var(0, height_ub, "z")
        y = [
            model.new_int_var(0, height_ub - height, f"y_{j}")
            for j, (_, height) in enumerate(self.items)
        ]

        options = self._horizontal_options()
        x = {}
        y_intervals = {}
        for j, (_, height) in enumerate(self.items):
            for p in options[j]:
                x[j, p] = model.new_bool_var(f"x_{j}_{p}")
                y_intervals[j, p] = model.new_optional_fixed_size_interval_var(
                    y[j], height, x[j, p], f"y_interval_{j}_{p}"
                )

        for j in range(len(self.items)):
            model.add_exactly_one(x[j, p] for p in options[j])

        for j, (_, height) in enumerate(self.items):
            model.add(y[j] + height <= z)

        # Constraint (7): column load lower bound on z.
        # Constraint (9): selected items covering the same column cannot overlap vertically.
        for q in range(self.strip_width):
            load_terms = []
            intervals_covering_q = []
            for j, (width, height) in enumerate(self.items):
                for p in options[j]:
                    if p <= q < p + width:
                        load_terms.append(height * x[j, p])
                        intervals_covering_q.append(y_intervals[j, p])
            model.add(sum(load_terms) <= z)
            if len(intervals_covering_q) > 1:
                model.add_no_overlap(intervals_covering_q)

        model.minimize(z)

        solver = cp_model.CpSolver()
        if time_limit_seconds is not None and time_limit_seconds > 0:
            solver.parameters.max_time_in_seconds = float(time_limit_seconds)

        status = solver.solve(model)
        status_name = self._status_name(status)
        if status in (cp_model.OPTIMAL, cp_model.FEASIBLE):
            placements: Dict[int, Coordinate] = {}
            for j in range(len(self.items)):
                for p in options[j]:
                    if solver.boolean_value(x[j, p]):
                        placements[j] = (p, solver.value(y[j]))
                        break
            return CPSATResult(
                status=status_name,
                height=solver.value(z),
                placements=placements,
                objective_bound=solver.best_objective_bound,
                wall_time=solver.wall_time,
            )

        return CPSATResult(
            status=status_name,
            height=None,
            placements={},
            objective_bound=solver.best_objective_bound,
            wall_time=solver.wall_time,
        )

    def _validate_input(self) -> None:
        if self.strip_width <= 0:
            raise ValueError("strip_width must be positive")
        if not self.items:
            raise ValueError("items must not be empty")
        for width, height in self.items:
            if width <= 0 or height <= 0:
                raise ValueError("item dimensions must be positive")
            if width > self.strip_width:
                raise ValueError("item width cannot exceed strip_width")
        if self.upper_bound is not None:
            if self.upper_bound <= 0:
                raise ValueError("upper_bound must be positive")
            if any(height > self.upper_bound for _, height in self.items):
                raise ValueError("upper_bound is smaller than at least one item height")

    def _horizontal_options(self) -> List[List[int]]:
        if not self.use_normal_patterns:
            return [
                list(range(self.strip_width - width + 1))
                for width, _ in self.items
            ]
        return [
            self._normal_patterns_for_item(j)
            for j in range(len(self.items))
        ]

    def _normal_patterns_for_item(self, item_index: int) -> List[int]:
        width = self.items[item_index][0]
        limit = self.strip_width - width
        reachable = [False] * (limit + 1)
        reachable[0] = True
        for other_index, (other_width, _) in enumerate(self.items):
            if other_index == item_index:
                continue
            for p in range(limit - other_width, -1, -1):
                if reachable[p]:
                    reachable[p + other_width] = True
        return [p for p, is_reachable in enumerate(reachable) if is_reachable]

    @staticmethod
    def _status_name(status: int) -> str:
        if status == cp_model.OPTIMAL:
            return "optimal"
        if status == cp_model.FEASIBLE:
            return "feasible"
        if status == cp_model.INFEASIBLE:
            return "infeasible"
        if status in (cp_model.MODEL_INVALID, cp_model.UNKNOWN):
            return "timeout"
        return "unknown"


CP_SAT_Solver = CPSATSolver
