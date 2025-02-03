open Core
open Rgb_matrix

let scale_col val_ lo hi =
  if Float.(val_ < lo)
  then 0
  else if Float.(val_ > hi)
  then 255
  else Int.of_float (255.0 *. (val_ -. lo) /. (hi -. lo))
;;

let rotate x y sin_ cos_ =
  let open Float in
  let x = of_int x in
  let y = of_int y in
  x * cos_ - y * sin_, x * sin_ + y * cos_
;;

let run ~rows ~cols ~chain_length ~parallel ~hardware_mapping =
  let options =
    Options.create
      ~rows
      ~cols
      ~chain_length
      ~parallel
      ~hardware_mapping:(Some hardware_mapping)
      ()
  in
  let matrix = Matrix.create_from_options options None in
  let width = cols in
  let height = rows in
  let cent_x = Float.of_int width /. 2.0 in
  let cent_y = Float.of_int height /. 2.0 in
  let rotate_square =
    Float.of_int (Int.min width height) *. 1.41
  in
  let min_rotate = cent_x -. (rotate_square /. 2.0) in
  let max_rotate = cent_x +. (rotate_square /. 2.0) in
  let display_square =
    Float.of_int (Int.min width height) *. 0.7
  in
  let min_display = cent_x -. (display_square /. 2.0) in
  let max_display = cent_x +. (display_square /. 2.0) in
  let deg_to_rad = 2.0 *. Float.pi /. 360.0 in
  let canvas = Matrix.get_canvas matrix in
  let rec loop rotation =
    let rotation = (rotation + 1) % 360 in
    let angle = Float.of_int rotation *. deg_to_rad in
    let sin_ = Float.sin angle in
    let cos_ = Float.cos angle in
    for x = Int.of_float min_rotate to Int.of_float max_rotate do
      for y = Int.of_float min_rotate to Int.of_float max_rotate do
        let rot_x, rot_y =
          rotate
            (x - Int.of_float cent_x)
            (y - Int.of_float cent_x)
            sin_
            cos_
        in
        let x_col =
          if Float.of_int x >= min_display
             && Float.of_int x < max_display
             && Float.of_int y >= min_display
             && Float.of_int y < max_display
          then
            scale_col (Float.of_int x) min_display max_display
          else 0
        in
        let y_col =
          if Float.of_int x >= min_display
             && Float.of_int x < max_display
             && Float.of_int y >= min_display
             && Float.of_int y < max_display
          then
            scale_col (Float.of_int y) min_display max_display
          else 0
        in
        Canvas.set_pixel
          canvas
          ~x:(Int.of_float (rot_x +. cent_x))
          ~y:(Int.of_float (rot_y +. cent_y))
          ~r:x_col
          ~g:(255 - y_col)
          ~b:y_col
      done
    done;
    (* Small sleep to control animation speed *)
    Core_unix.nanosleep 0.01 |> ignore;
    loop rotation
  in
  loop 0
;;

let command =
  Command.basic
    ~summary:"Display a rotating block pattern on an RGB LED matrix"
    (let%map_open.Command rows =
       flag
         "-rows"
         (optional_with_default 32 int)
         ~doc:"INT number of rows (default: 32)"
     and cols =
       flag
         "-cols"
         (optional_with_default 32 int)
         ~doc:"INT number of columns (default: 32)"
     and chain_length =
       flag
         "-chain"
         (optional_with_default 1 int)
         ~doc:"INT number of daisy-chained panels (default: 1)"
     and parallel =
       flag
         "-parallel"
         (optional_with_default 1 int)
         ~doc:"INT number of parallel chains (default: 1)"
     and hardware_mapping =
       flag
         "-hardware-mapping"
         (optional_with_default "regular" string)
         ~doc:"STRING hardware mapping (default: regular)"
     in
     fun () ->
       run ~rows ~cols ~chain_length ~parallel ~hardware_mapping)
;;

let () = Command_unix.run command
