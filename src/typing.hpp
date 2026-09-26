#pragma once
#include "ast.hpp"

enum class TypingType {
  VOID, NIL,

  I8, I16, I32, I64,
  U8, U16, U32, U64,
  F32, F64,

  STRING,

  FUNC,
};

struct Typing {
  TypingType type = TypingType::NIL;
  Typing* return_type = nullptr; 
  std::vector<Typing*> parameters_types = {};

  Typing* pointer_to = nullptr;

  size_t array_dimension = 0;
};


void fill_typing(AstNode& ast);

std::string stringify_typing(const Typing* typing);
