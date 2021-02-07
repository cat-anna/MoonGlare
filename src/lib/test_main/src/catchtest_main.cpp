#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <orbit_logger/sink/file_sink.h>
#include <orbit_logger/sink/stdout_sink.h>
#include <string>
#include <vector>

using OrbitLogger::LogCollector;
using OrbitLogger::StdFileLoggerSink;
using OrbitLogger::StdOutSink;

struct MyListener : Catch::TestEventListenerBase {

    using TestEventListenerBase::TestEventListenerBase; // inherit constructor

    // Perform some setup before a test case is run
    void testCaseStarting(Catch::TestCaseInfo const &testInfo) override {
        AddLog(Test, fmt::format("*** Starting test {}.{}", testInfo.className, testInfo.name));
    }

    // Tear-down after a test case is run
    void testCaseEnded(Catch::TestCaseStats const &testCaseStats) override {
        AddLog(Test, fmt::format("*** Test {}.{} completed with {}",
                                 testCaseStats.testInfo.className, testCaseStats.testInfo.name,
                                 !testCaseStats.aborting ? "success" : "failure"));
    }

    // Called after a failed assertion or a SUCCESS().
    // void OnTestPartResult(const testing::TestPartResult &test_part_result) override {
    //     AddLog(Test, fmt::format("*** {} in {}:{} -> {} : {}",
    //                              (test_part_result.failed() ? "Failure" : "Success"),
    //                              test_part_result.file_name(), test_part_result.line_number(),
    //                              test_part_result.summary(), test_part_result.message()));
    // }

    // The whole test run, starting and ending
    // virtual void testRunStarting( TestRunInfo const& testRunInfo );
    // virtual void testRunEnded( TestRunStats const& testRunStats );

    // Test cases starting and ending
    // virtual void testCaseStarting( TestCaseInfo const& testInfo );
    // virtual void testCaseEnded( TestCaseStats const& testCaseStats );

    // Sections starting and ending
    // virtual void sectionStarting( SectionInfo const& sectionInfo );
    // virtual void sectionEnded( SectionStats const& sectionStats );

    // Assertions before/ after
    // virtual void assertionStarting( AssertionInfo const& assertionInfo );
    // virtual bool assertionEnded( AssertionStats const& assertionStats );

    // A test is being skipped (because it is "hidden")
    // virtual void skipTest( TestCaseInfo const& testInfo );
};

CATCH_REGISTER_LISTENER(MyListener)

std::string GetLogFileName(const char *exe_name) {
    auto path = std::filesystem::path(exe_name).parent_path();
    auto exe = std::filesystem::path(exe_name).filename();
    exe.replace_extension("log");
    return path.generic_string() + "/logs/" + exe.generic_string();
}

int main(int argc, char *argv[]) {
    OrbitLogger::ThreadInfo::SetName("MAIN", true);
    LogCollector::Start();
    std::string log_file = GetLogFileName(argv[0]);
    std::filesystem::create_directories("logs");
    LogCollector::AddLogSink<StdFileLoggerSink>(log_file.c_str(), false);
    LogCollector::SetChannelName(OrbitLogger::LogChannels::Test, "TEST", true);

    // testing::InitGoogleTest(&argc, argv);
    // testing::TestEventListeners &listeners = testing::UnitTest::GetInstance()->listeners();
    // listeners.Append(new TestProgressLogger);
    // testing::FLAGS_gtest_death_test_style = "fast";
    int result = Catch::Session().run(argc, argv);
    // global clean-up...
    return result;
    // return RUN_ALL_TESTS();
}
