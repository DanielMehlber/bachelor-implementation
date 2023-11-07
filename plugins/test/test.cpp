#include "TestPlugin.h"
#include "messaging/MessagingFactory.h"
#include "plugins/impl/BoostPluginManager.h"
#include "resourcemgmt/ResourceFactory.h"
#include <gtest/gtest.h>

#define sharednullptr(x) std::shared_ptr<x>(nullptr)

TEST(PluginsTest, lifecycle_test) {
  auto job_manager = std::make_shared<jobsystem::JobManager>();
  auto resource_manager =
      resourcemgmt::ResourceFactory::CreateResourceManager();

  auto plugin_context = std::make_shared<plugins::PluginContext>(
      job_manager, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

  auto plugin_manager = std::make_shared<plugins::BoostPluginManager>(
      plugin_context, resource_manager);

  auto plugin = std::make_shared<TestPlugin>();

  plugin_manager->InstallPlugin(plugin);
  plugin_manager->UninstallPlugin(plugin->GetName());

  job_manager->InvokeCycleAndWait();

  ASSERT_EQ(plugin->GetInitCount(), 1);
  ASSERT_EQ(plugin->GetShutdownCount(), 1);
}

int main(int argc, char **argv) {

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}