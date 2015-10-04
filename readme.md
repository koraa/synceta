# Synceta

Synchronize cache writes to persistent storage and display
statistics (like the speed, or the ETA) in the process

Like sync(1), but supports no parameters and displays
a progress bar.

## Why?

I use this mostly when transferring data to FAT formatted USB
sticks or sd cards.

## Usage

```shell
$ cd synceta
$ make # Compilation
$ sudo PREFIX=/usr/local make install # Install in the system
$ synceta
```

## TODO

* Improve signal handling: It should be able to exit easily,
  syncing should continue though
* Make the Progress Bar 'feel' stable
* Clean up all this terrible mathematical code
* Add the few missing comments
* Add packages for arch/ubuntu/debian

## License

Copyright © 2015 by Karolin Varner <karo@cupdev.net>.

License: GPLv3 or newer. See LICENSE.GPL_3.0 or visit
http://www.gnu.org/licenses/ to get a copy.

You can also buy me a drink when you meet me :)

--

This program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version
3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public
License along with this program. If not, see http://www.gnu.org/licenses/.
