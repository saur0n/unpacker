# unpacker
This tool can unpack:
- Nokia flash images
- Chromium resource packages
- Android sparse boot images
- some other file types (support is experimental)

## Dependencies
* [https://github.com/saur0n/libunixpp](libunix++)
* `zlib-devel` (called `libz-dev` on Ubuntu)
* `libopenssl-3-devel` (called `libssl-dev` on Ubuntu)

## Usage
At this moment, this tool can be build for Linux only.

### Unpacking Nokia firmwares
The tool can unpack files from ROFS and FPSX Nokia firmwares for BB5. Note that sometimes firmares come as EXE files: in this case FPSX and ROFS files should be exracted first (by installing the EXE file or with 7-Zip file manager).

FPSX files were used by Nokia to distribute updates for Symbian operating system. They contain different blocks inside. This command will print list of blocks inside Nokia N76 firmware and extract most valuable of them to `tmp`:
```
./unpacker -o tmp RM149_20.1.015_026_001_U54_uda.fpsx
```

ROFS files contain read-only file system for Symbian. Unpacker for ROFS files is not stable now.

## Unpacking Chrome resources
Chrome and Chromium-bases browsers contain resources packed with special format. File types of these files have extension `.pak`.

This command will extract one of Opera resource files to directory named `opera`:
```
./unpacker -o opera /usr/lib64/opera/opera_250_percent.pak
```

## Unpacking Android sparse images
This command will extract Android sparse image to the current directory:
```
./unpacker android_image.android
```

If there is an error, try [another tool](https://github.com/anestisb/android-simg2img).

## Getting information about SPI files
SPI files contain information about ECOM plugins on Symbian. Usually, SPI files are located in directory `Z:/Private/10009D8F/`.

This command will print information about SPI file:
```
./unpacker ecom.spi
```

Note that this command will not create any files.

## Unpacking Qt resource files
Qt resource files usually have extension `.rcc`. Note that the unpacker at the moment does not extract resources built into executable files.

This command will extract resources from file `installer.rcc` to directory `installer`:
```
./unpacker -o installer installer.rcc`
```
