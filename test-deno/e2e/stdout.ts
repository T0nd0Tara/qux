import { assertEquals } from "jsr:@std/assert";
import { runQuxProgram } from "../utils/build.ts";
import { Suite, test } from "../utils/test.ts";
import type { CmdOutput } from "../types.ts";
import randomstring from 'randomstring';
import { randomInt } from 'node:crypto';

const args = new Set([
  '--no-write',
  '--stdin',
]);

export default new Suite('stdout', 
  test(
    'random output message',
    async () => {
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
  ),
  test(
    'random int output message',
    async () => {
      const randomNumber = randomInt(100);

      const quxProgram = `
main :: () {
  print(${randomNumber});
  return 0;
};
`
      const quxRes: CmdOutput = await runQuxProgram(quxProgram, args);

      assertEquals(quxRes.stdout, randomNumber.toString());
      assertEquals(quxRes.stderr, "");
      assertEquals(quxRes.exitCode, 0);
    }
  )
);
