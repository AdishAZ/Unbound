.PHONY: proto-go build-server run-server build-cpp

# Regenerates server/internal/protocol/pb/packets.pb.go from proto/packets.proto.
# Requires `protoc` and `protoc-gen-go` (see proto/README.md).
proto-go:
	protoc --go_out=. --go_opt=module=unbound-mp/server \
	       --proto_path=proto \
	       proto/packets.proto

build-server: proto-go
	cd server && go build -o ../bin/unbound-mp-server ./cmd/server

run-server: build-server
	./bin/unbound-mp-server

# The C++ side generates its protobuf code via CMake's protobuf_generate_cpp()
# at configure time - no explicit target needed here, just:
#   cmake -S cpp -B build && cmake --build build
build-cpp:
	cmake -S cpp -B build
	cmake --build build
