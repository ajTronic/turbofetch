{ fetchFromGitHub, stdenv, musl, clang, lib }:
stdenv.mkDerivation rec {
  pname = "turbofetch";
  version = "1.1.0";

  src = ./.;

  buildInputs = [ musl clang ];

  buildPhase = ''
    TMPDIR=$(mktemp -d)
    cd $src
    musl-clang main.c -o $TMPDIR/turbofetch
  '';

	installPhase = ''
		mkdir -p $out/bin
		cp -r $TMPDIR/turbofetch $out/bin/
	'';
}