CTIDYFLAGS ?= -quiet
clang_tidy_targets := $(addprefix clang-tidy-, $(all_sources))

tidy: check-clang-tidy
check-clang-tidy: $(clang_tidy_targets)

$(clang_tidy_targets): clang-tidy-%:
	$(E) TIDY $*
	$(Q) tools/clang-tidy.sh $(CTIDYFLAGS) $*


.PHONY: tidy check-clang-tidy $(clang_tidy_targets)
