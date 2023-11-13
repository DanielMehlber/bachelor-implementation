#include "networking/websockets/WebSocketMessageConsumerJob.h"
#include <functional>
#include <utility>

using namespace networking::websockets;
using namespace std::placeholders;

WebSocketMessageConsumerJob::WebSocketMessageConsumerJob(
    SharedWebSocketMessageConsumer consumer, SharedWebSocketMessage message,
    WebSocketConnectionInfo connection_info)
    : jobsystem::job::Job(
          std::bind(&WebSocketMessageConsumerJob::ConsumeMessage, this, _1),
          "consume-web-socket-message-type-" + message->GetType()),
      m_consumer{std::move(consumer)}, m_message{message},
      m_connection_info(std::move(connection_info)) {}

jobsystem::job::JobContinuation
networking::websockets::WebSocketMessageConsumerJob::ConsumeMessage(
    [[maybe_unused]] jobsystem::JobContext *context) {
  m_consumer->ProcessReceivedMessage(m_message, m_connection_info);
  return jobsystem::job::JobContinuation::DISPOSE;
}