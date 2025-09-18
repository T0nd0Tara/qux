import { assertEquals } from "jsr:@std/assert";
import { runQuxProgram } from "../utils/build.ts";
import { Suite, test } from "../utils/test.ts";
import type { CmdOutput } from "../types.ts";
import randomstring from 'randomstring';


export default new Suite('stdout', 
  test(
    'random output message',
    async () => {
      const args = new Set([
        '--no-write',
        '--stdin',
      ]);
      const randomMessage = randomstring.generate();

      const quxProgram = `
main :: () {
  print("${randomMessage}");
  return 0;
};
`
      const quxRes: CmdOutput = await runQuxProgram(quxProgram, args);

      assertEquals(quxRes.stdout, randomMessage);
      assertEquals(quxRes.stderr, "");
      assertEquals(quxRes.exitCode, 0);
    }
  )
);
