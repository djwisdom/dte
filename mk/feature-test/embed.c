// ccache currently produces stale objects when #embed is used, so deliberately
// error out for now
#error "See: https://github.com/ccache/ccache/issues/1540#issuecomment-2701575061"

#if !__has_embed("config/syntax/dte")
    #error "embed file not found; see '--embed-dir' in mk/compiler.sh"
#endif

// See: ISO C23 §6.10.3 "Binary resource inclusion"
static const char a[] = {
    #embed "config/syntax/dte" limit(9)
};

int main(void)
{
    static_assert(sizeof(a) == 9);
    return !(a[2] == 'q' && a[8] == 'c');
}
