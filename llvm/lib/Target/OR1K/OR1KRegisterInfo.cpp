//===-- OR1KRegisterInfo.cpp - OR1K Register Information --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the OR1K implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "OR1K.h"
#include "OR1KRegisterInfo.h"
#include "OR1KSubtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "OR1KGenRegisterInfo.inc"
using namespace llvm;

static_assert(OR1K::R0 + 1 == OR1K::R1, "Register list not consecutive.");
static_assert(OR1K::R0 + 31 == OR1K::R31, "Register list not consecutive.");

OR1KRegisterInfo::OR1KRegisterInfo(const TargetInstrInfo &tii)
  : OR1KGenRegisterInfo(OR1K::R9), TII(tii) {
}

const uint16_t*
OR1KRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  const TargetFrameLowering *TFI = getFrameLowering(*MF);

  return TFI->hasFP(*MF) ? CSR_FP_SaveList : CSR_SaveList;
}

BitVector OR1KRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const TargetFrameLowering *TFI = getFrameLowering(MF);

  Reserved.set(OR1K::R0);             // Zero register
  Reserved.set(OR1K::R1);             // Stack pointer
  if (TFI->hasFP(MF))
    Reserved.set(OR1K::R2);           // Frame pointer
  Reserved.set(getRARegister());      // Return address pointer
  Reserved.set(OR1K::R10);            // Thread-local pointer
  Reserved.set(OR1K::R16);            // Global pointer
  if (hasBasePointer(MF))
    Reserved.set(getBaseRegister());  // Base pointer
  return Reserved;
}

bool
OR1KRegisterInfo::requiresRegisterScavenging(const MachineFunction &MF) const {
  return true;
}

void
OR1KRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                      int SPAdj, unsigned FIOperandNum,
                                      RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetFrameLowering *TFI = getFrameLowering(MF);
  bool HasFP = TFI->hasFP(MF);
  DebugLoc dl = MI.getDebugLoc();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();

  int Offset = MFI.getObjectOffset(FrameIndex) +
               MI.getOperand(FIOperandNum+1).getImm();

  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();
  int MinCSFI = 0;
  int MaxCSFI = -1;

  if (CSI.size()) {
    MinCSFI = CSI[0].getFrameIdx();
    MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
  }

  unsigned FrameReg;
  if ((FrameIndex >= MinCSFI && FrameIndex <= MaxCSFI) &&
      needsStackRealignment(MF)) {
    // When stack is realigned, callee-saved registers are always addressed
    // using the frame pointer to be consistent with CFI, since CFA cannot
    // be easily realigned.
    FrameReg = getFrameRegister(MF);
  } else {
    // Addressable stack objects are addressed using neg. offsets from fp
    // or pos. offsets from sp/basepointer
    if (!HasFP || (needsStackRealignment(MF) && FrameIndex >= 0))
      Offset += MFI.getStackSize();

    FrameReg = getFrameRegister(MF);
    if (FrameIndex >= 0) {
      if (hasBasePointer(MF))
        FrameReg = getBaseRegister();
      else if (needsStackRealignment(MF))
        FrameReg = OR1K::R1;
    }
  }

  // Replace frame index with a frame pointer reference.
  // If the offset is small enough to fit in the immediate field, directly
  // encode it.
  // Otherwise scavenge a register and encode it in to a MOVHI - ORI sequence
  if (!isInt<16>(Offset)) {
    assert(RS && "Register scavenging must be on");
    unsigned Reg = RS->FindUnusedReg(&OR1K::GPRRegClass);
    if (!Reg)
       Reg = RS->scavengeRegister(&OR1K::GPRRegClass, II, SPAdj);
    assert(Reg && "Register scavenger failed");

    // Reg = hi(offset) | lo(offset)
    BuildMI(*MI.getParent(), II, dl, TII.get(OR1K::MOVHI), Reg)
      .addImm((uint32_t)Offset >> 16);
    BuildMI(*MI.getParent(), II, dl, TII.get(OR1K::ORI), Reg)
      .addReg(Reg).addImm(Offset & 0xffffU);
    // Reg = Reg + Sp
    BuildMI(*MI.getParent(), II, dl, TII.get(OR1K::ADD), Reg)
      .addReg(Reg).addReg(FrameReg);

    MI.getOperand(FIOperandNum).ChangeToRegister(Reg,
                                                 /*isDef=*/false,
                                                 /*isImp=*/false,
                                                 /*isKill=*/true);
    MI.getOperand(FIOperandNum+1).ChangeToImmediate(0);

    return;
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, /*isDef=*/false);
  MI.getOperand(FIOperandNum+1).ChangeToImmediate(Offset);
}

bool OR1KRegisterInfo::hasBasePointer(const MachineFunction &MF) const {
   const MachineFrameInfo &MFI = MF.getFrameInfo();
   // When we need stack realignment and there are dynamic allocas, we can't
   // reference off of the stack pointer, so we reserve a base pointer.
   if (needsStackRealignment(MF) && MFI.hasVarSizedObjects())
     return true;

   return false;
}

unsigned OR1KRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = getFrameLowering(MF);

  return TFI->hasFP(MF) ? OR1K::R2 : OR1K::R1;
}

unsigned OR1KRegisterInfo::getBaseRegister() const {
  return OR1K::R14;
}
