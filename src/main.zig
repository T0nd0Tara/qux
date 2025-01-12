const std = @import("std");

const help_message =
    \\ Lol you need help?
;

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
    defer allocator.free(program);
}
