#include "spark/spark-service.h"

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>

#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_int32(port, 8080, "Listening port of Spark RPC service");
DEFINE_string(my_ip, "192.168.1.1", "My IP on VPN");
DEFINE_string(peer_ip, "192.168.1.2", "Peer IP on VPN");

static void RunServer() {
  std::stringstream server_address;
  server_address << "0.0.0.0:" << FLAGS_port;
  spark::SparkService service(FLAGS_my_ip, FLAGS_peer_ip);

  LOG(INFO) << "Server listens on " << server_address.str();
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address.str(),
                           grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::SetUsageMessage("A research project");
  google::ParseCommandLineFlags(&argc, &argv, false);
  google::InitGoogleLogging(argv[0]);
  grpc_init();
  RunServer();

  grpc_shutdown();
  return 0;
}
