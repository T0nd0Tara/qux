const std = @import("std");
const lexer = @import("lexer.zig");

const help_message =
    \\ Lol you need help?
;

const CompileErrors = error{
    LexerError,
    ParsingError,
};

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();

    const allocator = arena.allocator();

    const args = try std.process.argsAlloc(allocator);

    if (args.len != 2) {
        std.debug.print(help_message, .{});
        return error.ExpectedFileInput;
    }

    const file_name = args[1];
    const program = std.fs.cwd().readFileAlloc(allocator, file_name, std.math.maxInt(usize)) catch |err| {
        std.debug.print("Couldn't read file {s}\nError: {any}\n", .{ file_name, err });
        return err;
    };

    const lexer_data = lexer.lex_program(allocator, program);
    switch (lexer_data) {
        lexer.LexerDataEnum.err => |err_data| {
            std.debug.print("Lexer Error: {s}\nAt position {any}:{any}\n", .{ err_data.message, err_data.line, err_data.column });
            return error.LexerError;
        },
        lexer.LexerDataEnum.ok => |value| {
            _ = value;
        },
    }

    std.debug.print("All is goot", .{});
}
