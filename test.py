import runner_lib_py

lim = runner_lib_py.Limits()
lim.thread = 5
lim.memory = 100 # kb
lim.time = 2000 # ms
x = runner_lib_py.Runner("path_to_executable", "input", lim)

print(x.GetOutput().res)
print(x.GetOutput().output)
