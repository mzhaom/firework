load("/third_party/grpc/grpc_proto", "proto_library")

proto_library(
  name = "rpc",
  src = "rpc.proto")

cc_library(
  name = "tun",
  hdrs = [
    "tun.h"
  ],
  srcs = [
    "tun.cc"
  ],
  deps = [
    "//third_party/glog",
    "//third_party/folly",
  ],
)

cc_library(
  name = "service",
  hdrs = [
    "spark-service.h"
  ],
  srcs = [
    "spark-service.cc"
  ],
  deps = [
    ":rpc",
    ":tun",
  ],
)

# server
cc_binary(
  name = "flame",
  srcs = [
    "flame.cc"
  ],
  deps = [
    ":service",
    "//third_party/gflags",
    "//third_party/glog",
  ]
)

# client
cc_binary(
  name = "spark",
  srcs = [
    "spark.cc"
  ],
  deps = [
    ":service",
    "//third_party/gflags",
    "//third_party/glog",
  ]
)
  
