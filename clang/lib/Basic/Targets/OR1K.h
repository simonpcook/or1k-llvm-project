//===--- OR1K.h - Declare OR1K target feature support -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares OR1K TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_OR1K_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_OR1K_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY OR1KTargetInfo : public TargetInfo {
  static const char *const GCCRegNames[];

public:
  OR1KTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    LongLongAlign = DoubleAlign = LongDoubleAlign = SuitableAlign = 32;
    resetDataLayout("E-m:e-p:32:32-i8:8:8-i16:16:16-i64:32:32-"
                    "f64:32:32-v64:32:32-v128:32:32-a0:0:32-n32");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
  ArrayRef<Builtin::Info> getTargetBuiltins() const override {
    return None;
  }

  bool hasFeature(StringRef Feature) const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  const char *getClobbers() const override { return ""; }
  ArrayRef<const char *> getGCCRegNames() const override;
  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return None;
  }
  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override;

  int getEHDataRegisterNumber(unsigned RegNo) const override;
};

} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_LE64_H
