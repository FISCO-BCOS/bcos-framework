/**
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @brief Construct a new boost auto test case object for Worker
 *
 * @file Worker.cpp
 * @author: tabsu
 */

#include <bcos-framework/libutilities/Worker.h>
#include <bcos-test/libutils/TestPromptFixture.h>
#include <boost/test/unit_test.hpp>

using namespace bcos;
using namespace std;

namespace bcos
{
namespace test
{
class TestWorkerImpl : public Worker
{
public:
    TestWorkerImpl() : Worker("TestWorkerImpl", 10) {}
    void run() { startWorking(); }
    void stop() { stopWorking(); }

protected:
    virtual void initWorker() { cout << "initWorker..." << endl; }
    virtual void executeWorker()
    {
        cout << "count:" << count << endl;
        count++;
    }
    virtual void finishWorker() { cout << "finishWorker..." << endl; }

private:
    int count = 0;
};

BOOST_FIXTURE_TEST_SUITE(Worker, TestPromptFixture)

BOOST_AUTO_TEST_CASE(testWorker)
{
    TestWorkerImpl workerImpl;
    workerImpl.run();
    usleep(1000 * 10 * 5);
    workerImpl.stop();
}


BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos
