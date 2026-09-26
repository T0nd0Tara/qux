#include "ir.hpp"
#include "ast.hpp"
#include "typing.hpp"
#include <cassert>
#include <sstream>
#include <unordered_map>

#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Driver/CreateInvocationFromArgs.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Tool.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <lld/Common/Driver.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>
 
LLD_HAS_DRIVER(elf)

#ifndef CLANG_RESOURCE_DIR
#warning CLANG_RESOURCE_DIR not defined, some problems may occur
#endif // CLANG_RESOURCE_DIR

std::unordered_map<std::string, Typing*> typing_map = {};

void typing_to_ir(const Typing* typing, std::string_view variable_name, std::stringstream& ss) {
  assert(typing != nullptr);

  switch (typing->type) {
    case TypingType::I8:  ss << "int8_t";  return;
    case TypingType::I16: ss << "int16_t"; return;
    case TypingType::I32: ss << "int32_t"; return;
    case TypingType::I64: ss << "int64_t"; return;

    case TypingType::U8:  ss << "uint8_t";  return;
    case TypingType::U16: ss << "uint16_t"; return;
    case TypingType::U32: ss << "uint32_t"; return;
    case TypingType::U64: ss << "uint64_t"; return;

    case TypingType::F32: ss << "float"; return;
    case TypingType::F64: ss << "double"; return;

    case TypingType::FUNC:
      typing_to_ir(typing->return_type, "", ss);
      ss << " ";
      ss << variable_name;
      ss << "(";
      for (const auto& param_type: typing->parameters_types) {
        typing_to_ir(param_type, "", ss);
      }
      ss << ")";
      
      return;
  }
}
void generate_decleration(const AstNode& ast, std::stringstream& ss) {
  assert(ast.type == AstNodeType::DECLARE);

  const auto& variable_node = ast.children[0];

  assert(variable_node.type == AstNodeType::VARIABLE);

  typing_map[variable_node.variable_name] = variable_node.typing;
  typing_to_ir(variable_node.typing, variable_node.variable_name, ss);
  ss << ";\n";
}
void generate_expr_ir(const AstNode& ast, std::stringstream &ss) {
  switch (ast.type) {
    case AstNodeType::INT_LITERAL: {
      ss << ast.str_val;
      return;
    }
    case AstNodeType::STRING_LITERAL: {
      ss << '"' << ast.str_val << '"';
      return;
    }
  }
}

void generate_assignment(const AstNode& ast, std::stringstream& ss);
void hoist_all_declerations(const AstNode& ast, std::stringstream& ss);

void generate_scope_ir(const AstNode& ast, std::stringstream &ss) {
  hoist_all_declerations(ast, ss);

  for (const auto& node: ast.children) {
    switch (node.type){
      case AstNodeType::ASSIGN_COM: generate_assignment(node, ss); break;
      case AstNodeType::FUNC_CALL: {
        const auto& variable = node.children[0];
        assert(variable.type == AstNodeType::VARIABLE);

        const auto& args = node.children[1];
        assert(args.type == AstNodeType::ARGS);

        ss << variable.variable_name;
        ss << "(";
        for (size_t i = 0; i < args.children.size(); i++) {
          const auto& arg = args.children[i];
          generate_expr_ir(arg, ss);
          if (i < args.children.size() - 1) ss << ", ";
        }
        ss << ");\n";

        break;
      }
      case AstNodeType::RETURN: {
        ss << "return ";
        generate_expr_ir(node.children[0], ss);
        ss << ";\n";
        break;
      }
    }
  }
}

void generate_assignment(const AstNode& ast, std::stringstream& ss) {
  assert(ast.type == AstNodeType::ASSIGN_COM || ast.type == AstNodeType::ASSIGN_RUN);
  const auto& variable_node = ast.children[0];
  const Typing* typing = typing_map[variable_node.variable_name];
  assert(typing != nullptr);

  typing_to_ir(typing, variable_node.variable_name, ss);

  const auto& rvalue_node = ast.children[1];
  if (rvalue_node.type != AstNodeType::FUNC) {
    ss << " = ";
    ss << ";";
    return;
  }

  ss << "{\n";
  generate_scope_ir(rvalue_node, ss);
  ss << "\n}";
}

void hoist_all_declerations(const AstNode& ast, std::stringstream& ss) {
  for (const auto& node : ast.children) {
    if (node.type == AstNodeType::DECLARE)
      generate_decleration(node, ss);
  }
}

