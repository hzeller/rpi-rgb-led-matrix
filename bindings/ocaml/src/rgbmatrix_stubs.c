#include <stdio.h>
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <string.h>
#include "led-matrix-c.h"

void led_matrix_print_flags(FILE *out);

CAMLprim value caml_led_matrix_print_flags(value v_unit) {
  CAMLparam1(v_unit);
  led_matrix_print_flags(stderr);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_create(value v_unit) {
  CAMLparam1(v_unit);
  CAMLlocal1(v_opts);
  v_opts = caml_alloc(sizeof(struct RGBLedMatrixOptions), Abstract_tag);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  memset(opts, 0, sizeof(struct RGBLedMatrixOptions));
  CAMLreturn(v_opts);
}

CAMLprim value caml_led_matrix_options_set_rows(value v_opts, value v_rows) {
  CAMLparam2(v_opts, v_rows);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->rows = Int_val(v_rows);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_rows(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->rows));
}

CAMLprim value caml_led_matrix_options_set_cols(value v_opts, value v_cols) {
  CAMLparam2(v_opts, v_cols);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->cols = Int_val(v_cols);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_cols(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->cols));
}

CAMLprim value caml_led_matrix_options_set_chain_length(value v_opts, value v_chain_length) {
  CAMLparam2(v_opts, v_chain_length);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->chain_length = Int_val(v_chain_length);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_chain_length(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->chain_length));
}

CAMLprim value caml_led_matrix_options_set_parallel(value v_opts, value v_parallel) {
  CAMLparam2(v_opts, v_parallel);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->parallel = Int_val(v_parallel);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_parallel(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->parallel));
}

CAMLprim value caml_led_matrix_options_set_pwm_bits(value v_opts, value v_pwm_bits) {
  CAMLparam2(v_opts, v_pwm_bits);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->pwm_bits = Int_val(v_pwm_bits);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_pwm_bits(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->pwm_bits));
}

CAMLprim value caml_led_matrix_options_set_pwm_lsb_nanoseconds(value v_opts, value v_pwm_lsb_nanoseconds) {
  CAMLparam2(v_opts, v_pwm_lsb_nanoseconds);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->pwm_lsb_nanoseconds = Int_val(v_pwm_lsb_nanoseconds);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_pwm_lsb_nanoseconds(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->pwm_lsb_nanoseconds));
}

CAMLprim value caml_led_matrix_options_set_pwm_dither_bits(value v_opts, value v_pwm_dither_bits) {
  CAMLparam2(v_opts, v_pwm_dither_bits);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->pwm_dither_bits = Int_val(v_pwm_dither_bits);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_pwm_dither_bits(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->pwm_dither_bits));
}

CAMLprim value caml_led_matrix_options_set_brightness(value v_opts, value v_brightness) {
  CAMLparam2(v_opts, v_brightness);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->brightness = Int_val(v_brightness);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_brightness(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->brightness));
}

CAMLprim value caml_led_matrix_options_set_scan_mode(value v_opts, value v_scan_mode) {
  CAMLparam2(v_opts, v_scan_mode);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->scan_mode = Int_val(v_scan_mode);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_scan_mode(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->scan_mode));
}

CAMLprim value caml_led_matrix_options_set_row_address_type(value v_opts, value v_row_address_type) {
  CAMLparam2(v_opts, v_row_address_type);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->row_address_type = Int_val(v_row_address_type);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_row_address_type(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->row_address_type));
}

CAMLprim value caml_led_matrix_options_set_multiplexing(value v_opts, value v_multiplexing) {
  CAMLparam2(v_opts, v_multiplexing);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->multiplexing = Int_val(v_multiplexing);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_multiplexing(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->multiplexing));
}

CAMLprim value caml_led_matrix_options_set_disable_hardware_pulsing(value v_opts, value v_disable_hardware_pulsing) {
  CAMLparam2(v_opts, v_disable_hardware_pulsing);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->disable_hardware_pulsing = Bool_val(v_disable_hardware_pulsing);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_disable_hardware_pulsing(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_bool(opts->disable_hardware_pulsing));
}

CAMLprim value caml_led_matrix_options_set_show_refresh_rate(value v_opts, value v_show_refresh_rate) {
  CAMLparam2(v_opts, v_show_refresh_rate);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->show_refresh_rate = Bool_val(v_show_refresh_rate);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_show_refresh_rate(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_bool(opts->show_refresh_rate));
}

CAMLprim value caml_led_matrix_options_set_inverse_colors(value v_opts, value v_inverse_colors) {
  CAMLparam2(v_opts, v_inverse_colors);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->inverse_colors = Bool_val(v_inverse_colors);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_inverse_colors(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_bool(opts->inverse_colors));
}

CAMLprim value caml_led_matrix_options_set_led_rgb_sequence(value v_opts, value v_led_rgb_sequence) {
  CAMLparam2(v_opts, v_led_rgb_sequence);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->led_rgb_sequence = String_val(v_led_rgb_sequence);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_led_rgb_sequence(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(caml_copy_string(opts->led_rgb_sequence));
}

CAMLprim value caml_led_matrix_options_set_pixel_mapper_config(value v_opts, value v_pixel_mapper_config) {
  CAMLparam2(v_opts, v_pixel_mapper_config);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->pixel_mapper_config = String_val(v_pixel_mapper_config);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_pixel_mapper_config(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(caml_copy_string(opts->pixel_mapper_config));
}

CAMLprim value caml_led_matrix_options_set_panel_type(value v_opts, value v_panel_type) {
  CAMLparam2(v_opts, v_panel_type);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->panel_type = String_val(v_panel_type);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_panel_type(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(caml_copy_string(opts->panel_type));
}

CAMLprim value caml_led_matrix_options_set_limit_refresh_rate_hz(value v_opts, value v_limit_refresh_rate_hz) {
  CAMLparam2(v_opts, v_limit_refresh_rate_hz);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->limit_refresh_rate_hz = Int_val(v_limit_refresh_rate_hz);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_limit_refresh_rate_hz(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_int(opts->limit_refresh_rate_hz));
}

CAMLprim value caml_led_matrix_options_set_disable_busy_waiting(value v_opts, value v_disable_busy_waiting) {
  CAMLparam2(v_opts, v_disable_busy_waiting);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  opts->disable_busy_waiting = Bool_val(v_disable_busy_waiting);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_disable_busy_waiting(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(Val_bool(opts->disable_busy_waiting));
}

CAMLprim value caml_led_matrix_options_set_hardware_mapping(value v_opts, value v_hardware_mapping) {
  CAMLparam2(v_opts, v_hardware_mapping);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  const char *hardware_mapping = String_val(v_hardware_mapping);
  char *dup_hardware_mapping = strdup(hardware_mapping);
  if (dup_hardware_mapping == NULL) {
    caml_failwith("strdup failed");
  }
  opts->hardware_mapping = dup_hardware_mapping;
  CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_options_get_hardware_mapping(value v_opts) {
  CAMLparam1(v_opts);
  struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
  CAMLreturn(caml_copy_string(opts->hardware_mapping));
}

