A simple file scanner
install via "sudo apt install ./lab7" (given that you're in the same directory)
Parameters are:
- **dir=...** - directories to scan

- **x_dir=...** - directories NOT to scan

- **lvl=...** - set 0 not to dive into folders

- **mn_sz=...** - set minimum valid file size

- **b_sz=...** - set block size

- **hsh=...** - choose "crc16" or "crc32" (default)

- **msk=...** - select a mask for files (possible symbols: *, ?, .)