
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
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

class TestProgressLogger : public testing::EmptyTestEventListener {
    // Called before a test starts.
    void OnTestStart(const testing::TestInfo &test_info) override {
        AddLog(Test, fmt::format("*** Starting test {}.{}", test_info.test_suite_name(),
                                 test_info.name()));
    }

    // Called after a failed assertion or a SUCCESS().
    void OnTestPartResult(const testing::TestPartResult &test_part_result) override {
        AddLog(Test, fmt::format("*** {} in {}:{} -> {} : {}",
                                 (test_part_result.failed() ? "Failure" : "Success"),
                                 test_part_result.file_name(), test_part_result.line_number(),
                                 test_part_result.summary(), test_part_result.message()));
    }

    // Called after a test ends.
    void OnTestEnd(const testing::TestInfo &test_info) override {
        AddLog(Test,
               fmt::format("*** Test {}.{} completed with {}", test_info.test_suite_name(),
                           test_info.name(), test_info.result()->Passed() ? "success" : "failure"));
    }
};

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

    testing::InitGoogleTest(&argc, argv);
    testing::TestEventListeners &listeners = testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new TestProgressLogger);
    testing::FLAGS_gtest_death_test_style = "fast";
    return RUN_ALL_TESTS();
}
