# incremental-disk-cleaner

> [!CAUTION]
> Although the program cannot remove any of your files and, in theory, can only modify `~/.local/share/idc/known_paths.txt`, I cannot guarantee that it is completely safe to run. However, I use it every day, so it is likely to work fine.

> [!NOTE]
> This program will likely work on Linux only. It relies on `$HOME` and `$XDG_DATA_HOME` (or assumes `$HOME/.local/share` if `$XDG_DATA_HOME` is not set) to run.


`idc` is a simple command-line tool that helps you explore your filesystem tree and gradually identify the largest or most crowded directories/files.

Instead of scanning everything at once, it interactively guides you step by step to the "deepest" file or directory based on total size or total child count, so you can decide whether to mark it as "known" and ignore it in future runs.

This makes it easier to incrementally clean up space or understand the structure of your disk usage without being overwhelmed.

## Features

- Traverse your home directory.
- Selects the "deepest" file by following the largest/most-crowded child at each level.
- Two traversal strategies:
	- size - picks the largest directory/file by total size.
	- count - picks the directory with the most children.
- Lets you add the discovered directory/file or its parent directories to a known paths list so they won’t be considered next time.
- Stores known paths in: `~/.local/share/idc/known_paths.txt`.
- Can be run directly or installed into your `$PATH`.

## Installation

Clone the repository and use the provided build script:

```bash
git clone https://github.com/cynumini/incremental-disk-cleaner
cd incremental-disk-cleaner
./build.sh install ~/.local/bin
```

This installs the idc command into `~/.local/bin`. Make sure `~/.local/bin` is in your `$PATH`.

## Usage

You can run `idc` after installing it into your `$PATH`:

```bash
idc size
idc count
```
Or run the debug build inside the project directory:

```bash
./build.sh run size
./build.sh run count
```

## Example flow

1. Run `idc size`.
2. The program finds the largest path by recursively descending into the biggest subdirectory.
3. It displays the result (deepest file or directory).
4. You can choose to:
   1. Remove this path or its parent path using an external tool (`rm`, `mv`, or a file manager of your choice).
   2. Add this path or its parents to the known paths list at `~/.local/share/idc/know_paths.txt`. Use the `a` key to add, and `u`/`d` to navigate between parent paths.
5. Run the program again.

# License

MIT License. See [LICENSE](LICENSE) for details.