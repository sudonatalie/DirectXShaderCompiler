///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxil2spv.cpp                                                                 //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides the entry point for the dxil2spv console program.                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "clang/SPIRV/SpirvInstruction.h"
#include "clang/SPIRV/SpirvFunction.h"
#include "clang/SPIRV/SpirvBasicBlock.h" 
#include "clang/SPIRV/SpirvModule.h"
#include "clang/SPIRV/SpirvBuilder.h"
#include "clang/SPIRV/SpirvBuilder2.h"
#include "clang/SPIRV/SpirvType.h"
#include "clang/SPIRV/SpirvContext.h"

#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"
#include <vector>
#include <string>

#include "dxc/dxcapi.h"
#include "dxc/dxcapi.internal.h"
#include "dxc/dxctools.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/microcom.h"
#include "dxc/DXIL/DxilUtil.h"
#include <comdef.h>
#include <iostream>
#include <limits>

#include "llvm/Support/FileSystem.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/Module.h"
#include "spirv-tools/optimizer.hpp"

inline bool wcseq(LPCWSTR a, LPCWSTR b) {
  return (a == nullptr && b == nullptr) || (a != nullptr && b != nullptr && wcscmp(a, b) == 0);
}
inline bool wcsieq(LPCWSTR a, LPCWSTR b) { return _wcsicmp(a, b) == 0; }
inline bool wcsistarts(LPCWSTR text, LPCWSTR prefix) {
  return wcslen(text) >= wcslen(prefix) && _wcsnicmp(text, prefix, wcslen(prefix)) == 0;
}
inline bool wcsieqopt(LPCWSTR text, LPCWSTR opt) {
  return (text[0] == L'-' || text[0] == L'/') && wcsieq(text + 1, opt);
}

static dxc::DxcDllSupport g_DxcSupport;

enum class ProgramAction {
  PrintHelp,
  RunOptimizer,
};

const wchar_t *STDIN_FILE_NAME = L"-";
bool isStdIn(LPCWSTR fName) {
  return wcscmp(STDIN_FILE_NAME, fName) == 0;
}

// Arg does not start with '-' or '/' and so assume it is a filename,
// or next arg equals '-' which is the name of stdin.
bool isFileInputArg(LPCWSTR arg) {
  const bool isNonOptionArg = !wcsistarts(arg, L"-") && !wcsistarts(arg, L"/");
  return isNonOptionArg || isStdIn(arg);
}

static HRESULT ReadStdin(std::string &input) {
  HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
  std::vector<unsigned char> buffer(1024);
  DWORD numBytesRead = -1;
  BOOL ok = FALSE;

  // Read all data from stdin.
  while (ok = ReadFile(hStdIn, buffer.data(), buffer.size(), &numBytesRead, NULL)) {
    if (numBytesRead == 0)
      break;
    std::copy(buffer.begin(), buffer.begin() + numBytesRead, std::back_inserter(input));
  }

  DWORD lastError = GetLastError();

  // Make sure we reached finished successfully.
  if (ok)
    return S_OK;
  // Or reached the end of a pipe.
  else if (!ok && numBytesRead == 0 && lastError == ERROR_BROKEN_PIPE)
    return S_OK;
  else
    return HRESULT_FROM_WIN32(lastError);
}

