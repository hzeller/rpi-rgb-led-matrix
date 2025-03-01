val print_flags : unit -> unit

module Options : sig
  type t
  val create : unit -> t
  val set_rows : t -> int -> unit
  val get_rows : t -> int
  val set_cols : t -> int -> unit
  val get_cols : t -> int
  val set_chain_length : t -> int -> unit
  val get_chain_length : t -> int
  val set_parallel : t -> int -> unit
  val get_parallel : t -> int
  val set_pwm_bits : t -> int -> unit
  val get_pwm_bits : t -> int
  val set_pwm_lsb_nanoseconds : t -> int -> unit
  val get_pwm_lsb_nanoseconds : t -> int
  val set_pwm_dither_bits : t -> int -> unit
  val get_pwm_dither_bits : t -> int
  val set_brightness : t -> int -> unit
  val get_brightness : t -> int
  val set_scan_mode : t -> int -> unit
  val get_scan_mode : t -> int
  val set_row_address_type : t -> int -> unit
  val get_row_address_type : t -> int
  val set_multiplexing : t -> int -> unit
  val get_multiplexing : t -> int
  val set_disable_hardware_pulsing : t -> bool -> unit
  val get_disable_hardware_pulsing : t -> bool
  val set_show_refresh_rate : t -> bool -> unit
  val get_show_refresh_rate : t -> bool
  val set_inverse_colors : t -> bool -> unit
  val get_inverse_colors : t -> bool
  val set_led_rgb_sequence : t -> string -> unit
  val get_led_rgb_sequence : t -> string
  val set_pixel_mapper_config : t -> string -> unit
  val get_pixel_mapper_config : t -> string
  val set_panel_type : t -> string -> unit
  val get_panel_type : t -> string
  val set_limit_refresh_rate_hz : t -> int -> unit
  val get_limit_refresh_rate_hz : t -> int
  val set_disable_busy_waiting : t -> bool -> unit
  val get_disable_busy_waiting : t -> bool
  val set_hardware_mapping : t -> string -> unit
  val get_hardware_mapping : t -> string
end

module Color : sig
  type t
  val create : unit -> t
  val set_r : t -> int -> unit
  val get_r : t -> int
  val set_g : t -> int -> unit
  val get_g : t -> int
  val set_b : t -> int -> unit
  val get_b : t -> int
end

module RuntimeOptions : sig
  type t
  
  (** Create a new runtime options object with default settings *)
  val create : unit -> t
  
  (** [set_gpio_slowdown t n] sets GPIO slowdown. Range: 0..4 *)
  val set_gpio_slowdown : t -> int -> unit
  
  (** [set_daemon t b] when set to 1, goes into background and becomes a daemon *)
  val set_daemon : t -> int -> unit
  
  (** [set_drop_privileges t b] when set to 1, drops privileges from 'root' to 'daemon' *)
  val set_drop_privileges : t -> int -> unit
  
  (** [set_do_gpio_init t b] when set to 0, skips the GPIO initialization *)
  val set_do_gpio_init : t -> int -> unit
end

module Canvas : sig
  type t
  
  (** Get the size of the canvas as (width, height) tuple *)
  val get_size : t -> int * int
  
  (** Set pixel at (x, y) with color (r, g, b) *)
  val set_pixel : t -> x:int -> y:int -> r:int -> g:int -> b:int -> unit
  
  (** Set multiple pixels at once *)
  val set_pixels : t -> x:int -> y:int -> width:int -> height:int -> Color.t array -> unit
  
  (** Clear the canvas (set all pixels to black) *)
  val clear : t -> unit
  
  (** Fill the canvas with a single color *)
  val fill : t -> r:int -> g:int -> b:int -> unit
  
  (** Draw a circle *)
  val draw_circle : t -> x:int -> y:int -> radius:int -> r:int -> g:int -> b:int -> unit
  
  (** Draw a line *)
  val draw_line : t -> x0:int -> y0:int -> x1:int -> y1:int -> r:int -> g:int -> b:int -> unit
end

module Matrix : sig
  type t
  
  (** Create a matrix from options and runtime options *)
  val create : Options.t -> RuntimeOptions.t -> t
  
  (** Delete the matrix and free resources *)
  val delete : t -> unit
  
  (** Get the active canvas for the matrix *)
  val get_canvas : t -> Canvas.t
  
  (** Create an off-screen canvas for double-buffering *)
  val create_offscreen_canvas : t -> Canvas.t
  
  (** Swap the offscreen canvas with the currently active one on vsync *)
  val swap_on_vsync : t -> Canvas.t -> Canvas.t
  
  (** Get the current brightness *)
  val get_brightness : t -> int
  
  (** Set the brightness (0-100) *)
  val set_brightness : t -> int -> unit
end
