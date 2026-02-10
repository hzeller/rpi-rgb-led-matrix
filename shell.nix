# This is a nix-shell for use with the nix package manager.
# If you have nix installed, you may simply run `nix-shell`
# in this repo, and have all dependencies ready in the new shell.

{ pkgs ? import <nixpkgs> {} }:
let
  #build_used_stdenv = pkgs.stdenv;
  build_used_stdenv = pkgs.clang19Stdenv;
in
build_used_stdenv.mkDerivation {
  name = "build-environment";
  buildInputs = with pkgs;
    [
      # Provide gcc-ar or llvm-ar
      gcc15
      llvmPackages_19.llvm

      graphicsmagick
      libwebp
      ffmpeg
      pkg-config
      python311
      python311Packages.setuptools
      python311Packages.cython
      python311Packages.distutils-extra
    ];
    shellHook = ''
      export CYTHON=cython
      export NIX_ENFORCE_NO_NATIVE=0
    '';
}
