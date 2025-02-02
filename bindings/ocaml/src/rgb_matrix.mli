open Ctypes

(** RGB LED Matrix bindings *)

module Canvas : sig
  type t = private canvas structure ptr

  val set_pixel : t -> x:int -> y:int -> r:int -> g:int -> b:int -> unit
  val clear : t -> unit
  val fill : t -> r:int -> g:int -> b:int -> unit
end

module Font : sig
  type t = private font structure ptr

  val load : string -> t
  val destroy : t -> unit
end

module Matrix : sig
  type t = private matrix structure ptr

  val create : rows:int -> chained:int -> parallel:int -> t
  val destroy : t -> unit
  val get_canvas : t -> Canvas.t
  val create_offscreen_canvas : t -> Canvas.t
  val set_brightness : t -> brightness:int -> unit
  val get_brightness : t -> int
end
