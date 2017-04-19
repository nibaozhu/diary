protoc hello.proto --cpp_out=./
ctags `find ./ -regex ".+\.\(h\|cc\)"`
