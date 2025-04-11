#include "mscmptest/asserts.h"
#include "util/str.h"


void split_commas() {
    const auto testcase1 = "oief, woeif, weoifh,oiq,qwoif qwifh";
    const auto expect1 = std::vector<std::string>{ "oief", " woeif", " weoifh", "oiq", "qwoif qwifh" };
    assert_eq(expect1, split_string(testcase1, ','));

    const auto testcase2 = "oief, woeif, weoifh,oiq,qwoif qwifh,";
    const auto expect2 = std::vector<std::string>{ "oief", " woeif", " weoifh", "oiq", "qwoif qwifh" };
    assert_eq(expect2, split_string(testcase2, ','));

    const auto testcase3 = "oief, woeif, weoifh,oiq,qwoif qwifh,a";
    const auto expect3 = std::vector<std::string>{ "oief", " woeif", " weoifh", "oiq", "qwoif qwifh", "a" };
    assert_eq(expect3, split_string(testcase3, ','));
}

void trim_whitespace() {
    const auto testcase1 = " weih ";
    const auto expect1 = "weih";
    assert_eq(expect1, trim_string(testcase1));

    const auto testcase2 = "   weih   ";
    const auto expect2 = "weih";
    assert_eq(expect2, trim_string(testcase2));

    const auto testcase3 = " we  ih ";
    const auto expect3 = "we  ih";
    assert_eq(expect3, trim_string(testcase3));

    const auto testcase4 = "   we  ih   ";
    const auto expect4 = "we  ih";
    assert_eq(expect4, trim_string(testcase4));

    const auto testcase5 = "";
    const auto expect5 = "";
    assert_eq(expect5, trim_string(testcase5));

    const auto testcase6 = "   ";
    const auto expect6 = "";
    assert_eq(expect6, trim_string(testcase6));
}

