#include <gtest/gtest.h>
#include "splay_tree.hpp"
#include <array>
#include <random>

TEST(insert_test, insert_operator)
{
  splay_tree<int, double> map;
  EXPECT_TRUE(map.empty());

  for (std::size_t j = 0; j < 1000; j++)
  {
    map[j] = 1.1 * j;
    EXPECT_EQ(j + 1, map.size());
  }

  for (std::size_t j = 0; j < 1000; j++)
  {
    EXPECT_NE(map.find(j), map.end());
    EXPECT_EQ(map.find(j)->first, j);
  }
}

TEST(insert_test, insert_method)
{
  splay_tree<int, double> map;
  EXPECT_TRUE(map.empty());

  for (std::size_t j = 0; j < 1000; j++)
  {
    auto result = map.insert(std::pair<int, double>{ j, j * 1.1 });
    EXPECT_TRUE(result.second);
    EXPECT_EQ(j + 1, map.size());
  }

  for (std::size_t j = 0; j < 1000; j++)
  {
    EXPECT_NE(map.find(j), map.end());
    EXPECT_EQ(map.find(j)->first, j);
  }
}

TEST(erase_test, erase)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map(ar.begin(), ar.end());

  EXPECT_EQ(map.size(), 4);

  EXPECT_TRUE(map.erase(1));

  EXPECT_EQ(map.find(1), map.end());
  EXPECT_EQ(map.find(4)->first, 4);
  EXPECT_EQ(map.size(), 3);
}

TEST(erase_test, erase_range)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map(ar.begin(), ar.end());

  EXPECT_EQ(map.erase(map.begin(), --map.end())->first, 4);
  EXPECT_EQ(map.size(), 1);
}

TEST(erase_test, erase_not_existing_value)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map(ar.begin(), ar.end());

  EXPECT_FALSE(map.erase(100));
}

TEST(erase_test, clear)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map(ar.begin(), ar.end());

  map.clear();
  EXPECT_TRUE(map.empty());
}

template <class T = std::size_t>
T random_int(T n)
{
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<T> dist(0, n);
	return dist(rng);
}

TEST(insert_test, insert_range_with_random_values)
{
	std::vector<std::pair<int, double>> v;
	std::ranges::generate_n(std::back_inserter(v), 0x1000, []{ return std::pair<int, double>{ random_int(10000), 0. }; });

	std::ranges::sort(v, [](const auto& p1, const auto& p2) { return p1.first < p2.first; });
	auto range = std::ranges::unique(v, [](const auto& p1, const auto& p2) { return p1.first == p2.first; });
	v.erase(range.begin(), range.end());

	splay_tree<int, double> map;
	map.insert(v);
	EXPECT_EQ(v.size(), map.size());

	auto v_pairs = v | std::views::keys;
	auto m_pairs = v | std::views::keys;
	EXPECT_TRUE(std::ranges::equal(v_pairs, m_pairs, [](auto v1, auto v2) -> bool { return v1 == v2; }));
}

TEST(search_test, find)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map(ar.begin(), ar.end());

  for (auto key : map | std::views::keys)
  {
    EXPECT_EQ(map.find(key)->first, key);
  }

  EXPECT_EQ(map.find(100), map.end());
}

TEST(search_test, at)
{
  std::array<std::pair<int, int>, 4> ar{ std::pair{1, 1}, {2, 2}, {3, 3}, {4, 4} };
  splay_tree<int, double> map(ar.begin(), ar.end());

  for(auto key : map | std::views::keys)
  {
    EXPECT_EQ(map.at(key), key);
  }
}

TEST(search_test, throwing_at)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map(ar.begin(), ar.end());

  EXPECT_THROW(map.at(5), std::out_of_range);
  EXPECT_EQ(map.at(1), 1.1);

  map.erase(1);
  EXPECT_THROW(map.at(1), std::out_of_range);
}

TEST(emplace_test, emplace)
{
  splay_tree<int, std::unique_ptr<double>> map;

  for(std::size_t j = 0; j < 10; j++)
  {
    map.emplace(j, new double(1.1 * j));
  }

  for(std::size_t j = 0; j < 10; j++)
  {
    EXPECT_NE(map.find(j), map.end());
    EXPECT_EQ(*map.find(j)->second, j * 1.1);
  }
}

