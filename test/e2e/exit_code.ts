import { assertEquals } from "jsr:@std/assert";
import { runQuxProgram } from "../utils/build.ts";
import { Suite, test } from "../utils/test.ts";
import type { CmdOutput } from "../types.ts";

const quxProgram = `
main :: () {
  print("Hello World! 420!!!");
  return 69;
};
`;

export default new Suite('exit code', 
  test('exit code',

    async () => {
      const args = new Set([
        '--no-write',
        '--stdin',
      ]);

      const quxRes: CmdOutput = await runQuxProgram(quxProgram, args);

      assertEquals(quxRes.stdout, "Hello World! 420!!!");
      assertEquals(quxRes.stderr, "");
      assertEquals(quxRes.exitCode, 69);
    }
  ) 
);
