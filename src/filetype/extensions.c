static const struct FileExtensionMap {
    const char ext[11];
    const uint8_t filetype; // FileTypeEnum
} extensions[] = {
    {"ada", ADA},
    {"adb", ADA},
    {"adoc", ASCIIDOC},
    {"ads", ADA},
    {"asciidoc", ASCIIDOC},
    {"asd", LISP},
    {"asm", ASM},
    {"auk", AWK},
    {"automount", INI},
    {"avsc", JSON},
    {"awk", AWK},
    {"bash", SH},
    {"bat", BATCH},
    {"bats", SH}, // https://github.com/bats-core/bats-core
    {"bbl", TEX},
    {"bib", BIBTEX},
    {"btm", BATCH},
    {"bzl", PYTHON}, // https://bazel.build/rules/language
    {"c++", CPLUSPLUS},
    {"cbl", COBOL},
    {"cc", CPLUSPLUS},
    {"cjs", JAVASCRIPT},
    {"cl", LISP},
    {"clj", CLOJURE},
    {"cls", TEX},
    {"cmake", CMAKE},
    {"cmd", BATCH},
    {"cnc", GCODE},
    {"cob", COBOL},
    {"cobol", COBOL},
    {"cocci", COCCINELLE},
    {"coffee", COFFEESCRIPT},
    {"cpp", CPLUSPLUS},
    {"cr", RUBY},
    {"cs", CSHARP},
    {"csh", CSH},
    {"cshrc", CSH},
    {"cson", COFFEESCRIPT},
    {"css", CSS},
    {"csv", CSV},
    {"cxx", CPLUSPLUS},
    {"d2", D2},
    {"dart", DART},
    {"desktop", INI},
    {"di", D},
    {"diff", DIFF},
    {"dircolors", CONFIG},
    {"doap", XML},
    {"docbook", XML},
    {"docker", DOCKER},
    {"dot", DOT},
    {"doxy", CONFIG},
    {"dterc", DTE},
    {"dts", DEVICETREE},
    {"dtsi", DEVICETREE},
    {"dtx", TEX},
    {"ebuild", SH},
    {"el", LISP},
    {"emacs", LISP},
    {"eml", MAIL},
    {"eps", POSTSCRIPT},
    {"erl", ERLANG},
    {"ex", ELIXIR},
    {"exs", ELIXIR},
    {"fish", FISH},
    {"flatpakref", INI},
    {"frag", GLSL},
    {"gawk", AWK},
    {"gco", GCODE},
    {"gcode", GCODE},
    {"gd", GDSCRIPT},
    {"gemspec", RUBY},
    {"geojson", JSON},
    {"gitignore", GITIGNORE},
    {"glsl", GLSL},
    {"glslf", GLSL},
    {"glslv", GLSL},
    {"gltf", JSON},
    {"gml", XML},
    {"gnuplot", GNUPLOT},
    {"go", GO},
    {"god", RUBY},
    {"gp", GNUPLOT},
    {"gperf", GPERF},
    {"gpi", GNUPLOT},
    {"groovy", GROOVY},
    {"grxml", XML},
    {"gsed", SED},
    {"gv", DOT},
    {"har", JSON},
    {"hh", CPLUSPLUS},
    {"hpp", CPLUSPLUS},
    {"hrl", ERLANG},
    {"hs", HASKELL},
    {"htm", HTML},
    {"html", HTML},
    {"hxx", CPLUSPLUS},
    {"ini", INI},
    {"ins", TEX},
    {"java", JAVA},
    {"jl", JULIA},
    {"js", JAVASCRIPT},
    {"jslib", JAVASCRIPT},
    {"json", JSON},
    {"jsonl", JSON},
    {"jspre", JAVASCRIPT},
    {"kml", XML},
    {"ksh", SH},
    {"kt", KOTLIN},
    {"kts", KOTLIN},
    {"latex", TEX},
    {"lsp", LISP},
    {"ltx", TEX},
    {"lua", LUA},
    {"m3u", CONFIG},
    {"m3u8", CONFIG},
    {"m4", M4},
    {"mak", MAKE},
    {"make", MAKE},
    {"markdown", MARKDOWN},
    {"mawk", AWK},
    {"mcmeta", JSON},
    {"md", MARKDOWN},
    {"mdown", MARKDOWN},
    {"mk", MAKE},
    {"mkd", MARKDOWN},
    {"mkdn", MARKDOWN},
    {"ml", OCAML},
    {"mli", OCAML},
    {"moon", MOONSCRIPT},
    {"mount", INI},
    {"nawk", AWK},
    {"nft", NFTABLES},
    {"nginx", NGINX},
    {"nginxconf", NGINX},
    {"nim", NIM},
    {"ninja", NINJA},
    {"nix", NIX},
    {"odin", ODIN},
    {"opml", XML},
    {"page", XML},
    {"pas", PASCAL},
    {"patch", DIFF},
    {"path", INI},
    {"pc", PKGCONFIG},
    {"perl", PERL},
    {"php", PHP},
    {"pl", PERL},
    {"plist", XML},
    {"pls", INI},
    {"plt", GNUPLOT},
    {"pm", PERL},
    {"po", GETTEXT},
    {"podspec", RUBY},
    {"pot", GETTEXT},
    {"pp", RUBY},
    {"proto", PROTOBUF},
    {"ps", POSTSCRIPT},
    {"ps1", POWERSHELL},
    {"psd1", POWERSHELL},
    {"psm1", POWERSHELL},
    {"py", PYTHON},
    {"py3", PYTHON},
    {"qrc", XML},
    {"rake", RUBY},
    {"rb", RUBY},
    {"rbi", RUBY},
    {"rbx", RUBY},
    {"rdf", XML},
    {"re", C}, // re2c
    {"rkt", RACKET},
    {"rktd", RACKET},
    {"rktl", RACKET},
    {"rockspec", LUA},
    {"roff", ROFF},
    {"rs", RUST},
    {"rss", XML},
    {"rst", RST},
    {"ru", RUBY},
    {"sarif", JSON},
    {"scala", SCALA},
    {"scm", SCHEME},
    {"scss", SCSS},
    {"scxml", XML},
    {"sed", SED},
    {"service", INI},
    {"sh", SH},
    {"sld", SCHEME},
    {"slice", INI},
    {"sls", SCHEME},
    {"socket", INI},
    {"spec", RPMSPEC},
    {"sql", SQL},
    {"ss", SCHEME},
    {"sty", TEX},
    {"supp", CONFIG},
    {"svg", XML},
    {"swift", SWIFT},
    {"target", INI},
    {"tcl", TCL},
    {"tcsh", CSH},
    {"tcshrc", CSH},
    {"terminfo", TERMINFO},
    {"tex", TEX},
    {"texi", TEXINFO},
    {"texinfo", TEXINFO},
    {"thor", RUBY},
    {"timer", INI},
    {"tlv", TLVERILOG},
    {"toml", TOML},
    {"topojson", JSON},
    {"tr", ROFF},
    {"ts", TYPESCRIPT},
    {"tsv", TSV},
    {"tsx", TYPESCRIPT},
    {"typ", TYPST},
    {"ui", XML},
    {"vala", VALA},
    {"vapi", VALA},
    {"vcard", VCARD},
    {"vcf", VCARD},
    {"ver", VERILOG},
    {"vert", GLSL},
    {"vh", VHDL},
    {"vhd", VHDL},
    {"vhdl", VHDL},
    {"vim", VIML},
    {"weechatlog", WEECHATLOG},
    {"wsgi", PYTHON},
    {"xbel", XML},
    {"xhtml", HTML},
    {"xml", XML},
    {"xsd", XML},
    {"xsl", XML},
    {"xslt", XML},
    {"xspf", XML},
    {"yaml", YAML},
    {"yml", YAML},
    {"zig", ZIG},
    {"zsh", SH},
};

static FileTypeEnum filetype_from_extension(const StringView ext)
{
    if (ext.length == 0 || ext.length >= sizeof(extensions[0].ext)) {
        return NONE;
    }

    if (ext.length == 1) {
        switch (ext.data[0]) {
        case '1': case '2': case '3':
        case '4': case '5': case '6':
        case '7': case '8': case '9':
            return ROFF;
        case 'c': case 'h':
            return C;
        case 'C': case 'H':
            return CPLUSPLUS;
        case 'S': case 's':
            return ASM;
        case 'd': return D;
        case 'l': return LEX;
        case 'm': return OBJC;
        case 'v': return VERILOG;
        case 'y': return YACC;
        }
        return NONE;
    }

    const struct FileExtensionMap *e = BSEARCH(&ext, extensions, ft_compare);
    return e ? e->filetype : NONE;
}
