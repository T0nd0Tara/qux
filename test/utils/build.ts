import type { CmdOutput } from "../types.ts";
import * as path from '@std/path';
import randomstring from 'randomstring';
import assert from "node:assert";

const td = new TextDecoder();
const te = new TextEncoder();

export function parseCmdOutput(cmdOutput: Deno.CommandOutput): CmdOutput {
  return {
    exitCode: cmdOutput.code,
    stdout: td.decode(cmdOutput.stdout),
    stderr: td.decode(cmdOutput.stderr),
  };
}

async function createQuxBuildProcess(program: string, args?: Set<string>): Promise<Deno.ChildProcess> {

  args ??= new Set();
  const cmd = new Deno.Command(
    path.join("build", "qux"),
    {
      cwd: path.join(import.meta.dirname, "..",".."),
      args: Array.from(args),
      stdin: "piped",
      stdout: "piped",
      stderr: "piped",
    },
  );
  const childProcess = cmd.spawn();
  const writer = childProcess.stdin.getWriter();
  try {
    await writer.write(te.encode(program));
    await writer.close();
  } catch {} // sometimes we want to check a compiler error, so the stdin can be close. that's ok
  return childProcess;
}
export async function buildQuxProgram(program: string, args?: Set<string>): Promise<CmdOutput> {
  const quxProcess = await createQuxBuildProcess(program, args);
  return parseCmdOutput(await quxProcess.output());
}
export async function runQuxProgram(program: string, buildIRArgs?: Set<string>, clangArgs?: Set<string>, quxArgs?: Set<string>): Promise<CmdOutput> {
  buildIRArgs ??= new Set();
  buildIRArgs.add('--stdin');
  buildIRArgs.add('--print-ir');
  buildIRArgs.add('--no-write');

  const genIRProcess = await createQuxBuildProcess(program, buildIRArgs);

  const randomFilename = randomstring.generate({
    length: 12,
    charset: 'alphabetic'
  }) + '.a';
  clangArgs ??= new Set();
  clangArgs.add('-xc');
  clangArgs.add(`-o${randomFilename}`);
  clangArgs.add('-');
  const clang = new Deno.Command(
    "clang",
    {
      args: Array.from(clangArgs),
      stdin: "piped",
      stdout: "piped",
      stderr: "piped",
    }
  ).spawn();
  genIRProcess.stdout.pipeTo(clang.stdin);
  const clangOutput = await clang.output();
  assert(clangOutput.code === 0);

  const quxProgramOutput = await new Deno.Command(
    `./${randomFilename}`,
    {
      args: Array.from(quxArgs ?? []),
    }
  ).output();

  // not awaiting on purpose, will happen when it'll happen
  Deno.remove(randomFilename);

  return parseCmdOutput(quxProgramOutput);
}
