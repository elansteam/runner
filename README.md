## To use it with python you must:
### 1. Use linux
### 2. Install git
```sudo apt install git```
### 3. Install python
```sudo apt install python3```
### 4. Install pybind
```pip3 install pybind```
### 6. Compile library
```python3 setup.py build_ext -i```
### 7. Include in your program library 
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