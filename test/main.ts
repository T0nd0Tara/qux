import pLimit from "p-limit";
import { readDir } from "@std/fs/unstable-read-dir";
import { DirEntry } from "@std/fs/unstable-types";
import * as path from "@std/path";
import assert from "node:assert";
import { program } from "commander";
import type { CmdOutput } from './types.ts';
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
  error: unknown | null;
}
interface TestOpts { }

async function runSuite(
  suitPath: string,
): Promise<unknown | null> {
  const testFunc: () => Promise<void> = (await import(suitPath)).default;
  try {
    await testFunc();
  }
  catch (err: unknown) {
    return err;
  }
  return null;

  // const irBuild = await new Deno.Command(
  //   "./build/qux",
  //   {
  //     cwd: "..",
  //     args: [
  //       `${import.meta.dirname}/${suitPath}.qux`,
  //     ],
  //   },
  // )
  //   .output();

  // if (irBuild.code !== 0 || prep?.compileIr === false) {
  //   const cmdOutput = parseCmdOutput(irBuild);
  //   return {
  //     compiles: false,
  //     ...cmdOutput,
  //   };
  // }

  // const build = await new Deno.Command(
  //   "clang",
  //   { args: ["-o", `${suitPath}.a`, `${suitPath}.c`] },
  // )
  //   .output();
  // assert(build.code === 0, "Can compile to IR, but not furthar");

  // const run = await new Deno.Command(`${suitPath}.a`)
  //   .output();
}
async function testSuite(
  suitsFolder: string,
  suitName: string,
  testOpts: TestOpts,
): Promise<SuitResult> {
  const suitPath = "./" + path.format({ dir: suitsFolder, name: suitName, ext: extension });


  const error: unknown | null = await runSuite(suitPath);

  const passed = error === null;
  const prefix = passed ? "[PASSED]" : "[ERROR ]";
  console.log(`${prefix}: ran ${suitName}`);
  return {
    name: suitName,
    error
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
      testsLimit(() =>
        testSuite(suitsFolder, suitName, { record: opts.record })
      )
    );
  const suitsResults = await Promise.all(suits);
  const erroredResults = suitsResults.filter((result) => result.error);
  if (erroredResults.length > 0) {
    console.log('-------')
    console.log('Errors:')
    console.log('-------')
    erroredResults
      .forEach((result) => {
        console.log();
        console.log(`${result.name}:`);
        console.log(result.error);
      });

    Deno.exit(1);
  }
})();
