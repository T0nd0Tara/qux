import pLimit from "p-limit";
import { readDir } from "@std/fs/unstable-read-dir";
import { DirEntry } from "@std/fs/unstable-types";
const testsLimit = pLimit(4);

const extension = ".qux";
(async () => {
  const testFiles = await Array.fromAsync(readDir("e2e"));
  const suits: string[] = testFiles
    .filter((dirEntry: DirEntry) => dirEntry.name.endsWith(extension))
    .map((dirEntry: DirEntry) => dirEntry.name.slice(0, -extension.length));
  console.log(suits);
})();
