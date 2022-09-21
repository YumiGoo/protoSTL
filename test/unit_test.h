#pragma once

#include <iostream>
#include <string>

//用宏对模板封装，以支持行号传递
#define UNIT_TEST_BEGIN(testname, equality, type) typedef Unit_test_template<equality<type>, type> testname
#define UNIT_TEST(testname, value1, value2) testname::expect_eq((value1), (value2), __FILE__, __LINE__)
#define UNIT_TEST_END(testname) testname::show_results()

//单元测试模板
template <typename __BinaryOperation, typename T>
class Unit_test_template {
private:
    static int main_ret;
    static int test_count;
    static int test_pass;
public:
    static void expect_eq(T, T, std::string, int);
    static void show_results();
};

//单元测试框架
template <typename __BinaryOperation, typename T>
void Unit_test_template<__BinaryOperation, T>::expect_eq(T expect, T actual, std::string file, int line) {
    ++test_count;
    if (__BinaryOperation()(expect, actual)) {
        ++test_pass;
    } else {
        std::cerr << file << ":"<< line << ": except:" << expect << " actual:" << actual << std::endl; 
    }
}

template <typename __BinaryOperation, typename T>
void Unit_test_template<__BinaryOperation, T>::show_results() {
    std::cout << test_pass << "/" << test_count << "(" << (double)test_pass / test_count * 100 << "%) passed" << std::endl;
}


// 初值设定

template <typename __BinaryOperation, typename T> int Unit_test_template<__BinaryOperation, T>::main_ret = 0;//测试返回值
template <typename __BinaryOperation, typename T> int Unit_test_template<__BinaryOperation, T>::test_count = 0;//总测试次数
template <typename __BinaryOperation, typename T> int Unit_test_template<__BinaryOperation, T>::test_pass = 0;//通过测试次数

// 比较函数

//朴素比较函数
template <typename T>
struct Plain_equal {
    bool operator()(const T& lhs, const T& rhs) const{
        return lhs == rhs;
    }
};
