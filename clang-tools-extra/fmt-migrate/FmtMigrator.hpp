#ifndef CLANG_TOOLS_EXTRA__FMT_MIGRATE__FMT_MIGRATOR_HPP_
#define CLANG_TOOLS_EXTRA__FMT_MIGRATE__FMT_MIGRATOR_HPP_
#include "clang/ASTMatchers/ASTMatchers.h"
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

} // namespace migrate
} // namespace fmt
} // namespace clang
#endif
