{}:

with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "clam";

  buildInputs = [ 
    cmake
    tinycc
    mdcat
    framac
    why3
    alt-ergo
  ];

}

