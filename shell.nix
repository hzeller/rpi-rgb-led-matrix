# This is a nix-shell for use with the nix package manager.
# If you have nix installed, you may simply run `nix-shell`
# in this repo, and have all dependencies ready in the new shell.

{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = with pkgs;
    [
      graphicsmagick
      libwebp
      ffmpeg
      pkg-config
      python311
      python311Packages.cython
      python311Packages.distutils-extra
    ];
    shellHook = ''
      export CYTHON=cython
    '';
}
