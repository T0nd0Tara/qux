package lexer
import "core:fmt"
import "core:strings"

TokenType :: enum {
  variable,
  decleration,
  runtime_assignment,
  comtime_assignment,
}

Token :: struct {
  type: TokenType,
  value: []rune,
}

lex_program :: proc(program: string) -> []Token  {
  tokens : [dynamic]Token = {};
  string_from_latest_token: [dynamic]rune;

  for c in program {
    if strings.is_ascii_space(c) {
      continue;
    }

    if c == ':' {
      //if 
      append(&tokens, Token{
        type = .variable,
        value = string_from_latest_token[:],
      });
      string_from_latest_token = {};
      continue;
    }

    append(&string_from_latest_token, c);
  }
  return tokens[:];
}
