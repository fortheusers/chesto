load('//:buckaroo_macros.bzl', 'buckaroo_deps')

cxx_library(
  name = 'chesto',
  header_namespace = 'chesto',
  exported_headers = glob(['./src/*.hpp']),
  srcs = glob(['./src/*.cpp']),
  licenses = [ 'LICENSE' ],
  visibility = [ 'PUBLIC' ],
  deps = buckaroo_deps(),
)
