static FileTypeEnum filetype_from_extension(const char *s, size_t len)
{
    switch (len) {
    case 1:
        switch (s[0]) {
        case '1':
            return ROFF;
        case '2':
            return ROFF;
        case '3':
            return ROFF;
        case '4':
            return ROFF;
        case '5':
            return ROFF;
        case '6':
            return ROFF;
        case '7':
            return ROFF;
        case '8':
            return ROFF;
        case '9':
            return ROFF;
        case 'C':
            return CPLUSPLUS;
        case 'H':
            return CPLUSPLUS;
        case 'S':
            return ASSEMBLY;
        case 'c':
            return C;
        case 'd':
            return D;
        case 'h':
            return C;
        case 'l':
            return LEX;
        case 'm':
            return OBJECTIVEC;
        case 's':
            return ASSEMBLY;
        case 'v':
            return VERILOG;
        case 'y':
            return YACC;
        }
        return 0;
    case 2:
        switch (s[0]) {
        case 'c':
            switch (s[1]) {
            case 'c':
                return CPLUSPLUS;
            case 'l':
                return COMMONLISP;
            case 'r':
                return RUBY;
            case 's':
                return CSHARP;
            }
            return 0;
        case 'd':
            return (s[1] != 'i') ? 0 : D;
        case 'e':
            return (s[1] != 'l') ? 0 : EMACSLISP;
        case 'g':
            switch (s[1]) {
            case 'o':
                return GO;
            case 'p':
                return GNUPLOT;
            case 'v':
                return DOT;
            }
            return 0;
        case 'h':
            switch (s[1]) {
            case 'h':
                return CPLUSPLUS;
            case 's':
                return HASKELL;
            }
            return 0;
        case 'j':
            return (s[1] != 's') ? 0 : JAVASCRIPT;
        case 'm':
            switch (s[1]) {
            case '4':
                return M4;
            case 'd':
                return MARKDOWN;
            case 'k':
                return MAKE;
            }
            return 0;
        case 'p':
            switch (s[1]) {
            case 'c':
                return PKGCONFIG;
            case 'l':
                return PERL;
            case 'm':
                return PERL;
            case 'o':
                return GETTEXT;
            case 's':
                return POSTSCRIPT;
            case 'y':
                return PYTHON;
            }
            return 0;
        case 'r':
            switch (s[1]) {
            case 'b':
                return RUBY;
            case 's':
                return RUST;
            }
            return 0;
        case 's':
            switch (s[1]) {
            case 'h':
                return SHELL;
            case 's':
                return SCHEME;
            }
            return 0;
        case 't':
            return (s[1] != 's') ? 0 : TYPESCRIPT;
        case 'u':
            return (s[1] != 'i') ? 0 : XML;
        case 'v':
            return (s[1] != 'h') ? 0 : VHDL;
        }
        return 0;
    case 3:
        switch (s[0]) {
        case 'a':
            switch (s[1]) {
            case 'd':
                switch (s[2]) {
                case 'a':
                    return ADA;
                case 'b':
                    return ADA;
                case 's':
                    return ADA;
                }
                return 0;
            case 's':
                switch (s[2]) {
                case 'd':
                    return COMMONLISP;
                case 'm':
                    return ASSEMBLY;
                }
                return 0;
            case 'u':
                return (s[2] != 'k') ? 0 : AWK;
            case 'w':
                return (s[2] != 'k') ? 0 : AWK;
            }
            return 0;
        case 'b':
            switch (s[1]) {
            case 'a':
                return (s[2] != 't') ? 0 : BATCHFILE;
            case 'b':
                return (s[2] != 'l') ? 0 : TEX;
            case 'i':
                return (s[2] != 'b') ? 0 : BIBTEX;
            case 't':
                return (s[2] != 'm') ? 0 : BATCHFILE;
            }
            return 0;
        case 'c':
            switch (s[1]) {
            case '+':
                return (s[2] != '+') ? 0 : CPLUSPLUS;
            case 'l':
                switch (s[2]) {
                case 'j':
                    return CLOJURE;
                case 's':
                    return TEX;
                }
                return 0;
            case 'm':
                return (s[2] != 'd') ? 0 : BATCHFILE;
            case 'p':
                return (s[2] != 'p') ? 0 : CPLUSPLUS;
            case 's':
                switch (s[2]) {
                case 's':
                    return CSS;
                case 'v':
                    return CSV;
                }
                return 0;
            case 'x':
                return (s[2] != 'x') ? 0 : CPLUSPLUS;
            }
            return 0;
        case 'd':
            switch (s[1]) {
            case 'o':
                return (s[2] != 't') ? 0 : DOT;
            case 't':
                return (s[2] != 'x') ? 0 : TEX;
            }
            return 0;
        case 'e':
            switch (s[1]) {
            case 'm':
                return (s[2] != 'l') ? 0 : EMAIL;
            case 'p':
                return (s[2] != 's') ? 0 : POSTSCRIPT;
            }
            return 0;
        case 'g':
            return memcmp(s + 1, "pi", 2) ? 0 : GNUPLOT;
        case 'h':
            switch (s[1]) {
            case 'p':
                return (s[2] != 'p') ? 0 : CPLUSPLUS;
            case 't':
                return (s[2] != 'm') ? 0 : HTML;
            case 'x':
                return (s[2] != 'x') ? 0 : CPLUSPLUS;
            }
            return 0;
        case 'i':
            switch (s[1]) {
            case 'n':
                switch (s[2]) {
                case 'i':
                    return INI;
                case 's':
                    return TEX;
                }
                return 0;
            }
            return 0;
        case 'k':
            return memcmp(s + 1, "sh", 2) ? 0 : SHELL;
        case 'l':
            switch (s[1]) {
            case 's':
                return (s[2] != 'p') ? 0 : COMMONLISP;
            case 't':
                return (s[2] != 'x') ? 0 : TEX;
            case 'u':
                return (s[2] != 'a') ? 0 : LUA;
            }
            return 0;
        case 'm':
            switch (s[1]) {
            case 'a':
                return (s[2] != 'k') ? 0 : MAKE;
            case 'k':
                return (s[2] != 'd') ? 0 : MARKDOWN;
            }
            return 0;
        case 'n':
            switch (s[1]) {
            case 'i':
                switch (s[2]) {
                case 'm':
                    return NIM;
                case 'x':
                    return NIX;
                }
                return 0;
            }
            return 0;
        case 'p':
            switch (s[1]) {
            case 'h':
                return (s[2] != 'p') ? 0 : PHP;
            case 'l':
                switch (s[2]) {
                case 's':
                    return INI;
                case 't':
                    return GNUPLOT;
                }
                return 0;
            case 'o':
                return (s[2] != 't') ? 0 : GETTEXT;
            case 'y':
                return (s[2] != '3') ? 0 : PYTHON;
            }
            return 0;
        case 'r':
            switch (s[1]) {
            case 'd':
                return (s[2] != 'f') ? 0 : XML;
            case 'k':
                return (s[2] != 't') ? 0 : RACKET;
            case 's':
                return (s[2] != 't') ? 0 : RESTRUCTUREDTEXT;
            }
            return 0;
        case 's':
            switch (s[1]) {
            case 'c':
                return (s[2] != 'm') ? 0 : SCHEME;
            case 'e':
                return (s[2] != 'd') ? 0 : SED;
            case 'l':
                switch (s[2]) {
                case 'd':
                    return SCHEME;
                case 's':
                    return SCHEME;
                }
                return 0;
            case 'q':
                return (s[2] != 'l') ? 0 : SQL;
            case 't':
                return (s[2] != 'y') ? 0 : TEX;
            case 'v':
                return (s[2] != 'g') ? 0 : XML;
            }
            return 0;
        case 't':
            switch (s[1]) {
            case 'c':
                return (s[2] != 'l') ? 0 : TCL;
            case 'e':
                return (s[2] != 'x') ? 0 : TEX;
            case 's':
                return (s[2] != 'x') ? 0 : TYPESCRIPT;
            }
            return 0;
        case 'v':
            switch (s[1]) {
            case 'c':
                return (s[2] != 'f') ? 0 : VCARD;
            case 'e':
                return (s[2] != 'r') ? 0 : VERILOG;
            case 'h':
                return (s[2] != 'd') ? 0 : VHDL;
            case 'i':
                return (s[2] != 'm') ? 0 : VIML;
            }
            return 0;
        case 'x':
            switch (s[1]) {
            case 'm':
                return (s[2] != 'l') ? 0 : XML;
            case 's':
                switch (s[2]) {
                case 'd':
                    return XML;
                case 'l':
                    return XML;
                }
                return 0;
            }
            return 0;
        case 'y':
            return memcmp(s + 1, "ml", 2) ? 0 : YAML;
        case 'z':
            return memcmp(s + 1, "sh", 2) ? 0 : SHELL;
        }
        return 0;
    case 4:
        switch (s[0]) {
        case 'b':
            return memcmp(s + 1, "ash", 3) ? 0 : SHELL;
        case 'c':
            return memcmp(s + 1, "son", 3) ? 0 : COFFEESCRIPT;
        case 'd':
            switch (s[1]) {
            case 'a':
                return memcmp(s + 2, "rt", 2) ? 0 : DART;
            case 'i':
                return memcmp(s + 2, "ff", 2) ? 0 : DIFF;
            case 'o':
                switch (s[2]) {
                case 'a':
                    return (s[3] != 'p') ? 0 : XML;
                case 'x':
                    return (s[3] != 'y') ? 0 : DOXYGEN;
                }
                return 0;
            }
            return 0;
        case 'f':
            return memcmp(s + 1, "rag", 3) ? 0 : GLSL;
        case 'g':
            switch (s[1]) {
            case 'a':
                return memcmp(s + 2, "wk", 2) ? 0 : AWK;
            case 'l':
                return memcmp(s + 2, "sl", 2) ? 0 : GLSL;
            }
            return 0;
        case 'h':
            return memcmp(s + 1, "tml", 3) ? 0 : HTML;
        case 'j':
            switch (s[1]) {
            case 'a':
                return memcmp(s + 2, "va", 2) ? 0 : JAVA;
            case 's':
                return memcmp(s + 2, "on", 2) ? 0 : JSON;
            }
            return 0;
        case 'm':
            switch (s[1]) {
            case 'a':
                switch (s[2]) {
                case 'k':
                    return (s[3] != 'e') ? 0 : MAKE;
                case 'w':
                    return (s[3] != 'k') ? 0 : AWK;
                }
                return 0;
            case 'k':
                return memcmp(s + 2, "dn", 2) ? 0 : MARKDOWN;
            case 'o':
                return memcmp(s + 2, "on", 2) ? 0 : MOONSCRIPT;
            }
            return 0;
        case 'n':
            return memcmp(s + 1, "awk", 3) ? 0 : AWK;
        case 'p':
            switch (s[1]) {
            case 'a':
                switch (s[2]) {
                case 'g':
                    return (s[3] != 'e') ? 0 : MALLARD;
                case 't':
                    return (s[3] != 'h') ? 0 : INI;
                }
                return 0;
            case 'e':
                return memcmp(s + 2, "rl", 2) ? 0 : PERL;
            }
            return 0;
        case 'r':
            switch (s[1]) {
            case 'a':
                return memcmp(s + 2, "ke", 2) ? 0 : RUBY;
            case 'k':
                switch (s[2]) {
                case 't':
                    switch (s[3]) {
                    case 'd':
                        return RACKET;
                    case 'l':
                        return RACKET;
                    }
                    return 0;
                }
                return 0;
            }
            return 0;
        case 's':
            return memcmp(s + 1, "css", 3) ? 0 : SCSS;
        case 't':
            switch (s[1]) {
            case 'e':
                return memcmp(s + 2, "xi", 2) ? 0 : TEXINFO;
            case 'o':
                return memcmp(s + 2, "ml", 2) ? 0 : TOML;
            }
            return 0;
        case 'v':
            switch (s[1]) {
            case 'a':
                switch (s[2]) {
                case 'l':
                    return (s[3] != 'a') ? 0 : VALA;
                case 'p':
                    return (s[3] != 'i') ? 0 : VALA;
                }
                return 0;
            case 'e':
                return memcmp(s + 2, "rt", 2) ? 0 : GLSL;
            case 'h':
                return memcmp(s + 2, "dl", 2) ? 0 : VHDL;
            }
            return 0;
        case 'w':
            return memcmp(s + 1, "sgi", 3) ? 0 : PYTHON;
        case 'x':
            return memcmp(s + 1, "slt", 3) ? 0 : XML;
        case 'y':
            return memcmp(s + 1, "aml", 3) ? 0 : YAML;
        }
        return 0;
    case 5:
        switch (s[0]) {
        case 'c':
            return memcmp(s + 1, "make", 4) ? 0 : CMAKE;
        case 'd':
            return memcmp(s + 1, "terc", 4) ? 0 : DTERC;
        case 'e':
            return memcmp(s + 1, "macs", 4) ? 0 : EMACSLISP;
        case 'g':
            switch (s[1]) {
            case 'l':
                if (memcmp(s + 2, "sl", 2)) {
                    return 0;
                }
                switch (s[4]) {
                case 'f':
                    return GLSL;
                case 'v':
                    return GLSL;
                }
                return 0;
            case 'p':
                return memcmp(s + 2, "erf", 3) ? 0 : GPERF;
            }
            return 0;
        case 'm':
            return memcmp(s + 1, "ount", 4) ? 0 : INI;
        case 'n':
            switch (s[1]) {
            case 'g':
                return memcmp(s + 2, "inx", 3) ? 0 : NGINX;
            case 'i':
                return memcmp(s + 2, "nja", 3) ? 0 : NINJA;
            }
            return 0;
        case 'p':
            switch (s[1]) {
            case 'a':
                return memcmp(s + 2, "tch", 3) ? 0 : DIFF;
            case 'r':
                return memcmp(s + 2, "oto", 3) ? 0 : PROTOBUF;
            }
            return 0;
        case 's':
            switch (s[1]) {
            case 'c':
                return memcmp(s + 2, "ala", 3) ? 0 : SCALA;
            case 'l':
                return memcmp(s + 2, "ice", 3) ? 0 : INI;
            }
            return 0;
        case 't':
            return memcmp(s + 1, "imer", 4) ? 0 : INI;
        case 'v':
            return memcmp(s + 1, "card", 4) ? 0 : VCARD;
        case 'x':
            return memcmp(s + 1, "html", 4) ? 0 : HTML;
        }
        return 0;
    case 6:
        switch (s[0]) {
        case 'c':
            return memcmp(s + 1, "offee", 5) ? 0 : COFFEESCRIPT;
        case 'd':
            return memcmp(s + 1, "ocker", 5) ? 0 : DOCKER;
        case 'e':
            return memcmp(s + 1, "build", 5) ? 0 : SHELL;
        case 'g':
            return memcmp(s + 1, "roovy", 5) ? 0 : GROOVY;
        case 's':
            return memcmp(s + 1, "ocket", 5) ? 0 : INI;
        case 't':
            return memcmp(s + 1, "arget", 5) ? 0 : INI;
        }
        return 0;
    case 7:
        switch (s[0]) {
        case 'd':
            switch (s[1]) {
            case 'e':
                return memcmp(s + 2, "sktop", 5) ? 0 : INI;
            case 'o':
                return memcmp(s + 2, "cbook", 5) ? 0 : XML;
            }
            return 0;
        case 'g':
            switch (s[1]) {
            case 'e':
                switch (s[2]) {
                case 'm':
                    return memcmp(s + 3, "spec", 4) ? 0 : RUBY;
                case 'o':
                    return memcmp(s + 3, "json", 4) ? 0 : JSON;
                }
                return 0;
            case 'n':
                return memcmp(s + 2, "uplot", 5) ? 0 : GNUPLOT;
            }
            return 0;
        case 's':
            return memcmp(s + 1, "ervice", 6) ? 0 : INI;
        case 't':
            return memcmp(s + 1, "exinfo", 6) ? 0 : TEXINFO;
        }
        return 0;
    case 8:
        switch (s[0]) {
        case 'm':
            return memcmp(s + 1, "arkdown", 7) ? 0 : MARKDOWN;
        case 'r':
            return memcmp(s + 1, "ockspec", 7) ? 0 : LUA;
        case 't':
            return memcmp(s + 1, "opojson", 7) ? 0 : JSON;
        }
        return 0;
    case 9:
        switch (s[0]) {
        case 'a':
            return memcmp(s + 1, "utomount", 8) ? 0 : INI;
        case 'n':
            return memcmp(s + 1, "ginxconf", 8) ? 0 : NGINX;
        }
        return 0;
    case 10:
        switch (s[0]) {
        case 'f':
            return memcmp(s + 1, "latpakref", 9) ? 0 : INI;
        case 'p':
            return memcmp(s + 1, "ostscript", 9) ? 0 : POSTSCRIPT;
        }
        return 0;
    case 11:
        return memcmp(s, "flatpakrepo", 11) ? 0 : INI;
    }
    return 0;
}
