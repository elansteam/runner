import pybind11
from distutils.core import setup, Extension

ext_modules = [
    Extension(
        'safe_runner_lib', # название нашей либы
        ['src/wrapper.cpp'], # файлики которые компилируем
        include_dirs=[pybind11.get_include(), 'include'],  # не забываем добавить инклюды pybind11
        language='c++',
        extra_compile_args=['-std=c++20'],  # используем с++20
    ),
]

setup(
    name='safe_runner_lib',
    version='0.0.1',
    author='PaRat07',
    author_email='pasha.at.phone@gmail.com',
    description='library, that affords you to execute a file safely',
    ext_modules=ext_modules,
    requires=['pybind11']  # не забываем указать зависимость от pybind11
)