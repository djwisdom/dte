Releasing dte
=============

Checklist
---------

1. Create release commit
   1. Update summary of changes in [`CHANGELOG.md`]
   2. Add link to release tarball and GPG signature in [`CHANGELOG.md`]
   3. Update tarball name in [`README.md`]
   4. Hard code `VERSION` variable in [`mk/build.mk`] to release version
   5. Update `RELEASE_VERSIONS` in [`tools/mk/dist.mk`]
   6. Update `<releases>` in [`share/dte.appdata.xml`]
   7. Remove `-g` from default [`CFLAGS`]
   8. `make vars bump-manpage-dates && make man`
   9. `git commit -m "Prepare v${VER} release"`

2. Tag and upload
   1. `git tag -s -m "Release v${VER}" v${VER} ${COMMIT}`
   2. `make distcheck`
   3. Copy generated tarball to `public/dist/dte/` in [releases repo]
   4. Run `make generate` in [releases repo]
   5. Commit tarball and generated signature/checksums to [releases repo]
   6. Push [releases repo] and wait for [GitLab Pages job] to finish
   7. Check tarball link in [`README.md`] works
   8. Push tag to remotes (`git push origin v${VER}`)

3. Create post-release commit
   1. Reset `VERSION` and `CFLAGS`
   2. `git commit -m 'Post-release updates'`

4. Create [GitLab release] and [GitHub release]
5. Update [AUR package]

6. Update [portable builds] in `CHANGELOG.md`
   1. Ensure [`musl-gcc`] is installed
   2. `git checkout v${VER}`
   3. `make portable`
   4. Copy generated tarball to `public/dist/dte/` in [releases repo]
   5. Run `make generate` in [releases repo]
   6. Commit and push tarball and generated files to [releases repo]
   7. `git checkout master`
   8. Update URL for [portable builds] in `CHANGELOG.md`

See Also
--------

* Example release commit: [`892aade3b33f`]
* Example post-release commit: [`4a8e255ef18a`]


[releases repo]: https://gitlab.com/craigbarnes/craigbarnes.gitlab.io/-/tree/master/public/dist/dte
[GitLab Pages job]: https://gitlab.com/craigbarnes/craigbarnes.gitlab.io/-/pipelines
[GitLab release]: https://gitlab.com/craigbarnes/dte/-/releases
[GitHub release]: https://github.com/craigbarnes/dte/releases
[AUR package]: https://aur.archlinux.org/packages/dte/
[portable builds]: https://gitlab.com/craigbarnes/dte/-/blob/master/CHANGELOG.md#portable-builds-for-linux
[`musl-gcc`]: https://www.musl-libc.org/how.html
[`892aade3b33f`]: https://gitlab.com/craigbarnes/dte/-/commit/892aade3b33fce047b89d5daaff3c5775b50452f
[`4a8e255ef18a`]: https://gitlab.com/craigbarnes/dte/-/commit/4a8e255ef18a5a12b18df37dcc99cd7e5f375639

[`CHANGELOG.md`]: ../CHANGELOG.md
[`README.md`]: ../README.md
[`mk/build.mk`]: ../mk/build.mk
[`tools/mk/dist.mk`]: ../tools/mk/dist.mk
[`share/dte.appdata.xml`]: ../share/dte.appdata.xml
[`CFLAGS`]: ../mk/build.mk
