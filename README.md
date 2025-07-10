# URCL LSP
A server side implementation of the Language Server Protocol for the URCL programming language.

## Current Feature Set

* Full URCL standard support
* @DEFINE support
* URCX extension support (custom ports, @DEBUG, relatives without sign, etc.)
* IRIS extension support (floats, RW, hardware stack, _ operand, custom ports)
* urcl-ld support (symbols, subobjects, multi-file editing)
* User configuration (for URCL feature set and urcl-ld includes)
* Semantic Highlighting
* Go to definition (for labels, defined constants, urcl-ld symbols)
* Completion Suggestions (for labels, constants, ports, and urcl-ld symbols)
* Error checking (partially incomplete)
* Folding ranges for urcl-ld subobjects

## To Come

* Improved error checking
* Hovers
* References

## Building

This project uses cmake for compilation and installation.

To build:
`cmake -S . -B build && cmake --build build --parallel`

To install:
`cmake --install build` 

Installation requires elevated permissions (use sudo or on windows run the command prompt as administrator).

## Dependencies

This language server depends on the c++20 lsp-framework (https://github.com/leon-bckl/lsp-framework). (automatically installed when using cmake)

Consequently, a c++20 compiler is required.

## Configuration
To find custom configuration files, URCL LSP searches from the current file location up to the root directory for a file named lsp.txt. If no file is found, the default configuration is used (all features enabled, loose error checking). Otherwise, the configuration is determined based on the first file found.

Thus, global configuration can be controlled via a lsp.txt file in the root directory.

When a file is saved, the configuration is reloaded.

A configuration file contains two types of lines. Lines may have leading or trailing whitespace. Lines starting with `#` are ignored.


#### Feature Line
A feature line allows one to enable or disable a language feature. These lines start with either a `-` or a `+`. Using a `-` will disable the feature and using a `+` will enable the feature.

Following the symbol is a feature name. The current list of features is as follows:

##### core (enabled by default)
Controls the availability of the core instructions
##### basic (enabled by default)
Controls the availability of the basic instructions
##### complex (enabled by default)
Controls the availability of the complex instructions
##### urcx (disabled by default)
Controls the availability of urcx extensions
##### iris (disabled by default)
Controls the availability of iris extensions
##### irix (disabled by default)
A shortcut for urcx + iris
##### standard (disabled by default)
Controls the enforcement of strict standard compliance
##### lowercase (disabled by default)
Controls the use of lowercase letters for text completion of case insensitive values

#### Include Line
An include line specifies a group of urcl files that will be linked together at link time. The language server will use the first include line in the config file that contains the active file to resolve definitions that span across multiple files. 

If no valid include line is found, the language server will treat the file as a standalone file.

An include line consists of a number of relative paths separated by spaces. Spaces within paths may be escaped using backslash.

## Notes

In addition to the base set of tokens, this language server also defines "escape" which corresponds to characters in escape sequences (including the leading backslash). This functionality can be disabled by passing the --no-escape command line option.

This language server should have no issue with most usages of unicode characters; however, there may be errors when using features like compound emojis.
