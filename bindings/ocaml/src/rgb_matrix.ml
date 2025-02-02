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
