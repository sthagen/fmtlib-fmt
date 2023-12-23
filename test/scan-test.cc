// Formatting library for C++ - scanning API test
//
// Copyright (c) 2019 - present, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.

#include "scan.h"

#include <time.h>

#include <climits>

#include "fmt/os.h"
#include "gmock/gmock.h"
#include "gtest-extra.h"

TEST(scan_test, read_text) {
  fmt::string_view s = "foo";
  auto end = fmt::scan(s, "foo");
  EXPECT_EQ(end, s.end());
  EXPECT_THROW_MSG(fmt::scan("fob", "foo"), fmt::format_error, "invalid input");
}

TEST(scan_test, read_int) {
  int n = 0;
  fmt::scan("42", "{}", n);
  EXPECT_EQ(n, 42);
  fmt::scan("-42", "{}", n);
  EXPECT_EQ(n, -42);
  EXPECT_THROW_MSG(fmt::scan(std::to_string(INT_MAX + 1u), "{}", n),
                   fmt::format_error, "number is too big");
}

TEST(scan_test, read_longlong) {
  long long n = 0;
  fmt::scan("42", "{}", n);
  EXPECT_EQ(n, 42);
  fmt::scan("-42", "{}", n);
  EXPECT_EQ(n, -42);
}

TEST(scan_test, read_uint) {
  unsigned n = 0;
  fmt::scan("42", "{}", n);
  EXPECT_EQ(n, 42);
  EXPECT_THROW_MSG(fmt::scan("-42", "{}", n), fmt::format_error,
                   "invalid input");
}

TEST(scan_test, read_ulonglong) {
  unsigned long long n = 0;
  fmt::scan("42", "{}", n);
  EXPECT_EQ(n, 42);
  EXPECT_THROW_MSG(fmt::scan("-42", "{}", n), fmt::format_error,
                   "invalid input");
}

TEST(scan_test, read_string) {
  std::string s;
  fmt::scan("foo", "{}", s);
  EXPECT_EQ(s, "foo");
}

TEST(scan_test, read_string_view) {
  fmt::string_view s;
  fmt::scan("foo", "{}", s);
  EXPECT_EQ(s, "foo");
}

TEST(scan_test, separator) {
  int n1 = 0, n2 = 0;
  fmt::scan("10 20", "{} {}", n1, n2);
  EXPECT_EQ(n1, 10);
  EXPECT_EQ(n2, 20);
}

#ifdef FMT_HAVE_STRPTIME
struct num {
  int value;
};

namespace fmt {
template <> struct scanner<num> {
  bool hex = false;

  auto parse(scan_parse_context& ctx) -> scan_parse_context::iterator {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && *it == 'x') hex = true;
    if (it != end && *it != '}') throw_format_error("invalid format");
    return it;
  }

  template <class ScanContext>
  auto scan(num&, ScanContext& ctx) const -> typename ScanContext::iterator {
    // TODO
    //return fmt::scan({ctx.begin(), ctx.end()}, "{}", n.value);
    return ctx.begin();
  }
};
}  // namespace fmt

TEST(scan_test, read_custom) {
  auto input = "42";
  auto n = num();
  fmt::scan(input, "{:}", n);
  //EXPECT_EQ(n, 42);
}
#endif

TEST(scan_test, invalid_format) {
  EXPECT_THROW_MSG(fmt::scan("", "{}"), fmt::format_error,
                   "argument index out of range");
  EXPECT_THROW_MSG(fmt::scan("", "{"), fmt::format_error,
                   "invalid format string");
}

TEST(scan_test, example) {
  std::string key;
  int value = 0;
  fmt::scan("answer = 42", "{} = {}", key, value);
  EXPECT_EQ(key, "answer");
  EXPECT_EQ(value, 42);
}

#if FMT_USE_FCNTL
TEST(scan_test, file) {
  fmt::file read_end, write_end;
  fmt::file::pipe(read_end, write_end);
  fmt::string_view input = "42";
  write_end.write(input.data(), input.size());
  write_end.close();
  int value = 0;
  fmt::scan(read_end.fdopen("r").get(), "{}", value);
  EXPECT_EQ(value, 42);
}
#endif  // FMT_USE_FCNTL
