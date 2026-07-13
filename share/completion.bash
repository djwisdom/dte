#!/usr/bin/bash

# shellcheck disable=SC2207
_dte() {
    local dte="$1"
    local cur="$2"
    local prev="$3"

    case "$cur" in
    -)
        COMPREPLY=($(compgen -W "-b -c -h -r -s -t -B -H -K -P -R -V" -- "$cur"))
        return;;
    -[bcrstHR])
        COMPREPLY=("$cur")
        return;;
    -*) # -[hBKPV]
        return;;
    esac

    case "$prev" in
    -b)
        COMPREPLY=($(compgen -W "$($dte -B)" -- "$cur"))
        return;;
    -t)
        local format="(if (prefix? \$name \"$cur\") (list \$name #t) #f)"
        COMPREPLY=($(readtags -F "$format" -l 2>/dev/null))
        return;;
    -[chBKPV])
        return;;
    esac

    compopt -o bashdefault -o default
}

complete -F _dte dte
