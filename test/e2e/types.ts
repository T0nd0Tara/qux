import { assertEquals } from "jsr:@std/assert";
import { buildQuxProgram } from "../utils/build.ts";
import type { CmdOutput } from "../types.ts";
import { Suite, test } from "../utils/test.ts";
const quxProgram = `
main :: () {
  return 1;
};
`;

export default new Suite('types', 
  test(
    'func type',
    async () => {
      const args = new Set([
        '--no-write',
        '--stdin',
        '--print-types',
      ]);

      const quxRes: CmdOutput = await buildQuxProgram(quxProgram, args);

      assertEquals(quxRes.stdout, "main: func: () -> (int_ !> void_) !> void_\n\n\n");
      assertEquals(quxRes.stderr, "");
      assertEquals(quxRes.exitCode, 0);
    }

  ));
