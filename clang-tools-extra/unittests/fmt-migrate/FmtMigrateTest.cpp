#include "FmtMigrator.hpp"
#include "clang/Testing/CommandLineArgs.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/Transformer/RangeSelector.h"
#include "clang/Tooling/Transformer/RewriteRule.h"
#include "clang/Tooling/Transformer/Stencil.h"
#include "clang/Tooling/Transformer/Transformer.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"
namespace clang {
namespace fmt {
namespace migrate {

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
		ROS_INFO_STREAM("hello _world " << 123 << " jhkl " << var);
		int j = 0;
		for(auto i : {1,2,3,4}){
		  j+=i;
			ROS_INFO_STREAM("i is L" << 
			i << " longer" );
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
} // namespace clang
