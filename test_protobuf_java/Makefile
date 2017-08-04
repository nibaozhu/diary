all: bin/MyServer.class
	java -classpath lib/protobuf-java-2.5.0.jar:bin/ MyServer hello world

bin/MyServer.class: src/MyServer.java src/RegisterCenter.java
	javac -classpath lib/protobuf-java-2.5.0.jar src/*.java -d bin/

src/RegisterCenter.java: proto/RegisterCenter.proto
	protoc --java_out=src/ proto/RegisterCenter.proto

clean:
	rm -rf bin/* tmp/* 
