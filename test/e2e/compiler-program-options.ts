import { assertEquals } from "jsr:@std/assert";
import { buildQuxProgram } from "../utils/build.ts";
import { Suite, test } from "../utils/test.ts";
import type { CmdOutput } from "../types.ts";
import randomstring from 'randomstring';

const quxProgram = `
main :: () {
  return 0;
};
`;

export default new Suite('Compiler Program Options',
  test('Error on no file',

    async () => {
      const args = new Set([
      ]);

      const quxRes: CmdOutput = await buildQuxProgram(quxProgram, args);

      assertEquals(quxRes.stdout, "");
      assertEquals(quxRes.stderr, "Please provide an input file\n");
      assertEquals(quxRes.exitCode, 1);
    }
  ),
  test('Error on provided file and stdin',
    async () => {
      const filename = randomstring.generate();
      const args = new Set([
        filename,
        '--stdin'
      ]);

      const quxRes: CmdOutput = await buildQuxProgram(quxProgram, args);

      assertEquals(quxRes.stdout, "");
      assertEquals(quxRes.stderr, "You cannot set a filename and the --stdin flag\n");
      assertEquals(quxRes.exitCode, 1);
    }
  ),
  test('Error on non existing file',
    async () => {
      const filename = randomstring.generate();
      const args = new Set([
        filename,
      ]);

      const quxRes: CmdOutput = await buildQuxProgram(quxProgram, args);

      assertEquals(quxRes.stdout, "");
      assertEquals(quxRes.stderr, `Couldn't read file '${filename}'. exiting...\n`);
      assertEquals(quxRes.exitCode, 1);
    }
  )
);
