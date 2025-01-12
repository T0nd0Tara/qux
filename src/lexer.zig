const std = @import("std");

// The reason we don't use the zig's error is cause they don't allow us to pass in extra information
// There is an issue for this: https://github.com/ziglang/zig/issues/2647. But it doesn't look like it's coming in
pub const LexerError = struct {
    line: usize,
    column: usize,
    message: []const u8,
};

pub const LexerOk = struct {};

pub const LexerDataEnum = enum {
    err,
    ok,
};

pub const LexerData = union(LexerDataEnum) {
    err: LexerError,
    ok: LexerOk,
};

pub fn lex_program(allocator: std.mem.Allocator, program: []u8) LexerData {
    _ = allocator;
    _ = program;
    return LexerData{ .err = .{
        .line = 69,
        .column = 420,
        .message = "YOOOOO",
    } };
}
