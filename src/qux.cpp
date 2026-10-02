#include <magic_enum/magic_enum.hpp>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <sstream>
#include "ast.hpp"
#include "ir.hpp"
#include "lexer.hpp"
#include "typing.hpp"


int read_program(boost::program_options::variables_map& vm, std::string& buffer, std::string& output_file) {
    const auto resolve_output_filename = [&](std::string default_filename) {
      output_file = default_filename;
      if (vm.count("output")) {
        output_file = vm["output"].as<std::string>();
      }
    };

    if (vm["stdin"].as<bool>()) {
      resolve_output_filename("out");
      // don't skip the whitespace while reading
      std::cin >> std::noskipws;

      std::istream_iterator<char> it(std::cin);
      std::istream_iterator<char> end;
      buffer = std::string(it, end);
      return 0;
    }

    std::string input_file = vm["filename"].as<std::string>();

    std::string input_file_without_extention = input_file.substr(0, input_file.find_last_of("."));
    resolve_output_filename(input_file_without_extention);

    std::ifstream f(input_file);
    if (!f.is_open()) {
      return 1;
    }
    buffer = std::string(std::istreambuf_iterator<char>(f), {});
    f.close();
    return 0;
}
int write_ir(std::string filename, std::string_view ir) {
    std::ofstream f(filename, std::ios::trunc);
    if (!f.is_open()) {
      return 1;
    }
    f << ir;
    f.close();
    return 0;
}

int cli_handle(int argc, char** argv) {
  namespace po = boost::program_options;
  po::options_description desc("Options");
  bool no_write, print_ir, print_types, print_tokens, print_ast, is_stdin, run_jit;
  std::string input_file, output_file;

  desc.add_options()
    ("help,h", "produce help message")
    ("output,o", po::value<std::string>(&output_file), "output file")
    ("filename", po::value<std::string>(&input_file), "input file")
    ("run", po::bool_switch(&run_jit), "auto runs the program")
    ("print-tokens", po::bool_switch(&print_tokens), "prints the lexer tokens to the console")
    ("print-ast", po::bool_switch(&print_ast), "prints the AST to the console")
    ("print-types", po::bool_switch(&print_types), "prints the varibales types to the console")
    ("print-ir", po::bool_switch(&print_ir), "prints the intermediate representation to the console")
    ("no-write", po::bool_switch(&no_write), "do not write the executable to a file")
    ("stdin", po::bool_switch(&is_stdin), "read program from stdin")
  ;
  po::positional_options_description pos;
  pos.add("filename", 1);  // first positional arg is filename

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
    .options(desc)
    .positional(pos)
    .run()
    , vm);
  po::notify(vm);    

  if (vm.count("help")) {
      std::cout << "Usage: " << argv[0] << " [options] filename\n";
      std::cout << desc << "\n";
      return 0;
  }
  if (is_stdin && vm.count("filename") != 0) {
      std::cerr << "You cannot set a filename and the --stdin flag\n";
      return 1;
  }
  if (!is_stdin && vm.count("filename") == 0) {
      std::cerr << "Please provide an input file\n";
      return 1;
  }

  if (run_jit && vm.count("output")) {
    std::cerr << "Cannot set both `--run` and `-o,--output` flags\n";
    return 1;
  }

  int ret;
  std::string buffer;
  ret = read_program(vm, buffer, output_file);
  if (ret) {
    std::cerr << "Couldn't read file '" << input_file << "'. exiting...\n";
    return ret;
  }
  const auto [tokens, lex_errors] = lex_tokens(buffer);
  if (lex_errors.size() > 0) {
    std::stringstream ss_errors;
    stringify_lexer_errors(lex_errors, ss_errors);
    std::cerr << "Lexing Errors:\n\n" << ss_errors.str() << "\n";
    return 1;
  }


  if (print_tokens) {
    std::stringstream ss_tokens;  
    stringify_tokens(tokens, ss_tokens);
    std::cout << ss_tokens.str() << std::endl;
    return 0;
  }

  auto [ast, ast_errors] = get_ast(tokens);
  if (ast_errors.size() > 0) {
    std::stringstream ss_errors;
    stringify_ast_errors(ast_errors, ss_errors);
    std::cerr << "AST Errors:\n\n" << ss_errors.str() << "\n";
    return 1;
  }
  fill_typing(ast);

  if (print_ast) {
    std::stringstream ss_ast;

    stringify_ast(ast, ss_ast);
    std::cout << ss_ast.str();
    return 0;
  }


  std::stringstream ir_ss;
  generate_ir(ast, ir_ss);
  const std::string ir = ir_ss.str();
  if (print_ir) {
    std::cout << ir;
    std::cout << "\n\n";
  }

  if (run_jit) { 
    JitResult res = create_jit_run_func(ir);
    if (!res.compilation_succeeded) {
      std::cerr << "Could not create a jit compilation\n";
      return 1;
    }
    return res.return_code;
  }

  if (!no_write) {
    std::cerr << "generating executable to " << output_file << "\n";
    if (!write_machine_code(ir, output_file)) {
      std::cerr << "Could not generate executable\n";
      return 1;
    }
  }

  return 0;
}
int main(int argc, char** argv)
{
  try {
    return cli_handle(argc, argv);
  }
  catch(std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
  }
  catch(...) {
    std::cerr << "Exception of unknown type!\n";
  }

  return 1;

}


