#include "spark/spark-service.h"

#include <unistd.h>
#include <grpc++/stream.h>
#include "spark/tun.h"
#include "spark/packet-stream.h"

namespace spark {

SparkService::SparkService(const std::string& my_ip,
                           const std::string& peer_ip)
    : my_ip_(my_ip),
      peer_ip_(peer_ip) {
}

SparkService::~SparkService() {
}

grpc::Status SparkService::CreateTunnel(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<Bullet, Bullet>* stream) {
  {
    Bullet req;
    if (!stream->Read(&req)) {
      return grpc::Status::OK;
    }
    Bullet reply;
    CreateTunnelReply* response = reply.mutable_create_tunnel_reply();
    response->set_ip(peer_ip_);
    response->set_peer_ip(my_ip_);
    stream->Write(reply);
  }
  // Now create the tun interface and pipe packets
  Tun* tun = Tun::Allocate("flame");
  tun->Configure(my_ip_, peer_ip_);
  PacketStream ps(tun, stream, stream);
  ps.Run();
  return grpc::Status::OK;
}

}
