#pragma once

#include <memory>
#include <grpc++/stream.h>
#include "spark/tun.h"
#include "spark/rpc.pb.h"

namespace spark {

class PacketStream {
 public:
  PacketStream(Tun* tun, 
               grpc::ReaderInterface<Bullet>* reader,
               grpc::WriterInterface<Bullet>* writer);
  ~PacketStream();

  // Starts another thread to read from the stream and write to tun
  // interface. It also blocks the current thread read from the tun
  // interface and translates the packet to stream message.
  void Run();

 private:
  void DrainStream();
  void DrainTun();

  std::unique_ptr<Tun> tun_;
  grpc::ReaderInterface<Bullet>* const reader_;
  grpc::WriterInterface<Bullet>* const writer_;
  bool closing_;
};
};
