# Example LLVM pass plugin

To run the example pass, first build the plugin:

1. Set the environment variable `LLVM_PATH` to the build folder of your LLVM install.
2. Configure: `cmake --preset default`
3. Build: `cmake --build build`
4. Test: `./verify.sh`

Thanks to:

- https://layle.me/posts/using-llvm-with-cmake/
- https://weliveindetail.github.io/blog/post/2017/07/17/notes-setup.html