static void BlobFromFile(LPCWSTR pFileName, IDxcBlob **ppBlob) {
  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcBlobEncoding> pFileBlob;
  IFT(g_DxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  if (isStdIn(pFileName)) {
    std::string input;
    IFT(ReadStdin(input));
    IFTBOOL(input.size() < std::numeric_limits<UINT32>::max(), E_FAIL);
    IFT(pLibrary->CreateBlobWithEncodingOnHeapCopy(input.data(), (UINT32)input.size(), CP_UTF8, &pFileBlob))
  }
  else {
    IFT(pLibrary->CreateBlobFromFile(pFileName, nullptr, &pFileBlob));
  }
  *ppBlob = pFileBlob.Detach();
}

static void PrintOptOutput(LPCWSTR pFileName, IDxcBlob *pBlob, IDxcBlobEncoding *pOutputText) {
  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcBlobEncoding> pOutputText16;
  IFT(g_DxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(pLibrary->GetBlobAsUtf16(pOutputText, &pOutputText16));
  wprintf(L"%*s", (int)pOutputText16->GetBufferSize(),
          (wchar_t *)pOutputText16->GetBufferPointer());
  if (pBlob && pFileName && *pFileName) {
    dxc::WriteBlobToFile(pBlob, pFileName, DXC_CP_UTF8); // TODO: Support DefaultTextCodePage
  }
}

static void ReadFileOpts(LPCWSTR pPassFileName, IDxcBlobEncoding **ppPassOpts, std::vector<LPCWSTR> &passes, LPCWSTR **pOptArgs, UINT32 *pOptArgCount) {
  *ppPassOpts = nullptr;
  // If there is no file, there is no work to be done.
  if (!pPassFileName || !*pPassFileName) {
    return;
  }

  CComPtr<IDxcBlob> pPassOptsBlob;
  CComPtr<IDxcBlobUtf16> pPassOpts;
  BlobFromFile(pPassFileName, &pPassOptsBlob);
  IFT(hlsl::DxcGetBlobAsUtf16(pPassOptsBlob, hlsl::GetGlobalHeapMalloc(), &pPassOpts));
  LPWSTR pCursor = const_cast<LPWSTR>(pPassOpts->GetStringPointer());
  while (*pCursor) {
    passes.push_back(pCursor);
    while (*pCursor && *pCursor != L'\n' && *pCursor != L'\r') {
      ++pCursor;
    }
    while (*pCursor && (*pCursor == L'\n' || *pCursor == L'\r')) {
      *pCursor = L'\0';
      ++pCursor;
    }
  }

  // Remove empty entries and comments.
  size_t i = passes.size();
  do {
    --i;
    if (wcslen(passes[i]) == 0 || passes[i][0] == L'#') {
      passes.erase(passes.begin() + i);
    }
  } while (i != 0);

  *pOptArgs = passes.data();
  *pOptArgCount = passes.size();
  pPassOpts->QueryInterface(ppPassOpts);
}

static void PrintHelp() {
  wprintf(L"%s",
    L"Performs optimizations on a bitcode file by running a sequence of passes.\n\n"
    L"dxil2spv [-? | -pf [PASS-FILE] | [-o=OUT-FILE] | IN-FILE OPT-ARGUMENTS ...]\n\n"
    L"Arguments:\n"
    L"  -?  Displays this help message\n"
    L"  -pf PASS-FILE  Loads passes from the specified file\n"
    L"  -o=OUT-FILE    Output file for processed module\n"
    L"  IN-FILE        File with with bitcode to optimize\n"
    L"  OPT-ARGUMENTS  One or more passes to run in sequence\n"
    L"\n"
    L"Text that is traced during optimization is written to the standard output.\n"
  );
}


void SpirvGen(const llvm::Module *module) { return; }


HRESULT STDMETHODCALLTYPE RunTranslator(
    IDxcBlob *pBlob, _In_count_(optionCount) LPCWSTR *ppOptions,
    UINT32 optionCount, _COM_Outptr_ IDxcBlob **ppOutputModule,
              _COM_Outptr_opt_ IDxcBlobEncoding **ppOutputText) {
  AssignToOutOpt(nullptr, ppOutputModule);
  AssignToOutOpt(nullptr, ppOutputText);
  if (pBlob == nullptr)
    return E_POINTER;
  if (optionCount > 0 && ppOptions == nullptr)
    return E_POINTER;

  //DxcThreadMalloc TM(m_pMalloc);

  // Setup input buffer.
  //
  // The ir parsing requires the buffer to be null terminated. We deal with
  // both source and bitcode input, so the input buffer may not be null
  // terminated; we create a new membuf that copies and appends for this.
  //
  // If we have the beginning of a DXIL program header, skip to the bitcode.
  //
  llvm::LLVMContext Context;
  llvm::SMDiagnostic Err;
  std::unique_ptr<llvm::MemoryBuffer> memBuf;
  std::unique_ptr<llvm::Module> M;
  const char *pBlobContent =
      reinterpret_cast<const char *>(pBlob->GetBufferPointer());
  unsigned blobSize = pBlob->GetBufferSize();
  const hlsl::DxilProgramHeader *pProgramHeader =
      reinterpret_cast<const hlsl::DxilProgramHeader *>(pBlobContent);
  if (IsValidDxilProgramHeader(pProgramHeader, blobSize)) {
    std::string DiagStr;
    GetDxilProgramBitcode(pProgramHeader, &pBlobContent, &blobSize);
    M = hlsl::dxilutil::LoadModuleFromBitcode(
        llvm::StringRef(pBlobContent, blobSize), Context, DiagStr);
  } else {
    llvm::StringRef bufStrRef(pBlobContent, blobSize);
    memBuf = llvm::MemoryBuffer::getMemBufferCopy(bufStrRef);
    M = parseIR(memBuf->getMemBufferRef(), Err, Context);
  }

  if (M == nullptr) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }

  printf("!!! DXIL Module successfully parsed\n");
  printf("!!! module = %s\n", M->getName().data());
  for (llvm::Function &F : M->getFunctionList()) {
    printf("!!! function = %s\n", F.getName().data());
    //llvm::Type* type = F.getReturnType();
      //clang types are only from AST context: https://clang.llvm.org/doxygen/Type_8h_source.html#l02518
  // TODO: Add overload to create spirvfunction with llvm type, then we'll need to translate llvm types to spirv types
    //clang::spirv::SpirvFunction *fn =
    //    new clang::spirv::SpirvFunction(type, {}, F.getName());
    for (llvm::BasicBlock &BB : F) {
      printf("!!! basic block = %s\n", BB.getName().data());
      for (auto it = BB.begin(), end = BB.end(); it != end;) {
        llvm::Instruction *I = &*(it++);
        printf("!!! opcode = %s\n", I->getOpcodeName());
      }
    }
  }

  
/* GOAL: generate this spirv (from
  cloud:~/code/amber/tests/cases/draw_triangle_list_spv.amber) SHADER fragment
  frag_shader SPIRV-ASM TARGET_ENV spv1.0
  ; SPIR-V
  ; Version: 1.0
  ; Generator: Google spiregg; 0
  ; Bound: 12
  ; Schema: 0
  OpCapability Shader
  OpMemoryModel Logical GLSL450
  OpEntryPoint Fragment %psmain "psmain" %in_var_COLOR
  %out_var_SV_TARGET
  OpExecutionMode %psmain OriginUpperLeft
  OpSource HLSL 600
                 OpName %in_var_COLOR "in.var.COLOR"
                 OpName %out_var_SV_TARGET "out.var.SV_TARGET"
                 OpName %psmain "psmain"
                 OpDecorate %in_var_COLOR Location 0
                 OpDecorate %out_var_SV_TARGET Location 0
        %float = OpTypeFloat 32
      %v4float = OpTypeVector %float 4
  %_ptr_Input_v4float = OpTypePointer Input %v4float
  %_ptr_Output_v4float = OpTypePointer Output %v4float
         %void = OpTypeVoid
            %9 = OpTypeFunction %void
  %in_var_COLOR = OpVariable %_ptr_Input_v4float Input
  %out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
       %psmain = OpFunction %void None %9
           %10 = OpLabel
           %11 = OpLoad %v4float %in_var_COLOR
                 OpStore %out_var_SV_TARGET %11
                 OpReturn
                 OpFunctionEnd
  END
  from this hlsl (from
  cloud:~/code/amber/tests/cases/draw_triangle_list_hlsl.amber) SHADER fragment
  frag_shader HLSL float4 main(float4 color : COLOR) : SV_TARGET { return color;
  }
  END
  */

  // first do it manually
  clang::spirv::SpirvModule module;
  clang::QualType type;
  clang::SourceLocation loc;
  clang::spirv::SpirvFunction* fn =
      new clang::spirv::SpirvFunction(type, loc, "psmain");
  // clang::spirv::SpirvEntryPoint *entryPt = new clang::spirv::SpirvEntryPoint(
  //     loc, execmodel, entryPt, "entrypt", new llvm::ArrayRef());
  // module.addEntryPoint(entryPt);
  clang::spirv::SpirvBasicBlock* bb = new clang::spirv::SpirvBasicBlock("block");
  /*clang::spirv::SpirvInstruction instruction(kind, opcode, resulttype, loc);
  bb.addFirstInstruction(instruction);*/
  fn->addBasicBlock(bb);

  //clang::spirv::SpirvEntryPoint entryPoint();
  //module.addEntryPoint(entryPoint);

   clang::spirv::SpirvContext spvContext;
  clang::spirv::SpirvBuilder2 spvBuilder2(spvContext,
                                          clang::spirv::SpirvCodeGenOptions());
   clang::spirv::SpirvType spvType = clang::spirv::IntegerType(32, false);
   spvBuilder2.setMemoryModel(spv::AddressingModel::Logical,
                              spv::MemoryModel::GLSL450);
   llvm::ArrayRef<clang::spirv::SpirvVariable *> interfaces;
   spvBuilder2.addEntryPoint(
       spv::ExecutionModel::Fragment,
       fn, "psmain",
       interfaces);
   // Output the constructed module.
   std::vector<uint32_t> m = spvBuilder2.takeModule();
   //const char *output[m.size() * 4];
   printf(reinterpret_cast<const char *>(m.data()));
   spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
   std::string output;
   bool success = tools.Disassemble(m.data(), m.size(), &output);
   if (success) {
     printf(" !!! START disassemble\n");
     printf(output.c_str());
     printf("\n!!! END disassemble\n");
   } else {
     printf(" !!! FAILED disassemble\n");
   }

  return S_OK;
}

int __cdecl wmain(int argc, const wchar_t **argv_) {
  const char *pStage = "Operation";
  int retVal = 0;
  if (llvm::sys::fs::SetupPerThreadFileSystem())
    return 1;
  llvm::sys::fs::AutoCleanupPerThreadFileSystem auto_cleanup_fs;
  try {
    // Parse command line options.
    pStage = "Argument processing";

    ProgramAction action = ProgramAction::PrintHelp;
    LPCWSTR inFileName = nullptr;
    LPCWSTR outFileName = nullptr;
    LPCWSTR externalLib = nullptr;
    LPCWSTR externalFn = nullptr;
    LPCWSTR passFileName = nullptr;
    const wchar_t **optArgs = nullptr;
    UINT32 optArgCount = 0;

    int argIdx = 1;
    while (argIdx < argc) {
      LPCWSTR arg = argv_[argIdx];
      if (wcsieqopt(arg, L"?")) {
        action = ProgramAction::PrintHelp;
      }
      else if (wcsieqopt(arg, L"external")) {
        ++argIdx;
        if (argIdx == argc) {
          PrintHelp();
          return 1;
        }
        externalLib = argv_[argIdx];
      }
      else if (wcsieqopt(arg, L"external-fn")) {
        ++argIdx;
        if (argIdx == argc) {
          PrintHelp();
          return 1;
        }
        externalFn = argv_[argIdx];
      }
      else if (wcsieqopt(arg, L"pf")) {
        ++argIdx;
        if (argIdx == argc) {
          PrintHelp();
          return 1;
        }
        passFileName = argv_[argIdx];
      }
      else if (wcsistarts(arg, L"-o=")) {
        outFileName = argv_[argIdx] + 3;
      }
      else {
        action = ProgramAction::RunOptimizer;
        // See if arg is file input specifier.
        if (isFileInputArg(arg)) {
          inFileName = arg;
          argIdx++;
        }
        // No filename argument give so read from stdin.
        else {
          inFileName = STDIN_FILE_NAME;
        }

        // The remaining arguments are optimizer args.
        optArgs = argv_ + argIdx;
        optArgCount = argc - argIdx;
        break;
      }
      ++argIdx;
    }

    if (action == ProgramAction::PrintHelp) {
      PrintHelp();
      return retVal;
    }

    if (passFileName && optArgCount) {
      wprintf(L"%s", L"Cannot specify both command-line options and an pass option file.\n");
      return 1;
    }

    if (externalLib) {
      CW2A externalFnA(externalFn, CP_UTF8);
      IFT(g_DxcSupport.InitializeForDll(externalLib, externalFnA));
    }
    else {
      IFT(g_DxcSupport.Initialize());
    }

    CComPtr<IDxcBlob> pBlob;
    CComPtr<IDxcBlob> pOutputModule;
    CComPtr<IDxcBlobEncoding> pOutputText;
    // CComPtr<IDxcSpirvTranslator> pTranslator;
    CComPtr<IDxcBlobEncoding> pPassOpts;
    std::vector<LPCWSTR> passes;
    //IFT(g_DxcSupport.CreateInstance(CLSID_DxcOptimizer, &pTranslator));
    switch (action) {
    case ProgramAction::RunOptimizer:
      pStage = "Optimizer processing";
      BlobFromFile(inFileName, &pBlob);
      ReadFileOpts(passFileName, &pPassOpts, passes, &optArgs, &optArgCount);
      //llvm::Module* module;
      IFT(RunTranslator(pBlob, optArgs, optArgCount, &pOutputModule, &pOutputText));
      //SpirvGen(module);
      PrintOptOutput(outFileName, pOutputModule, pOutputText);
      break;
    }
  } catch (const ::hlsl::Exception &hlslException) {
    try {
      const char *msg = hlslException.what();
      Unicode::acp_char printBuffer[128]; // printBuffer is safe to treat as
                                          // UTF-8 because we use ASCII only errors
      if (msg == nullptr || *msg == '\0') {
        sprintf_s(printBuffer, _countof(printBuffer),
                  "Operation failed - error code 0x%08x.\n", hlslException.hr);
        msg = printBuffer;
      }

      dxc::WriteUtf8ToConsoleSizeT(msg, strlen(msg));
      printf("\n");
    } catch (...) {
      printf("%s failed - unable to retrieve error message.\n", pStage);
    }

    return 1;
  } catch (std::bad_alloc &) {
    printf("%s failed - out of memory.\n", pStage);
    return 1;
  } catch (...) {
    printf("%s failed - unknown error.\n", pStage);
    return 1;
  }

  return retVal;
}

clang::QualType llvmTypeToClangType(llvm::Type type) {
  clang::QualType output;
  return output;

}
