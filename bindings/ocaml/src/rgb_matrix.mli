open Ctypes
open Rgb_matrix_types

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
  (** Represents a drawable surface on the LED matrix. *)
  type t = private canvas structure ptr

  (** [set_pixel canvas ~x ~y ~r ~g ~b] sets the pixel at coordinates (x,y) to 
      the specified RGB color. Each color component should be in range 0-255.
      If coordinates are out of bounds, the operation is silently ignored. *)
  val set_pixel : t -> x:int -> y:int -> r:int -> g:int -> b:int -> unit

  (** [clear canvas] sets all pixels to black (0,0,0). *)
  val clear : t -> unit

  (** [fill canvas ~r ~g ~b] sets all pixels to the specified RGB color.
      Each color component should be in range 0-255. *)
  val fill : t -> r:int -> g:int -> b:int -> unit
end

module Font : sig
  (** Represents a BDF font that can be used for text rendering. *)
  type t = private font structure ptr

  (** [load path] loads a BDF font file from the specified path.
      @raise Failure if the font file cannot be loaded. *)
  val load : string -> t

  (** [destroy font] frees the resources associated with the font.
      The font should not be used after calling this function. *)
  val destroy : t -> unit
end

module Matrix : sig
  (** Represents the LED matrix hardware interface. *)
  type t = private matrix structure ptr

  (** [create ~rows ~chained ~parallel] creates a new matrix interface.
      @param rows Number of rows (height) of each matrix
      @param chained Number of matrices daisy-chained together
      @param parallel Number of parallel chains
      @raise Failure if the matrix cannot be initialized *)
  val create : rows:int -> chained:int -> parallel:int -> t

  (** [destroy matrix] frees all resources associated with the matrix.
      The matrix should not be used after calling this function. *)
  val destroy : t -> unit

  (** [get_canvas matrix] returns the primary drawing canvas for the matrix. *)
  val get_canvas : t -> Canvas.t

  (** [create_offscreen_canvas matrix] creates a new offscreen canvas for
      double-buffered drawing. Use this for smooth animations. *)
  val create_offscreen_canvas : t -> Canvas.t

  (** [set_brightness matrix ~brightness] sets the display brightness.
      @param brightness Value between 0 and 100 inclusive
      @raise Invalid_argument if brightness is out of range *)
  val set_brightness : t -> brightness:int -> unit

  (** [get_brightness matrix] returns the current brightness setting (0-100). *)
  val get_brightness : t -> int
end
