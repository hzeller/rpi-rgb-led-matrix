(lang dune 3.17)
(name rgb_matrix)

(using ctypes 0.3)
(generate_opam_files true)

(package
 (name rgb_matrix)
 (synopsis "OCaml bindings for rpi-rgb-led-matrix")
 (description "OCaml bindings for the Raspberry Pi RGB LED matrix library")
 (depends
  (ocaml (>= 4.13.0))
  ctypes
  core
  core_unix
  ppx_jane))
