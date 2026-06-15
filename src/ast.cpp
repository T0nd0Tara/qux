#include "ast.hpp"
#include "lexer.hpp"
#include <algorithm>
#include <boost/program_options/errors.hpp>
#include <cstddef>
#include <functional>
#include <iterator>
#include <iostream>
#include <format>
#include <utility>
#include <vector>
#include <map>
#include <cassert>
#include <magic_enum/magic_enum.hpp>


static const std::map<TokenType, TokenType> brace_map = {
  { TokenType::OPEN_PAREN, TokenType::CLOSE_PAREN },
  { TokenType::OPEN_CURLY_BRACKET, TokenType::CLOSE_CURLY_BRACKET },
};

size_t find_matching_brace(const std::vector<Token>& tokens, size_t from_index) {
  const auto& current_token = tokens[from_index];
  const TokenType open_type = current_token.type;

  const auto close_type_it = brace_map.find(open_type);
  assert(close_type_it != brace_map.end() && "Parenthesis type not supported");

  const TokenType close_type = close_type_it->second;

  size_t stack = 0;
  for (size_t i = from_index + 1; i < tokens.size(); i++) {
    const TokenType it_type = tokens[i].type;

    if (it_type == open_type) stack++;
    if (it_type == close_type) {
      if (stack == 0) return i;
      stack--;
    }
  }

  
  return -1;
}

size_t find_next_delim(const std::vector<Token>& tokens, size_t from_index, std::function<bool(TokenType)> pred) {
  assert(from_index <= tokens.size() && "invalid from_index paramater");
  size_t i = from_index;
  while (i < tokens.size()) {
    const auto& curr_token = tokens[i];
    if (brace_map.contains(curr_token.type)) {
      const size_t end_brace = find_matching_brace(tokens, i);
      if (end_brace == (size_t)(-1)) {
        return -1;
      }

      i = end_brace + 1;
      continue;
    }
    if (pred(curr_token.type)) {
      return i;
    }

    ++i;
  }
  return -1;
}

AstError error_from_token(const Token& token, std::string message) {
  return AstError{
    .row = token.row,
    .col = token.col,
    .message = message,
  };
}

//
// Parses The statements of a scope
std::pair<std::vector<AstNode>, std::vector<AstError>> parse_statements(const std::vector<Token>& tokens, size_t& index, const std::vector<AstNode>& parsed_statements);


std::pair<AstNode, std::vector<AstError>> parse_expr(const std::vector<Token>& tokens, size_t& index, size_t end_index, const std::vector<AstNode>& parsed_statements) {
  // std::cout << "parsing expr: " << tokens[index] << " - " << tokens[end_index] << std::endl;
  const auto& curr_token = tokens[index];
  std::vector<AstError> errors;

  const auto expect_end_of_expr = [&]() {
      if (end_index - index != 1) {
        errors.push_back(error_from_token(tokens[std::min(index + 1, tokens.size() - 1)], "Expected End Of expresion"));
        ++index;
      } else {
        // +1 to get to the COMMA or CLOSE_PAREN 
        // +1 to get to the next arguement
        index = end_index + 2;
      }

  };
  switch (curr_token.type){
    case TokenType::STRING_LITERAL:{
      expect_end_of_expr();
      return std::make_pair(AstNode{
        .type = AstNodeType::STRING_LITERAL,
        .str_val = curr_token.str_val
      }, errors);
    }
    case TokenType::INT_LITERAL:{
      expect_end_of_expr();
      return std::make_pair(AstNode{
        .type = AstNodeType::INT_LITERAL,
        .str_val = curr_token.str_val
      }, errors);
    }
    case TokenType::OPEN_PAREN: {
      // TODO: OPEN_PAREN doesnt necessarily mean function, it can be an expression like `(1 + 3 * 5)`
      const size_t close_paren = find_matching_brace(tokens, index);
      if (close_paren == (size_t)(-1)) {
        // I don't want to continue parsing if there's no matching close parenthesis, 
        // because I cannot know how to parse that 
        index = tokens.size();
        return std::make_pair(
          AstNode{
            .type = AstNodeType::NOOP,
          }, std::vector<AstError>{
            AstError{
              .row = curr_token.row,
              .col = curr_token.col,
              .message = "expected matching closed parenthesis"
            }
          });
      }

      if (close_paren - index > 1) {
        std::cerr << "TODO: parse_statement: OPEN_PAREN: allow stuff in brackets" << std::endl;
        exit(1);
      }


      index = close_paren + 1;

      auto [children, children_errors] = parse_statements(tokens, index, parsed_statements);
      errors.insert(errors.end(), children_errors.begin(), children_errors.end());
      expect_end_of_expr();
      return std::make_pair(AstNode{
        .type = AstNodeType::FUNC,
        .children = children,
      }, errors);


    }

  }
  index++;
  errors.push_back(error_from_token(curr_token, std::format("trying to parse token '{}' as expression", magic_enum::enum_name(curr_token.type))));
  return std::make_pair(
    AstNode{
      .type = AstNodeType::NOOP,
    }, errors);
  
}

