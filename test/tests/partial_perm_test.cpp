#include <algorithm>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "gmock/gmock.h"

#include "partial_perm.hpp"
#include "perm.hpp"

#include "test_main.cpp"

using namespace mpsym;
using namespace mpsym::internal;

TEST(PartialPermTest, CanConstructPartialPerm)
{
  struct ConstructionTest
  {
    ConstructionTest(PartialPerm pperm,
                     std::vector<int> const &expected_mapping = {},
                     std::vector<int> const &expected_dom = {},
                     std::vector<int> const &expected_im = {})
    : pperm(pperm),
      expected_mapping(expected_mapping),
      expected_dom(expected_dom),
      expected_im(expected_im)
    {}

    PartialPerm pperm;
    std::vector<int> expected_mapping, expected_dom, expected_im;
  };

  std::vector<ConstructionTest> tests {
    ConstructionTest(
      PartialPerm()),
    ConstructionTest(
      PartialPerm({})),
    ConstructionTest(
      PartialPerm({}, {})),
    ConstructionTest(
      PartialPerm(5),
      {0, 1, 2, 3, 4},
      {0, 1, 2, 3, 4},
      {0, 1, 2, 3, 4}),
    ConstructionTest(
      PartialPerm({-1, 3, -1, 2, -1, 8, 5, -1, 6, -1, 10}),
      {-1, 3, -1, 2, -1, 8, 5, -1, 6, -1, 10},
      {1, 3, 5, 6, 8, 10},
      {2, 3, 5, 6, 8, 10}),
    ConstructionTest(
      PartialPerm({1, 3, 5, 6, 8, 10}, {3, 2, 8, 5, 6, 10}),
      {-1, 3, -1, 2, -1, 8, 5, -1, 6, -1, 10},
      {1, 3, 5, 6, 8, 10},
      {2, 3, 5, 6, 8, 10}),
    ConstructionTest(
      PartialPerm({4, 8, 9, 10, -1, -1, -1, -1, -1, 11, 3, 2}),
      {4, 8, 9, 10, -1, -1, -1, -1, -1, 11, 3, 2},
      {0, 1, 2, 3, 9, 10, 11},
      {2, 3, 4, 8, 9, 10, 11}),
    ConstructionTest(
      PartialPerm({11, 10, 0, 1, 2, 3, 9}, {2, 3, 4, 8, 9, 10, 11}),
      {4, 8, 9, 10, -1, -1, -1, -1, -1, 11, 3, 2},
      {0, 1, 2, 3, 9, 10, 11},
      {2, 3, 4, 8, 9, 10, 11})
  };

  for (auto const &test : tests) {
    for (auto i = 0u; i < test.expected_mapping.size(); ++i) {
      EXPECT_EQ(test.expected_mapping[i], test.pperm[i])
        << "Can apply partial permutation.";
    }

    EXPECT_EQ(test.expected_dom, test.pperm.dom())
      << "Partial permutation domain constructed correct.";

    if (test.expected_dom.empty()) {
      EXPECT_EQ(-1, test.pperm.dom_min())
        << "Partial permutation domain lower limit correct.";

      EXPECT_EQ(-1, test.pperm.dom_max())
        << "Partial permutation domain uppter limit correct.";

    } else {
      EXPECT_EQ(*std::min_element(test.expected_dom.begin(),
                                  test.expected_dom.end()),
                test.pperm.dom_min())
        << "Partial permutation domain lower limit correct.";

      EXPECT_EQ(*std::max_element(test.expected_dom.begin(),
                                  test.expected_dom.end()),
                test.pperm.dom_max())
        << "Partial permutation domain upper limit correct.";
    }

    EXPECT_EQ(test.expected_im, test.pperm.im())
      << "Partial permutation image constructed correct.";

    if (test.expected_im.empty()) {
      EXPECT_EQ(-1, test.pperm.im_min())
        << "Partial permutation domain lower limit correct.";

      EXPECT_EQ(-1, test.pperm.im_max())
        << "Partial permutation domain uppter limit correct.";

    } else {
      EXPECT_EQ(*std::min_element(test.expected_im.begin(),
                                  test.expected_im.end()),
                test.pperm.im_min())
        << "Partial permutation image lower limit correct.";

      EXPECT_EQ(*std::max_element(test.expected_im.begin(),
                                  test.expected_im.end()),
                test.pperm.im_max())
        << "Partial permutation image upper limit correct.";
    }
  }
}

