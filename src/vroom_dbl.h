#pragma once

#include "altrep.h"

#include "vroom_vec.h"
#include <Rcpp.h>

#include "parallel.h"
#include "parse_dbl.h"

Rcpp::NumericVector read_dbl(vroom_vec_info* info) {

  R_xlen_t n = info->idx->num_rows();

  Rcpp::NumericVector out(n);

  parallel_for(
      n,
      [&](size_t start, size_t end, size_t id) {
        size_t i = start;
        for (const auto& str :
             info->idx->get_column(info->column, start, end)) {
          out[i++] = parse_number(str.begin());
          // char buf[128];
          // std::copy(str.begin(), str.end(), buf);
          // buf[str.length()] = '\0';

          // out[i++] = R_strtod(buf, NULL);
        }
      },
      info->num_threads);

  return out;
}

#ifdef HAS_ALTREP

/* Vroom Dbl */

class vroom_dbl : public vroom_vec {

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
        "vroom_dbl (len=%d, materialized=%s)\n",
        Length(x),
        R_altrep_data2(x) != R_NilValue ? "T" : "F");
    return TRUE;
  }

  // ALTREAL methods -----------------

  // the element at the index `i`
  static double real_Elt(SEXP vec, R_xlen_t i) {
    SEXP data2 = R_altrep_data2(vec);
    if (data2 != R_NilValue) {
      return REAL(data2)[i];
    }

    auto str = vroom_vec::Get(vec, i);

    return bsd_strtod(str.begin(), str.end());
  }

  // --- Altvec
  static SEXP Materialize(SEXP vec) {
    SEXP data2 = R_altrep_data2(vec);
    if (data2 != R_NilValue) {
      return data2;
    }

    auto out = read_dbl(&Info(vec));
    R_set_altrep_data2(vec, out);

    // Once we have materialized we no longer need the info
    Finalize(R_altrep_data1(vec));

    return out;
  }

  static void* Dataptr(SEXP vec, Rboolean writeable) {
    return STDVEC_DATAPTR(Materialize(vec));
  }

  // -------- initialize the altrep class with the methods above
  static void Init(DllInfo* dll) {
    vroom_dbl::class_t = R_make_altreal_class("vroom_dbl", "vroom", dll);

    // altrep
    R_set_altrep_Length_method(class_t, Length);
    R_set_altrep_Inspect_method(class_t, Inspect);

    // altvec
    R_set_altvec_Dataptr_method(class_t, Dataptr);
    R_set_altvec_Dataptr_or_null_method(class_t, Dataptr_or_null);

    // altinteger
    R_set_altreal_Elt_method(class_t, real_Elt);
  }
};

R_altrep_class_t vroom_dbl::class_t;

// Called the package is loaded (needs Rcpp 0.12.18.3)
// [[Rcpp::init]]
void init_vroom_dbl(DllInfo* dll) { vroom_dbl::Init(dll); }

#else
void init_vroom_dbl(DllInfo* dll) {}
#endif
