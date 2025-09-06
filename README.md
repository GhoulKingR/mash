# Mash (Mini-bASH)

A bash-like shell. Mash is short for **M**ini-b**ASH**.

## Building the project

You can build this project using CMake by following these steps:
1. Create a build directory:
```bash
mkdir build
cd build
```

2. Run the CMake command to create the Makefiles:
```bash
cmake ..
```

3. Compile the project:
```bash
make
```

4. Run the executable:
```bash
cd MiniBash
./mash
```

## What can you do in Mash

- You can use any of bash's regular commands, your installed commands:
```bash
ls -la .
pwd
tree .
brew install cmake
make
node
python3
```
- You can use the handful of already implemented commands:
```bash
exit
echo hello\ world
set NAME=/User/name/Desktop
cd $NAME
```

## What you can't do in Mash

- No configurations (at least yet)
- No command chains (e.g. `>`, `>>`, `|`, `&`, `&&`)
