#pragma warning(push, 1)
#pragma warning(disable : 4996)

#define _WIN32_WINNT 0x0600

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/process.hpp>


#include "WaitForProcess.h"

namespace MoonGlare::OS {

int WaitForProcess(const std::list<std::string> &arguments, const std::list<std::string> &inputLines,
                   std::function<void(std::string)> onOutput) {
    namespace bp = boost::process;

    boost::asio::io_service ios;
    bp::async_pipe ap(ios);
    bp::opstream in;

    auto str = boost::join(arguments, std::string(" "));
    bp::child c;
    try {
        c = bp::child(str, bp::std_out > ap, bp::std_in<in, bp::std_err> ap);
    } catch (...) {
        ap.close();
        throw;
    }

    std::thread feedThread;

    if (!inputLines.empty()) {
        feedThread = std::thread([&]() {
            for (const auto &line : inputLines)
                in << line << "\n" << std::flush;
        });
    }

    boost::asio::streambuf b;
    std::function<void()> doRead;
    auto handler = [&](const boost::system::error_code &ec, std::size_t size) {
        if (ec)
            return;

        std::string str{boost::asio::buffers_begin(b.data()), boost::asio::buffers_begin(b.data()) + size};
        b.consume(size);

        std::vector<std::string> lines;
        boost::split(lines, str, boost::is_any_of("\n"));
        for (auto &line : lines) {
            boost::trim(line);
            if (!line.empty() && onOutput)
                onOutput(line);
        }
        doRead();
    };
    doRead = [&]() { boost::asio::async_read_until(ap, b, '\n', handler); };

    doRead();

    ios.run();
    c.wait();
    if (feedThread.joinable())
        feedThread.join();
    if (ap.is_open())
        ap.close();
    int result = c.exit_code();

    if (result != 0) {
        throw "process failed!";
    }
    return result;
}

} // namespace MoonGlare::OS
