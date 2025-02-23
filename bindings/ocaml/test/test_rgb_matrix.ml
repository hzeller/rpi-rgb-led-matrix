
(*
open! Base
open! Stdio

let%expect_test "" =
  let _color = Rgb_matrix.Color.create ~r:100 ~g:0 ~b:0 () in
  print_endline "nothing to see here folk"
;;
*)
(*
   let%test_unit "matrix creation" =
  let matrix = Rgb_matrix.Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
  assert (not (Ctypes.is_null (Rgb_matrix.Matrix.to_ptr matrix)));
  Rgb_matrix.Matrix.destroy matrix
;;

let%test_unit "canvas operations" =
  let matrix = Rgb_matrix.Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
  let canvas = Rgb_matrix.Matrix.get_canvas matrix in
  Rgb_matrix.Canvas.clear canvas;
  Rgb_matrix.Canvas.fill canvas ~r:255 ~g:0 ~b:0;
  Rgb_matrix.Canvas.set_pixel canvas ~x:0 ~y:0 ~r:0 ~g:255 ~b:0;
  Rgb_matrix.Matrix.destroy matrix
;;

let%test_unit "brightness" =
  let matrix = Rgb_matrix.Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
  Rgb_matrix.Matrix.set_brightness matrix ~brightness:50;
  assert (Rgb_matrix.Matrix.get_brightness matrix = 50);
  Rgb_matrix.Matrix.destroy matrix
;;
*)
