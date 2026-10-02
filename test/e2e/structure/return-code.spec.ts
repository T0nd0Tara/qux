import { expect, describe, test } from "bun:test";
import { randomInt } from "node:crypto";
import { createQuxBuildProcess } from '@/utils/build.ts'

describe("correct return code", () => {
  for (let i = 0; i < 10; i++) {
    const returnCode = randomInt(0, 256); 
    test(`return status code - ${returnCode}`, async () => {
      const proc = createQuxBuildProcess(String.raw`
main :: () {
  return ${returnCode};
};
`, ['--run']);
      expect(await proc.exited).toBe(returnCode);
    });
  }

});
