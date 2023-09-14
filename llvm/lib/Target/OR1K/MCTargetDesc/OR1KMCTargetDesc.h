//===-- OR1KMCTargetDesc.h - OR1K Target Descriptions -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides OR1K specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef OR1KMCTARGETDESC_H
#define OR1KMCTARGETDESC_H

#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;
class Triple;
class StringRef;
class raw_pwrite_stream;

extern Target TheOR1KTarget;

MCCodeEmitter *createOR1KMCCodeEmitter(const MCInstrInfo &MII,
                                       MCContext &Ctx);

MCAsmBackend *createOR1KAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                   const MCRegisterInfo &MRI,
                                   const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter> createOR1KELFObjectWriter(uint8_t OSABI);
} // End llvm namespace

// Defines symbolic names for OR1K registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "OR1KGenRegisterInfo.inc"

// Defines symbolic names for the OR1K instructions.
//
#define GET_INSTRINFO_ENUM
#include "OR1KGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "OR1KGenSubtargetInfo.inc"

#endif
