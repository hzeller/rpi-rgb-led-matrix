(** Rotating Block Generator

    This example demonstrates how to use the RGB LED Matrix library to create an animated
    display of a rotating colored block. The block's colors are determined by its x and y
    coordinates, creating a gradient effect that rotates around the center of the display.

    The animation works by:
    1. Setting up a matrix with the specified dimensions and configuration
    2. Computing the display area and rotation parameters
    3. For each frame:
       - Rotating the coordinate system around the center
       - Mapping spatial coordinates to colors
       - Drawing the result to the LED matrix
    
    This is a direct port of the Python example with the same name, demonstrating
    how to use the OCaml bindings to achieve the same effect.
*)

open Core
open Rgb_matrix
open Float.O

(** [scale_col val_ lo hi] maps a float value to an RGB color component (0-255).
    
    @param val_ The value to scale
    @param lo The lower bound of the input range
    @param hi The upper bound of the input range
    @return An integer in the range [0,255] representing the color intensity
*)
let scale_col val_ lo hi =
  if val_ < lo
  then 0
  else if val_ > hi
  then 255
  else Int.of_float (255.0 *. (val_ -. lo) /. (hi -. lo))
;;

(** [rotate x y sin_ cos_] rotates a point (x,y) around the origin using the given
    sine and cosine values.
    
    @param x The x coordinate to rotate
    @param y The y coordinate to rotate
    @param sin_ Sine of the rotation angle
    @param cos_ Cosine of the rotation angle
    @return A tuple (x',y') containing the rotated coordinates
*)
let rotate x y sin_ cos_ =
  let open Float in
  let x = of_int x in
  let y = of_int y in
  (x * cos_) - (y * sin_), (x * sin_) + (y * cos_)
;;

(** [run] is the main animation loop that creates and displays the rotating block.
    
    @param rows Number of rows in the LED matrix
    @param cols Number of columns in the LED matrix
    @param chain_length Number of displays daisy-chained together
    @param parallel Number of parallel chains
    @param hardware_mapping The GPIO hardware mapping to use
*)
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
  let rotate_square = Float.of_int (Int.min width height) *. 1.41 in
  let min_rotate = cent_x -. (rotate_square /. 2.0) in
  let max_rotate = cent_x +. (rotate_square /. 2.0) in
  let display_square = Float.of_int (Int.min width height) *. 0.7 in
  let min_display = cent_x -. (display_square /. 2.0) in
  let max_display = cent_x +. (display_square /. 2.0) in
  let deg_to_rad = 2.0 *. Float.pi /. 360.0 in
  let canvas = Matrix.get_canvas matrix in
  let rec loop rotation =
    let rotation = Int.O.((rotation + 1) % 360) in
    let angle = Float.of_int rotation *. deg_to_rad in
    let sin_ = Float.sin angle in
    let cos_ = Float.cos angle in
    for x = Int.of_float min_rotate to Int.of_float max_rotate do
      for y = Int.of_float min_rotate to Int.of_float max_rotate do
        let rot_x, rot_y =
          rotate
            Int.O.(x - Int.of_float cent_x)
            Int.O.(y - Int.of_float cent_x)
            sin_
            cos_
        in
        let x_col =
          let x_float = Float.of_int x in
          let y_float = Float.of_int y in
          if
            x_float >= min_display
            && x_float < max_display
            && y_float >= min_display
            && y_float < max_display
          then scale_col x_float min_display max_display
          else 0
        in
        let y_col =
          let x_float = Float.of_int x in
          let y_float = Float.of_int y in
          if
            x_float >= min_display
            && x_float < max_display
            && y_float >= min_display
            && y_float < max_display
          then scale_col y_float min_display max_display
          else 0
        in
        Canvas.set_pixel
          canvas
          ~x:(Int.of_float (rot_x +. cent_x))
          ~y:(Int.of_float (rot_y +. cent_y))
          ~r:x_col
          ~g:Int.O.(255 - y_col)
          ~b:y_col
      done
    done;
    (* Small sleep to control animation speed *)
    Core_unix.nanosleep 0.01 |> ignore;
    loop rotation
  in
  loop 0
;;

(** Command line interface for the rotating block generator.
    Provides options to configure the LED matrix dimensions and hardware setup. *)
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
     fun () -> run ~rows ~cols ~chain_length ~parallel ~hardware_mapping)
;;

let () = Command_unix.run command
