//===-- OR1KAsmParser.cpp - Parse OR1K assembly to MCInst instructions ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/STLExtras.h"
#include "MCTargetDesc/OR1KMCTargetDesc.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

namespace {
struct OR1KOperand;

class OR1KAsmParser : public MCTargetAsmParser {
  MCAsmParser &Parser;
  MCAsmParser &getParser() const { return Parser; }
  MCAsmLexer &getLexer() const { return Parser.getLexer(); }
  MCSubtargetInfo &STI;

  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               SmallVectorImpl<MCParsedAsmOperand*> &Operands,
                               MCStreamer &Out, unsigned &ErrorInfo,
                               bool matchingInlineAsm);

  bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc);

  OR1KOperand *ParseRegister(unsigned &RegNo);

  OR1KOperand *ParseImmediate();

  MCSymbolRefExpr::VariantKind getVariantKind(StringRef Symbol);
  const MCExpr *evaluateRelocExpr(const MCExpr *Expr, MCSymbolRefExpr::VariantKind VK);
  OR1KOperand *ParseSymbolOrRelocExpr();

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc,
                        SmallVectorImpl<MCParsedAsmOperand*> &Operands);

  bool ParseDirective(AsmToken DirectiveID);

  bool ParseOperand(SmallVectorImpl<MCParsedAsmOperand*> &Operands);

  // Auto-generated instruction matching functions
#define GET_ASSEMBLER_HEADER
#include "OR1KGenAsmMatcher.inc"

public:
  OR1KAsmParser(MCSubtargetInfo &sti, MCAsmParser &_Parser,
                const MCInstrInfo &MII)
    : MCTargetAsmParser(), Parser(_Parser), STI(sti) {
      setAvailableFeatures(ComputeAvailableFeatures(STI.getFeatureBits()));
  }

};

/// OR1KOperand - Instances of this class represented a parsed machine
/// instruction
struct OR1KOperand : public MCParsedAsmOperand {

  enum KindTy {
    Token,
    Register,
    Immediate,
    Memory
  } Kind;

  SMLoc StartLoc, EndLoc;

  struct Token {
    const char *Data;
    unsigned Length;
  };

  struct RegOp {
    unsigned RegNum;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  struct MemOp {
    unsigned BaseReg;
    const MCExpr *Off;
  };

  union {
    struct Token Tok;
    struct RegOp Reg;
    struct ImmOp Imm;
    struct MemOp Mem;
  };

  OR1KOperand(KindTy K) : MCParsedAsmOperand(), Kind(K) {}
public:
  OR1KOperand(const OR1KOperand &o) : MCParsedAsmOperand() {
    Kind = o.Kind;
    StartLoc = o.StartLoc;
    EndLoc = o.EndLoc;
    switch (Kind) {
      case Register:
        Reg = o.Reg;
        break;
      case Immediate:
        Imm = o.Imm;
        break;
      case Token:
        Tok = o.Tok;
        break;
      case Memory:
        Mem = o.Mem;
        break;
    }
  }

  /// getStartLoc - Gets location of the first token of this operand
  SMLoc getStartLoc() const { return StartLoc; }

  /// getEndLoc - Gets location of the last token of this operand
  SMLoc getEndLoc() const { return EndLoc; }

  unsigned getReg() const {
    assert(Kind == Register && "Invalid type access!");
    return Reg.RegNum;
  }

  const MCExpr *getImm() const {
    assert (Kind == Immediate && "Invalid type access!");
    return Imm.Val;
  }

  StringRef getToken() const {
    assert (Kind == Token && "Invalid type access!");
    return StringRef(Tok.Data, Tok.Length);
  }

  // Functions for testing operand type
  bool isReg() const { return Kind == Register; }
  bool isImm() const { return Kind == Immediate; }
  bool isToken() const { return Kind == Token; }
  bool isMem() const { return Kind == Memory; }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    // Add as immediates where possible. Null MCExpr = 0
    if (Expr == 0)
      Inst.addOperand(MCOperand::CreateImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::CreateImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::CreateExpr(Expr));
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::CreateReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  // FIXME: Implement this
  void print(raw_ostream &OS) const {}


