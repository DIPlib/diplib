# (c)2020, Wouter Caarls
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


if __name__ == '__main__':
    if (
            ('download_bioformats' in sys.argv) or
            ('download' in sys.argv and 'bioformats' in sys.argv)
    ):
        url = 'https://downloads.openmicroscopy.org/bio-formats/7.0.0/artifacts/bioformats_package.jar'
        filename = os.path.join(os.path.dirname(__file__), 'bioformats_package.jar')
        print('Retrieving', url)
        urllib.request.urlretrieve(url, filename, progress)
        print()
