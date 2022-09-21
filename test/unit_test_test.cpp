#include "unit_test.h"

int main(int argc, char const *argv[])
{
    typedef Unit_test_template<Plain_equal<int>, int> Unit_test;
    Unit_test::expect_eq(3, 6);
    Unit_test::expect_eq(4, 6);
    Unit_test::expect_eq(9, 6);
    Unit_test::expect_eq(3, 3);
    Unit_test::show_results();
    return 0;
}
