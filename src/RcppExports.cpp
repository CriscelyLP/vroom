// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

// read_tsv_
SEXP read_tsv_(const std::string& filename, R_xlen_t skip, int num_threads);
RcppExport SEXP _vroom_read_tsv_(SEXP filenameSEXP, SEXP skipSEXP, SEXP num_threadsSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const std::string& >::type filename(filenameSEXP);
    Rcpp::traits::input_parameter< R_xlen_t >::type skip(skipSEXP);
    Rcpp::traits::input_parameter< int >::type num_threads(num_threadsSEXP);
    rcpp_result_gen = Rcpp::wrap(read_tsv_(filename, skip, num_threads));
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_vroom_read_tsv_", (DL_FUNC) &_vroom_read_tsv_, 3},
    {NULL, NULL, 0}
};

void init_vroom_real(DllInfo* dll);
void init_vroom_int(DllInfo* dll);
void init_vroom_string(DllInfo* dll);
RcppExport void R_init_vroom(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
    init_vroom_real(dll);
    init_vroom_int(dll);
    init_vroom_string(dll);
}