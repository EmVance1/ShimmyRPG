#include "pch.h"
#include "signal.h"


std::unique_ptr<Content> into_content(const std::string& value) {
    return std::make_unique<StringWrapper>(value);
}


/*
void test_dyn_content() {
    auto [gp_get, gp_set] = new_signal<uint32_t>(100);
    auto [sp_get, sp_set] = new_signal<uint32_t>(10);

    DynamicContent content;
    content.set_dynamic(gp_get, "gp, ", sp_get, "sp");

    std::cout << content.get() << "\n";
    std::cout << content.get() << "\n";
    gp_set.set(80);
    std::cout << content.get() << "\n";
    sp_set.set(6);
    std::cout << content.get() << "\n";
}
*/

