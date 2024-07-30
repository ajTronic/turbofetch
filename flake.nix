{
  description =
    "A flake that contains the devshell, package, and package overlay of turbofetch";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs = inputs@{ self, flake-parts, nixpkgs, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [ "x86_64-linux" "i686-linux" "aarch64-linux" ];
      flake = {
        overlays.default = final: prev: {
          turbofetch = final.callPackage ./default.nix {
            inherit (prev) fetchFromGitHub stdenv musl clang;
          };
        };
      };

      perSystem = { config, system, pkgs, ... }: {
        _module.args.pkgs = import nixpkgs {
          inherit system;
          overlays = [ self.overlays.default ];
        };

        devShells.default = pkgs.mkShell {};

        packages.turbofetch = pkgs.turbofetch;
        packages.default = config.packages.turbofetch;

        apps.turbofetch.program = "${config.packages.turbofetch}/bin/turbofetch";
        apps.default.program = config.apps.turbofetch.program;

        formatter = pkgs.nixpkgs-fmt;
      };
    };
}