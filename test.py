import runner_lib_py

lim = runner_lib_py.Runner.Limits()
lim.thread = 5
lim.memory = 1000 # kb
lim.time = 2000 # ms
x = runner_lib_py.Runner("/home/mf/projects/elan/runner/tester.o", "input", lim)

print(x.GetOutput().res)
print(x.GetOutput().output)
