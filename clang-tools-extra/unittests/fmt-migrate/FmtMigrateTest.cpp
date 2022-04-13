#include "FmtMigrator.hpp"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Testing/CommandLineArgs.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/Transformer/RangeSelector.h"
#include "clang/Tooling/Transformer/RewriteRule.h"
#include "clang/Tooling/Transformer/Stencil.h"
#include "clang/Tooling/Transformer/Transformer.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"
namespace fmt {
namespace migrate {

class FormatStringBuilder {
public:
  FormatStringBuilder(clang::SourceManager &Sm) : Sm_(Sm) {}
  void addStringLiteral(const clang::StringLiteral &Sl) {
    auto Str = Sl.getString();
    FmtStringComponents.push_back(Str.str());
  }

  void addFormatExpr(const clang::Expr &Ex) {
    // copy from start to end loc
    FmtStringComponents.push_back("{}");
    auto Loc = Ex.getExprLoc();
    auto range = Ex.getSourceRange();

    clang::LangOptions lo;
    // get the text from lexer including newlines and other formatting?

    auto start_loc = Sm_.getSpellingLoc(range.getBegin());
    auto last_token_loc = Sm_.getSpellingLoc(range.getEnd());
    auto end_loc =
        clang::Lexer::getLocForEndOfToken(last_token_loc, 0, Sm_, lo);
    auto printable_range = clang::SourceRange{start_loc, end_loc};
    auto Str = clang::Lexer::getSourceText(
        clang::CharSourceRange::getCharRange(printable_range), Sm_,
        clang::LangOptions());
    FmtArgsComponents.push_back(Str.str());
  }

  std::string GetFormatString() {
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
  clang::SourceManager &Sm_;
  llvm::SmallVector<std::string> FmtStringComponents;
  llvm::SmallVector<std::string> FmtArgsComponents;
};

void visitCallExpr(const clang::Expr &a0, const clang::Expr &a1,
                   FormatStringBuilder &FSB);

void visitArg(const clang::Expr &a, FormatStringBuilder &FSB) {
  if (const clang::CXXOperatorCallExpr *Oper =
          llvm::dyn_cast<const clang::CXXOperatorCallExpr>(
              &a)) // CXXOperatorCall("<<")
  {
    visitCallExpr(*Oper->getArg(0), *Oper->getArg(1), FSB);
  } else if (const clang::ImplicitCastExpr *Cast =
                 llvm::dyn_cast<const clang::ImplicitCastExpr>(&a)) {
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
      FSB.addFormatExpr(a);
    } else {
      FSB.addFormatExpr(a);
    }
  } else {
    if (const auto *DeclRef = llvm::dyn_cast<const clang::DeclRefExpr>(&a)) {
      if (DeclRef->getNameInfo().getName().getAsString() == "cout") {
        return;
      }
    }
    FSB.addFormatExpr(a);
  }
}

void visitCallExpr(const clang::Expr &a0, const clang::Expr &a1,
                   FormatStringBuilder &FSB) {
  visitArg(a0, FSB);
  visitArg(a1, FSB);
}

class LoopPrinter : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  virtual void
  run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
    llvm::outs() << "callback\n";
    if (const clang::CXXOperatorCallExpr *FS =
            Result.Nodes.getNodeAs<clang::CXXOperatorCallExpr>("logcode")) {
      llvm::outs() << "got log code\n";

      // FS->dumpColor();
      auto Arg0 = FS->getArg(0);
      auto Arg1 = FS->getArg(1);
      FormatStringBuilder FSB(*Result.SourceManager);
      visitCallExpr(*Arg0, *Arg1, FSB);
      llvm::outs() << FSB.GetFormatString() << "\n";
    } // FS->dumpColor();
      // ExtractFmtStringAndArgsVisitor Visitor(Result.Context);
      // FS->dumpColor();
      // for (const auto &node : FS->body()) {
      //  std::cout << "Bodu: " << node->getStmtClass() << " "
      //            << node->getStmtClassName() << std::endl;
      //}

    // find each ostream operator call expr and extract Left and Right and put
    // them into Args list Construct the CallExpr to spdlog::log() with args,
    // construct the format string while visiting
    // and add the required include
  }
};

TEST(FmtMigrateUnitests, FindsStmts) {
  std::string Input = R"cc(
	#include <iostream>
	#define ROS_INFO_STREAM(args) \
  do \
  { \
		std::cout << args << std::endl; \
  } while(false);

	void f(){
		int var = 0;
		ROS_INFO_STREAM("hello _world" << 123 << "jhkl" << var);
		int j = 0;
		for(auto i : {1,2,3,4}){
		  j+=i;
			ROS_INFO_STREAM("i is L" << i );
		}
	}
	)cc";
  auto Matcher = clang::fmt::migrate::getLogExpression(
      clang::fmt::migrate::getRosLoggers());
  clang::ast_matchers::StatementMatcher MyMatch = Matcher.bind("log");

  clang::ast_matchers::MatchFinder Finder;
  LoopPrinter Printer;
  Finder.addMatcher(MyMatch, &Printer);

  std::unique_ptr<clang::tooling::FrontendActionFactory> Factory(
      clang::tooling::newFrontendActionFactory(&Finder));

  clang::tooling::runToolOnCode(Factory->create(), Input);
}
} // namespace migrate
} // namespace fmt
