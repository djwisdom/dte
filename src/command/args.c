#include <stdlib.h>
#include <string.h>
#include "args.h"
#include "util/str-array.h"
#include "util/xstring.h"

/*
 * Flags and first "--" are removed.
 * Flag arguments are moved to beginning.
 * Other arguments come right after flag arguments.
 *
 * a->args field should be set before calling.
 * If parsing succeeds, the other fields are set and 0 is returned.
 */
ArgParseError do_parse_args(const Command *cmd, CommandArgs *a)
{
    char **args = a->args;
    BUG_ON(!args);

    const char *flag_desc = cmd->flags;
    bool flags_after_arg = !(cmd->cmdopts & CMDOPT_NO_FLAGS_AFTER_ARGS);
    size_t argc = string_array_length(args);
    size_t nr_flags = 0;
    size_t nr_flag_args = 0;

    for (size_t i = 0; args[i]; ) {
        char *arg = args[i];
        if (unlikely(streq(arg, "--"))) {
            // Move the NULL too
            memmove(args + i, args + i + 1, (argc - i) * sizeof(*args));
            free(arg);
            argc--;
            break;
        }

        if (arg[0] != '-' || arg[1] == '\0') {
            if (!flags_after_arg) {
                break;
            }
            i++;
            continue;
        }

        for (size_t j = 1; arg[j]; j++) {
            unsigned char flag = arg[j];
            const char *flagp = strchr(flag_desc, flag);
            if (unlikely(!flagp || flag == '=')) {
                a->flags[0] = flag;
                return ARGERR_INVALID_OPTION;
            }

            a->flag_set |= cmdargs_flagset_bit(flag);
            a->flags[nr_flags++] = flag;
            if (unlikely(nr_flags == ARRAYLEN(a->flags))) {
                return ARGERR_TOO_MANY_OPTIONS;
            }

            if (likely(flagp[1] != '=')) {
                continue;
            }

            if (unlikely(j > 1 || arg[j + 1])) {
                a->flags[0] = flag;
                return ARGERR_OPTION_ARGUMENT_NOT_SEPARATE;
            }

            char *flag_arg = args[i + 1];
            if (unlikely(!flag_arg)) {
                a->flags[0] = flag;
                return ARGERR_OPTION_ARGUMENT_MISSING;
            }

            // Move flag argument before any other arguments
            if (i != nr_flag_args) {
                // farg1 arg1  arg2 -f   farg2 arg3
                // farg1 farg2 arg1 arg2 arg3
                size_t count = i - nr_flag_args;
                char **ptr = args + nr_flag_args;
                memmove(ptr + 1, ptr, count * sizeof(*args));
            }
            args[nr_flag_args++] = flag_arg;
            i++;
        }

        memmove(args + i, args + i + 1, (argc - i) * sizeof(*args));
        free(arg);
        argc--;
    }

    a->flags[nr_flags] = '\0';
    a->nr_flags = nr_flags;
    a->nr_args = argc - nr_flag_args;
    a->nr_flag_args = nr_flag_args;

    if (unlikely(a->nr_args < cmd->min_args)) {
        return ARGERR_TOO_FEW_ARGUMENTS;
    }

    static_assert_compatible_types(cmd->max_args, uint8_t);
    if (unlikely(a->nr_args > cmd->max_args && cmd->max_args != 0xFF)) {
        return ARGERR_TOO_MANY_ARGUMENTS;
    }

    return 0;
}

static bool arg_parse_error_msg (
    const Command *cmd,
    const CommandArgs *a,
    ArgParseError err,
    ErrorBuffer *ebuf
) {
    const char *name = cmd->name;
    switch (err) {
    case ARGERR_INVALID_OPTION:
        return error_msg_for_cmd(ebuf, name, "Invalid option -%c", a->flags[0]);
    case ARGERR_TOO_MANY_OPTIONS:
        return error_msg_for_cmd(ebuf, name, "Too many options given");
    case ARGERR_OPTION_ARGUMENT_MISSING:
        return error_msg_for_cmd(ebuf, name, "Option -%c requires an argument", a->flags[0]);
    case ARGERR_OPTION_ARGUMENT_NOT_SEPARATE:
        return error_msg_for_cmd (
            ebuf, name,
            "Option -%c must be given separately because it requires an argument",
            a->flags[0]
        );
    case ARGERR_TOO_FEW_ARGUMENTS:
        return error_msg_for_cmd (
            ebuf, name,
            "Too few arguments (got: %zu, minimum: %u)",
            a->nr_args, (unsigned int)cmd->min_args
        );
    case ARGERR_TOO_MANY_ARGUMENTS:
        return error_msg_for_cmd (
            ebuf, name,
            "Too many arguments (got: %zu, maximum: %u)",
            a->nr_args, (unsigned int)cmd->max_args
        );
    case ARGERR_NONE:
        break;
    }

    BUG("unhandled error type");
    return false;
}

bool parse_args(const Command *cmd, CommandArgs *a, ErrorBuffer *ebuf)
{
    ArgParseError err = do_parse_args(cmd, a);
    return likely(err == ARGERR_NONE) || arg_parse_error_msg(cmd, a, err, ebuf);
}
