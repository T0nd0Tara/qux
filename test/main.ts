import pLimit from "p-limit";
import { readDir } from "@std/fs/unstable-read-dir";
import { DirEntry } from "@std/fs/unstable-types";
import * as path from "@std/path";
import assert from "node:assert";
import { program } from "commander";
import type { CmdOutput } from './types.ts';
import { Suite, Test } from './utils/test.ts';
const testsLimit = pLimit(4);

const extension = ".ts";
const td = new TextDecoder();

type SuitExpectedResult = CmdOutput & {
  compiles: boolean; // Whether we even can compile it to C
};

interface SuitPreperation {
  quxCompArgs?: string[];
  compileIr?: boolean;
}
interface SuitResult {
  name: string;
  errors: {[key: string]: unknown | null};
}
interface TestOpts { }

function hasSuitPassed(errors: SuitResult['errors']): boolean {
  return Object.values(errors).every(err => err === null);
}

async function runTest(test: Test): Promise<unknown | null> {
  try {
    await testsLimit(() => test.func());
  }
  catch (err: unknown) {
    return err;
  }
  return null;

}
async function runSuite(
  suitPath: string,
): Promise<SuitResult['errors']> {
  const testSuite: Suite = (await import(suitPath)).default;
  const tests = await Promise.all(testSuite.tests.map(async test => ({ [test.name]: await runTest(test) })));
  return tests.reduce((prevValue, test) => ({ ...prevValue, ...test }), {});
}
async function testSuite(
  suitsFolder: string,
  suitName: string,
  testOpts: TestOpts,
): Promise<SuitResult> {
  const suitPath = "./" + path.format({ dir: suitsFolder, name: suitName, ext: extension });


  const suitErrors: SuitResult['errors'] = await runSuite(suitPath);

  const suitPassed = hasSuitPassed(suitErrors);
  const prefix = suitPassed ? "[PASSED]" : "[ERROR ]";
  console.log(`${prefix}: ran ${suitName} (${Array.from(Object.keys(suitErrors)).length} tests)`);
  return {
    name: suitName,
    errors: suitErrors,
  }
}

async function buildQux() {
  console.log("Building qux...");
  const build_output = await new Deno.Command("make", { cwd: ".." }).output();
  if (!build_output.success) {
    console.error(td.decode(build_output.stderr));
    Deno.exit(build_output.code);
  }

  console.log(td.decode(build_output.stdout));
}

(async () => {
  // program
  //   .option("--record");
  program.parse();
  const opts = program.opts();

  await buildQux();

  const suitsFolder = "e2e";
  const testFiles = await Array.fromAsync(readDir(suitsFolder));
  const suits: Promise<SuitResult>[] = testFiles
    .filter((dirEntry: DirEntry) => dirEntry.name.endsWith(extension))
    .map((dirEntry: DirEntry) => dirEntry.name.slice(0, -extension.length))
    .map((suitName) =>
        testSuite(suitsFolder, suitName, { record: opts.record })
    );
  const t0 = performance.now();
  const suitsResults = await Promise.all(suits);
  const t1 = performance.now();

  const erroredSuits = suitsResults.filter((result) => !hasSuitPassed(result.errors));
  if (erroredSuits.length > 0) {
    console.log('-------')
    console.log('Errors:')
    console.log('-------')
    erroredSuits
      .forEach((result: SuitResult) => {
        console.log();
        console.log(`${result.name}:`);
        Array.from(Object.entries(result.errors))
          .filter(([_, error]) => error !== null)
          .forEach(([testName, error]) => console.log(`\t${testName}: \n\t\t${String(error).replaceAll('\n', '\n\t\t')}`))
        ;
      });

  }

  const numOfTests: number = suitsResults
    .map((result) => Array.from(Object.keys(result.errors)).length)
    .reduce((totalTestsCount, testsCount) => totalTestsCount + testsCount, 0)
  ;

  console.log(`Ran ${numOfTests} in ${(t1 - t0).toFixed(3)}ms`);

  Deno.exit(Number(erroredSuits.length > 0));
})();
