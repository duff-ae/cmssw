//
//  SerialTaskQueue_test.cpp
//  DispatchProcessingDemo
//
//  Created by Chris Jones on 9/27/11.
//

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>
#include <chrono>
#include <memory>
#include <atomic>
#include <thread>
#include <iostream>
#include "oneapi/tbb/task.h"
#include "FWCore/Concurrency/interface/SerialTaskQueueChain.h"

class SerialTaskQueueChain_test : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SerialTaskQueueChain_test);
  CPPUNIT_TEST(testPush);
  CPPUNIT_TEST(testPushOne);
  CPPUNIT_TEST(stressTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void testPush();
  void testPushOne();
  void stressTest();
  void setUp() {}
  void tearDown() {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(SerialTaskQueueChain_test);
using namespace std::chrono_literals;

void SerialTaskQueueChain_test::testPush() {
  std::atomic<unsigned int> count{0};

  std::vector<std::shared_ptr<edm::SerialTaskQueue>> queues = {std::make_shared<edm::SerialTaskQueue>(),
                                                               std::make_shared<edm::SerialTaskQueue>()};
  edm::SerialTaskQueueChain chain(queues);
  {
    oneapi::tbb::task_group group;
    std::atomic<int> waiting{3};
    chain.push(group, [&count, &waiting] {
      CPPUNIT_ASSERT(count++ == 0);
      std::this_thread::sleep_for(10us);
      --waiting;
    });

    chain.push(group, [&count, &waiting] {
      CPPUNIT_ASSERT(count++ == 1);
      std::this_thread::sleep_for(10us);
      --waiting;
    });

    chain.push(group, [&count, &waiting] {
      CPPUNIT_ASSERT(count++ == 2);
      std::this_thread::sleep_for(10us);
      --waiting;
    });

    do {
      group.wait();
    } while (0 != waiting.load());
    CPPUNIT_ASSERT(count == 3);
    while (chain.outstandingTasks() != 0)
      ;
  }
}

void SerialTaskQueueChain_test::testPushOne() {
  std::atomic<unsigned int> count{0};

  std::vector<std::shared_ptr<edm::SerialTaskQueue>> queues = {std::make_shared<edm::SerialTaskQueue>()};
  edm::SerialTaskQueueChain chain(queues);
  {
    oneapi::tbb::task_group group;
    std::atomic<int> waiting{3};

    chain.push(group, [&count, &waiting] {
      CPPUNIT_ASSERT(count++ == 0);
      std::this_thread::sleep_for(10us);
      --waiting;
    });

    chain.push(group, [&count, &waiting] {
      CPPUNIT_ASSERT(count++ == 1);
      std::this_thread::sleep_for(10us);
      --waiting;
    });

    chain.push(group, [&count, &waiting] {
      CPPUNIT_ASSERT(count++ == 2);
      std::this_thread::sleep_for(10us);
      --waiting;
    });

    do {
      group.wait();
    } while (0 != waiting.load());
    CPPUNIT_ASSERT(count == 3);
    while (chain.outstandingTasks() != 0)
      ;
  }
}

namespace {
  void join_thread(std::thread* iThread) {
    if (iThread->joinable()) {
      iThread->join();
    }
  }
}  // namespace

void SerialTaskQueueChain_test::stressTest() {
  std::vector<std::shared_ptr<edm::SerialTaskQueue>> queues = {std::make_shared<edm::SerialTaskQueue>(),
                                                               std::make_shared<edm::SerialTaskQueue>()};

  edm::SerialTaskQueueChain chain(queues);

  unsigned int index = 100;
  const unsigned int nTasks = 1000;
  while (0 != --index) {
    oneapi::tbb::task_group group;
    std::atomic<int> waiting{2};
    std::atomic<unsigned int> count{0};

    std::atomic<bool> waitToStart{true};
    {
      std::thread pushThread([&chain, &waitToStart, &waiting, &group, &count] {
        while (waitToStart.load()) {
        };
        for (unsigned int i = 0; i < nTasks; ++i) {
          ++waiting;
          chain.push(group, [&count, &waiting] {
            ++count;
            --waiting;
          });
        }
        --waiting;
      });

      waitToStart = false;
      for (unsigned int i = 0; i < nTasks; ++i) {
        ++waiting;
        chain.push(group, [&count, &waiting] {
          ++count;
          --waiting;
        });
      }
      --waiting;
      std::shared_ptr<std::thread>(&pushThread, join_thread);
    }
    do {
      group.wait();
    } while (0 != waiting.load());

    CPPUNIT_ASSERT(2 * nTasks == count);
  }
  while (chain.outstandingTasks() != 0)
    ;
}
