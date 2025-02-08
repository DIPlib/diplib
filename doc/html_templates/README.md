These HTML templates are modified from  [m.css](https://mcss.mosra.cz/).

Copyright 2020, Cris Luengo.

Copyright © 2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

---

# Changes

The biggest changes to these templates are:
- Removed dir.html and entry-dir.html
- Renamed annotated.html -> classes.html
- Renamed group.html -> module.html
- Renamed details-define.html -> details-macro.html and entry-define.html -> entry-macro.html
- Renamed details-func.html -> details-function.html and entry-func.html -> entry-function.html
- Renamed details-typedef.html -> details-alias.html and entry-typedef.html -> entry-alias.html
- Renamed details-var.html -> details-variable.html and entry-var.html -> entry-variable.html
- Lots of changes, big and small, to the templates themselves.

---

# Templates

Index pages:
- `classes.html`
- `files.html`
- `modules.html`
- `namespaces.html`
- `pages.html`

Reference pages:
- `class.html`
- `example.html` -> this is the only one we don't yet implement here
- `file.html`
- `module.html`
- `namespace.html`
- `page.html`
- `struct.html`
- `union.html`

---

# Variables

All pages can define:
- `PROJECT_NAME`
- `PROJECT_BRIEF`
- `PROJECT_VERSION`
- `MAIN_PROJECT_URL`
- `PROJECT_DOWNLOAD_URL`
- `PROJECT_LOGO`
- `STYLESHEETS`
- `FAVICON`
- `SEARCH_DISABLED`
- `SEARCH_BASE_URL`
- `SEARCH_EXTERNAL_URL`
- `SEARCH_DOWNLOAD_BINARY`
- `SEARCHDATA_FORMAT_VERSION` (required)
- `THEME_COLOR`
- `HTML_HEADER`
- `LINKS_NAVBAR1`
- `LINKS_NAVBAR2`
- `PAGE_HEADER`
- `FILENAME` (required)
- `FINE_PRINT`

All reference pages define:
- `compound`
File/module/namespace reference pages, `compound` contains:
- `compound.breadcrumb` array of (name, target, clean_name)
- `compound.inline` (for namespace only)
- `compound.since` # TODO
- `compound.include` (not for file)
- `compound.module` (for namespace only) (name, target)
- `compound.brief`
- `compound.sections`
- `compound.modules`
- `compound.namespaces`
- `compound.classes`
- `compound.enums`
- `compound.aliases`
- `compound.functions`
- `compound.variables`
- `compound.macros`
- `compound.doc`
- `compound.has_class_details`
- `compound.has_enum_details`
- `compound.has_alias_details`
- `compound.has_function_details`
- `compound.has_variable_details`
- `compound.has_macro_details`
Class/struct/union reference pages, `compound` contains:
- `compound.breadcrumb` array of (name, target, clean_name)
- `compound.member_type`
- `compound.templated`
- `compound.template_parameters` (if `compound.templated`)
- `compound.include`
- `compound.module` (name, target)
- `compound.final`
- `compound.since` # TODO
- `compound.brief`
- `compound.has_template_details` # TODO
- `compound.sections`
- `compound.base_classes`
- `compound.derived_classes`
- `compound.typeless_functions`
- `compound.groups`
- `compound.classes`
- `compound.enums`
- `compound.aliases`
- `compound.functions`
- `compound.variables`
- `compound.related`
- `compound.doc`
- `compound.has_class_details`
- `compound.has_enum_details`
- `compound.has_alias_details`
- `compound.has_function_details`
- `compound.has_variable_details`
- `compound.has_macro_details`
Page reference pages, `compound` contains:
- `compound.breadcrumb` array of (name, target, clean_name)
- `compound.footer_navigation` (prev,parent,next), with each (target, name)
- `compound.since` # TODO
- `compound.brief`
- `compound.sections`
- `compound.doc`

All index pages define:
- `index`
Classes/Namespaces index pages, `index` contains:
- `index.symbols[i].member_type`
- `index.symbols[i].page_id`
- `index.symbols[i].id`
- `index.symbols[i].name`
- `index.symbols[i].inline`
- `index.symbols[i].deprecated`
- `index.symbols[i].since` # TODO
- `index.symbols[i].brief`
- `index.symbols[i].children`
- `index.symbols[i].final`
Files index pages, `index` contains:
- `index.files[i].name`
- `index.files[i].children` # iff directory
- `index.files[i].page_id`
- `index.files[i].deprecated` # TODO
- `index.files[i].since` # TODO
- `index.files[i].brief`
Modules index pages, `index` contains:
- `index.modules[i].page_id`
- `index.modules[i].name`
- `index.modules[i].deprecated` # TODO
- `index.modules[i].since` # TODO
- `index.modules[i].brief`
- `index.modules[i].children`
Pages index pages, `index` contains:
- `index.pages[i].children`
- `index.pages[i].page_id`
- `index.pages[i].title`
- `index.pages[i].deprecated` # TODO
- `index.pages[i].since` # TODO
- `index.pages[i].brief`

For Entries:

- `alias.has_details`
- `alias.page_id`
- `alias.id`
- `alias.templated`
- `alias.template_parameters` (if `alias.templated`)
- `alias.name` & `alias.fully_qualified_name`
- `alias.type`
- `alias.deprecated`
- `alias.access`
- `alias.since` # TODO
- `alias.brief`
- `alias.include`
- `alias.oldfashioned`
- `alias.has_template_details` # TODO
- `alias.doc`

- `class.simple` (true if it only has documented variables)
- `class.has_details` (can only be true if `class.simple`)
- `class.templated`
- `class.template_parameters` (if `class.templated`)
- `class.member_type`
- `class.page_id`
- `class.name` & `class.fully_qualified_name`
- `class.access`
- `class.final`
- `class.abstract`
- `class.deprecated`
- `class.since` # TODO
- `class.brief`
- `class.variables`
- `class.doc`

- `enum.has_details`
- `enum.page_id`
- `enum.id`
- `enum.scoped`
- `enum.name` & `enum.fully_qualified_name`
- `enum.type`
- `enum.values`
- `enum.deprecated`
- `enum.access`
- `enum.since` # TODO
- `enum.brief`
- `enum.include`
- `enum.doc`
- `enum.has_value_details`

- `function.has_details`
- `function.page_id`
- `function.id`
- `function.templated`
- `function.template_parameters` (if `function.templated`)
- `function.type`
- `function.name` & `function.fully_qualified_name`
- `function.params`
- `function.deprecated`
- `function.access`
- `function.defaulted`
- `function.deleted`
- `function.explicit`
- `function.final`
- `function.override`
- `function.pure_virtual`
- `function.virtual`
- `function.constexpr`
- `function.noexcept`
- `function.since` # TODO
- `function.brief`
- `function.include`
- `function.has_template_details` # TODO
- `function.has_param_details` # TODO
- `function.return_value`
- `function.return_values`
- `function.exceptions`
- `function.doc`

- `macro.has_details`
- `macro.page_id`
- `macro.id`
- `macro.name` & `macro.fully_qualified_name`
- `macro.params`
- `macro.deprecated`
- `macro.since` # TODO
- `macro.brief`
- `macro.include`
- `macro.has_param_details` # TODO
- `macro.return_value` # TODO
- `macro.doc`

- `variable.has_details`
- `variable.page_id`
- `variable.id`
- `variable.templated`
- `variable.template_parameters` (if `variable.templated`)
- `variable.static`
- `variable.mutable`
- `variable.type`
- `variable.name` & `variable.fully_qualified_name`
- `variable.deprecated`
- `variable.access`
- `variable.constexpr`
- `variable.since` # TODO
- `variable.brief`
- `variable.include`
- `variable.has_template_details` # TODO
- `variable.doc`

- `file.page_id`
- `file.name`
- `file.deprecated` # TODO
- `file.since` # TODO
- `file.brief`

- `module.page_id`
- `module.name`
- `module.deprecated` # TODO
- `module.since` # TODO
- `module.brief`

- `namespace.page_id`
- `namespace.name`
- `namespace.inline`
- `namespace.deprecated`
- `namespace.since` # TODO
- `namespace.brief`
