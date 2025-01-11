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

  tokens, error_data := lexer.lex_program(string(program));
  defer delete(tokens)

  if error_data.is_error {
    fmt.println("Lexer Error: ", error_data.message);
    fmt.println("At index: ", error_data.position);
    os.exit(1);
  }

  fmt.println(tokens);
}
