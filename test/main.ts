import pLimit from "p-limit";
import { readDir } from "@std/fs/unstable-read-dir";
import { DirEntry } from "@std/fs/unstable-types";
import * as path from "@std/path";
import assert from "node:assert";
import { program } from "commander";
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

interface SuitPreperation {
  quxCompArgs?: string[];
  compileIr?: boolean;
}

interface SuitMetdata {
  prep?: SuitPreperation;
  result: SuitExpectedResult;
}
interface SuitResult {
  name: string;
  mismatches: {
    type: keyof SuitExpectedResult;
    actual: SuitExpectedResult[keyof SuitExpectedResult];
    expected: SuitExpectedResult[keyof SuitExpectedResult];
  }[];
}
interface TestOpts {
  record?: true;
}
function parseCmdOutput(cmdOutput: Deno.CommandOutput): CmdOutput {
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
  const metadata: SuitMetdata = JSON.parse(
    await Deno.readTextFile(suitName + ".json"),
  );
  metadata.result.stderr ??= "";
  // @ts-expect-error: metadata.result.compiles can be undefined in file, not in code
  metadata.result.compiles ??= metadata.prep?.compileIr;

  return metadata;
}

async function runSuite(
  prep: SuitPreperation | undefined,
  suitPath: string,
): Promise<SuitExpectedResult> {
  const irBuild = await new Deno.Command(
    "./build/qux",
    {
      cwd: "..",
      args: [
        `${import.meta.dirname}/${suitPath}.qux`,
        ...(prep?.quxCompArgs ?? []),
      ],
    },
  )
    .output();

  if (irBuild.code !== 0 || prep?.compileIr === false) {
    const cmdOutput = parseCmdOutput(irBuild);
    return {
      compiles: false,
      ...cmdOutput,
    };
  }

  const build = await new Deno.Command(
    "clang",
    { args: ["-o", `${suitPath}.a`, `${suitPath}.c`] },
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
async function testSuite(
  suitsFolder: string,
  suitName: string,
  testOpts: TestOpts,
): Promise<SuitResult> {
  const suitPath = path.format({ dir: suitsFolder, name: suitName });

  const metadata = await getSuitMetadata(suitPath);

  const actual: SuitExpectedResult = await runSuite(metadata.prep, suitPath);

  const result: SuitResult = {
    name: suitName,
    mismatches: getMismatches(metadata.result, actual),
  };
  const passed =  result.mismatches.length === 0;
  const prefix =  passed ? "[PASSED]" : "[ERROR ]";
  console.log(`${prefix}: ran ${suitName}`);
  if (!passed && testOpts.record) {
    metadata.result = actual;""""
    await Deno.writeTextFile(suitPath + ".json", JSON.stringify(metadata, null, 2));
  }
  return result;
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
  program
    .option("--record");
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
  const erroredResults = suitsResults.filter((result) =>
    result.mismatches.length > 0
  );
  erroredResults
    .forEach((result) => {
      console.log(result.name);
      result.mismatches.forEach((mismatch) => {
        console.log(`  ${mismatch.type}`);
        console.log(`    actual:   ${mismatch.actual}`);
        console.log(`    expected: ${mismatch.expected}`);
      });
    });
})();
