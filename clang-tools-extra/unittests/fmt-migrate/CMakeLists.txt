get_filename_component(FmtMigrateLocation
	"${CMAKE_CURRENT_SOURCE_DIR}/../../fmt-migrate/" REALPATH)
include_directories(
	${FmtMigrateLocation}
  )

add_extra_unittest(FmtMigrateUnittests
	FmtMigrateTest.cpp
  )

clang_target_link_libraries(FmtMigrateUnittests
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangDynamicASTMatchers
  clangFrontend
  clangSerialization
  clangTooling
  clangBasic
  clangToolingCore
  clangToolingRefactoring
  clangTransformer
  )
