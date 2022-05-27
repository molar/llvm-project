// RUN: %check_clang_tidy  %s modernize-RosStreamToFmt %t

class StringStream {
public:
  StringStream(){};

  StringStream &operator<<(const char *p) { return *this; };
  StringStream &operator<<(int p) { return *this; };
};
#define ROS_INFO_STREAM(args) \
  do {                        \
    StringStream ss;          \
    ss << args;               \
  } while (0);

int func2(int a, int b) {
  return a + b;
}
void fun() {

  int Var = 0;
  ROS_INFO_STREAM("hello _world " << 123 << " jhkl " << Var);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: Rewrite to use format style instead [modernize-RosStreamToFmt]
  // CHECK-FIXES: fmt::format("hello _world 123 jhkl {}",Var);

  ROS_INFO_STREAM("hello _world "
                  << 123 << " jhkl " << Var);
  // CHECK-MESSAGES: :[[@LINE-2]]:3: warning: Rewrite to use format style instead [modernize-RosStreamToFmt]
  // CHECK-FIXES: fmt::format("hello _world 123 jhkl {}",Var);

  ROS_INFO_STREAM("hello _world " << func2(1, 123) << " jhkl " << Var);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: Rewrite to use format style instead [modernize-RosStreamToFmt]
  // CHECK-FIXES: fmt::format("hello _world {} jhkl {}",func2(1, 123),Var);
}
