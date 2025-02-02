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
end