  static OR1KOperand *CreateToken(StringRef Str, SMLoc S) {
    OR1KOperand *Op = new OR1KOperand(Token);
    Op->Tok.Data = Str.data();
    Op->Tok.Length = Str.size();
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static OR1KOperand *CreateReg(unsigned RegNo, SMLoc S, SMLoc E) {
    OR1KOperand *Op = new OR1KOperand(Register);
    Op->Reg.RegNum = RegNo;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static OR1KOperand *CreateImm(const MCExpr *Val, SMLoc S, SMLoc E) {
    OR1KOperand *Op = new OR1KOperand(Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }
};
} // end anonymous namespace.

// Auto-generated by TableGen
static unsigned MatchRegisterName(StringRef Name);

bool OR1KAsmParser::
MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                        SmallVectorImpl<MCParsedAsmOperand*> &Operands,
                        MCStreamer &Out, unsigned &ErrorInfo,
                        bool matchingInlineAsm) {
  MCInst Inst;
  SMLoc ErrorLoc;

  switch (MatchInstructionImpl(Operands, Inst, ErrorInfo, matchingInlineAsm)) {
    default: break;
    case Match_Success:
      Out.EmitInstruction(Inst, STI);
      return false;
    case Match_MissingFeature:
      return Error(IDLoc, "Instruction use requires option to be enabled");
    case Match_MnemonicFail:
      return Error(IDLoc, "Unrecognized instruction mnemonic");
    case Match_InvalidOperand:
      ErrorLoc = IDLoc;
      if (ErrorInfo != ~0U) {
        if (ErrorInfo >= Operands.size())
          return Error(IDLoc, "Too few operands for instruction");

        ErrorLoc = ((OR1KOperand*)Operands[ErrorInfo])->getStartLoc();
        if (ErrorLoc == SMLoc())
          ErrorLoc = IDLoc;
      }
      return Error(IDLoc, "Invalid operand for instruction");
  }

  llvm_unreachable("Unknown match type detected!");
}

bool OR1KAsmParser::
ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) {
  return (ParseRegister(RegNo) == 0);
}

OR1KOperand *OR1KAsmParser::ParseRegister(unsigned &RegNo) {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() -1);

  switch(getLexer().getKind()) {
    default: return 0;
    case AsmToken::Identifier:
      RegNo = MatchRegisterName(getLexer().getTok().getIdentifier());
      if (RegNo == 0)
        return 0;
      getLexer().Lex();
      return OR1KOperand::CreateReg(RegNo, S, E);
  }
  return 0;
}

OR1KOperand *OR1KAsmParser::ParseImmediate() {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() -1);

  const MCExpr *EVal;
  switch(getLexer().getKind()) {
    default: return 0;
    case AsmToken::LParen:
    case AsmToken::Plus:
    case AsmToken::Minus:
    case AsmToken::Integer:
      if(getParser().parseExpression(EVal))
        return 0;

      int64_t ans;
      EVal->EvaluateAsAbsolute(ans);
      return OR1KOperand::CreateImm(EVal, S, E);
  }
}

MCSymbolRefExpr::VariantKind OR1KAsmParser::getVariantKind(StringRef Symbol) {
  MCSymbolRefExpr::VariantKind VK =
      StringSwitch<MCSymbolRefExpr::VariantKind>(Symbol)
          .Case("hi", MCSymbolRefExpr::VK_OR1K_ABS_HI)
          .Case("lo", MCSymbolRefExpr::VK_OR1K_ABS_LO)
          .Case("plt", MCSymbolRefExpr::VK_OR1K_PLT)
          .Case("got", MCSymbolRefExpr::VK_OR1K_GOT)
          .Case("gotpchi", MCSymbolRefExpr::VK_OR1K_GOTPCHI)
          .Case("gotpclo", MCSymbolRefExpr::VK_OR1K_GOTPCLO)
          .Case("gotoffhi", MCSymbolRefExpr::VK_OR1K_GOTOFFHI)
          .Case("gotofflo", MCSymbolRefExpr::VK_OR1K_GOTOFFLO)
          .Default(MCSymbolRefExpr::VK_None);

  return VK;
}

const MCExpr *OR1KAsmParser::evaluateRelocExpr(const MCExpr *Expr,
                                               MCSymbolRefExpr::VariantKind VK) {
  // Check the type of the expression.
  if (const MCConstantExpr *MCE = dyn_cast<MCConstantExpr>(Expr)) {
    // It's a constant, evaluate lo or hi value.
    if (VK == MCSymbolRefExpr::VK_OR1K_ABS_LO) {
      short Val = MCE->getValue();
      return MCConstantExpr::Create(Val, getContext());
    } else if (MCSymbolRefExpr::VK_OR1K_ABS_HI) {
      int Val = MCE->getValue();
      int LoSign = Val & 0x8000;
      Val = (Val & 0xffff0000) >> 16;
      // Lower part is treated as a signed int, so if it is negative
      // we must add 1 to the hi part to compensate.
      if (LoSign)
        Val++;
      return MCConstantExpr::Create(Val, getContext());
    } else {
      return 0;
    }
  }

  if (const MCSymbolRefExpr *MSRE = dyn_cast<MCSymbolRefExpr>(Expr)) {
    // It's a symbol, create a symbolic expression from the symbol.
    StringRef Symbol = MSRE->getSymbol().getName();
    return MCSymbolRefExpr::Create(Symbol, VK, getContext());
  }

  if (const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(Expr)) {
    // It's a binary expression, map operands.
    const MCExpr *LExp = evaluateRelocExpr(BE->getLHS(), VK);
    const MCExpr *RExp = evaluateRelocExpr(BE->getRHS(), VK);
    return MCBinaryExpr::Create(BE->getOpcode(), LExp, RExp, getContext());
  }

  if (const MCUnaryExpr *UN = dyn_cast<MCUnaryExpr>(Expr)) {
    // It's an unary expression, map operand.
    const MCExpr *UnExp = evaluateRelocExpr(UN->getSubExpr(), VK);
    return MCUnaryExpr::Create(UN->getOpcode(), UnExp, getContext());
  }

  return 0;
}

