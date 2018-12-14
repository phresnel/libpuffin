When using SDL2 from within the vendor-directory (as opposed to a system-wide one as typical on Linux machines), follow this:

- download SDL2 for Windows:
  - https://www.libsdl.org/download-2.0.php
  - for MSVC, it's https://www.libsdl.org/release/SDL2-devel-2.0.8-VC.zip
  - unpack to vendor/

- above mentioned ustream does not put header files in SDL2-subdir. so:
  - go to vendor/<SDL2-dir>/include
  - move all headers into new sub-folder vendor/<SDL2-dir>/include/SDL2/