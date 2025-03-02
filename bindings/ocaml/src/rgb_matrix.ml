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

module Cmdliner = struct
  (* Cmdliner module provides terms for command-line parsing of matrix options *)
  
  open Cmdliner

  (* Conversion functions for custom types *)
  
  (* hardware_mapping conversion *)
  let hardware_mapping_conv =
    let parse s =
      match String.lowercase_ascii s with
      | "regular" -> Ok Regular
      | "adafruit-hat" -> Ok AdafruitHat
      | "adafruit-hat-pwm" -> Ok AdafruitHatPWM
      | "regular-pi1" -> Ok RegularPi1
      | "classic" -> Ok Classic
      | "classic-pi1" -> Ok ClassicPi1
      | "compute-module" -> Ok ComputeModule
      | s -> Ok (Custom s)
    in
    let print fmt m =
      Format.fprintf fmt "%s"
        (match m with
         | Regular -> "regular"
         | AdafruitHat -> "adafruit-hat"
         | AdafruitHatPWM -> "adafruit-hat-pwm"
         | RegularPi1 -> "regular-pi1"
         | Classic -> "classic"
         | ClassicPi1 -> "classic-pi1"
         | ComputeModule -> "compute-module"
         | Custom s -> s)
    in
    Arg.conv (parse, print)

  (* scan_mode conversion *)
  let scan_mode_conv =
    let parse s =
      match String.lowercase_ascii s with
      | "progressive" -> Ok Progressive
      | "interlaced" -> Ok Interlaced
      | _ -> Error (`Msg "scan_mode must be 'progressive' or 'interlaced'")
    in
    let print fmt m =
      Format.fprintf fmt "%s"
        (match m with
         | Progressive -> "progressive"
         | Interlaced -> "interlaced")
    in
    Arg.conv (parse, print)

  (* row_address_type conversion *)
  let row_address_type_conv =
    let parse s =
      match String.lowercase_ascii s with
      | "direct" | "0" -> Ok DirectRowAddress
      | "ab" | "1" -> Ok ABRowAddress
      | _ -> Error (`Msg "row_address_type must be 'direct' or 'ab'")
    in
    let print fmt m =
      Format.fprintf fmt "%s"
        (match m with
         | DirectRowAddress -> "direct"
         | ABRowAddress -> "ab")
    in
    Arg.conv (parse, print)

  (* multiplexing conversion *)
  let multiplexing_conv =
    let parse s =
      try
        let n = int_of_string s in
        if n >= 0 then Ok (match n with
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
          | n -> Custom n)
        else Error (`Msg "multiplexing must be a non-negative integer")
      with Failure _ ->
        Error (`Msg "multiplexing must be a valid integer")
    in
    let print fmt m =
      Format.fprintf fmt "%d"
        (match m with
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
         | Custom n -> n)
    in
    Arg.conv (parse, print)

  (* panel_type conversion *)
  let panel_type_conv =
    let parse s =
      match String.lowercase_ascii s with
      | "" | "default" -> Ok Default
      | "fm6126a" -> Ok FM6126A
      | s -> Ok (Custom s)
    in
    let print fmt p =
      Format.fprintf fmt "%s"
        (match p with
         | Default -> "default"
         | FM6126A -> "FM6126A"
         | Custom s -> s)
    in
    Arg.conv (parse, print)

  (* rgb_sequence conversion *)
  let rgb_sequence_conv =
    let parse s =
      match String.uppercase_ascii s with
      | "RGB" -> Ok RGB
      | "RBG" -> Ok RBG
      | "GRB" -> Ok GRB
      | "GBR" -> Ok GBR
      | "BRG" -> Ok BRG
      | "BGR" -> Ok BGR
      | _ -> Error (`Msg "rgb_sequence must be one of: RGB, RBG, GRB, GBR, BRG, BGR")
    in
    let print fmt seq =
      Format.fprintf fmt "%s"
        (match seq with
         | RGB -> "RGB"
         | RBG -> "RBG"
         | GRB -> "GRB"
         | GBR -> "GBR"
         | BRG -> "BRG"
         | BGR -> "BGR")
    in
    Arg.conv (parse, print)

  (* daemon_mode conversion *)
  let daemon_mode_conv =
    let parse s =
      match String.lowercase_ascii s with
      | "off" | "0" | "false" -> Ok Off
      | "on" | "1" | "true" -> Ok On 
      | _ -> Error (`Msg "daemon mode must be 'on' or 'off'")
    in
    let print fmt m =
      Format.fprintf fmt "%s"
        (match m with
         | Off -> "off"
         | On -> "on")
    in
    Arg.conv (parse, print)

  (* privilege_mode conversion *)
  let privilege_mode_conv =
    let parse s =
      match String.lowercase_ascii s with
      | "maintain" | "0" | "false" -> Ok Maintain
      | "drop" | "1" | "true" -> Ok Drop
      | _ -> Error (`Msg "privilege mode must be 'maintain' or 'drop'")
    in
    let print fmt m =
      Format.fprintf fmt "%s"
        (match m with
         | Maintain -> "maintain"
         | Drop -> "drop")
    in
    Arg.conv (parse, print)

  (* gpio_init_mode conversion *)
  let gpio_init_mode_conv =
    let parse s =
      match String.lowercase_ascii s with
      | "skip" | "0" | "false" -> Ok Skip
      | "initialize" | "1" | "true" -> Ok Initialize
      | _ -> Error (`Msg "gpio init mode must be 'skip' or 'initialize'")
    in
    let print fmt m =
      Format.fprintf fmt "%s"
        (match m with
         | Skip -> "skip"
         | Initialize -> "initialize")
    in
    Arg.conv (parse, print)

  (* Matrix options command-line terms *)
  let rows =
    let doc = "Number of rows (height) in each chained panel." in
    Arg.(value & opt int 32 & info ["r"; "rows"] ~docv:"ROWS" ~doc)

  let cols =
    let doc = "Number of columns (width) in each chained panel." in
    Arg.(value & opt int 32 & info ["c"; "cols"] ~docv:"COLS" ~doc)

  let chain_length =
    let doc = "Number of daisy-chained panels (output of one panel connected to input of the next)." in
    Arg.(value & opt int 1 & info ["chain-length"] ~docv:"N" ~doc)

  let parallel =
    let doc = "Number of parallel chains connected to the Pi." in
    Arg.(value & opt int 1 & info ["parallel"] ~docv:"N" ~doc)

  let hardware_mapping =
    let doc = "GPIO mapping for your board. One of: regular, adafruit-hat, adafruit-hat-pwm, \
               regular-pi1, classic, classic-pi1, compute-module, or custom." in
    Arg.(value & opt hardware_mapping_conv Regular & 
         info ["hardware-mapping"] ~docv:"MAPPING" ~doc)

  let brightness =
    let doc = "Initial brightness of the panel (1-100)." in
    Arg.(value & opt int 100 & info ["brightness"] ~docv:"PERCENT" ~doc)

  let pwm_bits =
    let doc = "PWM bits used for output (1-11). Lower values use less CPU." in
    Arg.(value & opt int 11 & info ["pwm-bits"] ~docv:"BITS" ~doc)

  let pwm_lsb_nanoseconds =
    let doc = "Time-unit for the on-time in the lowest significant bit in nanoseconds." in
    Arg.(value & opt int 130 & info ["pwm-lsb-nanoseconds"] ~docv:"NANOS" ~doc)

  let pwm_dither_bits =
    let doc = "Number of bits to use for time-dithering to achieve higher refresh rate." in
    Arg.(value & opt int 0 & info ["pwm-dither-bits"] ~docv:"BITS" ~doc)

  let scan_mode =
    let doc = "Panel scan mode (progressive or interlaced)." in
    Arg.(value & opt scan_mode_conv Progressive & info ["scan-mode"] ~docv:"MODE" ~doc)

  let row_address_type =
    let doc = "Row address type (default: direct; alternative: AB addressing for 64x64 panels)." in
    Arg.(value & opt row_address_type_conv DirectRowAddress & 
         info ["row-address-type"] ~docv:"TYPE" ~doc)

  let multiplexing =
    let doc = "Panel multiplexing type (0-20 or custom value)." in
    Arg.(value & opt multiplexing_conv DirectMultiplexing & 
         info ["multiplexing"] ~docv:"TYPE" ~doc)

  let disable_hardware_pulsing =
    let doc = "Disable hardware pin-pulse generation (force software timing)." in
    Arg.(value & flag & info ["disable-hardware-pulsing"] ~doc)

  let show_refresh_rate =
    let doc = "Show refresh rate on terminal." in
    Arg.(value & flag & info ["show-refresh-rate"] ~doc)

  let inverse_colors =
    let doc = "Inverse colors (useful for panels with inverse RGB led connections)." in
    Arg.(value & flag & info ["inverse-colors"] ~doc)

  let led_rgb_sequence =
    let doc = "RGB sequence order for LED strip color (RGB, RBG, GRB, GBR, BRG, BGR)." in
    Arg.(value & opt rgb_sequence_conv RGB & info ["led-rgb-sequence"] ~docv:"SEQ" ~doc)

  let pixel_mapper_config =
    let doc = "Semicolon-separated list of pixel-mappers to arrange panels." in
    Arg.(value & opt string "" & info ["pixel-mapper-config"] ~docv:"CONFIG" ~doc)

  let panel_type =
    let doc = "Panel type (default or FM6126A)." in
    Arg.(value & opt panel_type_conv Default & info ["panel-type"] ~docv:"TYPE" ~doc)

  let limit_refresh_rate_hz =
    let doc = "Limit refresh rate to specified Hz (<=0 for no limit)." in
    Arg.(value & opt int 0 & info ["limit-refresh-rate-hz"] ~docv:"HZ" ~doc)

  let disable_busy_waiting =
    let doc = "Use sleep instead of busy waiting when limiting refresh rate." in
    Arg.(value & flag & info ["disable-busy-waiting"] ~doc)

  (* Runtime options command-line terms *)
  let gpio_slowdown =
    let doc = "GPIO slowdown factor (0-4). Needed for faster Pi versions." in
    Arg.(value & opt int 1 & info ["gpio-slowdown"] ~docv:"VAL" ~doc)

  let daemon =
    let doc = "Run as daemon (off=foreground, on=background)." in
    Arg.(value & opt daemon_mode_conv Off & info ["daemon"] ~docv:"MODE" ~doc)
    
  let drop_privileges =
    let doc = "Drop privileges from 'root' to 'daemon' after initializing hardware." in
    Arg.(value & opt privilege_mode_conv Maintain & info ["drop-privileges"] ~docv:"MODE" ~doc)
    
  let do_gpio_init =
    let doc = "Initialize GPIO pins (skip=don't initialize, useful for non-Pi platforms)." in
    Arg.(value & opt gpio_init_mode_conv Initialize & info ["do-gpio-init"] ~docv:"MODE" ~doc)

  (* Apply options from command line to Options.t *)
  let apply_options options
      ~rows ~cols ~chain_length ~parallel ~hardware_mapping ~brightness 
      ~pwm_bits ~pwm_lsb_nanoseconds ~pwm_dither_bits ~scan_mode ~row_address_type 
      ~multiplexing ~disable_hardware_pulsing ~show_refresh_rate ~inverse_colors
      ~led_rgb_sequence ~pixel_mapper_config ~panel_type ~limit_refresh_rate_hz 
      ~disable_busy_waiting =
    Options.set_rows options rows;
    Options.set_cols options cols;
    Options.set_chain_length options chain_length;
    Options.set_parallel options parallel;
    Options.set_hardware_mapping options hardware_mapping;
    Options.set_brightness options brightness;
    Options.set_pwm_bits options pwm_bits;
    Options.set_pwm_lsb_nanoseconds options pwm_lsb_nanoseconds;
    Options.set_pwm_dither_bits options pwm_dither_bits;
    Options.set_scan_mode options scan_mode;
    Options.set_row_address_type options row_address_type;
    Options.set_multiplexing options multiplexing;
    Options.set_disable_hardware_pulsing options disable_hardware_pulsing;
    Options.set_show_refresh_rate options show_refresh_rate;
    Options.set_inverse_colors options inverse_colors;
    Options.set_led_rgb_sequence options led_rgb_sequence;
    Options.set_pixel_mapper_config options pixel_mapper_config;
    Options.set_panel_type options panel_type;
    Options.set_limit_refresh_rate_hz options limit_refresh_rate_hz;
    Options.set_disable_busy_waiting options disable_busy_waiting;
    options

  (* Apply runtime options from command line to RuntimeOptions.t *)
  let apply_runtime_options rt_options ~gpio_slowdown ~daemon ~drop_privileges ~do_gpio_init =
    RuntimeOptions.set_gpio_slowdown rt_options gpio_slowdown;
    RuntimeOptions.set_daemon rt_options daemon;
    RuntimeOptions.set_drop_privileges rt_options drop_privileges;
    RuntimeOptions.set_do_gpio_init rt_options do_gpio_init;
    rt_options
    
  (* Term to create and populate Options.t from command line arguments *)
  let options_term =
    let combine rows cols chain_length parallel hardware_mapping brightness 
                pwm_bits pwm_lsb_nanoseconds pwm_dither_bits scan_mode row_address_type 
                multiplexing disable_hardware_pulsing show_refresh_rate inverse_colors
                led_rgb_sequence pixel_mapper_config panel_type limit_refresh_rate_hz 
                disable_busy_waiting =
      let options = Options.create () in
      apply_options options
        ~rows ~cols ~chain_length ~parallel ~hardware_mapping ~brightness 
        ~pwm_bits ~pwm_lsb_nanoseconds ~pwm_dither_bits ~scan_mode ~row_address_type 
        ~multiplexing ~disable_hardware_pulsing ~show_refresh_rate ~inverse_colors
        ~led_rgb_sequence ~pixel_mapper_config ~panel_type ~limit_refresh_rate_hz 
        ~disable_busy_waiting
    in
    Term.(const combine $ rows $ cols $ chain_length $ parallel $ hardware_mapping $ brightness 
          $ pwm_bits $ pwm_lsb_nanoseconds $ pwm_dither_bits $ scan_mode $ row_address_type 
          $ multiplexing $ disable_hardware_pulsing $ show_refresh_rate $ inverse_colors
          $ led_rgb_sequence $ pixel_mapper_config $ panel_type $ limit_refresh_rate_hz 
          $ disable_busy_waiting)

  (* Term to create and populate RuntimeOptions.t from command line arguments *)
  let runtime_options_term =
    let combine gpio_slowdown daemon drop_privileges do_gpio_init =
      let rt_options = RuntimeOptions.create () in
      apply_runtime_options rt_options ~gpio_slowdown ~daemon ~drop_privileges ~do_gpio_init
    in
    Term.(const combine $ gpio_slowdown $ daemon $ drop_privileges $ do_gpio_init)
end


