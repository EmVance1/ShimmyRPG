#pragma once
#include "scripting/lexer.h"
#include "bytecode.h"


std::vector<Instruction> parse_script(Lexer lexer);

