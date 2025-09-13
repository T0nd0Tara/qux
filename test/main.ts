import pLimit from "p-limit";
import { readDir } from "@std/fs/unstable-read-dir";
import { DirEntry } from "@std/fs/unstable-types";
import * as path from "@std/path";
import assert from "node:assert";
const testsLimit = pLimit(4);

const extension = ".qux";
const td = new TextDecoder();

interface CmdOutput {
  stdout: string;
  stderr: string;
  exitCode: number;
}
type SuitExpectedResult = CmdOutput & {
compiles: boolean; // Whether we even can compile it to C
};


interface SuitMetdata {
  result: SuitExpectedResult;
}
interface SuitResult {
  name: string,
  mismatches: {
    type: keyof SuitExpectedResult;
    actual: SuitExpectedResult[keyof SuitExpectedResult];
    expected: SuitExpectedResult[keyof SuitExpectedResult];
  }[];
}
function parseCmdOutput(cmdOutput: Deno.CommandOptions): CmdOutput {
  return {
    exitCode: cmdOutput.code,
    stdout: td.decode(cmdOutput.stdout),
    stderr: td.decode(cmdOutput.stderr),
  };
}
function getMismatches(
  expected: SuitExpectedResult,
  actual: SuitExpectedResult,
): SuitResult["mismatches"] {
  const mismatches: SuitResult["mismatches"] = [];
  // @ts-expect-error: I don't care they don't believe me it's the keys
  Object.keys(expected).forEach((key: keyof SuitExpectedResult) => {
    if (expected[key] !== actual[key]) {
      mismatches.push({
        type: key,
        actual: actual[key],
        expected: expected[key],
      });
    }
  });
  return mismatches;
}
async function getSuitMetadata(
  suitName: string,
): Promise<SuitMetdata> {
  const expected: SuitMetdata = JSON.parse(
    await Deno.readTextFile(suitName + ".json"),
  );
  expected.result.stderr ??= "";
  return expected;
}

async function runSuite(suitPath: string): Promise<SuitExpectedResult> {
  const irBuild = await new Deno.Command(
    "./build/qux",
    { cwd: "..", args: [`${import.meta.dirname}/${suitPath}.qux`] },
  )
    .output();

  if (irBuild.code !== 0) {
    const cmdOutput = parseCmdOutput(irBuild);
    return {
      compiles: false,
      ...cmdOutput,
    };
  }

  const build = await new Deno.Command(
    "clang",
    {args: ["-o", `${suitPath}.a`, `${suitPath}.c`] },
  ) 
  .output();
  assert(build.code === 0, "Can compile to IR, but not furthar");

  const run = await new Deno.Command(`${suitPath}.a`)
    .output();
  return {
    compiles: true,
    ...parseCmdOutput(run),
  };
}
async function testSuite(suitsFolder: string, suitName: string): SuitResult {
  const suitPath = path.format({ dir: suitsFolder, name: suitName });

  const metadata = await getSuitMetadata(suitPath);

  const actual = await runSuite(suitPath);

  const result: SuitResult = {
    name: suitName,
    mismatches: getMismatches(metadata.result, actual),
  };
  const prefix = result.mismatches.length > 0 ? "[ERROR ]" : "[PASSED]";
  console.log(`${prefix}: ran ${suitName}`);
  return result;
}
(async () => {
  console.log("Building qux...");
  const build_output = await new Deno.Command("make", { cwd: ".." }).output();
  if (!build_output.success) {
    console.error(td.decode(build_output.stderr));
    Deno.exit(build_output.code);
  }

  console.log(td.decode(build_output.stdout));

  const suitsFolder = "e2e";
  const testFiles = await Array.fromAsync(readDir(suitsFolder));
  const suits: Promise<SuitResult>[] = testFiles
    .filter((dirEntry: DirEntry) => dirEntry.name.endsWith(extension))
    .map((dirEntry: DirEntry) => dirEntry.name.slice(0, -extension.length))
    .map((suitName) => testsLimit(() => testSuite(suitsFolder, suitName)));
  const suitsResults = await Promise.all(suits);
  const erroredResults = suitsResults.filter(result => result.mismatches.length > 0);
  erroredResults
    .forEach(result => {
      console.log(result.name)
      result.mismatches.forEach(mismatch => {
        console.log(`  ${mismatch.type}`);
        console.log(`    actual:   ${mismatch.actual}`);
        console.log(`    expected: ${mismatch.expected}`);
      });
  })
})();
