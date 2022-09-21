#include "unit_test.h"

//单元测试测试
int main(int argc, char const *argv[])
{
    //直接调用模板(无法自动传递行号)
    // typedef Unit_test_template<Plain_equal<int>, int> Unit_test;
    // Unit_test::expect_eq(3, 6, __FILE__, __LINE__);
    // Unit_test::expect_eq(4, 6, __FILE__, __LINE__);
    // Unit_test::expect_eq(9, 6, __FILE__, __LINE__);
    // Unit_test::expect_eq(3, 3, __FILE__, __LINE__);
    // Unit_test::show_results();
    
    //使用宏
    UNIT_TEST_BEGIN(unit_test, Plain_equal, int);
    UNIT_TEST(unit_test, 3, 6);
    UNIT_TEST(unit_test, 4, 6);
    UNIT_TEST(unit_test, 9, 6);
    UNIT_TEST(unit_test, 3, 3);
    UNIT_TEST_END(unit_test);
    return 0;
}