TEST(PartialPermTest, CanInvertPartialPerm)
{
  PartialPerm inv(~PartialPerm({-1, 3, -1, 2, -1, 8, 5, -1, 6, -1, 10}));
  PartialPerm expected({-1, -1, 3, 1, -1, 6, 8, -1, 5, -1, 10});

  EXPECT_EQ(expected, inv)
    << "Inverting partial permutation produces correct result.";

  EXPECT_EQ(expected.dom(), inv.dom())
    << "Inverting partial permutation produces correct domain.";

  EXPECT_EQ(expected.im(), inv.im())
    << "Inverting partial permutation produces correct image.";

  EXPECT_TRUE(expected.dom_min() == inv.dom_min() &&
              expected.dom_max() == inv.dom_max())
    << "Inverting partial permutation produces correct domain limits.";

  EXPECT_TRUE(expected.im_min() == inv.im_min() &&
              expected.im_max() == inv.im_max())
    << "Inverting partial permutation produces correct image limits.";
}

TEST(PartialPermTest, CanMultiplyPartialPerms)
{
  std::tuple<PartialPerm, PartialPerm, PartialPerm> const multiplications[] = {
    std::make_tuple(
      PartialPerm(),
      PartialPerm({4, 8, 9, 10, -1, -1, -1, -1, -1, 11, 3, 2}),
      PartialPerm()
    ),
    std::make_tuple(
      PartialPerm({4, 8, 9, 10, -1, -1, -1, -1, -1, 11, 3, 2}),
      PartialPerm(),
      PartialPerm()
    ),
    std::make_tuple(
      PartialPerm({-1, 3, -1, 2, -1, 8, 5, -1, 6, -1, 10}),
      PartialPerm({4, 8, 9, 10, -1, -1, -1, -1, -1, 11, 3, 2}),
      PartialPerm({-1, 10, -1, 9, -1, -1, -1, -1, -1, -1, 3})
    )
  };

  for (auto const &mult : multiplications) {
    PartialPerm lhs(std::get<0>(mult));
    PartialPerm rhs(std::get<1>(mult));
    PartialPerm expected(std::get<2>(mult));

    PartialPerm pperm_mult_assign(lhs);
    pperm_mult_assign *= rhs;

    PartialPerm pperm_mult = lhs * rhs;

    for (PartialPerm const &pperm : {pperm_mult_assign, pperm_mult}) {
      EXPECT_EQ(expected, pperm)
        << "Multiplying partial permutations produces correct result.";

      EXPECT_EQ(expected.dom(), pperm.dom())
        << "Multiplying partial permutations produces correct domain.";

      EXPECT_EQ(expected.im(), pperm.im())
        << "Multiplying partial permutations produces correct image.";

      EXPECT_TRUE(expected.dom_min() == pperm.dom_min() &&
                  expected.dom_max() == pperm.dom_max())
        << "Multiplying partial permutations produces correct domain limits.";

      EXPECT_TRUE(expected.im_min() == pperm.im_min() &&
                  expected.im_max() == pperm.im_max())
        << "Multiplying partial permutations produces correct image limits.";
    }
  }
}

TEST(PartialPermTest, PartialPermStringRepresentation)
{
  struct PPermStrRepr {
    PPermStrRepr(PartialPerm const &pperm, std::string const &str)
    : pperm(pperm),
      str(str)
    {}

    PartialPerm pperm;
    std::string str;
  };

  PPermStrRepr pperm_str_reprs[] = {
    PPermStrRepr(PartialPerm(),
                 "()"),
    PPermStrRepr(PartialPerm({0, -1, 2}),
                 "(0)(2)"),
    PPermStrRepr(PartialPerm({-1, 1, -1}),
                 "(1)"),
    PPermStrRepr(PartialPerm({1, -1, -1, 0}),
                 "[3, 0, 1]"),
    PPermStrRepr(PartialPerm({-1, 0, 4, -1, 1}),
                 "[2, 4, 1, 0]"),
    PPermStrRepr(PartialPerm({-1, -1, 2, 3, 0, -1}),
                 "[4, 0](2)(3)"),
    PPermStrRepr(PartialPerm({5, 8, 6, 0, -1, 4, 2, 9, -1, 10, 7}),
                 "[1, 8][3, 0, 5, 4](2, 6)(7, 9, 10)")
  };

  for (auto const &pperm_str_repr : pperm_str_reprs) {
    std::stringstream ss;
    ss << pperm_str_repr.pperm;

    EXPECT_EQ(pperm_str_repr.str, ss.str())
      << "Correct partial permutation string representation.";
  }
}

TEST(PartialPermTest, CanCheckIfPartialPermEmpty)
{
  std::vector<PartialPerm> empty_pperms {
    PartialPerm(),
    PartialPerm({}),
    PartialPerm({}, {})
  };

  for (auto const &pperm : empty_pperms) {
    EXPECT_TRUE(pperm.empty())
      << "Can identify partial permutation as empty (" << pperm << ").";
  }

  std::vector<PartialPerm> non_empty_pperms {
    PartialPerm(1),
    PartialPerm({0}, {0})
  };

  for (auto const &pperm : non_empty_pperms) {
    EXPECT_FALSE(pperm.empty())
      << "Can identify partial permutation as non-empty (" << pperm << ").";
  }
}

