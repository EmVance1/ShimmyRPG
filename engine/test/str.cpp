#include "vangotest/asserts.h"
#include "util/str.h"
using namespace shmy;


test(split_commas) {
    const auto testcase1 = "oief, woeif, weoifh,oiq,qwoif qwifh";
    const auto expect1 = std::vector<std::string>{ "oief", " woeif", " weoifh", "oiq", "qwoif qwifh" };
    assert_eq(expect1, str::split(testcase1, ','));

    const auto testcase2 = "oief, woeif, weoifh,oiq,qwoif qwifh,";
    const auto expect2 = std::vector<std::string>{ "oief", " woeif", " weoifh", "oiq", "qwoif qwifh" };
    assert_eq(expect2, str::split(testcase2, ','));

    const auto testcase3 = "oief, woeif, weoifh,oiq,qwoif qwifh,a";
    const auto expect3 = std::vector<std::string>{ "oief", " woeif", " weoifh", "oiq", "qwoif qwifh", "a" };
    assert_eq(expect3, str::split(testcase3, ','));
}

test(trim_whitespace) {
    const auto testcase1 = " weih ";
    const auto expect1 = "weih";
    assert_eq(expect1, str::trim(testcase1));

    const auto testcase2 = "   weih   ";
    const auto expect2 = "weih";
    assert_eq(expect2, str::trim(testcase2));

    const auto testcase3 = " we  ih ";
    const auto expect3 = "we  ih";
    assert_eq(expect3, str::trim(testcase3));

    const auto testcase4 = "   we  ih   ";
    const auto expect4 = "we  ih";
    assert_eq(expect4, str::trim(testcase4));

    const auto testcase5 = "";
    const auto expect5 = "";
    assert_eq(expect5, str::trim(testcase5));

    const auto testcase6 = "   ";
    const auto expect6 = "";
    assert_eq(expect6, str::trim(testcase6));
}

