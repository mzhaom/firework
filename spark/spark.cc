#include "spark/spark-service.h"

#include <grpc/grpc.h>
#include <grpc++/channel_arguments.h>
#include <grpc++/channel_interface.h>
#include <grpc++/create_channel.h>
#include <grpc++/credentials.h>
#include <grpc++/client_context.h>
#include <grpc++/status.h>

#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_string(server, "1.2.3.4:8080", "Address of the flame server");

static void RunClient() {
  std::shared_ptr<grpc::ChannelInterface> channel =
      grpc::CreateChannel(FLAGS_server, grpc::InsecureCredentials(),
                          grpc::ChannelArguments());
  spark::Spark::Stub client(channel);
  grpc::ClientContext ctx;
  spark::CreateTunnelRequest req;
  spark::CreateTunnelReply reply;
  client.CreateTunnel(&ctx, req, &reply);
  LOG(INFO) << reply.DebugString();
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::SetUsageMessage("A research client");
  google::ParseCommandLineFlags(&argc, &argv, false);
  google::InitGoogleLogging(argv[0]);
  grpc_init();
  RunClient();
  grpc_shutdown();
  return 0;
}
