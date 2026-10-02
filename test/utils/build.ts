import path from 'path';

export function createQuxBuildProcess(program: string, args?: string[]): Bun.Subprocess<'pipe', 'pipe', 'pipe'> {
  args ??= [];
  args.push('--stdin');
  const proc: Bun.Subprocess<'pipe', 'pipe', 'pipe'> = Bun.spawn(
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
