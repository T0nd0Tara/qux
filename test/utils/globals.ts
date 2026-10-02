import os from 'node:os'
import path from 'node:path'
import fs from 'node:fs/promises'

export const tmpdir: string = await fs.mkdtemp(path.join(os.tmpdir(), `qux-test`))
export const getNewTempFilePath = (): string => {
  return path.join(tmpdir, Bun.randomUUIDv7());
}
