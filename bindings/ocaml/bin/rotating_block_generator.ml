
let () = print_endline "hello rgb world"

let () = Rgb_matrix.print_flags ()

let () =
  let o = Rgb_matrix.Options.create () in
  Rgb_matrix.Options.set_rows o 64;
  Rgb_matrix.Options.set_cols o 64;
  Rgb_matrix.Options.set_hardware_mapping o "adafruit-hat-pwm";
  Rgb_matrix.Options.set_chain_length o 1;
  Rgb_matrix.Options.set_parallel o 1;
  Rgb_matrix.Options.set_pwm_bits o 11;
  Rgb_matrix.Options.set_pwm_lsb_nanoseconds o 130;
  Rgb_matrix.Options.set_brightness o 100;
  Rgb_matrix.Options.set_scan_mode o 0;
  Rgb_matrix.Options.set_row_address_type o 0;
  Rgb_matrix.Options.set_multiplexing o 0;
  Rgb_matrix.Options.set_disable_hardware_pulsing o false;
  Rgb_matrix.Options.set_show_refresh_rate o false;
  Rgb_matrix.Options.set_inverse_colors o false;
  ()