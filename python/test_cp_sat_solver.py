import unittest

from cp_sat_solver import CPSATSolver


def rectangles_overlap(a, b):
    ax, ay, aw, ah = a
    bx, by, bw, bh = b
    return not (ax + aw <= bx or bx + bw <= ax or ay + ah <= by or by + bh <= ay)


class CPSATSolverTest(unittest.TestCase):
    def test_solves_small_strip_packing_instance(self):
        items = [(2, 2), (2, 3), (3, 2), (1, 4)]
        result = CPSATSolver(items, 6).solve(time_limit_seconds=10)

        self.assertEqual(result.status, "optimal")
        self.assertEqual(result.height, 4)
        self.assertEqual(len(result.placements), len(items))

        rectangles = []
        for item_index, (width, height) in enumerate(items):
            x, y = result.placements[item_index]
            self.assertGreaterEqual(x, 0)
            self.assertGreaterEqual(y, 0)
            self.assertLessEqual(x + width, 6)
            self.assertLessEqual(y + height, result.height)
            rectangles.append((x, y, width, height))

        for i in range(len(rectangles)):
            for j in range(i + 1, len(rectangles)):
                self.assertFalse(rectangles_overlap(rectangles[i], rectangles[j]))

    def test_rejects_item_wider_than_strip(self):
        with self.assertRaises(ValueError):
            CPSATSolver([(7, 1)], 6)


if __name__ == "__main__":
    unittest.main()
