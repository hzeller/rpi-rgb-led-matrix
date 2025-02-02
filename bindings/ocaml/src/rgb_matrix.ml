open Ctypes
open Foreign
open Base

(* Bindings to the C types and functions *)
type canvas
let canvas : canvas structure typ = structure "Canvas"

type font
let font : font structure typ = structure "Font"

type matrix 
let matrix : matrix structure typ = structure "Matrix"

type rgb_led_matrix_options
let rgb_led_matrix_options : rgb_led_matrix_options structure typ = structure "RGBLedMatrixOptions"
let () = begin
  field rgb_led_matrix_options "hardware_mapping" (ptr char);
  field rgb_led_matrix_options "rows" int;
  field rgb_led_matrix_options "cols" int;
  field rgb_led_matrix_options "chain_length" int;
  field rgb_led_matrix_options "parallel" int;
  field rgb_led_matrix_options "pwm_bits" int;
  field rgb_led_matrix_options "pwm_lsb_nanoseconds" int;
  field rgb_led_matrix_options "pwm_dither_bits" int;
  field rgb_led_matrix_options "brightness" int;
  field rgb_led_matrix_options "scan_mode" int;
  field rgb_led_matrix_options "row_address_type" int;
  field rgb_led_matrix_options "multiplexing" int;
  field rgb_led_matrix_options "disable_hardware_pulsing" bool;
  field rgb_led_matrix_options "show_refresh_rate" bool;
  field rgb_led_matrix_options "inverse_colors" bool;
  field rgb_led_matrix_options "led_rgb_sequence" (ptr char);
  field rgb_led_matrix_options "pixel_mapper_config" (ptr char);
  field rgb_led_matrix_options "panel_type" (ptr char);
  field rgb_led_matrix_options "limit_refresh_rate_hz" int;
  field rgb_led_matrix_options "disable_busy_waiting" bool;
  seal rgb_led_matrix_options
end

type rgb_led_runtime_options
let rgb_led_runtime_options : rgb_led_runtime_options structure typ = structure "RGBLedRuntimeOptions"
let () = begin
  field rgb_led_runtime_options "gpio_slowdown" int;
  field rgb_led_runtime_options "daemon" int;
  field rgb_led_runtime_options "drop_privileges" int;
  field rgb_led_runtime_options "do_gpio_init" bool;
  field rgb_led_runtime_options "drop_priv_user" (ptr char);
  field rgb_led_runtime_options "drop_priv_group" (ptr char);
  seal rgb_led_runtime_options
end

(* C function bindings *)
let led_matrix_create =
  foreign "led_matrix_create" (int @-> int @-> int @-> returning (ptr matrix))
;;

let led_matrix_delete =
  foreign "led_matrix_delete" (ptr matrix @-> returning void)
;;

let led_matrix_get_canvas =
  foreign "led_matrix_get_canvas" (ptr matrix @-> returning (ptr canvas))
;;

let led_matrix_create_offscreen_canvas =
  foreign
    "led_matrix_create_offscreen_canvas"
    (ptr matrix @-> returning (ptr canvas))
;;

let led_matrix_set_brightness =
  foreign "led_matrix_set_brightness" (ptr matrix @-> int @-> returning void)
;;

let led_matrix_get_brightness =
  foreign "led_matrix_get_brightness" (ptr matrix @-> returning int)
;;

let led_matrix_create_from_options_and_rt_options =
  foreign "led_matrix_create_from_options_and_rt_options"
    (ptr rgb_led_matrix_options @-> ptr rgb_led_runtime_options @-> returning (ptr matrix))
;;

let led_canvas_set_pixel =
  foreign
    "led_canvas_set_pixel"
    (ptr canvas @-> int @-> int @-> int @-> int @-> int @-> returning void)
;;

let led_canvas_clear = foreign "led_canvas_clear" (ptr canvas @-> returning void)