typedef std::pair<AstNode, std::vector<AstError>> statement_parse_ret_type;
statement_parse_ret_type parse_return(const std::vector<Token>& tokens, size_t& index, const std::vector<AstNode>& parsed_statements) {
  const auto& return_token = tokens[index - 1];
  assert(return_token.str_val == "return");

  const auto curr_token = tokens[index];
  const size_t statement_end = find_next_delim(tokens, index + 1, [](TokenType type) { return type == TokenType::SEMICOLON; });
  if (statement_end == (size_t)(-1)) {
    index = tokens.size();
    return std::make_pair(
      AstNode {
    .type = AstNodeType::RETURN,
  }, 
      std::vector<AstError>({ error_from_token(return_token, "Return statement must end with a semicolun") } )
    );
  }

  const auto [expr, errors] = parse_expr(tokens, index, statement_end, parsed_statements);
  return std::make_pair(AstNode {
    .type = AstNodeType::RETURN,
    .children = std::vector<AstNode>({ expr })
  }, errors);
}
static const std::map<std::string, std::function<statement_parse_ret_type(const std::vector<Token>& tokens, size_t& index, const std::vector<AstNode>& parsed_statements)>> keyword_parse_map = {
  {"return", parse_return } 
};

std::pair<AstNode, std::vector<AstError>> parse_args(const std::vector<Token>& tokens, size_t& index) {
  // std::cout << "parsing args: " << tokens[index] << std::endl;
  const auto & open_paren = tokens[index];

  size_t close_paren_ind = find_matching_brace(tokens, index);
  if (close_paren_ind == (size_t)(-1)) {
    index = tokens.size();
    return std::make_pair(
      AstNode{
        .type = AstNodeType::NOOP,
      }, std::vector<AstError>{
        {
          .row = open_paren.row,
          .col = open_paren.col,
          .message = "Args closing parenthesis not found",
        }
      });
  }
  std::vector<AstNode> args;
  std::vector<AstError> errors;
  size_t arg_index = index + 1;
  while (arg_index < close_paren_ind) {
    const auto& token_it = tokens[arg_index];
    size_t end_index = find_next_delim(tokens, arg_index, [](TokenType type) { return type == TokenType::COMMA || type == TokenType::CLOSE_PAREN; });
    if (end_index == (size_t)(-1)) {
      return std::make_pair(
      AstNode{
        .type = AstNodeType::NOOP,
      }, std::vector<AstError>{
        {
          .row = token_it.row,
          .col = token_it.col,
          .message = "Arg doesn't seem to end",
        }
      });
    }

    auto [arg, arg_errors] = parse_expr(tokens, arg_index, end_index, std::vector<AstNode>());
    args.push_back(arg);
    errors.insert(errors.end(), arg_errors.begin(), arg_errors.end());

  }

  index = close_paren_ind + 1;

  return std::make_pair(
    AstNode{
      .type = AstNodeType::ARGS,
      .children = args

  }, errors);

  
}


std::pair<AstNode, std::vector<AstError>> parse_statement(const std::vector<Token>& tokens, size_t& index, const std::vector<AstNode>& parsed_statements) {
  // std::cout << "parsing statement: " << tokens[index] << std::endl;
  const auto& curr_token = tokens[index];

  switch (curr_token.type) {
    case TokenType::IDENTIFIER: {

      const auto& next_token = tokens[++index]; 
      switch (next_token.type) {
        case TokenType::COLON: {
          return std::make_pair(AstNode{
            .type = AstNodeType::DECLARE,
            .children = {
              AstNode{
                .type = AstNodeType::VARIABLE,
                .variable_name = curr_token.str_val,
              },
            },
          },std::vector<AstError>());
        }

        // Func Call
        case TokenType::OPEN_PAREN: {
          auto [args, errors] = parse_args(tokens, index);
          if (const auto& end_statement_token = tokens[std::max(index, tokens.size() - 1)];
            end_statement_token.type != TokenType::SEMICOLON) {
            errors.push_back(error_from_token(end_statement_token, "Expected Semicolon after function call"));
          }
          index++;
          return std::make_pair(AstNode{
            .type = AstNodeType::FUNC_CALL,
            .children = {
              AstNode {
                .type = AstNodeType::VARIABLE,
                .variable_name = curr_token.str_val,
              },
              args,
            }

          }, errors);
        }

      default: 
        auto keyword_parse = keyword_parse_map.find(curr_token.str_val);
        if (keyword_parse != keyword_parse_map.end()) {
          return keyword_parse->second(tokens, index, parsed_statements);
        }
        return std::make_pair(
            AstNode{
            .type = AstNodeType::NOOP,
          },
            std::vector<AstError>({
              { 
                .row = next_token.row,
                .col = next_token.col,
                .message = std::format("Unsupported Token '{}' after identifier", magic_enum::enum_name(next_token.type)),
              }
            }));
      }
        
    }

    case TokenType::COLON: {
      if (parsed_statements.size() == 0) {
        return std::make_pair(
          AstNode{
            .type = AstNodeType::NOOP,
          },
          std::vector<AstError>{
            AstError{
              .row = curr_token.row,
              .col = curr_token.col,
              .message = "Colon Can't Start a Block Statement"
            }});
      }

      const auto & last_statement = parsed_statements.back();
      if (last_statement.type == AstNodeType::DECLARE) {
         const auto [value, errors] = parse_statement(tokens, ++index, std::vector<AstNode>());

        return std::make_pair(
          AstNode{
            .type = AstNodeType::ASSIGN_COM,
            .children = {
              last_statement.children.front(),
              value,
            } 
          }
          , errors);
      }
      return std::make_pair(
        AstNode{
          .type = AstNodeType::NOOP,
        },
        std::vector<AstError>{
          AstError{
            .row = curr_token.row,
            .col = curr_token.col,
            .message = std::format("Cannot Parse Colon After '{}' statement", magic_enum::enum_name(last_statement.type)),
          }

        });

    }

    default:

      // Maybe it's an expression
      size_t end_expr = find_next_delim(tokens, index, [](TokenType type) { return type == TokenType::SEMICOLON; });
      if (end_expr == (size_t)(-1)) {
        return std::make_pair(
          AstNode{
            .type = AstNodeType::NOOP,
          },
          std::vector<AstError>{
            AstError{
              .row = curr_token.row,
              .col = curr_token.col,
              .message = "Missing Semicolon for expression",
            }

          });
      }
      return parse_expr(tokens, index, end_expr, parsed_statements);
  }
  return std::make_pair(AstNode{
    .type = AstNodeType::NOOP,
  }, std::vector<AstError>());
}