OR1KOperand *OR1KAsmParser::ParseSymbolOrRelocExpr() {
  SMLoc S = Parser.getTok().getLoc();
  StringRef Identifier;
  if (Parser.parseIdentifier(Identifier))
    return 0;
  SMLoc E =
      SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

  MCSymbolRefExpr::VariantKind VK = getVariantKind(Identifier);
  if(VK == MCSymbolRefExpr::VK_None) {
    // Create a symbol reference.
    MCSymbol *Sym = getContext().GetOrCreateSymbol(Identifier);
    const MCExpr *Res =
        MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_None, getContext());
    return OR1KOperand::CreateImm(Res, S, E);
  } else {
    // Parse a relocation expression.
    SMLoc ExprS = Parser.getTok().getLoc();
    if(getLexer().isNot(AsmToken::LParen)) {
      Error(Parser.getTok().getLoc(), "Expected a parenthesized expression");
      return 0;
    }
    getLexer().Lex();

    const MCExpr *EVal;
    if(getParser().parseExpression(EVal))
      return 0;

    SMLoc ExprE = Parser.getTok().getLoc();
    if(getLexer().isNot(AsmToken::RParen)) {
      Error(E, "Expected a closing parenthesis");
      return 0;
    }
    getLexer().Lex();

    const MCExpr *Res = evaluateRelocExpr(EVal, VK);
    if(!Res) {
      Error(ExprS, "Unsupported relocation expression");
      return 0;
    }

    return OR1KOperand::CreateImm(Res, S, ExprE);
  }
}

/// Looks at a token type and creates the relevant operand
/// from this information, adding to Operands.
/// If operand was parsed, returns false, else true.
/// FIXME: Replace with memory operand parsing?
bool OR1KAsmParser::
ParseOperand(SmallVectorImpl<MCParsedAsmOperand*> &Operands) {
  OR1KOperand *Op, *Op2 = 0;

  // Attempt to parse token as register
  unsigned RegNo;
  Op = ParseRegister(RegNo);

  // Attempt to parse token as immediate
  if(!Op) {
    Op = ParseImmediate();

    // If next token is left parenthesis, then memory operand, attempt to
    // parse next token as base of
    // FIXME: There should be a better way of doing this.
    if(Op) {
      if(getLexer().is(AsmToken::LParen)) {
        getLexer().Lex();
        // Swap tokens around so that they can be parsed
        Op2 = Op;
        Op = ParseRegister(RegNo);

        // Invalid memory operand, fail
        if(!Op || getLexer().isNot(AsmToken::RParen)) {
          Error(Parser.getTok().getLoc(), "Unsupported operand");
          return true;
        }
        getLexer().Lex();
      }
    } else {
      // Attempt to parse token as symbol
      Op = ParseSymbolOrRelocExpr();
      if(!Op) {
        return true;
      }
    }
  }

  // If the token could not be parsed then fail
  if(!Op) {
    Error(Parser.getTok().getLoc(), "Unsupported operand");
    return true;
  }

  // Push back parsed operand(s) into list of operands
  Operands.push_back(Op);
  if(Op2)
    Operands.push_back(Op2);

  return false;
}

bool OR1KAsmParser::
ParseInstruction(ParseInstructionInfo &Info, StringRef Name, SMLoc NameLoc,
                 SmallVectorImpl<MCParsedAsmOperand*> &Operands) {
  // First operand is token for instruction
  // FIXME: Can we have a more efficient implementation of this?
  size_t dotLoc = Name.find('.');
  Operands.push_back(OR1KOperand::CreateToken(Name.substr(0,dotLoc),NameLoc));
  if (dotLoc < Name.size()) {
    size_t dotLoc2 = Name.rfind('.');
    if (dotLoc == dotLoc2)
      Operands.push_back(OR1KOperand::CreateToken(Name.substr(dotLoc),NameLoc));
    else {
      Operands.push_back(OR1KOperand::CreateToken(Name.substr
                                        (dotLoc, dotLoc2-dotLoc), NameLoc));
      Operands.push_back(OR1KOperand::CreateToken(Name.substr
                                        (dotLoc2), NameLoc));
    }
  }

  // If there are no more operands, then finish
  if (getLexer().is(AsmToken::EndOfStatement))
    return false;

  // Parse first operand
  if (ParseOperand(Operands))
    return true;

  // Parse until end of statement, consuming commas between operands
  while (getLexer().isNot(AsmToken::EndOfStatement) &&
        getLexer().is(AsmToken::Comma)) {
    // Consume comma token
    getLexer().Lex();

    // Parse next operand
    if(ParseOperand(Operands))
      return true;
  }

  return false;
}

bool OR1KAsmParser::
ParseDirective(AsmToken DirectiveID) {
  return true;
}

extern "C" void LLVMInitializeOR1KAsmParser() {
  RegisterMCAsmParser<OR1KAsmParser> X(TheOR1KTarget);
}

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "OR1KGenAsmMatcher.inc"
