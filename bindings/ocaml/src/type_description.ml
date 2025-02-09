open Ctypes

module Types (F : Ctypes.TYPE) = struct
  open F

  type canvas
  let canvas : canvas structure typ = structure "LedCanvas"

  type font
  let font : font structure typ = structure "LedFont"

  type matrix
  let matrix : matrix structure typ = structure "RGBLedMatrix"

  module Options = struct
    type t
    let t : t structure typ = structure "RGBLedMatrixOptions"
    let hardware_mapping = field t "hardware_mapping" (ptr char)
    let led_rgb_sequence = field t "led_rgb_sequence" (ptr char)
    let pixel_mapper_config = field t "pixel_mapper_config" (ptr char)
    let panel_type = field t "panel_type" (ptr char)
    let rows = field t "rows" int
    let cols = field t "cols" int
    let chain_length = field t "chain_length" int
    let parallel = field t "parallel" int
    let pwm_bits = field t "pwm_bits" int
    let pwm_lsb_nanoseconds = field t "pwm_lsb_nanoseconds" int
    let pwm_dither_bits = field t "pwm_dither_bits" int
    let brightness = field t "brightness" int
    let scan_mode = field t "scan_mode" int
    let row_address_type = field t "row_address_type" int
    let multiplexing = field t "multiplexing" int
    let limit_refresh_rate_hz = field t "limit_refresh_rate_hz" int
    let disable_hardware_pulsing = field t "disable_hardware_pulsing" bool
    let show_refresh_rate = field t "show_refresh_rate" bool
    let inverse_colors = field t "inverse_colors" bool
    let disable_busy_waiting = field t "disable_busy_waiting" bool
    let () = seal t
  end

  module Color = struct
    type t
    let t : t structure typ = structure "Color"
    let r = field t "r" uint8_t
    let g = field t "g" uint8_t
    let b = field t "b" uint8_t
    let () = seal t
  end
end
