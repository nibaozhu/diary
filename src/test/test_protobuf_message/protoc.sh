protoc hello.proto --cpp_out=./
protoc hello.proto --python_out=./
ctags `find ./ -regex ".+\.\(h\|cc\)"`
