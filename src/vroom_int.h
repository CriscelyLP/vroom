#pragma once

#include "altrep.h"

#include "vroom_vec.h"

#include <Rcpp.h>

// A version of strtoi that doesn't need null terminated strings, to avoid
// needing to copy the data
int strtoi(const char* begin, const char* end) {
  int val = 0;
  bool is_neg = false;

  if (begin != end && *begin == '-') {
    is_neg = true;
    ++begin;
  }

  while (begin != end && isdigit(*begin)) {
    val = val * 10 + ((*begin++) - '0');
  }

  return is_neg ? -val : val;
}

// Normal reading of integer vectors
Rcpp::IntegerVector read_int(vroom_vec_info* info) {

  R_xlen_t n = info->column.size();

  Rcpp::IntegerVector out(n);

  parallel_for(
      n,
      [&](size_t start, size_t end, size_t id) {
        size_t i = start;
        for (const auto& str : info->column.slice(start, end)) {
          out[i++] = strtoi(str.begin(), str.end());
        }
      },
      info->num_threads);

  return out;
}

#ifdef HAS_ALTREP

class vroom_int : public vroom_vec {

public:
  static R_altrep_class_t class_t;

  static SEXP Make(vroom_vec_info* info) {

    SEXP out = PROTECT(R_MakeExternalPtr(info, R_NilValue, R_NilValue));
    R_RegisterCFinalizerEx(out, vroom_vec::Finalize, FALSE);

    SEXP res = R_new_altrep(class_t, out, R_NilValue);

    UNPROTECT(1);

    return res;
  }

  // ALTREP methods -------------------

  // What gets printed when .Internal(inspect()) is used
  static Rboolean Inspect(
      SEXP x,
      int pre,
      int deep,
      int pvec,
      void (*inspect_subtree)(SEXP, int, int, int)) {
    Rprintf(
        "vroom_int (len=%d, materialized=%s)\n",
        Length(x),
        R_altrep_data2(x) != R_NilValue ? "T" : "F");
    return TRUE;
  }

  // ALTINTEGER methods -----------------

  static SEXP Materialize(SEXP vec) {
    SEXP data2 = R_altrep_data2(vec);
    if (data2 != R_NilValue) {
      return data2;
    }

    auto out = read_int(&Info(vec));
    R_set_altrep_data2(vec, out);

    // Once we have materialized we no longer need the info
    Finalize(R_altrep_data1(vec));

    return out;
  }

  // the element at the index `i`
  static int int_Elt(SEXP vec, R_xlen_t i) {
    SEXP data2 = R_altrep_data2(vec);
    if (data2 != R_NilValue) {
      return INTEGER(data2)[i];
    }

    auto str = vroom_vec::Get(vec, i);

    return strtoi(str.begin(), str.end());
  }

  static void* Dataptr(SEXP vec, Rboolean writeable) {
    return STDVEC_DATAPTR(Materialize(vec));
  }

  // -------- initialize the altrep class with the methods above
  static void Init(DllInfo* dll) {
    vroom_int::class_t = R_make_altinteger_class("vroom_int", "vroom", dll);

    // altrep
    R_set_altrep_Length_method(class_t, Length);
    R_set_altrep_Inspect_method(class_t, Inspect);

    // altvec
    R_set_altvec_Dataptr_method(class_t, Dataptr);
    R_set_altvec_Dataptr_or_null_method(class_t, Dataptr_or_null);

    // altinteger
    R_set_altinteger_Elt_method(class_t, int_Elt);
  }
};

R_altrep_class_t vroom_int::class_t;

// Called the package is loaded (needs Rcpp 0.12.18.3)
// [[Rcpp::init]]
void init_vroom_int(DllInfo* dll) { vroom_int::Init(dll); }

#else
void init_vroom_int(DllInfo* dll) {}
#endif
