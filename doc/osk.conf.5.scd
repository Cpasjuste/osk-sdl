osk-sdl(5) "osk-sdl"

# NAME

OSK-SDL - configuration file

# DESCRIPTION

OSK-SDL expects a configuration file with options in the following format, with one option/value pair per line:

	<option> = <value>

The default configuration file is at /etc/osk.conf, OSK-SDL will use this configuration file if present. If it is not
present, then an alternative configuration file must be specified on the command line when running OSK-SDL.

# OPTIONS

*wallpaper* = <color>
	The background color of the application. Colors are specified in hex: #RRGGBB.

*keyboard-background* = <color>
	The keyboard background color. Colors are specified in hex: #RRGGBB.

*keyboard-map* = us
	Keyboard map layout, currently only 'us' is supported.

*keyboard-font* = <TTF font file>
	Path to the TTF font file to use for rendering keyboard caps. This must be an absolute path to the font file.

*keyboard-font-size* = <value>
	Size of keyboard cap font, in points.

*key-foreground* = <color>
	The keyboard key cap font color. Colors are specified in hex: #RRGGBB.

*key-foreground-highlighted* = <color>
	The keyboard key font color when the key is tapped/pressed. Colors are specified in hex: #RRGGBB.

*key-background-letter* = <color>
	The background color for letter key caps on the keyboard. Colors are specified in hex: #RRGGBB.

*key-background-return* = <color>
	The background color for RETURN key cap on the keyboard. Colors are specified in hex: #RRGGBB.

*key-background-other* = <color>
	The background color for the symbols/numbers key caps on the keyboard. Colors are specified in hex: #RRGGBB.

*key-background-highlighted* = <color>
	The background color for key caps on the keyboard when they are tapped/pressed. Colors are specified in hex: #RRGGBB.

*key-radius* = <value>
	Radius, in pixels, for rounding corners of keyboard key caps. A value of 0 disables rounded corners.

*key-vibrate-duration* = <value>
	Duration, in milliseconds, for haptic vibration on key press.

*key-preview-popup* = true|false
	Enables or disables the preview popup when a key on the keyboard is touched.

*inputbox-foreground* = <color>
	The font color for input box. Colors are specified in hex: #RRGGBB.

*inputbox-foreground-error* = <color>
	The font color for the input box when displaying an error. Colors are specified in hex: #RRGGBB.

*inputbox-background* = <color>
	The background color for the input box. Colors are specified in hex: #RRGGBB.

*inputbox-background-error* = <color>
	The background color for the input box when displaying an error. Colors are specified in hex: #RRGGBB.

*inputbox-radius* = <value>
	Radius, in pixels, for rounding corners of input box. A value of 0 disables rounded corners.

*inputbox-dot-glyph* = <value>
	Font character glyph to used to represent each character of the passphrase. Can be set to "none" (without quotes)
	disable displaying information about passphrase length on screen, or set to a valid utf8 character from the
	configured font.

*animations* = true|false
	Enables or disables animations in the application. Disabling animations might help with making the application
	more responsive on certain devices.

# SEE ALSO
	*osk-sdl*(1)

# AUTHORS

*Clayton Craft* <clayton@craftyguy.net>
