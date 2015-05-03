#include <string>

#include "spark/rpc.grpc.pb.h"

#include <grpc/grpc.h>
#include <grpc++/status.h>

namespace spark {

class SparkService : public Spark::Service {
 public:
  SparkService(const std::string& my_ip,
               const std::string& peer_ip);

  ~SparkService();

  virtual grpc::Status CreateTunnel(grpc::ServerContext* context,
                                    const CreateTunnelRequest* request, 
                                    CreateTunnelReply* response);

 private:
  const std::string my_ip_;  // My IP address on the VPN connection
  // Peer IP address on the VPN connection
  //
  // TODO: handle ip pool to support multiple clients.
  const std::string peer_ip_;  
};

}
