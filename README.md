# What does the repo do?

This repository offers the capability of solving the Strip Packing Problem exactly based on the solver proposed in the milestone paper https://pubsonline.informs.org/doi/abs/10.1287/opre.2013.1248.

## Requirements

- CMake >= 3.16
- C++17 compiler (`g++`/`clang++`)
- Bundled HiGHS dependency at `third_party/highs`
- Optional: GoogleTest (only for unit tests)

## Build

```bash
cmake -S . -B build -DSTRIP_PACKING_BUILD_TESTS=ON
cmake --build build -j
```

The executable is produced at `build/spp` (or `./spp` if you build in-source).

## Run

Only these CLI arguments are accepted:

- `--time_buget <seconds>` or `--time_budget <seconds>`: maximum runtime budget for solving
- `--problem_path <path>`: path to a single `.TXT` instance file

Examples:

```bash
./build/spp --time_buget 60 --problem_path ./test/2sp/HT01.TXT
./build/spp --problem_path ./test/2sp/HT01.TXT
```

If `--problem_path` is omitted, the default is `./test/2sp/HT01.TXT`.

## Output

After solving, the program reports:

- `state: completed` and `height: <value>` when solved within time budget
- `state: timeout` when the run exceeds the time budget or stops in pending/approximate mode

If time budget is `0`, the solver returns `state: timeout` immediately.

## Input Format

The solver expects a `.TXT` file with:

1. First line: number of items `n`
2. Second line: strip width `W`
3. Next exactly `n` lines: `item_id width height`

The parser validates:

- file can be opened
- `n` and `W` are positive integers
- each item row has exactly 3 integers
- item width/height are positive
- number of parsed item rows must equal `n`

## Tests

Run all configured tests:

```bash
cd build
ctest --output-on-failure
```

Current test set includes:

- unit tests for selected utility/rotation behavior (when GoogleTest is available)
- parser validation tests for malformed/valid input
- CLI integration tests for argument validation, known-height regressions, and timeout reporting

## Benchmark (Paper Comparison)

We benchmarked the solver on the canonical `test/2sp` instances copied into `benchmark_instances`:

- `BENG01..10`
- `CGCUT01..03`
- `GCUT01..04`
- `HT01..09`
- `NGCUT01..12`

Run command used for each instance:

```bash
./build/spp --time_buget 120 --problem_path <instance.TXT>
```

Results are stored in:

- `research/spp_run_120_results.csv`

This CSV includes both:

- solver output (`solver_height_120s`)
- paper-reported BLUE results (`opt_height_or_bound`, `blue_runtime_sec`, `proven_optimal`)

Summary (38 instances total):

| Set   | Instances | Matched Paper Optimum | Timeout (`-1`) |
|-------|-----------|-----------------------|----------------|
| BENG  | 10        | 10                    | 0              |
| CGCUT | 3         | 1                     | 2              |
| GCUT  | 4         | 3                     | 1              |
| HT    | 9         | 8                     | 1              |
| NGCUT | 12        | 12                    | 0              |
| Total | 38        | 34                    | 4              |

Interpretation:

- On all instances solved within the 120s limit, the returned height matches the paper optimum for this benchmark slice.
- The current implementation times out on 4/38 instances (`CGCUT02`, `CGCUT03`, `GCUT04`, `HT08`), while the paper's BLUE implementation reports optimal solutions for them.
- This supports that the solver behaves as an exact method when it closes an instance, but is currently weaker than the paper implementation in robustness/performance on harder cases.

## CP-SAT Solver Performance

A standalone OR-Tools CP-SAT prototype is available in `python/cp_sat_solver.py`. It is independent from the C++/HiGHS solver and directly models Section 2.2 with binary horizontal-position variables, integer vertical coordinates, column-load constraints, and per-column `add_no_overlap` constraints over optional intervals.

Run command used for each instance:

```bash
PYTHONPATH=python python - <<'PY'
from cp_sat_solver import CPSATSolver
# parse one benchmark_instances/*.TXT file into (items, strip_width), then:
result = CPSATSolver(items, strip_width).solve(time_limit_seconds=120)
PY
```

Partial results are stored in:

- `research/cpsat_run_120_results.csv`

The current CP-SAT run was intentionally stopped after 12/38 benchmark instances, so this is not a full-suite comparison. Status `optimal` means CP-SAT proved optimality within 120 seconds; status `feasible` means it found a non-overlapping layout but did not close the optimality gap within the time limit.

Summary of completed CP-SAT rows:

| Set   | Completed Instances | Optimal | Feasible Only |
|-------|---------------------|---------|---------------|
| BENG  | 10                  | 2       | 8             |
| CGCUT | 2                   | 1       | 1             |
| Total | 12                  | 3       | 9             |

Instance-level CP-SAT results so far:

| Instance | Items | Width | Status | Height 120s | Bound | Wall Time (s) |
|----------|-------|-------|--------|-------------|-------|---------------|
| BENG01   | 20    | 25    | optimal | 30          | 30    | 2.61          |
| BENG02   | 40    | 25    | feasible | 58         | 57    | 120.14        |
| BENG03   | 60    | 25    | feasible | 86         | 84    | 120.31        |
| BENG04   | 80    | 25    | feasible | 112        | 107   | 120.17        |
| BENG05   | 100   | 25    | feasible | 148        | 134   | 120.13        |
| BENG06   | 40    | 40    | optimal | 36          | 36    | 52.67         |
| BENG07   | 80    | 40    | feasible | 72         | 67    | 120.16        |
| BENG08   | 120   | 40    | feasible | 115        | 101   | 120.08        |
| BENG09   | 160   | 40    | feasible | 151        | 126   | 120.10        |
| BENG10   | 200   | 40    | feasible | 189        | 156   | 120.07        |
| CGCUT01  | 16    | 10    | optimal | 27          | 27    | 0.02          |
| CGCUT02  | 23    | 70    | feasible | 65         | 63    | 120.30        |

Interpretation:

- The direct CP-SAT model can solve small instances and prove optimality on `BENG01`, `BENG06`, and `CGCUT01` within 120 seconds.
- On most completed medium/larger BENG instances, CP-SAT finds feasible layouts but leaves a positive optimality gap at the 120-second limit.
- This direct full-model CP-SAT prototype is useful as a correctness baseline and independent comparison, but the current C++ Benders-style solver is substantially stronger on this benchmark slice.

## License

MIT. See [LICENSE](LICENSE).

## Python API (PyPI)

The package exposes:

- `pack(items: list[tuple[int, int]], bin_width: int, branch_and_bound: bool, benders: bool, timeout: int) -> spp_result`
- `plot_pack(result: spp_result)`

`spp_result` contains:

- `placements: dict[int, tuple[int, int]]` where key is item index and value is `(x, y)`
- `items: list[tuple[int, int]]` original input items

Additional fields are also provided: `bin_width`, `state`, and `height`.

Example:

```python
from spp import pack, plot_pack

items = [(3, 4), (2, 5), (4, 2)]
res = pack(items=items, bin_width=6, branch_and_bound=True, benders=True, timeout=30)
print(res.state, res.height, res.placements)
if res.state == "completed":
    fig, ax = plot_pack(res)
```


## Roadmap for V 0.2.1
* Better readme file to clarify what does the repo do;
* Make it clear that before v 1.0.0, the repo is still not in a stable state.