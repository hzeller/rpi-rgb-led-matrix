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