void generate_ir(const AstNode& ast, std::stringstream& ss) {
  assert(ast.type == AstNodeType::ROOT);
  
  ss << "#include <stdio.h>\n";
  ss << "#include <stdint.h>\n";

  ss << "#define print printf\n";

  generate_scope_ir(ast, ss);  
}



bool write_machine_code(const std::string& ir, std::string file_name) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
 
  auto MemFS = llvm::makeIntrusiveRefCnt<llvm::vfs::InMemoryFileSystem>();
  MemFS->addFile("/virtual/input.c", 0, llvm::MemoryBuffer::getMemBuffer(ir));
  auto FS = llvm::makeIntrusiveRefCnt<llvm::vfs::OverlayFileSystem>(
      llvm::vfs::getRealFileSystem());
  FS->pushOverlay(MemFS);
 
  clang::DiagnosticOptions diag_opts;
  diag_opts.IgnoreWarnings = true;
  clang::TextDiagnosticPrinter diag_printer(llvm::errs(), diag_opts);
  clang::DiagnosticsEngine diags(clang::DiagnosticIDs::create(), diag_opts,
                                 &diag_printer, /*ShouldOwnClient=*/false);
 
  std::vector<const char *> args = {"clang", "-w", "-O2", "/virtual/input.c",
                                    "-o", file_name.c_str()};
#ifdef CLANG_RESOURCE_DIR
  args.push_back("-resource-dir");
  args.push_back(CLANG_RESOURCE_DIR);
#endif

  clang::driver::Driver driver("clang", llvm::sys::getDefaultTargetTriple(), diags,
                          "in-memory clang", FS);
  std::unique_ptr<clang::driver::Compilation> compilation(driver.BuildCompilation(args));
  if (!compilation || diags.hasErrorOccurred()) return 1;
 
  bool ok = true;
  for (const clang::driver::Command &job : compilation->getJobs()) {
    const auto &job_args = job.getArguments();
 
    if (job.getCreator().isLinkJob()) {
      std::vector<const char *> link_args = {"ld.lld"};
      link_args.insert(link_args.end(), job_args.begin(), job_args.end());
      lld::Result result = lld::lldMain(link_args, llvm::outs(), llvm::errs(),
                                   {{lld::Gnu, &lld::elf::link}});
      ok = result.retCode == 0;
    } else {
      auto invovation = std::make_shared<clang::CompilerInvocation>();
      clang::CompilerInvocation::CreateFromArgs(
          *invovation, llvm::ArrayRef(job_args).drop_front(), diags);
      clang::CompilerInstance instance(invovation);
      instance.createVirtualFileSystem(FS);
      instance.createDiagnostics();
      clang::EmitObjAction action;
      ok = instance.ExecuteAction(action);
    }
    if (!ok) break;
  }
 
  compilation->CleanupFileList(compilation->getTempFiles());
  return ok;
}
JitResult create_jit_run_func(const std::string& ir) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
 
  std::vector<const char *> args = {"clang", "-w", "-O2", "input.c"};

#ifdef CLANG_RESOURCE_DIR
  args.push_back("-resource-dir");
  args.push_back(CLANG_RESOURCE_DIR);
#endif // CLANG_RESOURCE_DIR

  std::shared_ptr<clang::CompilerInvocation> invocation = clang::createInvocation(args);
  if (!invocation) return { .compilation_succeeded = false };
 
  invocation->getPreprocessorOpts().addRemappedFile(
      "input.c", llvm::MemoryBuffer::getMemBufferCopy(ir, "input.c").release());
 
  clang::CompilerInstance instance(invocation);
  instance.createVirtualFileSystem();
  instance.createDiagnostics();
  auto ctx = std::make_unique<llvm::LLVMContext>();
  clang::EmitLLVMOnlyAction action(ctx.get());
  if (!instance.ExecuteAction(action)) return { .compilation_succeeded = false };

  std::unique_ptr<llvm::Module> module = action.takeModule();
 
  auto jit = llvm::cantFail(llvm::orc::LLJITBuilder().create());
  llvm::cantFail(jit->addIRModule(
      llvm::orc::ThreadSafeModule(std::move(module), std::move(ctx))));
  auto main_addr = llvm::cantFail(jit->lookup("main"));
  return { .return_code = main_addr.toPtr<int (*)()>()() };
}
