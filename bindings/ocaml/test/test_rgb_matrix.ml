open Base

let%test_unit "matrix creation" =
  let matrix = Rgb_matrix.Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
  [%test_pred: Rgb_matrix.Matrix.t] (Fn.non Ctypes.is_null) matrix;
  Rgb_matrix.Matrix.destroy matrix

let%test_unit "canvas operations" =
  let matrix = Rgb_matrix.Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
  let canvas = Rgb_matrix.Matrix.get_canvas matrix in
  
  Canvas.clear canvas;
  Canvas.fill canvas ~r:255 ~g:0 ~b:0;
  Canvas.set_pixel canvas ~x:0 ~y:0 ~r:0 ~g:255 ~b:0;
  
  Rgb_matrix.Matrix.destroy matrix

let%test_unit "brightness" =
  let matrix = Rgb_matrix.Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
  Rgb_matrix.Matrix.set_brightness matrix ~brightness:50;
  [%test_result: int] (Rgb_matrix.Matrix.get_brightness matrix) ~expect:50;
  Rgb_matrix.Matrix.destroy matrix
