external print_flags : unit -> unit = "caml_led_matrix_print_flags"

module Options = struct
  type t
  external create : unit -> t = "caml_led_matrix_options_create"
  external set_rows : t -> int -> unit = "caml_led_matrix_options_set_rows"
  external get_rows : t -> int = "caml_led_matrix_options_get_rows"

  external set_cols : t -> int -> unit = "caml_led_matrix_options_set_cols"
  external get_cols : t -> int = "caml_led_matrix_options_get_cols"

  external set_chain_length : t -> int -> unit = "caml_led_matrix_options_set_chain_length"
  external get_chain_length : t -> int = "caml_led_matrix_options_get_chain_length"

  external set_parallel : t -> int -> unit = "caml_led_matrix_options_set_parallel"
  external get_parallel : t -> int = "caml_led_matrix_options_get_parallel"

  external set_pwm_bits : t -> int -> unit = "caml_led_matrix_options_set_pwm_bits"
  external get_pwm_bits : t -> int = "caml_led_matrix_options_get_pwm_bits"

  external set_pwm_lsb_nanoseconds : t -> int -> unit = "caml_led_matrix_options_set_pwm_lsb_nanoseconds"
  external get_pwm_lsb_nanoseconds : t -> int = "caml_led_matrix_options_get_pwm_lsb_nanoseconds"

  external set_pwm_dither_bits : t -> int -> unit = "caml_led_matrix_options_set_pwm_dither_bits"
  external get_pwm_dither_bits : t -> int = "caml_led_matrix_options_get_pwm_dither_bits"

  external set_brightness : t -> int -> unit = "caml_led_matrix_options_set_brightness"
  external get_brightness : t -> int = "caml_led_matrix_options_get_brightness"

  external set_scan_mode : t -> int -> unit = "caml_led_matrix_options_set_scan_mode"
  external get_scan_mode : t -> int = "caml_led_matrix_options_get_scan_mode"

  external set_row_address_type : t -> int -> unit = "caml_led_matrix_options_set_row_address_type"
  external get_row_address_type : t -> int = "caml_led_matrix_options_get_row_address_type"

  external set_multiplexing : t -> int -> unit = "caml_led_matrix_options_set_multiplexing"
  external get_multiplexing : t -> int = "caml_led_matrix_options_get_multiplexing"

  external set_disable_hardware_pulsing : t -> bool -> unit = "caml_led_matrix_options_set_disable_hardware_pulsing"
  external get_disable_hardware_pulsing : t -> bool = "caml_led_matrix_options_get_disable_hardware_pulsing"

  external set_show_refresh_rate : t -> bool -> unit = "caml_led_matrix_options_set_show_refresh_rate"
  external get_show_refresh_rate : t -> bool = "caml_led_matrix_options_get_show_refresh_rate"

  external set_inverse_colors : t -> bool -> unit = "caml_led_matrix_options_set_inverse_colors"
  external get_inverse_colors : t -> bool = "caml_led_matrix_options_get_inverse_colors"

  external set_led_rgb_sequence : t -> string -> unit = "caml_led_matrix_options_set_led_rgb_sequence"
  external get_led_rgb_sequence : t -> string = "caml_led_matrix_options_get_led_rgb_sequence"

  external set_pixel_mapper_config : t -> string -> unit = "caml_led_matrix_options_set_pixel_mapper_config"
  external get_pixel_mapper_config : t -> string = "caml_led_matrix_options_get_pixel_mapper_config"

  external set_panel_type : t -> string -> unit = "caml_led_matrix_options_set_panel_type"
  external get_panel_type : t -> string = "caml_led_matrix_options_get_panel_type"

  external set_limit_refresh_rate_hz : t -> int -> unit = "caml_led_matrix_options_set_limit_refresh_rate_hz"
  external get_limit_refresh_rate_hz : t -> int = "caml_led_matrix_options_get_limit_refresh_rate_hz"

  external set_disable_busy_waiting : t -> bool -> unit = "caml_led_matrix_options_set_disable_busy_waiting"
  external get_disable_busy_waiting : t -> bool = "caml_led_matrix_options_get_disable_busy_waiting"

  external set_hardware_mapping : t -> string -> unit = "caml_led_matrix_options_set_hardware_mapping"
  external get_hardware_mapping : t -> string = "caml_led_matrix_options_get_hardware_mapping"
end

module Color = struct
  type t
  external create : unit -> t = "caml_color_create"
  external set_r : t -> int -> unit = "caml_color_set_r"
  external get_r : t -> int = "caml_color_get_r"
  external set_g : t -> int -> unit = "caml_color_set_g"
  external get_g : t -> int = "caml_color_get_g"
  external set_b : t -> int -> unit = "caml_color_set_b"
  external get_b : t -> int = "caml_color_get_b"
end

module RuntimeOptions = struct
  type t
  external create : unit -> t = "caml_runtime_options_create"
  external set_gpio_slowdown : t -> int -> unit = "caml_runtime_options_set_gpio_slowdown"
  external set_daemon : t -> int -> unit = "caml_runtime_options_set_daemon"
  external set_drop_privileges : t -> int -> unit = "caml_runtime_options_set_drop_privileges"
  external set_do_gpio_init : t -> int -> unit = "caml_runtime_options_set_do_gpio_init"
end

module Canvas = struct
  type t
  
  external get_size : t -> int * int = "caml_led_canvas_get_size"
  
  external set_pixel_native : t -> int -> int -> int -> int -> int -> unit = "caml_led_canvas_set_pixel_bytecode" "caml_led_canvas_set_pixel"
  let set_pixel t ~x ~y ~r ~g ~b = set_pixel_native t x y r g b
  
  external set_pixels_native : t -> int -> int -> int -> int -> Color.t array -> unit = "caml_led_canvas_set_pixels_bytecode" "caml_led_canvas_set_pixels"
  let set_pixels t ~x ~y ~width ~height colors = set_pixels_native t x y width height colors
  
  external clear : t -> unit = "caml_led_canvas_clear"
  
  external fill_native : t -> int -> int -> int -> unit = "caml_led_canvas_fill"
  let fill t ~r ~g ~b = fill_native t r g b
  
  external draw_circle_native : t -> int -> int -> int -> int -> int -> int -> unit = "caml_draw_circle_bytecode" "caml_draw_circle"
  let draw_circle t ~x ~y ~radius ~r ~g ~b = draw_circle_native t x y radius r g b
  
  external draw_line_native : t -> int -> int -> int -> int -> int -> int -> int -> unit = "caml_draw_line_bytecode" "caml_draw_line"
  let draw_line t ~x0 ~y0 ~x1 ~y1 ~r ~g ~b = draw_line_native t x0 y0 x1 y1 r g b
end

module Matrix = struct
  type t
  
  external create : Options.t -> RuntimeOptions.t -> t = "caml_led_matrix_create_from_options_and_rt_options"
  external delete : t -> unit = "caml_led_matrix_delete"
  external get_canvas : t -> Canvas.t = "caml_led_matrix_get_canvas"
  external create_offscreen_canvas : t -> Canvas.t = "caml_led_matrix_create_offscreen_canvas"
  external swap_on_vsync : t -> Canvas.t -> Canvas.t = "caml_led_matrix_swap_on_vsync"
  external get_brightness : t -> int = "caml_led_matrix_get_brightness"
  external set_brightness : t -> int -> unit = "caml_led_matrix_set_brightness"
  (* TODO:avsm finaliser instead *)
end


