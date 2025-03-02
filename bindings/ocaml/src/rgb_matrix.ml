external print_flags : unit -> unit = "caml_led_matrix_print_flags"

(* Hardware mapping options *)
type hardware_mapping =
  | Regular
  | AdafruitHat
  | AdafruitHatPWM
  | RegularPi1
  | Classic
  | ClassicPi1
  | ComputeModule
  | Custom of string

(* Scan mode options *)
type scan_mode =
  | Progressive
  | Interlaced

(* Row address type options *)
type row_address_type =
  | DirectRowAddress       (* 0: Direct setting of row *)
  | ABRowAddress           (* 1: A/B addressing for 64x64 panels *)

(* Multiplexing type options *)
type multiplexing =
  | DirectMultiplexing      (* 0: Direct multiplexing *)
  | Stripe                  (* 1: Stripe multiplexing *)
  | Checker                 (* 2: Checker multiplexing (typical for 1:8) *)
  | Spiral                  (* 3: Spiral multiplexing *)
  | ZStripe                 (* 4: Z-Stripe multiplexing *)
  | ZnMirrorZStripe         (* 5: ZnMirrorZStripe multiplexing *)
  | Coreman                 (* 6: Coreman multiplexing *)
  | Kaler2Scan              (* 7: Kaler2Scan multiplexing *)
  | ZStripeUneven           (* 8: ZStripeUneven multiplexing *)
  | P10MapperZ              (* 9: P10MapperZ multiplexing *)
  | QiangLiQ8               (* 10: QiangLiQ8 multiplexing *)
  | InversedZStripe         (* 11: InversedZStripe multiplexing *)
  | P10Outdoor1R1G1_1       (* 12: P10Outdoor1R1G1_1 multiplexing *)
  | P10Outdoor1R1G1_2       (* 13: P10Outdoor1R1G1_2 multiplexing *)
  | P10Outdoor1R1G1_3       (* 14: P10Outdoor1R1G1_3 multiplexing *)
  | P10CoremanMapper        (* 15: P10CoremanMapper multiplexing *)
  | P8Outdoor1R1G1          (* 16: P8Outdoor1R1G1 multiplexing *)
  | FlippedStripe           (* 17: FlippedStripe multiplexing *)
  | P10Outdoor32x16HalfScan (* 18: P10Outdoor32x16HalfScan multiplexing *)
  | P10Outdoor32x16QuarterScan (* 19: P10Outdoor32x16QuarterScan multiplexing *)
  | P3Outdoor64x64MultiplexMapper (* 20: P3Outdoor64x64MultiplexMapper *)
  | Custom of int           (* Custom multiplexing specified as an integer *)

(* Panel type for special initialization sequences *)
type panel_type =
  | Default
  | FM6126A
  | Custom of string

