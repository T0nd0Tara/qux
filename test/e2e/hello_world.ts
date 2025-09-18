import { assertEquals } from "jsr:@std/assert";
import { runQuxProgram } from "../utils/build.ts";
import { Suite, test } from "../utils/test.ts";
import type { CmdOutput } from "../types.ts";

const quxProgram = `
main :: () {
  print("Hello World");
  return 0;
};
`;

export default new Suite('hello world', 
  test(
    'hello world',
    async () => {
      const args = new Set([
        '--no-write',
        '--stdin',
      ]);

      const quxRes: CmdOutput = await runQuxProgram(quxProgram, args);

      assertEquals(quxRes.stdout, "Hello World");
      assertEquals(quxRes.stderr, "");
      assertEquals(quxRes.exitCode, 0);
    }
  )
);
