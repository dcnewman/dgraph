#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "graphresponse.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using graph::Request;
using graph::Response;
using graph::Dgraph;

class GraphClient {
 public:
  GraphClient(std::shared_ptr<Channel> channel)
      : stub_(Dgraph::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string Query(const std::string& q) {
    // Data we are sending to the server
    Request request;
    request.set_query(q);

    // Container for the data we expect from the server.
    Response reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Query(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return "Subgraph uid:" + std::to_string(reply.n().uid()) +
	" attribute:" + reply.n().attribute();
    } else {
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<Dgraph::Stub> stub_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  GraphClient g(grpc::CreateChannel(
      "localhost:8081", grpc::InsecureChannelCredentials()));
  std::string q(
"{\n" \
"  me(_xid_: m.06pj8) {\n" \
"    type.object.name.en\n" \
"    film.director.film {\n" \
"      film.film.starring {\n" \
"        type.object.name.en\n" \
"        film.performance.actor {\n" \
"          film.director.film {\n" \
"            type.object.name.en\n" \
"          }\n" \
"          type.object.name.en\n" \
"        }\n" \
"	 film.performance.character {\n" \
"          type.object.name.en\n" \
"        }\n" \
"      }\n" \
"      film.film.initial_release_date\n" \
"      film.film.country\n" \
"      film.film.genre {\n" \
"        type.object.name.en\n" \
"      }\n" \
"    }\n" \
"  }\n" \
"}\n");
  std::string reply = g.Query(q);
  std::cout << "Response: " << reply << std::endl;
  return 0;
}
