# CLAM (Command Line Argument Machine)

CLAM is a single-header C library that gives simple tools to construct
command line parsing with explicit control flow.

### Example

```c
for (int a = 0; a < argc; a++) {
        const char *arg = argv[a];
        clam_match_result_t i = 0;
        bool longopt;

        if ((longopt = true, i = clam_match_posix_long_option(arg, "name")) ||
            (longopt = true, i = clam_match_posix_long_option(arg, "-name")) ||
            (longopt = false, i = clam_match_posix_long_option(arg, "n"))) {
                if (longopt && !clam_match_end(arg + i) && !clam_match_char(arg + i, '=')) {
                        printf("invalid trailer %s at %lu in %s\n", arg + i, i, arg);
                }
                arg += i;
                if (clam_match_end(arg)) {
                        arg = argv[++a];
                        if (a == argc) {
                                printf("name requires an argument\n");
                                return 1;
                        }

                        printf("name is %s\n", arg);
                        continue;
                }

                // ...

        }
```

## Why? (Rationale)

Existing command line argument handling libraries (such as getopt, argp, dropt)
provide a simple interface to define options, flags, commands and then
primarily act as "blackboxes" mapping command line arguments to those
definitions.

CLAM suggests a somewhat different approach. It gives you tools for convenient
command line arguments processing but leaves the control flow in your hands,
giving you more flexibility.

"Blackbox"-style libraries are effectively runtime interpreters of table-based
programs (lists of options) which leads to indirect code path.  CLAM makes
command line arguments processing straighforward, easy to follow and
inifinitely customizable.

## How do I run tests?

Simple test suite can be called with:

```
./test
```

If you have Frama-C, Why3 and some provers (Alt-Ergo recommended), you can also
try to validate the specification:

```
./test wp
```

## License

CLAM is distributed under the terms of MIT license.
