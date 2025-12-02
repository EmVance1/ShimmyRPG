#include "pch.h"
#include "scripting/speech/ctxt.h"
#include "scripting/expr.h"


namespace shmy { namespace speech { namespace detail {


void append_flag_expr(ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table);


std::string ParseContext::unwrap_next(TokenType expect, const std::string& error) {
    const auto value = lexer.next();
    if (value.has_value() && value->type == expect) {
        return value->val;
    } else {
        throw std::runtime_error(error + "token '" + value->val + "' at " + std::to_string(value->row) + ":" + std::to_string(value->col));
    }
}

std::string ParseContext::unwrap_next(uint32_t expect, const std::string& error) {
    const auto value = lexer.next();
    if (value.has_value() && ((uint32_t)value->type & expect)) {
        return value->val;
    } else {
        throw std::runtime_error(error + "token '" + value->val + "' at " + std::to_string(value->row) + ":" + std::to_string(value->col));
    }
}

uint32_t ParseContext::push_next_expr() {
    const auto ref = (uint32_t)graph->exprs.size();
    append_flag_expr(*this, graph->exprs, graph->strs);
    return ref;
}

uint32_t ParseContext::push_str(std::string&& val, bool dedup) {
    if (s_LUT.find(val) == s_LUT.end() || !dedup) {
        const auto ref = (uint32_t)graph->strs.size();
        s_LUT[val] = ref;
        graph->strs.emplace_back(std::move(val));
        return ref;
    } else {
        return s_LUT[val];
    }
}

uint32_t ParseContext::push_vertex(const std::string& name, bool init) {
    if (v_LUT.find(name) == v_LUT.end()) {
        const auto ref = (uint32_t)graph->verts.size();
        v_LUT[name] = { ref, init };
        graph->verts.emplace_back();
        return ref;
    } else {
        auto& ref = v_LUT[name];
        if (!ref.init) ref.init = init;
        return ref.idx;
    }
}

} } }
