import { afterAll } from "bun:test";
import fs from "node:fs/promises";
import { tmpdir } from '@/utils/globals.ts';

afterAll(async () => {
  await fs.rm(tmpdir, { recursive: true, force: true });
});
