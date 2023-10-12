#ifndef WEBSOCKETPEERTEST_H
#define WEBSOCKETPEERTEST_H

#include "jobsystem/JobSystem.h"
#include "messaging/Messaging.h"
#include "networking/NetworkingFactory.h"
#include "properties/PropertyFactory.h"

using namespace networking;
using namespace networking::websockets;
using namespace jobsystem;
using namespace props;
using namespace messaging;

SharedWebSocketPeer SetupWebSocketPeer(SharedJobManager job_manager, int port) {
  SharedBroker message_broker = MessagingFactory::CreateBroker(job_manager);
  SharedPropertyProvider property_provider =
      PropertyFactory::CreatePropertyProvider(message_broker);
  property_provider->Set<size_t>("net.ws.port", port);
  SharedWebSocketPeer server =
      NetworkingFactory::CreateWebSocketPeer(job_manager, property_provider);

  return server;
}

class TestConsumer : public IWebSocketMessageConsumer {
public:
  std::atomic<size_t> counter{0};
  const std::string GetMessageType() const noexcept override {
    return "test-type";
  }
  void ProcessReceivedMessage(
      SharedWebSocketMessage received_message) noexcept override {
    counter++;
  }
};

void SendMessageAndWait(SharedWebSocketMessage message,
                        SharedWebSocketPeer peer, std::string uri) {
  std::future<void> sending_result = peer->Send(uri, message);
  sending_result.wait();
  ASSERT_NO_THROW(sending_result.get());
}

TEST(WebSockets, websockets_connection_establishment) {
  SharedJobManager job_manager = std::make_shared<JobManager>();
  SharedWebSocketPeer peer1 = SetupWebSocketPeer(job_manager, 9003);
  SharedWebSocketPeer peer2 = SetupWebSocketPeer(job_manager, 9004);

  auto result = peer1->EstablishConnectionTo("ws://127.0.0.1:9004");
  result.wait();

  ASSERT_NO_THROW(result.get());
}

TEST(WebSockets, websockets_connection_closing) {
  SharedJobManager job_manager = std::make_shared<JobManager>();
  SharedWebSocketPeer peer1 = SetupWebSocketPeer(job_manager, 9003);
  SharedWebSocketMessage message =
      std::make_shared<WebSocketMessage>("test-type");
  {
    SharedWebSocketPeer peer2 = SetupWebSocketPeer(job_manager, 9004);
    auto result = peer1->EstablishConnectionTo("ws://127.0.0.1:9004");
    result.wait();

    SendMessageAndWait(message, peer1, "ws://127.0.0.1:9004");
  }

  ASSERT_THROW(peer1->Send("ws://127.0.0.1:9004", message),
               NoSuchPeerException);
}

TEST(WebSockets, websockets_message_sending_1_to_1) {
  SharedJobManager job_manager = std::make_shared<JobManager>();
  SharedWebSocketPeer peer1 = SetupWebSocketPeer(job_manager, 9003);
  SharedWebSocketPeer peer2 = SetupWebSocketPeer(job_manager, 9004);

  std::shared_ptr<TestConsumer> test_consumer_1 =
      std::make_shared<TestConsumer>();
  std::shared_ptr<TestConsumer> test_consumer_2 =
      std::make_shared<TestConsumer>();

  peer1->AddConsumer(test_consumer_1);
  peer2->AddConsumer(test_consumer_2);

  auto result1 = peer1->EstablishConnectionTo("ws://127.0.0.1:9004");
  result1.wait();
  ASSERT_NO_THROW(result1.get());

  SharedWebSocketMessage message =
      std::make_shared<WebSocketMessage>("test-type");

  SendMessageAndWait(message, peer1, "ws://127.0.0.1:9004");

  auto result2 = peer2->EstablishConnectionTo("ws://127.0.0.1:9003");
  result2.wait();
  ASSERT_NO_THROW(result2.get());

  SendMessageAndWait(message, peer2, "ws://127.0.0.1:9003");

  job_manager->InvokeCycleAndWait();
  ASSERT_EQ(test_consumer_1->counter, 1);
  ASSERT_EQ(test_consumer_2->counter, 1);
}

TEST(WebSockets, websockets_message_sending_1_to_n) {
  SharedJobManager job_manager = std::make_shared<JobManager>();
  SharedWebSocketPeer peer1 = SetupWebSocketPeer(job_manager, 9003);

  std::shared_ptr<TestConsumer> test_consumer_1 =
      std::make_shared<TestConsumer>();

  peer1->AddConsumer(test_consumer_1);

  std::vector<SharedWebSocketPeer> peers;
  for (size_t i = 9005; i < 9010; i++) {
    SharedWebSocketPeer peer = SetupWebSocketPeer(job_manager, i);

    auto result = peer->EstablishConnectionTo("ws://127.0.0.1:9003");
    result.wait();
    ASSERT_NO_THROW(result.get());

    SharedWebSocketMessage message =
        std::make_shared<WebSocketMessage>("test-type");

    SendMessageAndWait(message, peer, "ws://127.0.0.1:9003");

    peers.push_back(peer);
  }

  job_manager->InvokeCycleAndWait();
  ASSERT_EQ(test_consumer_1->counter, 5);
}

#endif /* WEBSOCKETPEERTEST_H */
