#include <stdio.h>
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include "led-matrix-c.h"

void led_matrix_print_flags(FILE *out);

CAMLprim value caml_led_matrix_print_flags(value v_unit) {
  CAMLparam1(v_unit);
  led_matrix_print_flags(stderr);
  CAMLreturn(Val_unit);
}