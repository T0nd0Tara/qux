package lexer

import "core:slice"

@(private)
get_latest_token_without_token_type :: proc(tokens: []Token, ignore_types: []TokenType = {}) -> ^Token
{
  if len(ignore_types) == 0 do return slice.last_ptr(tokens);

  #reverse for &token in tokens {
    if !slice.contains(ignore_types, token.type) do return &token;
  }
  return nil;
}