TEST(PartialPermTest, CanCheckIfPartialPermIsId)
{
  std::vector<PartialPerm> id_pperms {
    PartialPerm(),
    PartialPerm({}),
    PartialPerm({}, {}),
    PartialPerm(1),
    PartialPerm({0}, {0}),
    PartialPerm(7),
    PartialPerm({-1, 1, -1, 3, 4, -1, 6}),
    PartialPerm({2, 7, 8}, {2, 7, 8}),
    PartialPerm({0, 1, 2}, {2, 3, 0}) * PartialPerm({0, 3, 2}, {2, 1, 0})
  };

  for (auto const &pperm : id_pperms) {
    EXPECT_TRUE(pperm.id())
      << "Can identify partial permutation as identity ( " << pperm << " ).";
  }

  std::vector<PartialPerm> non_id_pperms {
    PartialPerm({-1, 0}),
    PartialPerm({0, 2}),
    PartialPerm({0, -1, 1}),
    PartialPerm({0}, {1}),
    PartialPerm({0, 1}, {0, 2}),
    PartialPerm(3) * PartialPerm({0, 1, 2}, {2, 1, 0}),
    PartialPerm({0, 1, 2}, {2, 1, 0}) * PartialPerm(2)
  };

  for (auto const &pperm : non_id_pperms) {
    EXPECT_FALSE(pperm.id())
      << "Can identify partial permutation as non-identity ( " << pperm << " ).";
  }
}

TEST(PartialPermTest, CanRestrictPartialPerm)
{
  struct RestrictionTest
  {
    RestrictionTest(PartialPerm pperm,
                    std::vector<unsigned> const &domain,
                    PartialPerm expected_pperm)
    : expected_pperm(expected_pperm),
      actual_pperm(pperm.restricted(domain.begin(), domain.end()))
    {}

    PartialPerm expected_pperm, actual_pperm;
  };

  RestrictionTest tests[] = {
    RestrictionTest(
      PartialPerm({-1, 3, -1, 2, -1, 8, 5, -1, 6, -1, 10}),
      {3, 4, 5, 8, 9},
      PartialPerm({-1, -1, -1, 2, -1, 8, -1, -1, 6})),
    RestrictionTest(
      PartialPerm({4, 8, 9, 10, -1, -1, -1, -1, -1, 11, 3, 2}),
      {0, 1, 2, 7, 8},
      PartialPerm({4, 8, 9}))
  };

  for (auto &test : tests) {
    EXPECT_EQ(test.expected_pperm, test.actual_pperm)
      << "Multiplying partial permutations produces correct result.";

    EXPECT_EQ(test.expected_pperm.dom(), test.actual_pperm.dom())
      << "Multiplying partial permutations produces correct domain.";

    EXPECT_EQ(test.expected_pperm.im(), test.actual_pperm.im())
      << "Multiplying partial permutations produces correct image.";

    EXPECT_TRUE(test.expected_pperm.dom_min() == test.actual_pperm.dom_min() &&
                test.expected_pperm.dom_max() == test.actual_pperm.dom_max())
      << "Multiplying partial permutations produces correct domain limits.";

    EXPECT_TRUE(test.expected_pperm.im_min() == test.actual_pperm.im_min() &&
                test.expected_pperm.im_max() == test.actual_pperm.im_max())
      << "Multiplying partial permutations produces correct image limits.";
  }
}

TEST(PartialPermTest, CanConvertPartialPermToPerm)
{
  std::vector<std::pair<PartialPerm, Perm>> conversions {
    {
      PartialPerm(),
      Perm()
    },
    {
      PartialPerm(),
      Perm(10, {})
    },
    {
      PartialPerm({0, 1}, {1, 0}),
      Perm(3, {{0, 1}})
    },
    {
      PartialPerm({1, 2, 4}, {2, 1, 4}),
      Perm(6, {{1, 2}})
    },
    {
      PartialPerm({3, 4, 5, 6, 7, 8}, {3, 6, 7, 4, 8, 5}),
      Perm(10, {{4, 6}, {5, 7, 8}})
    }
  };

  for (auto const &conv : conversions) {
    PartialPerm const &pperm = std::get<0>(conv);
    Perm const &perm = std::get<1>(conv);

    EXPECT_EQ(perm, pperm.to_perm(perm.degree()))
      << "Conversion from partial to 'complete' permutation correct.";
  }
}
