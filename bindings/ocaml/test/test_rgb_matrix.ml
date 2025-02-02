open OUnit2
open Rgb_matrix

let test_matrix_creation _ =
  let matrix = Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
  assert_bool "Matrix creation" (not (Ctypes.is_null matrix));
  Matrix.destroy matrix
;;

let test_canvas_operations _ =
  let matrix = Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
  let canvas = Matrix.get_canvas matrix in
  (* Test clear *)
  Canvas.clear canvas;
  (* Test fill *)
  Canvas.fill canvas ~r:255 ~g:0 ~b:0;
  (* Test pixel setting *)
  Canvas.set_pixel canvas ~x:0 ~y:0 ~r:0 ~g:255 ~b:0;
  Matrix.destroy matrix
;;

let test_brightness _ =
  let matrix = Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
  Matrix.set_brightness matrix ~brightness:50;
  assert_equal 50 (Matrix.get_brightness matrix);
  Matrix.destroy matrix
;;

let suite =
  "RGB Matrix Tests"
  >::: [ "test_matrix_creation" >:: test_matrix_creation
       ; "test_canvas_operations" >:: test_canvas_operations
       ; "test_brightness" >:: test_brightness
       ]
;;

let () = run_test_tt_main suite
