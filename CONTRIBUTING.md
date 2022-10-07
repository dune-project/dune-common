<!--
SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
-->

Contributing to the Dune Core Modules
=====================================

You've squashed an annoying bug or implemented a nifty new feature in DUNE?
And you're willing to share your improvements with the community? This page
explains how to get those changes to us and what to take care of.

Take a look at the DUNE coding style
------------------------------------

Your work will enjoy much smoother sailing if you take a look at the [Coding
Style](https://dune-project.org/dev/codingstyle/) and try to stick to it with
your changes. We understand that everyone has their personal preferences and
that there is no such thing as the *right* coding style (in the end, it's a
matter of taste), but DUNE is a pretty large project, and a consistent way of
doing things really helps a lot when trying to find your way around a body of
code as big as DUNE.

Make sure to install the Whitespace Hook before starting to work, because
our repositories enforce certain rules about whitespace and will not accept
commits that violate those rules. And a developer will be much more motivated
to merge your patch if doing so does not involve fixing a bunch of tab-based
indentations that you inadvertently added as part of your changes

Tests / Automated Testing
-------------------------

When contributing a new feature to Dune it should be accompanied by a unit test.
There are currently untested features and adding unit tests for such features
is always a good contribution.
When modifying a currently untested feature, add a unit test.

Unit tests in the Dune core modules are added in the `test` directory
of the folder containing the header with the feature code.

In Dune modules, tests are added to the
test suite (which is tested automatically with Gitlab-CI, e.g., when opening a merge request)
by adding them with the [`dune_add_test` function](https://gitlab.dune-project.org/core/dune-common/-/blob/master/cmake/modules/DuneTestMacros.cmake)
in the `CMakeLists.txt` file of the test folder.
Using `dune_add_test` ensures that the test is added to the top level `build_tests` target
which is used in the CI framework to build all tests (and can of course
also be used locally to build all tests of a Dune module, `make build_tests`).
The function also provides several convenient features such as the possibility
to automatically run a test several times with different numbers of processors.

If a specific more complex test setup is not supported by `dune_add_test`,
the test has to be added explicitly to the `build_tests` target. However,
for such cases we recommend instead to try to improve the `dune_add_test`
function to support the use case.


Use Git to your advantage
-------------------------

We know, Git can be a bit daunting at first, but trust us, it's really worth
investing half an hour to learn the basics! Even though you don't have any
commit rights to the DUNE repositories, Git still allows you to create local
commits on your machine, avoiding the usual ugly business of creating backup
copies, copying around code in files, commenting and uncommenting variants etc.
And when you're done and send the changes to us, we can simply import those
commits into our repositories. That saves a lot of time and when your changes
can be applied in five minutes using two or three commands, chances are a
developer will much more easily find the time to do so. Git is really popular,
so there are tons of tutorials all over the web. Here are some pointers:

* http://try.github.io/ is a very quick, hands-down introduction
  to Git that allows you to try out Git directly in your browser.
  Requires a GitHub account to continue at some point, though.
* http://git-scm.com/book is a very well-written and detailed resource
  for all things Git. Chapter 2 is a great introduction to Git that also explains
  a little bit how Git works, which really helps to reduce the number of
  *WTF just happened?* moments. ;-)
* http://eagain.net/articles/git-for-computer-scientists/ is a short and
  sweet explanation of what Git does at a fundamental level - just the thing for
  scientists! ;-)
* http://git-scm.com/doc/ext is a collection of both introductory and
  more in-depth Git resources.

Whatever you do, make sure to set your Git identity so that the commits tell us who authored them!

Getting the changes to us
-------------------------

You should get your changes to us in the following way:
* Get an account for [our GitLab instance](http://gitlab.dune-project.org).
* Fork the core module that you want to contribute to, just
  as you would do on GitHub.
* Push your changes to your fork on some branch.
* Open a merge request using the branch you pushed your changes
  to as the source branch and the master of the core module repository
  as the target branch. GitLab will usually ask you about opening
  a merge request if you browse it right after pushing to some branch.
* Follow the discussion on the merge request to see what improvements
  should be done to the branch before merging.

If you have any questions or complaints about this workflow of
contributing to Dune, please rant on the
[dune-devel mailing list](mailto:dune-devel@lists.dune-project.org).
