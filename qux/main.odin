package qux

import "core:fmt"
import "core:os"
import "lexer"

@(private)
help :: \
"Lol you need help :D" \
;


main :: proc() {
  if len(os.args) != 2 {
    fmt.println(help);
    os.exit(1);
  }

  program_file := os.args[1];

  program, ok := os.read_entire_file(program_file, context.allocator);
  if ! ok {
    fmt.printfln("Coultn't open file ", program_file)
    os.exit(1);
  }

  tokens: []lexer.Token = lexer.lex_program(string(program));
  fmt.println(tokens);
}
