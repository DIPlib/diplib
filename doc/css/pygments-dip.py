#
#   This file is part of m.css.
#
#   Copyright © 2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

from pygments.style import Style
from pygments.token import Keyword, Name, Comment, String, Error, \
    Literal, Number, Operator, Other, Punctuation, Text, Generic, \
    Whitespace

class DarkStyle(Style):
    background_color = None
    highlight_color = '#cbbdb2'
    default_style = ""

    styles = {
        # C++
        Comment:                '#198019',
        Comment.Preproc:        '#666666',
        Comment.PreprocFile:    '#666666',
        Keyword:                'bold #000000',
        Name:                   '#000000',
        String:                 '#208a8a',
        String.Char:            '#208a8a',
        Number:                 '#1616e6',
        Operator:               '#000000',
        Punctuation:            "#000000",

        # CMake
        Name.Builtin:           'bold #000000',
        Name.Variable:          '#1616e6',

        # reST, HTML
        Name.Tag:               'bold #232392',
        Name.Attribute:         'bold #232392',
        Name.Class:             'bold #232392',
        Operator.Word:          'bold #232392',
        Generic.Heading:        'bold #000000',
        Generic.Emph:           'italic #191919',
        Generic.Strong:         'bold #191919',

        # Diffs
        Generic.Subheading:     '#a46226',
        Generic.Inserted:       '#00b300',
        Generic.Deleted:        '#cc0000'
    }
