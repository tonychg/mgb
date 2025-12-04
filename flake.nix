{
  inputs = {
    utils.url = "github:numtide/flake-utils";
  };
  outputs = {
    self,
    nixpkgs,
    utils,
  }:
    utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShell = pkgs.mkShell {
        nativeBuildInputs = [
            pkgs.raylib
            pkgs.cjson
            pkgs.criterion
        ];
        buildInputs = with pkgs; [
          gnumake
          lldb
        ];
      };
    });
}