(* RGB sequence for panels with different color wiring *)
type rgb_sequence =
  | RGB
  | RBG
  | GRB
  | GBR
  | BRG
  | BGR

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

  (* Raw access to the C-level scan_mode *)
  external _set_scan_mode : t -> int -> unit = "caml_led_matrix_options_set_scan_mode"
  external _get_scan_mode : t -> int = "caml_led_matrix_options_get_scan_mode"
  
  (* Type-safe scan_mode interface *)
  let set_scan_mode t mode =
    let int_val = match mode with
      | Progressive -> 0
      | Interlaced -> 1
    in
    _set_scan_mode t int_val
    
  let get_scan_mode t =
    match _get_scan_mode t with
    | 0 -> Progressive
    | 1 -> Interlaced
    | n -> failwith (Printf.sprintf "Unknown scan_mode value: %d" n)

  (* Raw access to the C-level row_address_type *)
  external _set_row_address_type : t -> int -> unit = "caml_led_matrix_options_set_row_address_type"
  external _get_row_address_type : t -> int = "caml_led_matrix_options_get_row_address_type"
  
  (* Type-safe row_address_type interface *)
  let set_row_address_type t mode =
    let int_val = match mode with
      | DirectRowAddress -> 0
      | ABRowAddress -> 1
    in
    _set_row_address_type t int_val
    
  let get_row_address_type t =
    match _get_row_address_type t with
    | 0 -> DirectRowAddress
    | 1 -> ABRowAddress
    | n -> failwith (Printf.sprintf "Unknown row_address_type value: %d" n)

  (* Raw access to the C-level multiplexing *)
  external _set_multiplexing : t -> int -> unit = "caml_led_matrix_options_set_multiplexing"
  external _get_multiplexing : t -> int = "caml_led_matrix_options_get_multiplexing"
  
  (* Type-safe multiplexing interface *)
  let set_multiplexing t mode =
    let int_val = match mode with
      | DirectMultiplexing -> 0
      | Stripe -> 1
      | Checker -> 2
      | Spiral -> 3
      | ZStripe -> 4
      | ZnMirrorZStripe -> 5
      | Coreman -> 6
      | Kaler2Scan -> 7
      | ZStripeUneven -> 8
      | P10MapperZ -> 9
      | QiangLiQ8 -> 10
      | InversedZStripe -> 11
      | P10Outdoor1R1G1_1 -> 12
      | P10Outdoor1R1G1_2 -> 13
      | P10Outdoor1R1G1_3 -> 14
      | P10CoremanMapper -> 15
      | P8Outdoor1R1G1 -> 16
      | FlippedStripe -> 17
      | P10Outdoor32x16HalfScan -> 18
      | P10Outdoor32x16QuarterScan -> 19
      | P3Outdoor64x64MultiplexMapper -> 20
      | Custom n -> n
    in
    _set_multiplexing t int_val
    
  let get_multiplexing t =
    match _get_multiplexing t with
    | 0 -> DirectMultiplexing
    | 1 -> Stripe
    | 2 -> Checker
    | 3 -> Spiral
    | 4 -> ZStripe
    | 5 -> ZnMirrorZStripe
    | 6 -> Coreman
    | 7 -> Kaler2Scan
    | 8 -> ZStripeUneven
    | 9 -> P10MapperZ
    | 10 -> QiangLiQ8
    | 11 -> InversedZStripe
    | 12 -> P10Outdoor1R1G1_1
    | 13 -> P10Outdoor1R1G1_2
    | 14 -> P10Outdoor1R1G1_3
    | 15 -> P10CoremanMapper
    | 16 -> P8Outdoor1R1G1
    | 17 -> FlippedStripe
    | 18 -> P10Outdoor32x16HalfScan
    | 19 -> P10Outdoor32x16QuarterScan
    | 20 -> P3Outdoor64x64MultiplexMapper
    | n -> Custom n

  external set_disable_hardware_pulsing : t -> bool -> unit = "caml_led_matrix_options_set_disable_hardware_pulsing"
  external get_disable_hardware_pulsing : t -> bool = "caml_led_matrix_options_get_disable_hardware_pulsing"

  external set_show_refresh_rate : t -> bool -> unit = "caml_led_matrix_options_set_show_refresh_rate"
  external get_show_refresh_rate : t -> bool = "caml_led_matrix_options_get_show_refresh_rate"

  external set_inverse_colors : t -> bool -> unit = "caml_led_matrix_options_set_inverse_colors"
  external get_inverse_colors : t -> bool = "caml_led_matrix_options_get_inverse_colors"

  (* Raw access to the C-level led_rgb_sequence *)
  external _set_led_rgb_sequence : t -> string -> unit = "caml_led_matrix_options_set_led_rgb_sequence"
  external _get_led_rgb_sequence : t -> string = "caml_led_matrix_options_get_led_rgb_sequence"
  
  (* Type-safe rgb_sequence interface *)
  let set_led_rgb_sequence t seq =
    let str_val = match seq with
      | RGB -> "RGB"
      | RBG -> "RBG"
      | GRB -> "GRB"
      | GBR -> "GBR"
      | BRG -> "BRG"
      | BGR -> "BGR"
    in
    _set_led_rgb_sequence t str_val
    
  let get_led_rgb_sequence t =
    match _get_led_rgb_sequence t with
    | "RGB" -> RGB
    | "RBG" -> RBG
    | "GRB" -> GRB
    | "GBR" -> GBR
    | "BRG" -> BRG
    | "BGR" -> BGR
    | s -> 
        (* Default to RGB if unrecognized *)
        Printf.eprintf "Warning: Unknown RGB sequence: %s, defaulting to RGB\n" s;
        RGB

  external set_pixel_mapper_config : t -> string -> unit = "caml_led_matrix_options_set_pixel_mapper_config"
  external get_pixel_mapper_config : t -> string = "caml_led_matrix_options_get_pixel_mapper_config"

  (* Raw access to the C-level panel_type *)
  external _set_panel_type : t -> string -> unit = "caml_led_matrix_options_set_panel_type"
  external _get_panel_type : t -> string = "caml_led_matrix_options_get_panel_type"
  
  (* Type-safe panel_type interface *)
  let set_panel_type t panel =
    let str_val = match panel with
      | Default -> ""
      | FM6126A -> "FM6126A"
      | Custom s -> s
    in
    _set_panel_type t str_val
    
  let get_panel_type t =
    match _get_panel_type t with
    | "" -> Default
    | "FM6126A" -> FM6126A
    | s -> Custom s

  external set_limit_refresh_rate_hz : t -> int -> unit = "caml_led_matrix_options_set_limit_refresh_rate_hz"
  external get_limit_refresh_rate_hz : t -> int = "caml_led_matrix_options_get_limit_refresh_rate_hz"

  external set_disable_busy_waiting : t -> bool -> unit = "caml_led_matrix_options_set_disable_busy_waiting"
  external get_disable_busy_waiting : t -> bool = "caml_led_matrix_options_get_disable_busy_waiting"

  (* Raw access to the C-level hardware_mapping *)
  external _set_hardware_mapping : t -> string -> unit = "caml_led_matrix_options_set_hardware_mapping"
  external _get_hardware_mapping : t -> string = "caml_led_matrix_options_get_hardware_mapping"
  
  (* Type-safe hardware_mapping interface *)
  let set_hardware_mapping t mapping =
    let str_val = match mapping with
      | Regular -> "regular"
      | AdafruitHat -> "adafruit-hat"
      | AdafruitHatPWM -> "adafruit-hat-pwm"
      | RegularPi1 -> "regular-pi1"
      | Classic -> "classic"
      | ClassicPi1 -> "classic-pi1"
      | ComputeModule -> "compute-module"
      | Custom s -> s
    in
    _set_hardware_mapping t str_val
    
  let get_hardware_mapping t =
    match _get_hardware_mapping t with
    | "regular" -> Regular
    | "adafruit-hat" -> AdafruitHat
    | "adafruit-hat-pwm" -> AdafruitHatPWM
    | "regular-pi1" -> RegularPi1
    | "classic" -> Classic
    | "classic-pi1" -> ClassicPi1
    | "compute-module" -> ComputeModule
    | s -> Custom s
