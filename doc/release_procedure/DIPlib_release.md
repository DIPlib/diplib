# DIPlib project Release procedure

## Summary

1. Update change log
2. Update `CMakeLists.txt`
3. Test
4. Tag master branch
5. Run the "deploy" action on GitHub
6. Build the documentation
7. Update the website
8. Create release on GitHub
9. Build DIPimage releases

### 1. Update the change log

We try to keep `./changelogs/diplib_next.md` updated as we make changes to the master branch.
When making a release, rename this file to `./changelogs/diplib_<version_number>.md`, and create
a new, clear `diplib_next.md` file from `template.md`.

Change the "title" line at the top of the file to include the version number, and the "date" line
to the current date (should match the date for the commit tagged in step #4). Remove subsections
without anything in them, and write "None" in empty sections. Please look at any of the older
files for an example of what it should look like. It's nice to have these files be consistent.

Note that this file will be copied to the website as it is when you're done with this step.

### 2. Update `CMakeLists.txt`

`./CMakeLists.txt` (the root one) has, towards the top, the following lines:
```cmake
set(PROJECT_VERSION_MAJOR "3")
set(PROJECT_VERSION_MINOR "4")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(DIP_COPYRIGHT_YEAR "2023")
```
(Of course the version number and year could be different from what is shown here.)

Update the version number and the copyright year (if appropriate).

### 3. Test

Re-build everything and run the tests. Ensure everything passes.

### 4. Tag master branch

This is the moment when the changes made to the change log and `CMakeLists.txt` files should be committed.
This new commit should then be tagged. The tag is simply the version number, the three digits with dots in between.
For beta releases add a "b1" or "b2" at the end. For example, the tag for DIPlib version 3.4.0 is `3.4.0`.

Don't forget to push the commit and the tag to GitHub if you made them locally.

```shell
git tag 3.4.0
git push
git push --tags
```

### 5. Run the "deploy" action on GitHub

On GitHub, there is an "Actions" tab ([link](https://github.com/DIPlib/diplib/actions)). On the left of this
page there is a list of workflows. Select "deploy". On the right of the page there is a button "Run workflow ▾".
Press the button, and select the newly defined tag, then run the workflow. This will build the Python packages
and upload them to PyPI.

Note that, if one or more of the packages for one architecture didn't build for whatever reason (and there always
seems to be some reason), the action can still be successful. It only seems to fail if one of the wheels couldn't
be uploaded to PyPI. So it is important to check that all the wheels are available on PyPI when this action
completes. Check the "Download files" tab for the latest version, for example https://pypi.org/project/diplib/3.5.2/#files.

If any wheel is missing, update the `.github/workflows/deploy.yml` workflow and/or the build script files under
`tools/build/` to only build and upload the missing packages (and of course fix the reason these packages were
not build). Note that you cannot upload a package that already exists on PyPI, and the action will fail if you try.

### 6. Build the documentation

Build the documentation locally. This requires a machine with LaTeX, MATLAB and a bunch of other tools.
See [documentation.md](doc/src/Build/documentation.md) for detailed instructions.
Ensure that you have the latest version of *dox++* from its GitHub repo, and that the documentation built
correctly and is complete.

Update the [`diplib-docs` repo](https://github.com/DIPlib/diplib-docs) with the new documentation.
Ensure none of the old files stick around.
GitHub will automatically update the documentation at <https://diplib.org/diplib-docs/>. 

### 7. Update the website

The website is in the [`diplib.github.io` repo](https://github.com/DIPlib/diplib.github.io).
Create a new file `_posts/<year>-<month>-<day>-Release-v<version>.md` announcing the new release.
Take a look at recent release announcements for how to format the file. In fact, you should copy the previous
announcement over and update it for the new release.

This announcement should link to the change log, which is the `./changelogs/diplib_<version_number>.md` file
we created in the first step. Copy this file into the website repo under the `_changelogs` directory.

The remainder of the website should update automatically with these new pages.

The website at <https://diplib.org/> will be updated automatically when the repo on GitHub is updated.

### 8. Create release on GitHub

The DIPlib repo on GitHub, on the right-hand side of the landing page, should have a link
to the [Releases](https://github.com/DIPlib/diplib/releases) page. On the top-right of this page, click on
"Draft a new release". Now select the new tag, copy title and description from the previous release, update
the copied text for the new release, and select "Set as the latest release" if appropriate. Then press
"Publish release" at the bottom.

### 9. Build DIPimage releases

This step is optional, and in fact we haven't done it much at all. This requires building locally on a machine
with MATLAB installed, and requires one of those machines for each supported platform.

See [DIPimage_on_Windows.md](DIPimage_on_Windows.md) and [DIPimage_on_Linux.md](DIPimage_on_Linux.md) for
details on how to build these releases.

Upload the releases to GitHub by editing the release you created in step 8, and drag-dropping the release files
onto the web page, or by clicking the big gray attachment bar on the page and selecting the files to upload.
