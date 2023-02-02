# le
"ls Extended" for Linux (not so extended in features though, but can be extended with -w option! Manually.)

# Building
```bash
./compile.sh # compile with GCC
```
# Installing
```bash
sudo ./install.sh # installs le to /usr/local/bin
```
# Usage
```bash
le ~ -w=120 -d=3 -s=0 # list home directory with 3 levels of recursion, don't show hidden files, display names in 120-character field.
le / -d=1024 # list everything on PC (of course if it allows to do so =))
```
