let () =
  if (C.Types.foo_version <> 1) then
    failwith "foo only works with libfoo version 1";

  match C.Functions.foo_init () with
  | 0 ->
    C.Functions.foo_fnubar "fnubar!";
    C.Functions.foo_exit ()
  | err_code ->
    Printf.eprintf "foo_init failed: %d" err_code;
;;

(*
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

let rgb_led_matrix_options : rgb_led_matrix_options structure typ =
  structure "RGBLedMatrixOptions"
;;

let hardware_mapping =
  field rgb_led_matrix_options "hardware_mapping" (ptr char)
;;

let rows = field rgb_led_matrix_options "rows" int
let cols = field rgb_led_matrix_options "cols" int
let chain_length = field rgb_led_matrix_options "chain_length" int
let parallel = field rgb_led_matrix_options "parallel" int
let pwm_bits = field rgb_led_matrix_options "pwm_bits" int
let pwm_lsb_nanoseconds = field rgb_led_matrix_options "pwm_lsb_nanoseconds" int
let pwm_dither_bits = field rgb_led_matrix_options "pwm_dither_bits" int
let brightness = field rgb_led_matrix_options "brightness" int
let scan_mode = field rgb_led_matrix_options "scan_mode" int
let row_address_type = field rgb_led_matrix_options "row_address_type" int
let multiplexing = field rgb_led_matrix_options "multiplexing" int

let disable_hardware_pulsing =
  field rgb_led_matrix_options "disable_hardware_pulsing" bool
;;

let show_refresh_rate = field rgb_led_matrix_options "show_refresh_rate" bool
let inverse_colors = field rgb_led_matrix_options "inverse_colors" bool

let led_rgb_sequence =
  field rgb_led_matrix_options "led_rgb_sequence" (ptr char)
;;

let pixel_mapper_config =
  field rgb_led_matrix_options "pixel_mapper_config" (ptr char)
;;

let panel_type = field rgb_led_matrix_options "panel_type" (ptr char)

let limit_refresh_rate_hz =
  field rgb_led_matrix_options "limit_refresh_rate_hz" int
;;

let disable_busy_waiting =
  field rgb_led_matrix_options "disable_busy_waiting" bool
;;

let () = seal rgb_led_matrix_options

type rgb_led_runtime_options

let rgb_led_runtime_options : rgb_led_runtime_options structure typ =
  structure "RGBLedRuntimeOptions"
;;

let gpio_slowdown = field rgb_led_runtime_options "gpio_slowdown" int
let daemon = field rgb_led_runtime_options "daemon" int
let drop_privileges = field rgb_led_runtime_options "drop_privileges" int
let do_gpio_init = field rgb_led_runtime_options "do_gpio_init" bool
let drop_priv_user = field rgb_led_runtime_options "drop_priv_user" (ptr char)
let drop_priv_group = field rgb_led_runtime_options "drop_priv_group" (ptr char)
let () = seal rgb_led_runtime_options

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
  foreign
    "led_matrix_create_from_options_and_rt_options"
    (ptr rgb_led_matrix_options
     @-> ptr rgb_led_runtime_options
     @-> returning (ptr matrix))
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
  type t =
    { hardware_mapping : string option
    ; rows : int
    ; cols : int
    ; chain_length : int
    ; parallel : int
    ; pwm_bits : int
    ; pwm_lsb_nanoseconds : int
    ; pwm_dither_bits : int
    ; brightness : int
    ; scan_mode : int
    ; row_address_type : int
    ; multiplexing : int
    ; disable_hardware_pulsing : bool
    ; show_refresh_rate : bool
    ; inverse_colors : bool
    ; led_rgb_sequence : string option
    ; pixel_mapper_config : string option
    ; panel_type : string option
    ; limit_refresh_rate_hz : int
    ; disable_busy_waiting : bool
    }

  let create
        ?(hardware_mapping = None)
        ?(rows = 32)
        ?(cols = 32)
        ?(chain_length = 1)
        ?(parallel = 1)
        ?(pwm_bits = 11)
        ?(pwm_lsb_nanoseconds = 130)
        ?(pwm_dither_bits = 0)
        ?(brightness = 100)
        ?(scan_mode = 0)
        ?(row_address_type = 0)
        ?(multiplexing = 0)
        ?(disable_hardware_pulsing = false)
        ?(show_refresh_rate = false)
        ?(inverse_colors = false)
        ?(led_rgb_sequence = None)
        ?(pixel_mapper_config = None)
        ?(panel_type = None)
        ?(limit_refresh_rate_hz = 0)
        ?(disable_busy_waiting = false)
        ()
    =
    { hardware_mapping
    ; rows
    ; cols
    ; chain_length
    ; parallel
    ; pwm_bits
    ; pwm_lsb_nanoseconds
    ; pwm_dither_bits
    ; brightness
    ; scan_mode
    ; row_address_type
    ; multiplexing
    ; disable_hardware_pulsing
    ; show_refresh_rate
    ; inverse_colors
    ; led_rgb_sequence
    ; pixel_mapper_config
    ; panel_type
    ; limit_refresh_rate_hz
    ; disable_busy_waiting
    }
  ;;
end

module Runtime_options = struct
  type t =
    { gpio_slowdown : int
    ; daemon : int
    ; drop_privileges : int
    ; do_gpio_init : bool
    ; drop_priv_user : string option
    ; drop_priv_group : string option
    }

  let create
        ?(gpio_slowdown = 1)
        ?(daemon = 0)
        ?(drop_privileges = 1)
        ?(do_gpio_init = true)
        ?(drop_priv_user = None)
        ?(drop_priv_group = None)
        ()
    =
    { gpio_slowdown
    ; daemon
    ; drop_privileges
    ; do_gpio_init
    ; drop_priv_user
    ; drop_priv_group
    }
  ;;
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

  let to_ptr (t : t) : unit Ctypes.ptr =
    Ctypes.from_voidp Ctypes.void (Ctypes.to_voidp t)
  ;;

  let create ~rows ~chained ~parallel = led_matrix_create rows chained parallel
  let destroy matrix = led_matrix_delete matrix
  let get_canvas matrix = led_matrix_get_canvas matrix
  let create_offscreen_canvas matrix = led_matrix_create_offscreen_canvas matrix

  let set_brightness matrix ~brightness =
    led_matrix_set_brightness matrix brightness
  ;;

  let get_brightness matrix = led_matrix_get_brightness matrix

  let create_from_options options rt_options =
    let opts = make rgb_led_matrix_options in
    setf opts rows options.Options.rows;
    setf opts cols options.Options.cols;
    setf opts chain_length options.Options.chain_length;
    setf opts parallel options.Options.parallel;
    setf opts pwm_bits options.Options.pwm_bits;
    setf opts pwm_lsb_nanoseconds options.Options.pwm_lsb_nanoseconds;
    setf opts pwm_dither_bits options.Options.pwm_dither_bits;
    setf opts brightness options.Options.brightness;
    setf opts scan_mode options.Options.scan_mode;
    setf opts row_address_type options.Options.row_address_type;
    setf opts multiplexing options.Options.multiplexing;
    setf opts disable_hardware_pulsing options.Options.disable_hardware_pulsing;
    setf opts show_refresh_rate options.Options.show_refresh_rate;
    setf opts inverse_colors options.Options.inverse_colors;
    setf opts limit_refresh_rate_hz options.Options.limit_refresh_rate_hz;
    setf opts disable_busy_waiting options.Options.disable_busy_waiting;
    (* Handle string options *)
    let hardware_mapping_ptr =
      match options.Options.hardware_mapping with
      | None -> coerce (ptr void) (ptr char) null
      | Some s ->
        let p = allocate_n char ~count:(String.length s + 1) in
        String.iteri s ~f:(fun i c -> p +@ i <-@ c);
        p +@ String.length s <-@ '\000';
        coerce (ptr char) (ptr char) p
    in
    setf opts hardware_mapping hardware_mapping_ptr;
    let led_rgb_sequence_ptr =
      match options.Options.led_rgb_sequence with
      | None -> coerce (ptr void) (ptr char) null
      | Some s ->
        let p = allocate_n char ~count:(String.length s + 1) in
        String.iteri s ~f:(fun i c -> p +@ i <-@ c);
        p +@ String.length s <-@ '\000';
        coerce (ptr char) (ptr char) p
    in
    setf opts led_rgb_sequence led_rgb_sequence_ptr;
    let pixel_mapper_config_ptr =
      match options.Options.pixel_mapper_config with
      | None -> coerce (ptr void) (ptr char) null
      | Some s ->
        let p = allocate_n char ~count:(String.length s + 1) in
        String.iteri s ~f:(fun i c -> p +@ i <-@ c);
        p +@ String.length s <-@ '\000';
        coerce (ptr char) (ptr char) p
    in
    setf opts pixel_mapper_config pixel_mapper_config_ptr;
    let panel_type_ptr =
      match options.Options.panel_type with
      | None -> coerce (ptr void) (ptr char) null
      | Some s ->
        let p = allocate_n char ~count:(String.length s + 1) in
        String.iteri s ~f:(fun i c -> p +@ i <-@ c);
        p +@ String.length s <-@ '\000';
        coerce (ptr char) (ptr char) p
    in
    setf opts panel_type panel_type_ptr;
    let rt_opts =
      match rt_options with
      | None -> coerce (ptr void) (ptr rgb_led_runtime_options) null
      | Some rt ->
        let rt_opts = make rgb_led_runtime_options in
        setf rt_opts gpio_slowdown rt.Runtime_options.gpio_slowdown;
        setf rt_opts daemon rt.Runtime_options.daemon;
        setf rt_opts drop_privileges rt.Runtime_options.drop_privileges;
        setf rt_opts do_gpio_init rt.Runtime_options.do_gpio_init;
        let drop_priv_user_ptr =
          match rt.Runtime_options.drop_priv_user with
          | None -> coerce (ptr void) (ptr char) null
          | Some s ->
            let p = allocate_n char ~count:(String.length s + 1) in
            String.iteri s ~f:(fun i c -> p +@ i <-@ c);
            p +@ String.length s <-@ '\000';
            coerce (ptr char) (ptr char) p
        in
        setf rt_opts drop_priv_user drop_priv_user_ptr;
        let drop_priv_group_ptr =
          match rt.Runtime_options.drop_priv_group with
          | None -> coerce (ptr void) (ptr char) null
          | Some s ->
            let p = allocate_n char ~count:(String.length s + 1) in
            String.iteri s ~f:(fun i c -> p +@ i <-@ c);
            p +@ String.length s <-@ '\000';
            coerce (ptr char) (ptr char) p
        in
        setf rt_opts drop_priv_group drop_priv_group_ptr;
        addr rt_opts
    in
    led_matrix_create_from_options_and_rt_options (addr opts) rt_opts
  ;;
end
*)
