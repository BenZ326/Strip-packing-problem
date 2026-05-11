import time
import unittest

from spp import pack


def _rectangles_overlap(a, b):
    ax, ay, aw, ah = a
    bx, by, bw, bh = b
    return not (ax + aw <= bx or bx + bw <= ax or ay + ah <= by or by + bh <= ay)


class PackApiTest(unittest.TestCase):
    def test_pack_all_items_without_overlap(self):
        items = [(2, 2), (2, 3), (3, 2), (1, 4)]
        bin_width = 6
        res = pack(
            items=items,
            bin_width=bin_width,
            branch_and_bound=True,
            benders=True,
            timeout=10,
        )

        self.assertEqual(res.state, "completed")
        self.assertIsNotNone(res.height)
        self.assertEqual(len(res.placements), len(items))

        rects = []
        for idx, (w, h) in enumerate(items):
            self.assertIn(idx, res.placements)
            x, y = res.placements[idx]
            self.assertGreaterEqual(x, 0)
            self.assertGreaterEqual(y, 0)
            self.assertLessEqual(x + w, bin_width)
            self.assertLessEqual(y + h, res.height)
            rects.append((x, y, w, h))

        for i in range(len(rects)):
            for j in range(i + 1, len(rects)):
                self.assertFalse(_rectangles_overlap(rects[i], rects[j]), f"Items {i} and {j} overlap")

    def test_timeout_respected(self):
        items = [(2, 2), (2, 3), (3, 2), (1, 4)]
        start = time.perf_counter()
        res = pack(
            items=items,
            bin_width=6,
            branch_and_bound=True,
            benders=True,
            timeout=0,
        )
        elapsed = time.perf_counter() - start

        self.assertEqual(res.state, "timeout")
        self.assertIsNone(res.height)
        self.assertEqual(res.placements, {})
        self.assertLess(elapsed, 1.0)


if __name__ == "__main__":
    unittest.main()
