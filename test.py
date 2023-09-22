import safe_runner_lib

x = safe_runner_lib.SafeRunner("path", "args")
while not x.IsEnded():
    print("not ended")
print(x.GetOutput().res)
print(x.GetOutput().output)