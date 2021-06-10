# comxd
Another serial port tool

This is my practice work with [U++](http://www.ultimatepp.org/).

## ECMA-48

This is a kind of specification which defines control functions and their coded representations for use in a 7-bit code, an extended 7-bit code, an 8-bit code or an extended 8-bit code.

It says we do not have to support all features defined in ECMA-048. Yes, it's true. If we support all of them, it must be a complicated work I've never experienced.

## vt102

- Features/Sequences were ignored
  - Auto wrap. We have unlimited line width, I like it, so we ignore line wrap
  - VT52 control sequences.
  - DECLL
  - DECTST
  - MC
  - Printer. We response "No printer"

- Keys were supported

  - Control + ...

  - Direction keys
  - HOME