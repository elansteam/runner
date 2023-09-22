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
import safe_runner_lib

# Create an object of runner that opens file at path with args args
x = safe_runner_lib.SafeRunner("path", "args")
# Running 
while not x.IsEnded():
    print("not ended")
print(x.GetOutput())
```