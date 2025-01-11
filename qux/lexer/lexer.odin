package lexer
import "core:fmt"
import "core:strings"

TokenType :: enum {
  variable,
  decleration,
  runtime_assignment,
  comtime_assignment,
  typing,
}

Token :: struct {
  type: TokenType,
  value: []rune,
}

ErrorData :: struct {
  is_error: bool,
  message: string,
  position: int,
}

@(private)
State :: struct {
  index: int,
  last_non_typing_token: Token,

}

lex_program :: proc(program: string) -> ([]Token, ErrorData) {
  tokens : [dynamic]Token = {};
  state: State = {
    index = 0,
  }

  for state.index < len(program) {
    token, error_data := get_next_token(&state, tokens[:], program);
    if error_data.is_error do return tokens[:], error_data;
    if token == nil do break;

    append(&tokens, token);

    // Update state
    if token.type != .typing do state.last_non_typing_token = token;
    
  }
  return tokens[:], { };
};

get_next_token :: proc(state: ^State, tokens: []Token, program: string) -> (Token, ErrorData) {
  c : char = program[state.index];
  for (strings.is_ascii_space(c)) {
    state.index += 1;

    c = program[state.index];
  }

  if c == ':' {

    if state.last_non_typing_token == nil {
      return nil, {
        is_error = true,
        message = "Expected statement, found `:`",
        position = state.index,
      };
    }

    if state.last_non_typing_token.type == .variable {
      return Token {
        type = .decleration,
        value = {},
      }
    }
    if state.last_non_typing_token.type == .decleration {
      return Token {
        type = .comtime_assignment,
        value = {},
      }, {};
    }

    return  nil, {
      is_error = true,
      message = "Couldn't put here a `:`",
      position = state.index,
    };
  }
  
  //if c == '(' {
  //  if last_non_typing_token == nil || last_non_typing_token.type == 
  //  last_non_typing_token.type 
  //}
}
