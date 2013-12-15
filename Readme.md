
# clib-install

  clib(1)'s `install` command, ported to `c`

## Disclaimer

  This is a WIP!!

## Usage

```
  Usage: clib-install [options] [name ...]

  Options:

    -V, --version                 output program version
    -h, --help                    output help information
    -o, --out <dir>               change the output directory [deps]
```

## Examples

  Install local dependencies

    $ clib-install

  Install stephenmathieson/trim.c
  
    $ clib-install stephenmathieson/trim.c

  Install stephenmathieson/trim.c and jwerle/fs.c
  
    $ clib-install stephenmathieson/trim.c jwerle/fs.c

  Install clibs/ms@0.0.3 and clibs/file

    $ clib-install ms@0.0.3 file

  Install stephenmathieson/trim.c, along with its development dependencies

    $ clib-install stephenmathieson/trim.c --dev

## License

  MIT