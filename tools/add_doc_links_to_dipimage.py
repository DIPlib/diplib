# This script was used to add links to the DIPimage documentation.
# It finds references like "dip::Gauss", and replaces them with
# "<a href="https://diplib.org/diplib-docs/linear.html#dip-Gauss-Image-CL-Image-L-FloatArray--UnsignedArray--String-CL-StringArray-CL-dfloat-">dip::Gauss</a>".
# MATLAB displays these correctly as links on the terminal.
#
# NOTE: Do not run this script again, it will not see if there
# already is a link or not.
# We're keeping it here for future reference.

import glob
import re

from diplib.documentation_urls import doc_url_pairs

root_url = 'https://diplib.org/diplib-docs/'
doc_dict = {}
for name, url in doc_url_pairs:
    if name.startswith("dip."):  # ignore the macros and the interfaces
        name = name.casefold().replace(".", "::")
        if name in doc_dict:
            doc_dict[name].append(url)
        else:
            doc_dict[name] = [url]

dip_identifier = re.compile(r"dip::(?:\w|:)+")

mfiles = glob.glob("../dipimage/*.m")
mfiles += glob.glob("../dipimage/@dip_image/*.m")

for mf in mfiles:
    with open(mf, "rt") as fh:
        content = fh.read()
    names = dip_identifier.findall(content)
    if not names:
        continue
    for name in names:
        cname = name.casefold()
        if cname in doc_dict:
            urls = doc_dict[cname]
            if len(urls) > 1:
                print(f"File {mf} references {name}, which has more than one link. Will use first found link, edit manually if necessary.")
            url = root_url + urls[0]
            content = re.sub(f"\\b{name}\\b", f'<a href="{url}">{name}</a>', content, count=1)
        else:
            print(f"File {mf} references {name}, which I don't recognize.")
    with open(mf, "wt") as fh:
        fh.write(content)
