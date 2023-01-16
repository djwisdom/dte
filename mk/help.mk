# TODO: Mention macOS caveats for "make install" in a footnote

# TODO: Add help info for missing targets:
#  * coverage-report
#  * dte-%.tar.gz
#  * gen-unidata
#  * show-sizes
#  * bench
#  * check-aux
#  * check-release-digests

help: private P = @printf '   %-24s %s\n'
help:
	@printf '\n Targets:\n\n'
	$P all 'Build $(dte) (default target)'
	$P vars 'Print system/build information'
	$P tags 'Create tags(5) file using ctags(1)'
	$P clean 'Remove generated files'
	$P install 'Equivalent to all 5 install-* targets below'
	$P install-bin 'Install $(dte) binary'
	$P install-man 'Install man pages'
	$P install-bash-completion 'Install bash auto-completion script'
	$P install-desktop-file 'Install desktop entry file'
	$P install-appstream 'Install AppStream metadata'
	$P uninstall 'Uninstall files installed by "make install"'
	$P 'uninstall-*' 'Uninstall files installed by "make install-*"'
	$P installcheck 'Run "make install" and sanity test installed binary'
	$P check 'Equivalent to "make check-tests check-opts"'
	$P check-tests 'Compile and run unit tests'
	$P check-opts 'Test dte(1) command-line options and error handling'
	@echo
ifeq "$(DEVMK)" "loaded"
	@printf ' Dev targets:\n\n'
	$P git-hooks 'Enable git hooks from tools/git-hooks/*'
	$P docs 'Equivalent to "make man html htmlgz"'
	$P man 'Generate man pages'
	$P html 'Generate website'
	$P htmlgz 'Generate statically gzipped website (for GitLab Pages)'
	$P pdf 'Generate PDF user manual from man pages'
	$P dist 'Generate tarball for latest git commit'
	$P dist-latest-release 'Generate tarball for latest release'
	$P dist-all-releases 'Generate tarballs for all releases'
	$P check-codespell 'Check spelling errors with codespell(1)'
	$P check-shell-scripts 'Check shell scripts with shellcheck(1)'
	$P check-whitespace 'Check source files for indent/newline errors'
	$P check-docs 'Check HTTP status of URLs found in docs'
	$P clang-tidy 'Run clang-tidy(1) checks from .clang-tidy'
	$P check-desktop-file 'Run desktop-file-validate(1) checks'
	$P check-appstream 'Run appstream-util(1) checks'
	$P distcheck 'Run "make check" on the unpacked "make dist" tarball'
	@echo
endif


.PHONY: help
