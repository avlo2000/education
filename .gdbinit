set print pretty on
set print object on
set print static-members on
set print vtbl on
set print demangle on
set print array on
set print array-indexes on
set print elements 0
python
import sys
import os
# Add GCC's pretty-printers to the path
gcc_version_dirs = [
    '/mingw64/share/gcc-13/python',
    '/mingw64/share/gcc-12/python', 
    '/mingw64/share/gcc-11/python',
    '/mingw64/share/gcc-10/python'
]
for gcc_dir in gcc_version_dirs:
    if os.path.exists(gcc_dir):
        sys.path.insert(0, gcc_dir)
        break

try:
    from libstdcxx.v6.printers import register_libstdcxx_printers
    register_libstdcxx_printers(None)
    print("STL pretty-printers loaded successfully")
except ImportError:
    print("Could not load STL pretty-printers")
end
