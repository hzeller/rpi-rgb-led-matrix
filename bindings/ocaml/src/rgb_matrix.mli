open Base

(** RGB LED Matrix bindings for controlling LED matrix displays on Raspberry Pi.
    This library provides a safe OCaml interface to the C rpi-rgb-led-matrix library.

    Basic usage:
    {[
      (* Create a new 32x32 matrix *)
      let matrix = Matrix.create ~rows:32 ~chained:1 ~parallel:1 in
      let canvas = Matrix.get_canvas matrix in
      
      (* Draw something *)
      Canvas.fill canvas ~r:0 ~g:0 ~b:0;  (* Clear to black *)
      Canvas.set_pixel canvas ~x:0 ~y:0 ~r:255 ~g:0 ~b:0;  (* Red pixel at 0,0 *)
      
      (* Clean up *)
      Matrix.destroy matrix
    ]}
*)

module Canvas : sig
  type t [@@deriving sexp]

  val set_pixel : t -> x:int -> y:int -> r:int -> g:int -> b:int -> unit
  val clear : t -> unit
  val fill : t -> r:int -> g:int -> b:int -> unit
end

module Font : sig
  type t [@@deriving sexp]

  val load : string -> t
  val destroy : t -> unit
end

module Matrix : sig
  type t [@@deriving sexp]

  val create : rows:int -> chained:int -> parallel:int -> t
  val destroy : t -> unit
  val get_canvas : t -> Canvas.t
  val create_offscreen_canvas : t -> Canvas.t
  val set_brightness : t -> brightness:int -> unit
  val get_brightness : t -> int
  
  (* For testing purposes only *)
  val to_ptr : t -> unit Ctypes.ptr
end
