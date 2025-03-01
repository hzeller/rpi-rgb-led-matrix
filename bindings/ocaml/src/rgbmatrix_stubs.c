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

CAMLprim value caml_color_create(value v_unit) {
  CAMLparam1(v_unit);
  CAMLlocal1(v_color);
  v_color = caml_alloc(sizeof(struct Color), Abstract_tag);
  struct Color *color = (struct Color *)Data_abstract_val(v_color);
  memset(color, 0, sizeof(struct Color));
  CAMLreturn(v_color);
}

CAMLprim value caml_color_set_r(value v_color, value v_r) {
  CAMLparam2(v_color, v_r);
  struct Color *color = (struct Color *)Data_abstract_val(v_color);
  color->r = Int_val(v_r);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_color_get_r(value v_color) {
  CAMLparam1(v_color);
  struct Color *color = (struct Color *)Data_abstract_val(v_color);
  CAMLreturn(Val_int(color->r));
}

CAMLprim value caml_color_set_g(value v_color, value v_g) {
  CAMLparam2(v_color, v_g);
  struct Color *color = (struct Color *)Data_abstract_val(v_color);
  color->g = Int_val(v_g);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_color_get_g(value v_color) {
  CAMLparam1(v_color);
  struct Color *color = (struct Color *)Data_abstract_val(v_color);
  CAMLreturn(Val_int(color->g));
}

CAMLprim value caml_color_set_b(value v_color, value v_b) {
  CAMLparam2(v_color, v_b);
  struct Color *color = (struct Color *)Data_abstract_val(v_color);
  color->b = Int_val(v_b);
  CAMLreturn(Val_unit);
}

CAMLprim value caml_color_get_b(value v_color) {
  CAMLparam1(v_color);
  struct Color *color = (struct Color *)Data_abstract_val(v_color);
  CAMLreturn(Val_int(color->b));
}

// RuntimeOptions handling

/* Encapsulation of opaque runtime_options handles */
static struct custom_operations runtime_options_ops = {
    "rpi_rgb_led_matrix.runtime_options",
    custom_finalize_default,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default,
};

/* Allocating an OCaml custom block to hold the C runtime_options pointer */
static value alloc_runtime_options(struct RGBLedRuntimeOptions *rt_opt)
{
    value v = caml_alloc_custom(&runtime_options_ops, sizeof(struct RGBLedRuntimeOptions *), 0, 1);
    struct RGBLedRuntimeOptions **rt_opt_ptr = (struct RGBLedRuntimeOptions **) Data_custom_val(v);
    *rt_opt_ptr = rt_opt;
    return v;
}

/* Extracting the runtime_options pointer from the OCaml custom block */
static struct RGBLedRuntimeOptions *runtime_options_val(value v)
{
    struct RGBLedRuntimeOptions **rt_opt_ptr = (struct RGBLedRuntimeOptions **) Data_custom_val(v);
    return *rt_opt_ptr;
}

CAMLprim value caml_runtime_options_create(value unit)
{
    CAMLparam1(unit);
    struct RGBLedRuntimeOptions *rt_opt = malloc(sizeof(struct RGBLedRuntimeOptions));
    if (rt_opt == NULL) {
        caml_failwith("Failed to create runtime options");
    }
    memset(rt_opt, 0, sizeof(struct RGBLedRuntimeOptions));
    CAMLreturn(alloc_runtime_options(rt_opt));
}

CAMLprim value caml_runtime_options_set_gpio_slowdown(value rt_opt_v, value slowdown_v)
{
    CAMLparam2(rt_opt_v, slowdown_v);
    struct RGBLedRuntimeOptions *rt_opt = runtime_options_val(rt_opt_v);
    int slowdown = Int_val(slowdown_v);
    rt_opt->gpio_slowdown = slowdown;
    CAMLreturn(Val_unit);
}

CAMLprim value caml_runtime_options_set_daemon(value rt_opt_v, value daemon_v)
{
    CAMLparam2(rt_opt_v, daemon_v);
    struct RGBLedRuntimeOptions *rt_opt = runtime_options_val(rt_opt_v);
    int daemon = Int_val(daemon_v);
    rt_opt->daemon = daemon;
    CAMLreturn(Val_unit);
}

CAMLprim value caml_runtime_options_set_drop_privileges(value rt_opt_v, value drop_v)
{
    CAMLparam2(rt_opt_v, drop_v);
    struct RGBLedRuntimeOptions *rt_opt = runtime_options_val(rt_opt_v);
    int drop = Int_val(drop_v);
    rt_opt->drop_privileges = drop;
    CAMLreturn(Val_unit);
}

CAMLprim value caml_runtime_options_set_do_gpio_init(value rt_opt_v, value init_v)
{
    CAMLparam2(rt_opt_v, init_v);
    struct RGBLedRuntimeOptions *rt_opt = runtime_options_val(rt_opt_v);
    int init = Int_val(init_v);
    rt_opt->do_gpio_init = init;
    CAMLreturn(Val_unit);
}

/* Encapsulation of opaque matrix handles */
static struct custom_operations matrix_ops = {
    "rpi_rgb_led_matrix.matrix",
    custom_finalize_default,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default,
};

/* Allocating an OCaml custom block to hold the C matrix pointer */
static value alloc_matrix(struct RGBLedMatrix *matrix)
{
    value v = caml_alloc_custom(&matrix_ops, sizeof(struct RGBLedMatrix *), 0, 1);
    struct RGBLedMatrix **matrix_ptr = (struct RGBLedMatrix **) Data_custom_val(v);
    *matrix_ptr = matrix;
    return v;
}

/* Extracting the matrix pointer from the OCaml custom block */
static struct RGBLedMatrix *matrix_val(value v)
{
    struct RGBLedMatrix **matrix_ptr = (struct RGBLedMatrix **) Data_custom_val(v);
    return *matrix_ptr;
}

CAMLprim value caml_led_matrix_create_from_options_and_rt_options(value v_opts, value v_rt_opts)
{
    CAMLparam2(v_opts, v_rt_opts);
    struct RGBLedMatrixOptions *opts = (struct RGBLedMatrixOptions *)Data_abstract_val(v_opts);
    struct RGBLedRuntimeOptions *rt_opts = runtime_options_val(v_rt_opts);
    
    struct RGBLedMatrix *matrix = led_matrix_create_from_options_and_rt_options(opts, rt_opts);
    if (matrix == NULL) {
        caml_failwith("Failed to create LED matrix");
    }
    
    CAMLreturn(alloc_matrix(matrix));
}

CAMLprim value caml_led_matrix_delete(value v_matrix)
{
    CAMLparam1(v_matrix);
    struct RGBLedMatrix *matrix = matrix_val(v_matrix);
    led_matrix_delete(matrix);
    CAMLreturn(Val_unit);
}

/* Encapsulation of opaque canvas handles */
static struct custom_operations canvas_ops = {
    "rpi_rgb_led_matrix.canvas",
    custom_finalize_default,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default,
};

/* Allocating an OCaml custom block to hold the C canvas pointer */
static value alloc_canvas(struct LedCanvas *canvas)
{
    value v = caml_alloc_custom(&canvas_ops, sizeof(struct LedCanvas *), 0, 1);
    struct LedCanvas **canvas_ptr = (struct LedCanvas **) Data_custom_val(v);
    *canvas_ptr = canvas;
    return v;
}

/* Extracting the canvas pointer from the OCaml custom block */
static struct LedCanvas *canvas_val(value v)
{
    struct LedCanvas **canvas_ptr = (struct LedCanvas **) Data_custom_val(v);
    return *canvas_ptr;
}

CAMLprim value caml_led_matrix_get_canvas(value v_matrix)
{
    CAMLparam1(v_matrix);
    struct RGBLedMatrix *matrix = matrix_val(v_matrix);
    struct LedCanvas *canvas = led_matrix_get_canvas(matrix);
    CAMLreturn(alloc_canvas(canvas));
}

CAMLprim value caml_led_canvas_get_size(value v_canvas)
{
    CAMLparam1(v_canvas);
    CAMLlocal1(v_size);
    int width, height;
    struct LedCanvas *canvas = canvas_val(v_canvas);
    
    led_canvas_get_size(canvas, &width, &height);
    
    v_size = caml_alloc_tuple(2);
    Store_field(v_size, 0, Val_int(width));
    Store_field(v_size, 1, Val_int(height));
    
    CAMLreturn(v_size);
}

CAMLprim value caml_led_canvas_set_pixel(value v_canvas, value v_x, value v_y, 
                                        value v_r, value v_g, value v_b)
{
    CAMLparam5(v_canvas, v_x, v_y, v_r, v_g);
    CAMLxparam1(v_b);
    struct LedCanvas *canvas = canvas_val(v_canvas);
    
    led_canvas_set_pixel(canvas, Int_val(v_x), Int_val(v_y), 
                        Int_val(v_r), Int_val(v_g), Int_val(v_b));
    
    CAMLreturn(Val_unit);
}

CAMLprim value caml_led_canvas_set_pixels(value v_canvas, value v_x, value v_y,
                                         value v_width, value v_height, value v_colors)
{
    CAMLparam5(v_canvas, v_x, v_y, v_width, v_height);
    CAMLxparam1(v_colors);
    
    struct LedCanvas *canvas = canvas_val(v_canvas);
    int x = Int_val(v_x);
    int y = Int_val(v_y);
    int width = Int_val(v_width);
    int height = Int_val(v_height);
    int total_pixels = width * height;
    
    struct Color *colors = malloc(sizeof(struct Color) * total_pixels);
    if (colors == NULL) {
        caml_failwith("Failed to allocate memory for colors");
    }
    
    for (int i = 0; i < total_pixels; i++) {
        value v_color = Field(v_colors, i);
        struct Color *color = (struct Color *)Data_abstract_val(v_color);
        colors[i] = *color;
    }
    
    led_canvas_set_pixels(canvas, x, y, width, height, colors);
    
    free(colors);
    
    CAMLreturn(Val_unit);
}

CAMLprim value caml_led_canvas_clear(value v_canvas)
{
    CAMLparam1(v_canvas);
    struct LedCanvas *canvas = canvas_val(v_canvas);
    
    led_canvas_clear(canvas);
    
    CAMLreturn(Val_unit);
}

CAMLprim value caml_led_canvas_fill(value v_canvas, value v_r, value v_g, value v_b)
{
    CAMLparam4(v_canvas, v_r, v_g, v_b);
    struct LedCanvas *canvas = canvas_val(v_canvas);
    
    led_canvas_fill(canvas, Int_val(v_r), Int_val(v_g), Int_val(v_b));
    
    CAMLreturn(Val_unit);
}

CAMLprim value caml_led_matrix_create_offscreen_canvas(value v_matrix)
{
    CAMLparam1(v_matrix);
    struct RGBLedMatrix *matrix = matrix_val(v_matrix);
    struct LedCanvas *canvas = led_matrix_create_offscreen_canvas(matrix);
    
    CAMLreturn(alloc_canvas(canvas));
}

CAMLprim value caml_led_matrix_swap_on_vsync(value v_matrix, value v_canvas)
{
    CAMLparam2(v_matrix, v_canvas);
    struct RGBLedMatrix *matrix = matrix_val(v_matrix);
    struct LedCanvas *canvas = canvas_val(v_canvas);
    
    struct LedCanvas *previous_canvas = led_matrix_swap_on_vsync(matrix, canvas);
    
    CAMLreturn(alloc_canvas(previous_canvas));
}

CAMLprim value caml_led_matrix_get_brightness(value v_matrix)
{
    CAMLparam1(v_matrix);
    struct RGBLedMatrix *matrix = matrix_val(v_matrix);
    
    uint8_t brightness = led_matrix_get_brightness(matrix);
    
    CAMLreturn(Val_int(brightness));
}

CAMLprim value caml_led_matrix_set_brightness(value v_matrix, value v_brightness)
{
    CAMLparam2(v_matrix, v_brightness);
    struct RGBLedMatrix *matrix = matrix_val(v_matrix);
    
    led_matrix_set_brightness(matrix, Int_val(v_brightness));
    
    CAMLreturn(Val_unit);
}

CAMLprim value caml_draw_circle(value v_canvas, value v_x, value v_y, value v_radius,
                              value v_r, value v_g, value v_b)
{
    CAMLparam5(v_canvas, v_x, v_y, v_radius, v_r);
    CAMLxparam2(v_g, v_b);
    struct LedCanvas *canvas = canvas_val(v_canvas);
    
    draw_circle(canvas, Int_val(v_x), Int_val(v_y), Int_val(v_radius),
               Int_val(v_r), Int_val(v_g), Int_val(v_b));
    
    CAMLreturn(Val_unit);
}

CAMLprim value caml_draw_line(value v_canvas, value v_x0, value v_y0, value v_x1, value v_y1,
                            value v_r, value v_g, value v_b)
{
    CAMLparam5(v_canvas, v_x0, v_y0, v_x1, v_y1);
    CAMLxparam3(v_r, v_g, v_b);
    struct LedCanvas *canvas = canvas_val(v_canvas);
    
    draw_line(canvas, Int_val(v_x0), Int_val(v_y0), Int_val(v_x1), Int_val(v_y1),
             Int_val(v_r), Int_val(v_g), Int_val(v_b));
    
    CAMLreturn(Val_unit);
}

CAMLprim value caml_led_canvas_set_pixel_bytecode(value *argv, int argn)
{
    return caml_led_canvas_set_pixel(argv[0], argv[1], argv[2], 
                                   argv[3], argv[4], argv[5]);
}

CAMLprim value caml_led_canvas_set_pixels_bytecode(value *argv, int argn)
{
    return caml_led_canvas_set_pixels(argv[0], argv[1], argv[2],
                                    argv[3], argv[4], argv[5]);
}

CAMLprim value caml_draw_circle_bytecode(value *argv, int argn)
{
    return caml_draw_circle(argv[0], argv[1], argv[2], argv[3],
                          argv[4], argv[5], argv[6]);
}

CAMLprim value caml_draw_line_bytecode(value *argv, int argn)
{
    return caml_draw_line(argv[0], argv[1], argv[2], argv[3], 
                        argv[4], argv[5], argv[6], argv[7]);
}

