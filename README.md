# Strip Packing Problem

This repository implements a 2D strip-packing solver based on combinatorial Benders cuts (as described in the Operations Research paper: https://pubsonline.informs.org/doi/abs/10.1287/opre.2013.1248).

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
