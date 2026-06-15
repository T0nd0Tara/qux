import path from 'path';

export function createQuxBuildProcess(program: string, args?: Set<string>): Bun.Subprocess {
  args ??= new Set();
  args.add('--stdin');
  const proc: Bun.Subprocess = Bun.spawn(
    [path.join("bin", "qux"), ...Array.from(args)],
    {
      cwd: path.join(import.meta.dirname, "..",".."),
      stdin: "pipe",
      stdout: "pipe",
      stderr: "pipe",
    },
  );
  proc.stdin.write(program);
  proc.stdin.end();
  return proc;
}
