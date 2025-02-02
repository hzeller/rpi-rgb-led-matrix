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

let led_matrix_delete = 
  foreign "led_matrix_delete" (ptr matrix @-> returning void)

let led_matrix_get_canvas =
  foreign "led_matrix_get_canvas" (ptr matrix @-> returning (ptr canvas))

let led_matrix_create_offscreen_canvas =
  foreign "led_matrix_create_offscreen_canvas" (ptr matrix @-> returning (ptr canvas))

let led_matrix_set_brightness =
  foreign "led_matrix_set_brightness" (ptr matrix @-> int @-> returning void)

let led_matrix_get_brightness =
  foreign "led_matrix_get_brightness" (ptr matrix @-> returning int)

let led_canvas_set_pixel =
  foreign "led_canvas_set_pixel" 
    (ptr canvas @-> int @-> int @-> int @-> int @-> int @-> returning void)

let led_canvas_clear =
  foreign "led_canvas_clear" (ptr canvas @-> returning void)

let led_canvas_fill =
  foreign "led_canvas_fill" (ptr canvas @-> int @-> int @-> int @-> returning void)

let load_font =
  foreign "load_font" (string @-> returning (ptr font))

let delete_font =
  foreign "delete_font" (ptr font @-> returning void)

(* OCaml modules implementing the interface *)
module Canvas = struct
  type t = canvas structure ptr

  let set_pixel canvas ~x ~y ~r ~g ~b =
    led_canvas_set_pixel canvas x y r g b

  let clear canvas =
    led_canvas_clear canvas

  let fill canvas ~r ~g ~b =
    led_canvas_fill canvas r g b

  let%test_unit "pixel values should be in valid range" =
    let matrix = Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
    let canvas = Matrix.get_canvas matrix in
    [%test_result: unit] 
      (set_pixel canvas ~x:0 ~y:0 ~r:255 ~g:255 ~b:255)
      ~expect:();
    [%test_result: unit]
      (set_pixel canvas ~x:31 ~y:31 ~r:0 ~g:0 ~b:0)
      ~expect:();
    Matrix.destroy matrix
end

module Font = struct
  type t = font structure ptr

  let load path =
    load_font path

  let destroy font =
    delete_font font

  let%test_unit "font loading and destroying" =
    let font = load "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf" in
    [%test_pred: font structure ptr] (Fn.non Ctypes.is_null) font;
    [%test_result: unit] (destroy font) ~expect:()
end

module Matrix = struct
  type t = matrix structure ptr

  let create ~rows ~chained ~parallel =
    led_matrix_create rows chained parallel

  let destroy matrix =
    led_matrix_delete matrix

  let get_canvas matrix =
    led_matrix_get_canvas matrix

  let create_offscreen_canvas matrix =
    led_matrix_create_offscreen_canvas matrix

  let set_brightness matrix ~brightness =
    led_matrix_set_brightness matrix brightness

  let get_brightness matrix =
    led_matrix_get_brightness matrix

  let%test_unit "matrix creation and basic operations" =
    let matrix = create ~rows:32 ~chained:1 ~parallel:1 in
    [%test_pred: matrix structure ptr] (Fn.non Ctypes.is_null) matrix;
    
    let canvas = get_canvas matrix in
    [%test_pred: canvas structure ptr] (Fn.non Ctypes.is_null) canvas;
    
    [%test_result: unit] (set_brightness matrix ~brightness:50) ~expect:();
    [%test_result: int] (get_brightness matrix) ~expect:50;
    
    [%test_result: unit] (destroy matrix) ~expect:()
end