end

(* Daemon mode options *)
type daemon_mode =
  | Off        (* Run in foreground (default) *)
  | On         (* Run as daemon in background *)

type privilege_mode =
  | Maintain   (* Keep current privileges (typically root) *)
  | Drop       (* Drop privileges to 'daemon' user after initializing hardware *)

(* GPIO initialization options *)
type gpio_init_mode =
  | Skip       (* Skip GPIO initialization, useful for non-Raspberry Pi platforms *)
  | Initialize (* Initialize GPIO pins (default) *)

module Color = struct
  type t
  external create : unit -> t = "caml_color_create"
  external set_r : t -> int -> unit = "caml_color_set_r"
  external get_r : t -> int = "caml_color_get_r"
  external set_g : t -> int -> unit = "caml_color_set_g"
  external get_g : t -> int = "caml_color_get_g"
  external set_b : t -> int -> unit = "caml_color_set_b"
  external get_b : t -> int = "caml_color_get_b"
  
  (* Create a color with specified RGB components *)
  let create_rgb ~r ~g ~b =
    let color = create () in
    set_r color r;
    set_g color g;
    set_b color b;
    color
    
  (* Predefined colors *)
  let black = create_rgb ~r:0 ~g:0 ~b:0
  let white = create_rgb ~r:255 ~g:255 ~b:255
  let red = create_rgb ~r:255 ~g:0 ~b:0
  let green = create_rgb ~r:0 ~g:255 ~b:0
  let blue = create_rgb ~r:0 ~g:0 ~b:255
  let yellow = create_rgb ~r:255 ~g:255 ~b:0
  let cyan = create_rgb ~r:0 ~g:255 ~b:255
  let magenta = create_rgb ~r:255 ~g:0 ~b:255
end