TEST(constructors_test, initializer_list_constructor)
{
  splay_tree<int, double> map = { {1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  EXPECT_EQ(map.size(), 4);

  for (int j = 1; const auto& key : map | std::views::keys)
  {
    EXPECT_EQ(j, key);
    ++j;
  }
}

TEST(constructors_test, copy_constructor)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map1(ar.begin(), ar.end());
  splay_tree<int, double> map2(map1);

  auto map1_pairs = map1 | std::views::keys;
  auto map2_pairs = map2 | std::views::keys;
  EXPECT_TRUE(std::ranges::equal(map1_pairs, map2_pairs, [](auto v1, auto v2) -> bool { return v1 == v2; }));
  EXPECT_EQ(map1.size(), map2.size());
}

TEST(constructors_test, move_constructor)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map1(ar.begin(), ar.end());
  splay_tree<int, double> map2(std::move(map1));

  auto map1_pairs = map1 | std::views::keys;
  auto map2_pairs = map2 | std::views::keys;
  EXPECT_FALSE(std::ranges::equal(map1_pairs, map2_pairs, [](auto v1, auto v2) -> bool { return v1 == v2; }));
  EXPECT_NE(map1.size(), map2.size());
  EXPECT_TRUE(map1.empty());
}

TEST(constructors_test, begin_end_iterators)
{
  std::array<std::pair<const int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map(ar.begin(), ar.end());

  auto map_pairs = map | std::views::keys;
  auto ar_pairs = ar | std::views::keys;
  EXPECT_TRUE(std::ranges::equal(map, ar, [](auto v1, auto v2) -> bool { return v1 == v2; }));
}

TEST(assignment_operator, copy_assignment)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map1(ar.begin(), ar.end());
  splay_tree<int, double> map2;

  map2 = map1;

  auto map1_pairs = map1 | std::views::keys;
  auto map2_pairs = map2 | std::views::keys;
  EXPECT_TRUE(std::ranges::equal(map1_pairs, map2_pairs, [](auto v1, auto v2) -> bool { return v1 == v2; }));
  EXPECT_EQ(map1.size(), map2.size());
}

TEST(assignment_operator, move_assignment)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map1(ar.begin(), ar.end());
  splay_tree<int, double> map2;

  map2 = std::move(map1);

  auto map1_pairs = map1 | std::views::keys;
  auto map2_pairs = map2 | std::views::keys;
  EXPECT_FALSE(std::ranges::equal(map1_pairs, map2_pairs, [](auto v1, auto v2) -> bool { return v1 == v2; }));
  EXPECT_NE(map1.size(), map2.size());
  EXPECT_TRUE(map1.empty());
}

TEST(tree_merging, merging_of_different_trees)
{
  std::array<std::pair<int, double>, 4> ar1{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  std::array<std::pair<int, double>, 4> ar2{ std::pair{5, 5.5}, {6, 6.6}, {7, 7.7}, {8, 8.8} };
  splay_tree<int, double> map1(ar1.begin(), ar1.end());
  splay_tree<int, double> map2(ar2.begin(), ar2.end());

  map1.merge(map2);
  EXPECT_TRUE(map2.empty());

  for(int j = 1; const auto& key : map1 | std::views::keys)
  {
    EXPECT_EQ(j, key);
    ++j;
  }
}

TEST(tree_merging, merging_of_equal_trees)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map1(ar.begin(), ar.end());
  splay_tree<int, double> map2(ar.begin(), ar.end());

  map1.merge(map2);
  EXPECT_TRUE(map2.empty());

  for (int j = 1; const auto & key : map1 | std::views::keys)
  {
    EXPECT_EQ(j, key);
    ++j;
  }
}

TEST(tree_merging, merging_of_empty_trees)
{
  splay_tree<int, double> map1;
  splay_tree<int, double> map2;

  map1.merge(map2);

  EXPECT_TRUE(map1.empty());
  EXPECT_TRUE(map2.empty());

  map2.merge(map1);

  EXPECT_TRUE(map1.empty());
  EXPECT_TRUE(map2.empty());
}

TEST(tree_merging, merging_of_empty_tree_and_tree_with_nodes)
{
  splay_tree<int, double> map1 = { {1, 1.1}, {2, 2.2} };
  splay_tree<int, double> map2;

  map2.merge(map1);

  EXPECT_EQ(map2.size(), 2);
  EXPECT_TRUE(map1.empty());
}

TEST(iterators_test, bidirectional_iterating)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map(ar.begin(), ar.end());

  auto it = map.begin();
  ++it; ++it; ++it;
  --it; --it; --it;

  EXPECT_EQ(it->first, 1);
}

TEST(iterators_test, const_iterator)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double> map(ar.begin(), ar.end());

  int j = 1;

  for(auto it = map.cbegin(); it != map.cend(); ++it)
  {
    EXPECT_EQ(it->first, j);
    ++j;
  }
}

TEST(custom_comparator_test, greater_comparator)
{
  std::array<std::pair<int, double>, 4> ar{ std::pair{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4} };
  splay_tree<int, double, std::greater<>> map(ar.begin(), ar.end());

  for(std::size_t j = 4; j > 1; j--)
  {
    EXPECT_NE(map.find(j), map.end());
  }
}
