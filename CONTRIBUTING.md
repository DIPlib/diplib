# Contributing to the *DIPlib 3* project

We are most grateful for any type of contribution you want to make, be it as simple
as submitting a bug report, or as awesome as contributing your own algorithms to the
project.

To make best use of your and our time, please read the appropriate section below
before you start work on your contribution.

If you want to contribute but do not know how, you can start by reviewing
[open issues](https://github.com/DIPlib/diplib/issues).

**Note:** Your contributions will carry the same licencing terms as the rest of the
library, and you keep the copyright to any substantial contribution.


### I want to report a bug

First, perform a [cursory search](https://github.com/DIPlib/diplib/issues?q=is%3Aissue) to see
if the problem has already been reported. If it has and the issue is still open, add a comment
to the existing issue instead of opening a new one.

Also, make sure that you are on the latest development commit (we develop in the `master` branch),
and that the bug still exists there.

Otherwise, [create a new issue](https://github.com/DIPlib/diplib/issues/new?template=bug_report.md)
using the bug report template. This template contains a guide to help you decide what information
to include. Some information might be irrelevant (for example the OS version might be irrelevant when
discussing a bug in the documentation), feel free to ignore these fields if you are sure it is not
relevant. However, it is better to include too much information than too little.

If the bug concerns code, try to make a [minimal, reproducible example](https://stackoverflow.com/help/minimal-reproducible-example),
a smallest possible program that someone else can run on their own computer and reproduce the bug.
"*Minimal*" indicates to please remove anything that is not necessary to reproduce the bug.
"*Reproducible*" indicates that the code can run as-is on someone else's computer. This is not
easy to do, and in fact can sometimes take quite a bit of time. Be certain that every bit of that
time you invested will be highly appreciated!

Note that you can attach files (e.g. source code, a small test image, an error log) that could help
describe the problem.

The issue title is best when it is concise and gives an indication of what the issue is about.

### I found an error in the documentation

If you found an error in the documentation, we consider this a bug. See above how to report it, or
below for how to submit a fix.

### I have found a bug and want to submit a patch that fixes it

First, make sure that you are on the latest development commit (we develop in the `master` branch), and
that the bug still exists there.

Next, submit a pull request, see below for instructions. Make sure that the pull request clearly
states the issue that is being addressed, and how it is being addressed. You can reference an issue
by adding the issue number as follows: `#1`. Add "`Fixes #1`" to the commit message if there
was an issue in the tracker for the bug (use the actual issue number of course).

### I want to improve code or documentation in the project

In short, fork the project, create a new branch for your contribution, and send a pull request
(see below).

For C++ code, please follow our [style guide](https://diplib.org/diplib-docs/styleguide.html).
In general, try to match the style of the file you are making edits to.

Try to contain changes to single files and/or single functions as much as possible. This makes it
easier to review the changes. When combining multiple unrelated changes in the same pull request,
please use a separate commit for each one.

Please avoid submitting changes that are purely cosmetic, though cosmetic changes are certainly
welcome if combined with functionality changes. Purely cosmetic changes often require more review
effort than the benefit provided.

If making changes to *DIPlib*, run the unit tests with `make check`, and make sure no errors are
reported.

If you are uncertain that changes you wish to submit will be useful to the project, feel free to
[create a new issue](https://github.com/DIPlib/diplib/issues/new) on GitHub to ask about it.

If there exists an issue discussing the proposed contribution, reference it as follows: `#1`.
Add "`Closes #1`" to the commit message (use the actual issue number of course).

### I have an algorithm that I would like to include in the project

We are most grateful for any relevant functionality added to the project. But we will reject
pull requests for functionality that we feel is out of the scope of the project. Please
[create a new issue](https://github.com/DIPlib/diplib/issues/new) on GitHub to discuss your
proposed contribution before putting in the effort needed to adapt your code to *DIPlib*.

We prefer if functionality is provided as C++ code in *DIPlib*, though we will also accept
MATLAB M-files into *DIPimage*. *PyDIP* is currently a thin wrapper for *DIPlib*, and we have
no infrastructure to include Python code. However, we will gladly discuss such code as well.

Any new functionality must be sufficiently documented.

For any new *DIPlib* function, please include, if possible, a Python wrapper in *PyDIP* and
a MATLAB function in *DIPimage*. We are aware that not everyone will use both (or either) of
these, and we will not reject a contribution just because these are missing.

We expect C++ code to conform to our [style guide](https://diplib.org/diplib-docs/styleguide.html).

See below for how to submit a pull request. Make sure you reference the issue used to discuss
the contribution as follows: `#1`. Add "`Closes #1`" to the commit message
(use the actual issue number of course).

### I would like to contribute a tutorial

We would love to collect tutorials demonstrating and explaining the use of *DIPlib*, *DIPimage*
and *PyDIP*. We will create a separate project to host these, once we have material to put
in it.

Please [create a new issue](https://github.com/DIPlib/diplib/issues/new) on GitHub to discuss
your proposed contribution. Being the first to contribute a tutorial, you will get a chance
to influence how our tutorial collection will look!


## How to submit a pull request (PR)

To submit a pull request, fork the project, create a new branch for your contribution (off of
the `master` branch), add one or more commits, and [send a pull request](https://github.com/DIPlib/diplib/pulls).

Make sure that your commits can be merged to the `master` branch.

Commit messages should be descriptive of the changes included in the commit. Each commit should
address a specific issue, avoid commits that fix multiple, unrelated issues. If applicable, reference
issues in the issue tracker using the syntax `#<number>` (for example, `#1` to reference issue number 1).
If a commit fixes a bug reported in an issue, add "`Fixes #<number>`" to the commit message. If the commit
closes an issue that is not a bug, add "`Closes #<number>`".

We expect C++ code to conform to our [style guide](https://diplib.org/diplib-docs/styleguide.html).

When submitting the pull request, include a meaningful title and clearly describe the contents
of the pull request. This description should include the intended goal of the contribution, and
any relevant information to help us review it. Please also include references to relevant issues
in the issue tracker.

Don't be offended if you receive requests for modifications before your work is merged with
the project. We strive for high quality code, high quality documentation, and consistent formatting.

For help forking the project, creating a branch, adding commits, using Git, or using GitHub, please
start at the [GitHub help pages](https://help.github.com).
