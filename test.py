import safe_runner_lib
import time

x = safe_runner_lib.SafeRunner("/home/parat07/CLionProjects/project/runner/a", "13\n")
while not x.IsEnded():
    time.sleep(0.5)

print(x.GetOutput().res)
print(x.GetOutput().output)