module RuntimeOptions = struct
  type t
  
  (* Exposed external functions to the C API *)
  external create : unit -> t = "caml_runtime_options_create"
  external _set_gpio_slowdown : t -> int -> unit = "caml_runtime_options_set_gpio_slowdown"
  external _set_daemon : t -> int -> unit = "caml_runtime_options_set_daemon"
  external _set_drop_privileges : t -> int -> unit = "caml_runtime_options_set_drop_privileges"
  external _set_do_gpio_init : t -> int -> unit = "caml_runtime_options_set_do_gpio_init"
  
  (* Public GPIO slowdown functions *)
  let set_gpio_slowdown t value =
    if value < 0 || value > 4 then
      failwith "GPIO slowdown must be between 0 and 4"
    else
      _set_gpio_slowdown t value
      
  let get_gpio_slowdown _t =
    (* We don't have a getter in the C API, so we can't actually return the real value *)
    failwith "RuntimeOptions.get_gpio_slowdown not implemented in the C API"
  
  (* Type-safe daemon mode interface *)
  let set_daemon t mode =
    let int_val = match mode with
      | Off -> 0
      | On -> 1
    in
    _set_daemon t int_val
    
  let get_daemon _t =
    (* We don't have a getter in the C API, so default to Off *)
    Off
  
  (* Type-safe privilege drop interface *)
  let set_drop_privileges t mode =
    let int_val = match mode with
      | Maintain -> 0
      | Drop -> 1
    in
    _set_drop_privileges t int_val
    
  let get_drop_privileges _t =
    (* We don't have a getter in the C API, so default to Maintain *)
    Maintain
  
  (* Type-safe GPIO init interface *)
  let set_do_gpio_init t mode =
    let int_val = match mode with
      | Skip -> 0
      | Initialize -> 1
    in
    _set_do_gpio_init t int_val
    
  let get_do_gpio_init _t =
    (* We don't have a getter in the C API, so default to Initialize *)
    Initialize
end

module Canvas = struct
  type t
  
  external get_size : t -> int * int = "caml_led_canvas_get_size"
  
  external set_pixel_native : t -> int -> int -> int -> int -> int -> unit = "caml_led_canvas_set_pixel_bytecode" "caml_led_canvas_set_pixel"
  let set_pixel t ~x ~y ~r ~g ~b = set_pixel_native t x y r g b
  
  (* Set pixel with Color.t *)
  let set_pixel_color t ~x ~y ~color =
    set_pixel t ~x ~y ~r:(Color.get_r color) ~g:(Color.get_g color) ~b:(Color.get_b color)
  
  external set_pixels_native : t -> int -> int -> int -> int -> Color.t array -> unit = "caml_led_canvas_set_pixels_bytecode" "caml_led_canvas_set_pixels"
  let set_pixels t ~x ~y ~width ~height colors = set_pixels_native t x y width height colors
  
  external clear : t -> unit = "caml_led_canvas_clear"
  
  external fill_native : t -> int -> int -> int -> unit = "caml_led_canvas_fill"
  let fill t ~r ~g ~b = fill_native t r g b
  
  (* Fill with Color.t *)
  let fill_color t ~color =
    fill t ~r:(Color.get_r color) ~g:(Color.get_g color) ~b:(Color.get_b color)
  
  external draw_circle_native : t -> int -> int -> int -> int -> int -> int -> unit = "caml_draw_circle_bytecode" "caml_draw_circle"
  let draw_circle t ~x ~y ~radius ~r ~g ~b = draw_circle_native t x y radius r g b
  
  (* Draw circle with Color.t *)
  let draw_circle_color t ~x ~y ~radius ~color =
    draw_circle t ~x ~y ~radius ~r:(Color.get_r color) ~g:(Color.get_g color) ~b:(Color.get_b color)
  
  external draw_line_native : t -> int -> int -> int -> int -> int -> int -> int -> unit = "caml_draw_line_bytecode" "caml_draw_line"
  let draw_line t ~x0 ~y0 ~x1 ~y1 ~r ~g ~b = draw_line_native t x0 y0 x1 y1 r g b
  
  (* Draw line with Color.t *)
  let draw_line_color t ~x0 ~y0 ~x1 ~y1 ~color =
    draw_line t ~x0 ~y0 ~x1 ~y1 ~r:(Color.get_r color) ~g:(Color.get_g color) ~b:(Color.get_b color)
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


