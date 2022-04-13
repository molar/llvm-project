set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_library(
	fmt-migrator
	FmtMigrator.cpp
	FmtMigrator.hpp
	)

target_link_libraries(
	fmt-migrator
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  clangToolingRefactoring
	)

add_clang_executable(fmt-migrate
	FmtMigrate.cpp
	FmtMigrator.cpp
  )

clang_target_link_libraries(fmt-migrate
	PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  clangToolingRefactoring
  )

