#ifndef CLANG_TOOLS_EXTRA__FMT_MIGRATE__FMT_MIGRATOR_HPP_
#define CLANG_TOOLS_EXTRA__FMT_MIGRATE__FMT_MIGRATOR_HPP_
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/Transformer/RangeSelector.h"
#include "clang/Tooling/Transformer/RewriteRule.h"
#include "clang/Tooling/Transformer/Transformer.h"
#include <sstream>
namespace clang {
namespace fmt {
namespace migrate {

inline auto getRosLoggers() {
  return clang::ast_matchers::anyOf(
      clang::ast_matchers::isExpandedFromMacro("ROS_DEBUG_STREAM"),
      clang::ast_matchers::isExpandedFromMacro("ROS_INFO_STREAM"),
      clang::ast_matchers::isExpandedFromMacro("ROS_WARN_STREAM"),
      clang::ast_matchers::isExpandedFromMacro("ROS_ERROR_STREAM"),
      clang::ast_matchers::isExpandedFromMacro("ROS_FATAL_STREAM"));
}

template <typename T> inline auto getLogExpression(T Matchers) {
	using namespace clang::ast_matchers;
  return compoundStmt(
			Matchers, hasDescendant(
				cxxOperatorCallExpr(hasOperatorName("<<")).bind("logcode")),
			isExpansionInMainFile()
		);
}


class FormatStringBuilder {
public:
  FormatStringBuilder(clang::SourceManager &Sm) : Sm(Sm) {}
  void addStringLiteral(const clang::StringLiteral &Sl) {
    auto Str = Sl.getString();
    FmtStringComponents.push_back(Str.str());
  }

  void addFormatExpr(const clang::Expr &Ex) {
    // copy from start to end loc
    FmtStringComponents.push_back("{}");
    auto Range = Ex.getSourceRange();

    clang::LangOptions Lo;
    // get the text from lexer including newlines and other formatting?

    auto StartLoc = Sm.getSpellingLoc(Range.getBegin());
    auto LastTokenLoc = Sm.getSpellingLoc(Range.getEnd());
    auto EndLoc = clang::Lexer::getLocForEndOfToken(LastTokenLoc, 0, Sm, Lo);
    auto PrintableRange = clang::SourceRange{StartLoc, EndLoc};
    auto Str = clang::Lexer::getSourceText(
        clang::CharSourceRange::getCharRange(PrintableRange), Sm,
        clang::LangOptions());
    FmtArgsComponents.push_back(Str.str());
  }

  std::string getFormatString() {
    std::stringstream Ss;
    Ss << "fmt::format(\"";
    for (const auto &Fmt : FmtStringComponents) {
      Ss << Fmt;
    }
    Ss << "\"";
    for (const auto &Arg : FmtArgsComponents) {
      Ss << "," << Arg;
    }
    Ss << ")";
    return Ss.str();
  }

private:
  clang::SourceManager &Sm;
  llvm::SmallVector<std::string> FmtStringComponents;
  llvm::SmallVector<std::string> FmtArgsComponents;
};

void visitCallExpr(const clang::Expr &A0, const clang::Expr &A1,
                   FormatStringBuilder &FSB);

void visitArg(const clang::Expr &A, FormatStringBuilder &FSB) {
  if (const clang::CXXOperatorCallExpr *Oper =
          llvm::dyn_cast<const clang::CXXOperatorCallExpr>(
              &A)) // CXXOperatorCall("<<")
  {
    visitCallExpr(*Oper->getArg(0), *Oper->getArg(1), FSB);
  } else if (const clang::ImplicitCastExpr *Cast =
                 llvm::dyn_cast<const clang::ImplicitCastExpr>(&A)) {
    // check if it is a string literal or 'cout' or
    // or 'endl'
    if (const clang::StringLiteral *Lit =
            llvm::dyn_cast<const clang::StringLiteral>(Cast->getSubExpr())) {
      FSB.addStringLiteral(*Lit);
    } else if (const auto *DeclRef = llvm::dyn_cast<const clang::DeclRefExpr>(
                   Cast->getSubExpr())) {
      if (DeclRef->getNameInfo().getName().getAsString() == "endl") {
        return;
      }
      FSB.addFormatExpr(A);
    } else {
      FSB.addFormatExpr(A);
    }
  } else {
    if (const auto *DeclRef = llvm::dyn_cast<const clang::DeclRefExpr>(&A)) {
      if (DeclRef->getNameInfo().getName().getAsString() == "cout") {
        return;
      }
    }
    FSB.addFormatExpr(A);
  }
}

void visitCallExpr(const clang::Expr &A0, const clang::Expr &A1,
                   FormatStringBuilder &FSB) {
  visitArg(A0, FSB);
  visitArg(A1, FSB);
}

class LoopPrinter : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  virtual void
  run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
    if (const clang::CXXOperatorCallExpr *FS =
            Result.Nodes.getNodeAs<clang::CXXOperatorCallExpr>("logcode")) {
      const auto *Arg0 = FS->getArg(0);
      const auto *Arg1 = FS->getArg(1);
      FormatStringBuilder FSB(*Result.SourceManager);
      visitCallExpr(*Arg0, *Arg1, FSB);
      llvm::outs() << FSB.getFormatString() << "\n";
    }
  }
};
} // namespace migrate
} // namespace fmt
} // namespace clang
#endif
