MTB-LED Firmware
================

This repository contains firmware for ATmega328pb MCU of
[MTB-LED module](https://mtb.kmz-brno.cz/).

## Build & requirements

This firmware is developed in C language, compiled via `avr-gcc` with help
of `make`. You may also find tools like `avrdude` helpful.

Hex files are available in *Releases* section.

## Programming

Firmware could be programmed

1. via programming connector on MTB-LED module (ISP),
2. over MTBbus (if bootloader is already present).

This FW uses EEPROM, however no programming of EEPROM is required. There should
be just an empty EEPROM on fresh devices.

Flash main application, fuses & bootloader:

```bash
$ cd bootloader
$ make
$ cd ..
$ make
$ make fuses
$ make program
```

Flash fuses & bootloader only:

```bash
$ make fuses
$ cd bootloader
$ make
$ make program
```

## Author's toolkit

Text editor + `make`. No more, no less.

## See also

* [MTB-LED board schematics & pcb](https://github.com/kmzbrnoI/mtb-led-hw)
* [MTB project](https://mtb.kmz-brno.cz/)
* [MTBbus protocol](https://github.com/kmzbrnoI/mtbbus-protocol)

## License

This application is released under the [Apache License v2.0
](https://www.apache.org/licenses/LICENSE-2.0).
