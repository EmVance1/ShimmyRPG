#include "pch.h"
#include "parser.h"


static std::string unwrap_token(const std::optional<Token>& value, TokenType expect) {
    return (value.has_value() && value->type == expect) ? value->value : throw std::exception("WRONG TYPE");
}

std::vector<Instruction> parse_script(Lexer lexer) {
    auto result = std::vector<Instruction>();

    while (true) {
        auto next = lexer.next();
        if (!next.has_value()) { goto after_loop2; }
        const auto func = unwrap_token(next,         TokenType::Identifier);
                          unwrap_token(lexer.next(), TokenType::OpenParen);
        auto params = std::vector<std::string>();
        while (true) {
            next = lexer.next();
            switch (next->type) {
                case TokenType::Identifier: case TokenType::StringLiteral: case TokenType::IntLiteral: case TokenType::FloatLiteral:
                params.push_back(next->value);
                break;
            case TokenType::CloseParen:  goto after_loop1;
            default: throw std::exception();
            };
            next = lexer.next();
            switch (next->type) {
            case TokenType::CloseParen: goto after_loop1;
            case TokenType::Comma: break;
            default: throw std::exception();
            }
        }
after_loop1:

        if (func == "set_flag") {
            result.push_back(SetFlag{ params[0], (uint32_t)std::atoi(params[1].c_str()) });
        } if (func == "lock") {
            result.push_back(Lock{ params[0] });
        } else if (func == "unlock") {
            result.push_back(Unlock{ params[0] });
        } else if (func == "pause") {
            result.push_back(Pause{ (float)std::atof(params[0].c_str()) });
        } else if (func == "wait") {
            result.push_back(Wait{ (float)std::atof(params[0].c_str()) });
        } else if (func == "freeze") {
            result.push_back(Freeze{ params[0], (float)std::atoi(params[1].c_str()) });
        } else if (func == "set_path") {
            result.push_back(SetPath{ params[0], sf::Vector2f((float)std::atof(params[1].c_str()), (float)std::atof(params[2].c_str())) });
        } else if (func == "set_animation") {
            result.push_back(SetAnimation{ params[0], (uint32_t)std::atoi(params[1].c_str()) });
        } else if (func == "set_voicebank") {
            result.push_back(SetVoicebank{ params[0], (uint32_t)std::atoi(params[1].c_str()) });
        } else if (func == "start_dialogue") {
            result.push_back(StartDialogue{ params[0] });
        } else if (func == "start_script") {
            result.push_back(StartScript{ params[0] });
        }
    }
after_loop2:

    return result;
}

