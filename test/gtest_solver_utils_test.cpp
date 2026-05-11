#include <gtest/gtest.h>

#include "BLEU.h"
#include "datareader.h"
#include "spp.h"

#include <filesystem>
#include <fstream>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

class BLUETestHarness : public StripPacking::BLEU {
public:
	using StripPacking::BLEU::BLEU;
	using StripPacking::BLEU::ifRotateInstance;
	using StripPacking::BLEU::rotateInstance;
};

std::vector<StripPacking::item*> makeItems(const std::vector<std::pair<int, int>>& dims) {
	std::vector<StripPacking::item*> items;
	items.reserve(dims.size());
	for (size_t i = 0; i < dims.size(); ++i) {
		items.push_back(new StripPacking::item(static_cast<int>(i), dims[i].first, dims[i].second, static_cast<int>(i)));
	}
	return items;
}

std::vector<const StripPacking::item*> asConst(const std::vector<StripPacking::item*>& items) {
	std::vector<const StripPacking::item*> out;
	out.reserve(items.size());
	for (const auto* it : items) out.push_back(it);
	return out;
}

void freeItems(std::vector<StripPacking::item*>& items) {
	for (auto* it : items) delete it;
	items.clear();
}

void freeConstItems(std::vector<const StripPacking::item*>& items) {
	for (auto* it : items) delete it;
	items.clear();
}

std::filesystem::path makeTempInstanceFile(const std::string& content, const std::string& name) {
	const auto path = std::filesystem::temp_directory_path() / name;
	std::ofstream out(path);
	if (!out.is_open()) throw std::runtime_error("failed to create temp file");
	out << content;
	out.close();
	return path;
}

}  // namespace

TEST(ComputeFXTest, WidthPatternsForConstItems) {
	auto owned = makeItems({{2, 1}, {3, 2}, {5, 3}});
	auto items = asConst(owned);

	// Excluding item 0, subset sums of widths {3,5} up to 5 are {0,3,5}.
	const auto positions = StripPacking::computeFX(5, 0, items, true);
	const std::set<int> expected = {0, 3, 5};

	EXPECT_EQ(positions, expected);
	freeItems(owned);
}

TEST(ComputeFXTest, HeightPatternsForMutableItems) {
	auto items = makeItems({{3, 2}, {2, 4}, {1, 5}});

	// Excluding item 1, subset sums of heights {2,5} up to 7 are {0,2,5,7}.
	const auto positions = StripPacking::computeFX(7, 1, items, false);
	const std::set<int> expected = {0, 2, 5, 7};

	EXPECT_EQ(positions, expected);
	freeItems(items);
}

TEST(ComputeFXTest, NegativeCapacityReturnsEmpty) {
	auto owned = makeItems({{2, 1}, {3, 2}});
	auto constItems = asConst(owned);

	EXPECT_TRUE(StripPacking::computeFX(-1, 0, constItems, true).empty());
	EXPECT_TRUE(StripPacking::computeFX(-1, 0, owned, false).empty());
	freeItems(owned);
}

TEST(RotationTest, RotateInstanceSwapsItemAndBinDimensions) {
	auto seedOwned = makeItems({{1, 1}});
	auto seedConst = asConst(seedOwned);
	BLUETestHarness bleu(seedConst, 10, 1);

	auto items = makeItems({{4, 1}, {2, 7}, {5, 3}});
	int binWidth = 9;
	int binHeight = 11;

	bleu.rotateInstance(items, binWidth, binHeight);

	EXPECT_EQ(binWidth, 11);
	EXPECT_EQ(binHeight, 9);
	EXPECT_EQ(items[0]->width, 1);
	EXPECT_EQ(items[0]->height, 4);
	EXPECT_EQ(items[1]->width, 7);
	EXPECT_EQ(items[1]->height, 2);
	EXPECT_EQ(items[2]->width, 3);
	EXPECT_EQ(items[2]->height, 5);

	freeItems(items);
	freeItems(seedOwned);
}

TEST(RotationTest, IfRotateInstanceDetectsBetterOrientation) {
	auto seedOwned = makeItems({{1, 1}});
	auto seedConst = asConst(seedOwned);
	BLUETestHarness bleu(seedConst, 10, 1);

	auto items = makeItems({{4, 1}, {4, 1}, {1, 4}});
	// For this set, height-pattern count is smaller than width-pattern count.
	EXPECT_TRUE(bleu.ifRotateInstance(items, 5, 10));

	freeItems(items);
	freeItems(seedOwned);
}

TEST(RotationTest, IfRotateInstanceReturnsFalseWhenNotSmaller) {
	auto seedOwned = makeItems({{1, 1}});
	auto seedConst = asConst(seedOwned);
	BLUETestHarness bleu(seedConst, 10, 1);

	auto items = makeItems({{2, 2}, {3, 1}});
	// Equal-or-larger height-pattern count should not trigger rotation.
	EXPECT_FALSE(bleu.ifRotateInstance(items, 8, 5));

	freeItems(items);
	freeItems(seedOwned);
}

TEST(ReadDataTest, ParsesExactNumberOfRows) {
	const auto path = makeTempInstanceFile(
		"2\n"
		"10\n"
		"1 3 4\n"
		"2 2 5\n",
		"strip_packing_valid_instance.txt");

	std::vector<const StripPacking::item*> items;
	const int width = readData(path.string(), items);

	EXPECT_EQ(width, 10);
	ASSERT_EQ(items.size(), 2u);
	EXPECT_EQ(items[0]->width, 3);
	EXPECT_EQ(items[0]->height, 4);
	EXPECT_EQ(items[1]->width, 2);
	EXPECT_EQ(items[1]->height, 5);

	freeConstItems(items);
	std::filesystem::remove(path);
}

TEST(ReadDataTest, RejectsMismatchedRowCount) {
	const auto path = makeTempInstanceFile(
		"3\n"
		"10\n"
		"1 3 4\n"
		"2 2 5\n",
		"strip_packing_bad_instance.txt");

	std::vector<const StripPacking::item*> items;
	EXPECT_THROW(readData(path.string(), items), std::runtime_error);
	EXPECT_TRUE(items.empty());

	std::filesystem::remove(path);
}
