#pragma once

#include <Rcpp.h>

inline std::string
get_pb_format(const std::string& which, const std::string& filename = "") {
  Rcpp::Function fun = Rcpp::Environment::namespace_env(
      "vroom")[std::string("pb_") + which + "_format"];
  return Rcpp::as<std::string>(fun(filename));
}

inline int get_pb_width(const std::string& format) {
  Rcpp::Function fun = Rcpp::Environment::namespace_env("vroom")["pb_width"];
  return Rcpp::as<int>(fun(format));
}

template <typename T>
static size_t find_next_newline(const T& source, size_t start) {
  if (start > (source.size() - 1)) {
    return source.size() - 1;
  }

  auto begin = source.data() + start;
  auto res =
      static_cast<const char*>(memchr(begin, '\n', source.size() - start));
  if (!res) {
    return start;
  }
  return res - source.data();
}

template <typename T>
static char guess_delim(const T& source, size_t start, size_t guess_max = 5) {
  std::vector<std::string> lines;

  auto nl = find_next_newline(source, start);
  while (nl > start && guess_max > 0) {
    lines.push_back(std::string(source.data() + start, nl - start));
    start = nl + 1;
    nl = find_next_newline(source, start);
    --guess_max;
  }

  Rcpp::Function fun = Rcpp::Environment::namespace_env("vroom")["guess_delim"];
  return Rcpp::as<char>(fun(lines));
}

template <typename T> T get_option(const std::string& name, T default_value) {
  SEXP val = Rf_GetOption(Rf_install(name.c_str()), R_BaseEnv);
  if (Rf_isNull(val)) {
    return default_value;
  }

  return Rcpp::as<T>(val);
}

template <typename T> size_t skip_bom(const T& source) {
  /* Skip Unicode Byte Order Marks
     https://en.wikipedia.org/wiki/Byte_order_mark#Representations_of_byte_order_marks_by_encoding
     00 00 FE FF: UTF-32BE
     FF FE 00 00: UTF-32LE
     FE FF:       UTF-16BE
     FF FE:       UTF-16LE
     EF BB BF:    UTF-8
 */

  auto size = source.size();
  auto begin = source.data();

  switch (begin[0]) {
  // UTF-32BE
  case '\x00':
    if (size >= 4 && begin[1] == '\x00' && begin[2] == '\xFE' &&
        begin[3] == '\xFF') {
      return 4;
    }
    break;

  // UTF-8
  case '\xEF':
    if (size >= 3 && begin[1] == '\xBB' && begin[2] == '\xBF') {
      return 3;
    }
    break;

  // UTF-16BE
  case '\xfe':
    if (size >= 2 && begin[1] == '\xff') {
      return 2;
    }
    break;

  case '\xff':
    if (size >= 2 && begin[1] == '\xfe') {

      // UTF-32 LE
      if (size >= 4 && begin[2] == '\x00' && begin[3] == '\x00') {
        return 4;
      } else {
        // UTF-16 LE
        return 2;
      }
    }
    break;
  }

  return 0;
}

static bool is_blank_or_comment_line(const char* begin, char comment) {
  if (*begin == '\n') {
    return true;
  }

  while (*begin == ' ' || *begin == '\t') {
    ++begin;
  }

  if (*begin == '\n' || *begin == comment) {
    return true;
  }

  return false;
}

// This skips leading blank lines and comments (if needed)
template <typename T>
size_t find_first_line(const T& source, size_t skip, char comment) {

  auto begin = skip_bom(source);
  /* Skip skip parameters, comments and blank lines */

  while (bool should_skip =
             (begin < source.size() &&
              is_blank_or_comment_line(source.data() + begin, comment)) ||
             skip > 0) {
    begin = find_next_newline(source, begin) + 1;
    if (skip > 0) {
      --skip;
    }
  }

  return begin;
}
