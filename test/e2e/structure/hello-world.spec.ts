import { expect, describe, test } from "bun:test";
import { createQuxBuildProcess } from '@/utils/build.ts'
const program = String.raw`main :: () {
  print("Hello World\n");
  return 0;
};
`;

describe("hello world", () => {
  test("lexer tokens", async () => {
    const proc = createQuxBuildProcess(program, new Set([ '--print-tokens', '--no-write']));
    const output = await proc.stdout.text();
    expect(output).toBe(String.raw`[
  00. IDENTIFIER: "main" - 1:1
  01. COLON - 1:6
  02. COLON - 1:7
  03. OPEN_PAREN - 1:9
  04. CLOSE_PAREN - 1:10
  05. OPEN_CURLY_BRACKET - 1:12
  06. IDENTIFIER: "print" - 2:3
  07. OPEN_PAREN - 2:8
  08. STRING_LITERAL: "Hello World\n" - 2:9
  09. CLOSE_PAREN - 2:24
  10. SEMICOLON - 2:25
  11. IDENTIFIER: "return" - 3:3
  12. INT_LITERAL: "0" - 3:10
  13. SEMICOLON - 3:11
  14. CLOSE_CURLY_BRACKET - 4:1
  15. SEMICOLON - 4:2
]
`);
  });
  test("ast", async () => {
    const proc = createQuxBuildProcess(program, new Set([ '--print-ast', '--no-write']));
    const output = await proc.stdout.text();
    expect(output).toBe(`└──ROOT
   ├──DECLARE
   │   └──VARIABLE: "main"
   └──ASSIGN_COM
      ├──VARIABLE: "main"
      └──FUNC
         ├──FUNC_CALL
         │   ├──VARIABLE: "print"
         │   └──ARGS
         │      └──STRING_LITERAL: "Hello World\\n"
         └──RETURN
            └──INT_LITERAL
`);
  });
});