std::pair<std::vector<AstNode>, std::vector<AstError>> parse_statements(const std::vector<Token>& tokens, size_t& index, const std::vector<AstNode>& parsed_statements) {
  const auto& curr_token = tokens[index];

  if (curr_token.type != TokenType::OPEN_CURLY_BRACKET) {
    const auto [node, errors] = parse_statement(tokens, index, parsed_statements);
    return std::make_pair(std::vector<AstNode>{ node }, errors);
  }

  const size_t close_curly_bracket = find_matching_brace(tokens, index);
  if (close_curly_bracket == (size_t)(-1)) {
    return std::make_pair(std::vector<AstNode>(), std::vector<AstError>{
  {
        .row = curr_token.row,
        .col = curr_token.col,
        .message = "Expected Scop Close",
      }
    });
  }
  ++index;

  std::vector<AstNode> nodes;
  std::vector<AstError> errors;

  while (index < close_curly_bracket) {
    auto [node, statement_errors] = parse_statement(tokens, index, nodes);
    nodes.push_back(node);
    errors.insert(errors.end(), statement_errors.begin(), statement_errors.end());
  }

  index = close_curly_bracket;
  return std::make_pair(nodes, errors);
}

std::pair<std::vector<AstNode>, std::vector<AstError>> parse_all_statements(const std::vector<Token>& tokens) {
  std::vector<AstNode> statements;
  std::vector<AstError> errors;

  size_t i = 0;
  while (i < tokens.size()) {
    auto [statement, statement_errors] = parse_statement(tokens, i, statements);
    statements.push_back(statement);
    errors.insert(errors.end(), statement_errors.begin(), statement_errors.end());
    ++i;
  }

  return std::make_pair(statements, errors);
}


std::pair<AstNode, std::vector<AstError>> get_ast(const std::vector<Token>& tokens) {
  auto [statements, errors] = parse_all_statements(tokens);
  return std::make_pair(AstNode{
    .type = AstNodeType::ROOT,
    .children = statements,
  }, errors);
}


void stringify_ast(const AstNode& node, std::stringstream& ss, const std::string prefix, bool is_last) {
  ss << prefix;
  ss << (is_last ? "└──" : "├──");
  ss << magic_enum::enum_name(node.type);
  if (node.type == AstNodeType::VARIABLE) {
    ss << ": \"" << node.variable_name << "\"";
  }
  if (node.type == AstNodeType::STRING_LITERAL) {
    ss << ": \"" << node.str_val << "\"";
  }
  ss << '\n'; 
  std::string subprefix = is_last ? "   " : "│   ";
  std::string children_prefix = prefix + subprefix;
  for (size_t i = 0; i < node.children.size(); i++) {
    stringify_ast(node.children[i], ss, children_prefix, i == node.children.size() - 1);
  }
}
void stringify_ast(const AstNode& node, std::stringstream& ss) {
  stringify_ast(node, ss, "", true);
}

void stringify_ast_errors(const std::vector<AstError>& errors, std::stringstream& ss) {
  for (size_t i = 0; i< errors.size(); ++i) {
    const auto& error = errors[i];
    ss << error.message << ":" << (error.row + 1) << ":" << (error.col + 1);
    ss << "\n";
  }
}
