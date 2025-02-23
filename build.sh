#!/bin/sh
# temporary build script for ocaml
dune build -p rgb_matrix -x cross --verbose --no-buffer
