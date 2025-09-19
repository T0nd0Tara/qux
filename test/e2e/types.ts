import { assertEquals } from "jsr:@std/assert";
import { buildQuxProgram } from "../utils/build.ts";
import type { CmdOutput } from "../types.ts";
import { Suite, test } from "../utils/test.ts";

export default new Suite('types', 
  test(
    'func type',
    async () => {

      const quxProgram = `
main :: () {
  return 1;
};
`;
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

  ),
//   test(
//     'int type',
//     async () => {
//
//       const quxProgram = `
// a :: 4;
// `;
//       const args = new Set([
//         '--no-write',
//         '--stdin',
//         '--print-types',
//       ]);
//
//       const quxRes: CmdOutput = await buildQuxProgram(quxProgram, args);
//
//       assertEquals(quxRes.stdout, "a: int_ !> void_\n\n\n");
//       assertEquals(quxRes.stderr, "");
//       assertEquals(quxRes.exitCode, 0);
//     }
//
//   ),
);
