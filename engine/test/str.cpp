#include <vangotest/asserts2.h>
#include "util/str.h"
using namespace shmy;


template<typename T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vec) {
    stream << "[ ";
    for (size_t i = 0; i < vec.size(); i++) {
        stream << vec[i];
        if (i < vec.size() - 1) {
            stream << ", ";
        }
    }
    return stream << " ]";
}


vango_test(split_commas) {
    const auto testcase1 = "oief, woeif, weoifh,oiq,qwoif qwifh";
    const auto expect1 = std::vector<std::string>{ "oief", " woeif", " weoifh", "oiq", "qwoif qwifh" };
    vg_assert_eq(expect1, str::split(testcase1, ','));

    const auto testcase2 = "oief, woeif, weoifh,oiq,qwoif qwifh,";
    const auto expect2 = std::vector<std::string>{ "oief", " woeif", " weoifh", "oiq", "qwoif qwifh" };
    vg_assert_eq(expect2, str::split(testcase2, ','));

    const auto testcase3 = "oief, woeif, weoifh,oiq,qwoif qwifh,a";
    const auto expect3 = std::vector<std::string>{ "oief", " woeif", " weoifh", "oiq", "qwoif qwifh", "a" };
    vg_assert_eq(expect3, str::split(testcase3, ','));
}

vango_test(trim_whitespace) {
    const auto testcase1 = " weih ";
    const auto expect1 = "weih";
    vg_assert_eq(expect1, str::trim(testcase1));

    const auto testcase2 = "   weih   ";
    const auto expect2 = "weih";
    vg_assert_eq(expect2, str::trim(testcase2));

    const auto testcase3 = " we  ih ";
    const auto expect3 = "we  ih";
    vg_assert_eq(expect3, str::trim(testcase3));

    const auto testcase4 = "   we  ih   ";
    const auto expect4 = "we  ih";
    vg_assert_eq(expect4, str::trim(testcase4));

    const auto testcase5 = "";
    const auto expect5 = "";
    vg_assert_eq(expect5, str::trim(testcase5));

    const auto testcase6 = "   ";
    const auto expect6 = "";
    vg_assert_eq(expect6, str::trim(testcase6));
}

