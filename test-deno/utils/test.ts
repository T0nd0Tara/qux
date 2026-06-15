export class Test {
  constructor(public name: string, public func: () => Promise<void>) { }
};

export class Suite {
  public tests: Test[];

  constructor(public name: string, ...tests: Test[]) {
    this.tests = tests;
  }
};

export function test(name: string, func: () => Promise<void>): Test {
  return new Test(name, func);
}
