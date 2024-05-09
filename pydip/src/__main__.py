# (c)2020-2024, Wouter Caarls, Cris Luengo
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys, os, urllib.request


def progress(blocks, bs, size):
    barsize = 52
    pct = blocks * bs / float(size)
    bardone = int(pct * barsize)
    print('[{0}{1}] {2: >3}%'.format('=' * bardone, '.' * (barsize - bardone), int(pct * 100)), end='\r', flush=True)


def print_usage_and_exit():
    print()
    print("Usage:")
    print("    python -m diplib download_bioformats [-u] [<version>]")
    print("where the optional `-u` will overwrite any existing Bio-Formats file, and the")
    print("optional `<version>` specifies which version to install. The version defaults")
    print("to `latest`, one can specify a specific version, for example:")
    print("    python -m diplib download_bioformats 7.0.0")
    print()
    exit(1)


if __name__ == '__main__':

    # Simple command-line argument parsing. If it gets more complicated, use argparse.
    index = 1  # Useless, but prevents a linter warning
    if sys.argv[1] == 'download_bioformats':
        index = 2
    elif sys.argv[1] == 'download' and sys.argv[2] == 'bioformats':
        index = 3
    else:
        print("Unknown command.")
        print_usage_and_exit()

    version = "latest"
    update = False
    for ii in range(index, len(sys.argv)):
        opt = sys.argv[ii]
        if opt[0] == '-':
            if opt == "-u":
                update = True
            else:
                print("Unknown command option.")
                print_usage_and_exit()
        else:
            version = opt

    # Do we need to do anything?
    filename = os.path.join(os.path.dirname(__file__), 'bioformats_package.jar')
    if update or not os.path.isfile(filename):
        url = f"https://downloads.openmicroscopy.org/bio-formats/{version}/artifacts/bioformats_package.jar"
        print('Retrieving', url)
        urllib.request.urlretrieve(url, filename, progress)
        print()
    else:
        print(f"Bio-Formats already present at {filename}")
        print()
