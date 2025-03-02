let () = print_endline "hello rgb world"

let () = Rgb_matrix.print_flags ()

module O = Rgb_matrix.Options
module R = Rgb_matrix.RuntimeOptions
module M = Rgb_matrix.Matrix
module C = Rgb_matrix.Canvas

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

let draw_simple_circle canvas =
  (* Clear the canvas to black *)
  C.clear canvas;
  
  (* Get canvas dimensions *)
  let width, height = C.get_size canvas in
  Printf.printf "Canvas size: %dx%d\n%!" width height;
  
  (* Calculate the center of the canvas *)
  let center_x = width / 2 in
  let center_y = height / 2 in
  
  (* Draw a red circle with radius 10 in the center *)
  C.draw_circle canvas ~x:center_x ~y:center_y ~radius:10 ~r:255 ~g:0 ~b:0;
  
  (* Draw a smaller green circle inside *)
  C.draw_circle canvas ~x:center_x ~y:center_y ~radius:5 ~r:0 ~g:255 ~b:0;
  
  (* Draw a blue dot at the center *)
  C.set_pixel canvas ~x:center_x ~y:center_y ~r:0 ~g:0 ~b:255

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
    
    (* Get the canvas *)
    print_endline "Getting canvas...";
    let canvas = M.get_canvas matrix in
    print_endline "Canvas acquired!";
    
    (* Draw our circle *)
    print_endline "Drawing circle...";
    draw_simple_circle canvas;
    print_endline "Circle drawn!";
    
    (* Sleep to allow viewing the circle *)
    print_endline "Displaying for 10 seconds...";
    Unix.sleep 10;
    
    print_endline "Done!";
  with e ->
    Printf.printf "Error: %s\n" (Printexc.to_string e)