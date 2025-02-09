open Ctypes

module Types (F : Ctypes.TYPE) = struct
  open F

  type canvas
  let canvas : canvas structure typ = structure "Canvas"

  type font
  let font : font structure typ = structure "Font"

  type matrix
  let matrix : matrix structure typ = structure "Matrix"

end
