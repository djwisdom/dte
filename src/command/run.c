#include "run.h"
#include "args.h"
#include "parse.h"
#include "change.h"
#include "util/debug.h"
#include "util/ptr-array.h"
#include "util/xmalloc.h"

enum {
    MAX_RECURSION_DEPTH = 16, // Maximum number of `alias` expansions
};

static bool run_commands(CommandRunner *runner, const PointerArray *array);

// Recursion is limited by MAX_RECURSION_DEPTH
// NOLINTNEXTLINE(misc-no-recursion)
static bool run_command(CommandRunner *runner, char **av)
{
    const CommandSet *cmds = runner->cmds;
    const Command *cmd = cmds->lookup(av[0]);
    struct EditorState *e = runner->e;
    ErrorBuffer *ebuf = runner->ebuf;

    if (!cmd) {
        const char *name = av[0];
        if (!runner->lookup_alias) {
            return error_msg_for_cmd(ebuf, NULL, "No such command: %s", name);
        }

        const char *alias_value = runner->lookup_alias(e, name);
        if (unlikely(!alias_value)) {
            return error_msg_for_cmd(ebuf, NULL, "No such command or alias: %s", name);
        }

        PointerArray array = PTR_ARRAY_INIT;
        CommandParseError err = parse_commands(runner, &array, alias_value);
        if (unlikely(err != CMDERR_NONE)) {
            const char *err_msg = command_parse_error_to_string(err);
            ptr_array_free(&array);
            return error_msg_for_cmd(ebuf, NULL, "Parsing alias %s: %s", name, err_msg);
        }

        // Remove NULL
        array.count--;

        for (size_t i = 1; av[i]; i++) {
            ptr_array_append(&array, xstrdup(av[i]));
        }
        ptr_array_append(&array, NULL);

        bool r = run_commands(runner, &array);
        ptr_array_free(&array);
        return r;
    }

    if (unlikely(ebuf->config_filename && !(cmd->cmdopts & CMDOPT_ALLOW_IN_RC))) {
        return error_msg_for_cmd(ebuf, NULL, "Command %s not allowed in config file", cmd->name);
    }

    // Record command in macro buffer, if recording (this needs to be done
    // before parse_args() mutates the array)
    if ((runner->flags & CMDRUNNER_ALLOW_RECORDING) && runner->cmds->macro_record) {
        runner->cmds->macro_record(e, cmd, av + 1);
    }

    // By default change can't be merged with previous one.
    // Any command can override this by calling begin_change() again.
    begin_change(CHANGE_MERGE_NONE);

    CommandArgs a = cmdargs_new(av + 1);
    bool r = likely(parse_args(cmd, &a, ebuf)) && command_func_call(e, ebuf, cmd, &a);

    end_change();
    return r;
}

// Recursion is limited by MAX_RECURSION_DEPTH
// NOLINTNEXTLINE(misc-no-recursion)
static bool run_commands(CommandRunner *runner, const PointerArray *array)
{
    if (unlikely(runner->recursion_count > MAX_RECURSION_DEPTH)) {
        return error_msg_for_cmd(runner->ebuf, NULL, "alias recursion limit reached");
    }

    bool stop_at_first_err = (runner->flags & CMDRUNNER_STOP_AT_FIRST_ERROR);
    void **ptrs = array->ptrs;
    size_t len = array->count;
    size_t nfailed = 0;
    BUG_ON(len == 0);
    BUG_ON(ptrs[len - 1] != NULL);
    runner->recursion_count++;

    for (size_t s = 0, e = 0; s < len; ) {
        // Iterate over strings, until a terminating NULL is encountered
        while (ptrs[e]) {
            e++;
            BUG_ON(e >= len);
        }

        // If the value of `e` (end) changed, there's a run of at least
        // 1 string, which is a command followed by 0 or more arguments
        if (e != s) {
            if (!run_command(runner, (char**)ptrs + s)) {
                nfailed++;
                if (stop_at_first_err) {
                    goto out;
                }
            }
        }

        // Skip past the NULL, onto the next command (if any)
        s = ++e;
    }

out:
    runner->recursion_count--;
    return (nfailed == 0);
}

bool handle_command(CommandRunner *runner, const char *cmd)
{
    BUG_ON(runner->recursion_count != 0);
    PointerArray array = PTR_ARRAY_INIT;
    CommandParseError err = parse_commands(runner, &array, cmd);
    bool r;
    if (likely(err == CMDERR_NONE)) {
        r = run_commands(runner, &array);
        BUG_ON(runner->recursion_count != 0);
    } else {
        const char *str = command_parse_error_to_string(err);
        error_msg_for_cmd(runner->ebuf, NULL, "Command syntax error: %s", str);
        r = false;
    }
    ptr_array_free(&array);
    return r;
}
