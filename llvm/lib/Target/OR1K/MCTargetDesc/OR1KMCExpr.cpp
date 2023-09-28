//===-- OR1KMCExpr.cpp - OR1K specific MC expression classes --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "OR1KMCExpr.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

#define DEBUG_TYPE "ork1mcexpr"

const OR1KMCExpr *OR1KMCExpr::create(const MCExpr *Expr, VariantKind Kind,
                                     MCContext &Ctx) {
  return new (Ctx) OR1KMCExpr(Expr, Kind);
}

void OR1KMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  if (Kind != VK_OR1K_None) {
    OS << getVariantKindName(Kind) << '(';
    Expr->print(OS, MAI);
    OS << ')';
  } else
    Expr->print(OS, MAI);
}

bool OR1KMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                           const MCAsmLayout *Layout,
                                           const MCFixup *Fixup) const {

  if (!getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup))
    return false;

  if (Res.getRefKind() != MCSymbolRefExpr::VK_None)
    return false;

  if (Res.isAbsolute() && Fixup == nullptr) {
    int64_t AbsVal = Res.getConstant();
    switch (Kind) {
    case VK_OR1K_None:
      llvm_unreachable("VK_OR1K_None is invalid");
    case VK_OR1K_ABS_LO:
      AbsVal = SignExtend64<16>(AbsVal);
      break;
    case VK_OR1K_ABS_HI:
      AbsVal = SignExtend64<16>((AbsVal + 0x8000) >> 16);
      break;
    case VK_OR1K_PLT:
    case VK_OR1K_GOTPCHI:
    case VK_OR1K_GOTPCLO:
    case VK_OR1K_GOTOFFHI:
    case VK_OR1K_GOTOFFLO:
    case VK_OR1K_GOT:
      return false;
    }
    Res = MCValue::get(AbsVal);
    return true;
  }

  Res = MCValue::get(Res.getSymA(), Res.getSymB(), Res.getConstant(), Kind);

  return true;
}

void OR1KMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*Expr);
}

MCFragment *OR1KMCExpr::findAssociatedFragment() const {
  return Expr->findAssociatedFragment();
}

void OR1KMCExpr::fixELFSymbolsInTLSFixups(MCAssembler &Asm) const { }

OR1KMCExpr::VariantKind OR1KMCExpr::getVariantKindForName(StringRef Name) {
  return StringSwitch<VariantKind>(Name)
      .Case("hi", VK_OR1K_ABS_HI)
      .Case("lo", VK_OR1K_ABS_LO)
      .Case("plt", VK_OR1K_PLT)
      .Case("got", VK_OR1K_GOT)
      .Case("gotpchi", VK_OR1K_GOTPCHI)
      .Case("gotpclo", VK_OR1K_GOTPCLO)
      .Case("gotoffhi", VK_OR1K_GOTOFFHI)
      .Case("gotofflo", VK_OR1K_GOTOFFLO)
      .Default(VK_OR1K_None);
}

StringRef OR1KMCExpr::getVariantKindName(VariantKind Kind) {
  switch (Kind) {
  case VK_OR1K_None:
    return "<<none>>";
  case VK_OR1K_ABS_HI:
    return "hi";
  case VK_OR1K_ABS_LO:
    return "lo";
  case VK_OR1K_PLT:
    return "plt";
  case VK_OR1K_GOTPCHI:
    return "gotpchi";
  case VK_OR1K_GOTPCLO:
    return "gotpclo";
  case VK_OR1K_GOTOFFHI:
    return "gotoffhi";
  case VK_OR1K_GOTOFFLO:
    return "gotofflo";
  case VK_OR1K_GOT:
    return "got";
  }
}
