module O = Rgb_matrix.Options
module R = Rgb_matrix.RuntimeOptions
module M = Rgb_matrix.Matrix
module C = Rgb_matrix.Canvas
module Col = Rgb_matrix.Color

(* 3D vector type *)
type vec3 = { x : float; y : float; z : float }

(* Setup matrix options *)
let setup_options () =
  let o = O.create () in
  O.set_rows o 64;
  O.set_cols o 64;
  O.set_hardware_mapping o Rgb_matrix.AdafruitHat;
  O.set_chain_length o 1;
  O.set_parallel o 1;
  O.set_pwm_bits o 11;
  O.set_pwm_lsb_nanoseconds o 130;
  O.set_brightness o 100;
  O.set_scan_mode o Rgb_matrix.Progressive;
  O.set_row_address_type o Rgb_matrix.DirectRowAddress;
  O.set_multiplexing o Rgb_matrix.DirectMultiplexing;
  O.set_disable_hardware_pulsing o false;
  O.set_show_refresh_rate o false;
  O.set_inverse_colors o false;
  o

(* Define a cube centered at origin with side length 2 *)
let cube_vertices = [|
  { x = -1.0; y = -1.0; z = -1.0 };  (* 0: front bottom left *)
  { x =  1.0; y = -1.0; z = -1.0 };  (* 1: front bottom right *)
  { x =  1.0; y =  1.0; z = -1.0 };  (* 2: front top right *)
  { x = -1.0; y =  1.0; z = -1.0 };  (* 3: front top left *)
  { x = -1.0; y = -1.0; z =  1.0 };  (* 4: back bottom left *)
  { x =  1.0; y = -1.0; z =  1.0 };  (* 5: back bottom right *)
  { x =  1.0; y =  1.0; z =  1.0 };  (* 6: back top right *)
  { x = -1.0; y =  1.0; z =  1.0 };  (* 7: back top left *)
|]

(* Define cube edges as vertex index pairs *)
let cube_edges = [|
  (0, 1); (1, 2); (2, 3); (3, 0);  (* front face *)
  (4, 5); (5, 6); (6, 7); (7, 4);  (* back face *)
  (0, 4); (1, 5); (2, 6); (3, 7)   (* connecting edges *)
|]

(* Rotation matrices *)
let rotate_x theta point =
  let cos_t = cos theta in
  let sin_t = sin theta in
  {
    x = point.x;
    y = point.y *. cos_t -. point.z *. sin_t;
    z = point.y *. sin_t +. point.z *. cos_t;
  }

let rotate_y theta point =
  let cos_t = cos theta in
  let sin_t = sin theta in
  {
    x = point.x *. cos_t +. point.z *. sin_t;
    y = point.y;
    z = -.point.x *. sin_t +. point.z *. cos_t;
  }

let rotate_z theta point =
  let cos_t = cos theta in
  let sin_t = sin theta in
  {
    x = point.x *. cos_t -. point.y *. sin_t;
    y = point.x *. sin_t +. point.y *. cos_t;
    z = point.z;
  }

(* Project 3D point to 2D screen coordinates *)
let project point ~scale ~center_x ~center_y =
  (* Simple perspective projection *)
  let z_factor = 3.0 /. (5.0 +. point.z) in
  let x_projected = point.x *. scale *. z_factor in
  let y_projected = point.y *. scale *. z_factor in
  
  (* Convert to screen coordinates *)
  let screen_x = center_x + int_of_float x_projected in
  let screen_y = center_y + int_of_float y_projected in
  (screen_x, screen_y)

(* Clamp value between min and max *)
let clamp value min_val max_val =
  if value < min_val then min_val
  else if value > max_val then max_val
  else value

(* Draw the cube on the canvas *)
let draw_cube canvas vertices edges ~angle_x ~angle_y ~angle_z =
  C.clear canvas;
  
  let width, height = C.get_size canvas in
  let center_x = width / 2 in
  let center_y = height / 2 in
  
  (* Scale factor to fit the cube on the display *)
  let scale = float_of_int (min width height) *. 0.35 in
  
  (* Apply rotations to each vertex and draw the edges *)
  let transformed_vertices = Array.map (fun v ->
    v |> rotate_x angle_x |> rotate_y angle_y |> rotate_z angle_z
  ) vertices in
  
  (* Draw each edge *)
  Array.iter (fun (i, j) ->
    let v1 = transformed_vertices.(i) in
    let v2 = transformed_vertices.(j) in
    
    (* Project 3D points to 2D screen coordinates *)
    let (x1, y1) = project v1 ~scale ~center_x ~center_y in
    let (x2, y2) = project v2 ~scale ~center_x ~center_y in
    
    (* Compute color based on z-coordinate (depth) *)
    let z_avg = (v1.z +. v2.z) /. 2.0 in
    let brightness = int_of_float (255.0 *. (z_avg +. 1.0) /. 2.0) in
    let r = clamp brightness 50 255 in
    let g = clamp (brightness / 2) 0 255 in
    let b = clamp (255 - brightness) 50 255 in
    
    (* Draw the edge *)
    let x1 = clamp x1 0 (width-1) in
    let y1 = clamp y1 0 (height-1) in
    let x2 = clamp x2 0 (width-1) in
    let y2 = clamp y2 0 (height-1) in
    
    C.draw_line canvas ~x0:x1 ~y0:y1 ~x1:x2 ~y1:y2 ~r ~g ~b
  ) edges

let () =
  try
    (* Set up the options *)
    let options = setup_options () in
    let runtime_options = R.create () in
    R.set_gpio_slowdown runtime_options 2;
    Gc.compact ();
    
    (* Create the matrix *)
    print_endline "Creating matrix...";
    let matrix = M.create options runtime_options in
    print_endline "Matrix created!";
    
    (* Create an offscreen canvas for double buffering *)
    print_endline "Creating offscreen canvas...";
    let offscreen_canvas = M.create_offscreen_canvas matrix in
    print_endline "Canvas created!";
    
    (* Animation loop *)
    print_endline "Starting animation...";
    
    let start_time = Unix.gettimeofday () in
    let run_time_seconds = 60.0 in (* Run for 1 minute *)
    let rec animation_loop canvas angle_x angle_y angle_z =
      (* Draw the rotating cube *)
      draw_cube canvas cube_vertices cube_edges 
        ~angle_x ~angle_y ~angle_z;
      
      (* Swap canvases on vsync for smooth animation *)
      let canvas = M.swap_on_vsync matrix canvas in
      
      (* Increment rotation angles *)
      let angle_x' = angle_x +. 0.02 in
      let angle_y' = angle_y +. 0.03 in
      let angle_z' = angle_z +. 0.01 in
      
      (* Check if we should continue the animation *)
      let current_time = Unix.gettimeofday () in
      if current_time -. start_time < run_time_seconds then
        animation_loop canvas angle_x' angle_y' angle_z'
    in
    
    (* Start with initial angles *)
    animation_loop offscreen_canvas 0.0 0.0 0.0;
    
    print_endline "Animation done!";
    
    (* Clean up *)
    M.delete matrix;
  with e ->
    Printf.printf "Error: %s\n" (Printexc.to_string e)