const std = @import("std");
const vector = @import("vector.zig");

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

const TokenType = enum {
    variable,
    decleration,
    runtime_assignment,
    comtime_assignment,
    typing,
};

const Token = struct {
    ttype: TokenType,
    value: []u8,
};

const State = struct {
    index: usize,
    tokens: vector.DynamicArray(Token, ),
};

fn get_last_non_typing_token(tokens: []Token) ??Token {
    for (tokens.len..0) |i| {
        if (tokens[i - 1].ttype != .typing) return tokens[i - 1];
    }
    return null;
}
fn get_pos_from_index(index: usize, full_program: []u8) struct { line: usize, column: usize } {
    _ = index;
    _ = full_program;
    const line   : usize = 0;
    const column : usize = 0;

    return .{line, column};
}

fn lex_token(allocator: std.mem.Allocator, state: *State, full_program: []u8) ??LexerError {
    _ = allocator;
    var current_word: []u8 = {};
    while (true) {
        const c: u8 = full_program[state.index];

        if (c == ':') {
            if (current_word.len == 0) {
                state.tokens = state.tokens ++ Token{
                    .ttype = .decleration,
                };
                state.index += 1;
                return;
            }

            if (get_last_non_typing_token(state.tokens).ttype == .decleration) {
                state.tokens = state.tokens ++ Token{
                    .ttype = .comtime_assignment,
                };
                state.index += 1;
                return;
            }
            const pos = get_pos_from_index(state.index, full_program);
            return LexerError{
                pos.line, pos.column,
            };
        }

        current_word = current_word ++ c;
        state.index += 1;
    }
}

pub fn lex_program(allocator: std.mem.Allocator, program: []u8) LexerData {
    const state: State = State{
        .index = 0,
        .tokens = [dynamic],
    };

    while (state.index < program.len) {
        const token_result: ??LexerError = lex_token(allocator, &state, program);
        if (token_result != null) return token_result;
    }
}
