DIST_VERSIONS = 1.7 1.6 1.5 1.4 1.3 1.2 1.1 1.0
DIST_ALL = $(addprefix dte-, $(addsuffix .tar.gz, $(DIST_VERSIONS)))
GIT_HOOKS = $(addprefix .git/hooks/, commit-msg pre-commit)
GPERF = gperf
GPERF_FILTER = sed -f tools/gperf-filter.sed
GPERF_GEN = $(GPERF) -m50 $(2) $(1).gperf | $(GPERF_FILTER) > $(1).c

dist: $(firstword $(DIST_ALL))
dist-all: $(DIST_ALL)
git-hooks: $(GIT_HOOKS)

check-dist: dist-all
	@sha256sum -c mk/sha256sums.txt

$(DIST_ALL): dte-%.tar.gz:
	$(E) ARCHIVE $@
	$(Q) git archive --prefix='dte-$*/' -o '$@' 'v$*'

$(GIT_HOOKS): .git/hooks/%: tools/git-hooks/%
	$(E) CP $@
	$(Q) cp $< $@

gperf-gen:
	$(call GPERF_GEN, src/filetype/extensions, -D)
	$(call GPERF_GEN, src/filetype/basenames)
	$(call GPERF_GEN, src/filetype/interpreters)
	$(call GPERF_GEN, src/filetype/ignored-exts)

show-sizes: MAKEFLAGS += \
    -j$(NPROC) --no-print-directory \
    CFLAGS=-O2\ -pipe \
    TERMINFO_DISABLE=1 \
    DEBUG=0 USE_SANITIZER=

show-sizes:
	$(MAKE) USE_LUA=no dte=build/dte-dynamic
	-$(MAKE) USE_LUA=dynamic dte=build/dte-dynamic+lua
	$(MAKE) USE_LUA=static dte=build/dte-dynamic+staticlua
	$(MAKE) USE_LUA=no LDFLAGS=-static dte=build/dte-static
	$(MAKE) USE_LUA=static LDFLAGS=-static dte=build/dte-static+lua
	-$(MAKE) CC=musl-gcc USE_LUA=no LDFLAGS=-static dte=build/dte-musl-static
	-$(MAKE) CC=musl-gcc USE_LUA=static LDFLAGS=-static dte=build/dte-musl-static+lua
	@strip build/dte-*
	@du -h build/dte-*


CLEANFILES += dte-*.tar.gz
.PHONY: dist dist-all check-dist git-hooks gperf-gen show-sizes