let led_canvas_fill =
  foreign
    "led_canvas_fill"
    (ptr canvas @-> int @-> int @-> int @-> returning void)
;;

let load_font = foreign "load_font" (string @-> returning (ptr font))
let delete_font = foreign "delete_font" (ptr font @-> returning void)

module Options = struct
  type t = {
    hardware_mapping : string option;
    rows : int;
    cols : int;
    chain_length : int;
    parallel : int;
    pwm_bits : int;
    pwm_lsb_nanoseconds : int;
    pwm_dither_bits : int;
    brightness : int;
    scan_mode : int;
    row_address_type : int;
    multiplexing : int;
    disable_hardware_pulsing : bool;
    show_refresh_rate : bool;
    inverse_colors : bool;
    led_rgb_sequence : string option;
    pixel_mapper_config : string option;
    panel_type : string option;
    limit_refresh_rate_hz : int;
    disable_busy_waiting : bool;
  }

  let create 
      ?(hardware_mapping=None)
      ?(rows=32)
      ?(cols=32)
      ?(chain_length=1)
      ?(parallel=1)
      ?(pwm_bits=11)
      ?(pwm_lsb_nanoseconds=130)
      ?(pwm_dither_bits=0)
      ?(brightness=100)
      ?(scan_mode=0)
      ?(row_address_type=0)
      ?(multiplexing=0)
      ?(disable_hardware_pulsing=false)
      ?(show_refresh_rate=false)
      ?(inverse_colors=false)
      ?(led_rgb_sequence=None)
      ?(pixel_mapper_config=None)
      ?(panel_type=None)
      ?(limit_refresh_rate_hz=0)
      ?(disable_busy_waiting=false)
      () = {
    hardware_mapping;
    rows;
    cols;
    chain_length;
    parallel;
    pwm_bits;
    pwm_lsb_nanoseconds;
    pwm_dither_bits;
    brightness;
    scan_mode;
    row_address_type;
    multiplexing;
    disable_hardware_pulsing;
    show_refresh_rate;
    inverse_colors;
    led_rgb_sequence;
    pixel_mapper_config;
    panel_type;
    limit_refresh_rate_hz;
    disable_busy_waiting;
  }
end

module Runtime_options = struct
  type t = {
    gpio_slowdown : int;
    daemon : int;
    drop_privileges : int;
    do_gpio_init : bool;
    drop_priv_user : string option;
    drop_priv_group : string option;
  }

  let create
      ?(gpio_slowdown=1)
      ?(daemon=0)
      ?(drop_privileges=1)
      ?(do_gpio_init=true)
      ?(drop_priv_user=None)
      ?(drop_priv_group=None)
      () = {
    gpio_slowdown;
    daemon;
    drop_privileges;
    do_gpio_init;
    drop_priv_user;
    drop_priv_group;
  }
end

(* OCaml modules implementing the interface *)
module Canvas = struct
  type t = canvas structure ptr
  
  let sexp_of_t _ = Sexp.Atom "<canvas>"
  let t_of_sexp _ = failwith "Cannot create canvas from sexp"

  let set_pixel canvas ~x ~y ~r ~g ~b = led_canvas_set_pixel canvas x y r g b
  let clear canvas = led_canvas_clear canvas
  let fill canvas ~r ~g ~b = led_canvas_fill canvas r g b
end

module Font = struct
  type t = font structure ptr
  
  let sexp_of_t _ = Sexp.Atom "<font>"
  let t_of_sexp _ = failwith "Cannot create font from sexp"

  let load path = load_font path
  let destroy font = delete_font font
end

