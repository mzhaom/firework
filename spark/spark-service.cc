#include "spark/spark-service.h"


namespace spark {

SparkService::SparkService(const std::string& my_ip,
                           const std::string& peer_ip)
    : my_ip_(my_ip),
      peer_ip_(peer_ip) {
}

SparkService::~SparkService() {
}

grpc::Status SparkService::CreateTunnel(grpc::ServerContext* context,
                                        const CreateTunnelRequest* request, 
                                        CreateTunnelReply* response) {
  response->set_ip(peer_ip_);
  response->set_peer_ip(my_ip_);
  return grpc::Status::OK;
}

}
