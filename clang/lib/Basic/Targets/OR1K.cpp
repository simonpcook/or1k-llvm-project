//===--- OR1K.cpp - Implement OR1K target feature support -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements OR1K TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "OR1K.h"
#include "Targets.h"
#include "clang/Basic/MacroBuilder.h"

using namespace clang;
using namespace clang::targets;

void OR1KTargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  // Target identification.
  Builder.defineMacro("__or1k__");
  Builder.defineMacro("__OR1K__");

  // Target properties.
  Builder.defineMacro("_BIG_ENDIAN_");
  Builder.defineMacro("__BIG_ENDIAN__");

  // Subtarget options.
  Builder.defineMacro("__REGISTER_PREFIX__", "");
}

bool OR1KTargetInfo::hasFeature(StringRef Feature) const {
  return Feature == "or1k";
}

const char *const OR1KTargetInfo::GCCRegNames[] = {
  "r0",   "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",
  "r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "r15",
  "r16",  "r17",  "r18",  "r19",  "r20",  "r21",  "r22",  "r23",
  "r24",  "r25",  "r26",  "r27",  "r28",  "r29",  "r30",  "r31"
};

ArrayRef<const char *> OR1KTargetInfo::getGCCRegNames() const {
  return llvm::makeArrayRef(GCCRegNames);
}

bool OR1KTargetInfo::validateAsmConstraint(const char *&Name,
                                           TargetInfo::ConstraintInfo &Info) const {
  switch (*Name) {
  default: return false;
  case 'b': // Base register
  case 'f': // Floating point register
    Info.setAllowsRegister();
    return true;
  case 'I': // signed 16 bit immediate
    Info.setRequiresImmediate(-0x8000, 0x7fff);
    return true;
  case 'J': // integer zero
    Info.setRequiresImmediate(0, 0);
    return true;
  case 'K': // unsigned 16 bit immediate
    Info.setRequiresImmediate(0, 0xffff);
    return true;
  case 'L': // immediate in the range 0 to 31
    Info.setRequiresImmediate(0, 31);
    return true;
  case 'M': // signed 32 bit immediate where lower 16 bits are 0
    Info.setRequiresImmediate(-0x80000000, 0x7fffffff);
    return true;
  case 'N': // signed 26 bit immediate
    Info.setRequiresImmediate(-0x4000000, 0x3ffffff);
    return true;
  case 'O': // integer zero
    Info.setRequiresImmediate(0, 0);
    return true;
  }
}

int OR1KTargetInfo::getEHDataRegisterNumber(unsigned RegNo) const {
  if (RegNo == 0) return 3;
  if (RegNo == 1) return 4;
  return -1;
}