module Matrix = struct
  type t = matrix structure ptr
  
  let sexp_of_t _ = Sexp.Atom "<matrix>"
  let t_of_sexp _ = failwith "Cannot create matrix from sexp"
  let to_ptr (t : t) : unit Ctypes.ptr = Ctypes.from_voidp Ctypes.void (Ctypes.to_voidp t)

  let create ~rows ~chained ~parallel = led_matrix_create rows chained parallel
  let destroy matrix = led_matrix_delete matrix
  let get_canvas matrix = led_matrix_get_canvas matrix
  let create_offscreen_canvas matrix = led_matrix_create_offscreen_canvas matrix
  let set_brightness matrix ~brightness = led_matrix_set_brightness matrix brightness
  let get_brightness matrix = led_matrix_get_brightness matrix

  let create_from_options options rt_options =
    let opts = make rgb_led_matrix_options in
    setf opts "rows" options.Options.rows;
    setf opts "cols" options.Options.cols;
    setf opts "chain_length" options.Options.chain_length;
    setf opts "parallel" options.Options.parallel;
    setf opts "pwm_bits" options.Options.pwm_bits;
    setf opts "pwm_lsb_nanoseconds" options.Options.pwm_lsb_nanoseconds;
    setf opts "pwm_dither_bits" options.Options.pwm_dither_bits;
    setf opts "brightness" options.Options.brightness;
    setf opts "scan_mode" options.Options.scan_mode;
    setf opts "row_address_type" options.Options.row_address_type;
    setf opts "multiplexing" options.Options.multiplexing;
    setf opts "disable_hardware_pulsing" options.Options.disable_hardware_pulsing;
    setf opts "show_refresh_rate" options.Options.show_refresh_rate;
    setf opts "inverse_colors" options.Options.inverse_colors;
    setf opts "limit_refresh_rate_hz" options.Options.limit_refresh_rate_hz;
    setf opts "disable_busy_waiting" options.Options.disable_busy_waiting;
    
    (* Handle string options *)
    let hardware_mapping = match options.Options.hardware_mapping with
      | None -> coerce (ptr void) (ptr char) null
      | Some s -> coerce (ptr char) (ptr char) (allocate string s)
    in
    setf opts "hardware_mapping" hardware_mapping;
    
    let led_rgb_sequence = match options.Options.led_rgb_sequence with
      | None -> coerce (ptr void) (ptr char) null
      | Some s -> coerce (ptr char) (ptr char) (allocate string s)
    in
    setf opts "led_rgb_sequence" led_rgb_sequence;
    
    let pixel_mapper_config = match options.Options.pixel_mapper_config with
      | None -> coerce (ptr void) (ptr char) null
      | Some s -> coerce (ptr char) (ptr char) (allocate string s)
    in
    setf opts "pixel_mapper_config" pixel_mapper_config;
    
    let panel_type = match options.Options.panel_type with
      | None -> coerce (ptr void) (ptr char) null
      | Some s -> coerce (ptr char) (ptr char) (allocate string s)
    in
    setf opts "panel_type" panel_type;

    let rt_opts = match rt_options with
      | None -> coerce (ptr void) (ptr rgb_led_runtime_options) null
      | Some rt -> begin
          let rt_opts = make rgb_led_runtime_options in
          setf rt_opts "gpio_slowdown" rt.Runtime_options.gpio_slowdown;
          setf rt_opts "daemon" rt.Runtime_options.daemon;
          setf rt_opts "drop_privileges" rt.Runtime_options.drop_privileges;
          setf rt_opts "do_gpio_init" rt.Runtime_options.do_gpio_init;
          
          let drop_priv_user = match rt.Runtime_options.drop_priv_user with
            | None -> coerce (ptr void) (ptr char) null
            | Some s -> coerce (ptr char) (ptr char) (allocate string s)
          in
          setf rt_opts "drop_priv_user" drop_priv_user;
          
          let drop_priv_group = match rt.Runtime_options.drop_priv_group with
            | None -> coerce (ptr void) (ptr char) null
            | Some s -> coerce (ptr char) (ptr char) (allocate string s)
          in
          setf rt_opts "drop_priv_group" drop_priv_group;
          addr rt_opts
        end
    in
    
    led_matrix_create_from_options_and_rt_options (addr opts) rt_opts
end
