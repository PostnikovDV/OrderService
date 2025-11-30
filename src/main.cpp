#include "Utils/ServiceUtils.h"
#include "Orders/OrderService.h"
#include "Handlers/CreateOrderHandler.h"
#include "Handlers/GetOrderHandler.h"
#include "Handlers/DeleteOrderHandler.h"
#include "ServerUtils/HttpListener.h"
#include "ServerUtils/HttpSession.h"
#include <regex>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp; 

int main(int argc, char* argv[])
{
    auto const threads = std::max<int>(1, std::thread::hardware_concurrency());
    auto const address = net::ip::make_address("127.0.0.1");
    auto const port = static_cast<unsigned short>(8181);
    std::filesystem::path exePath = std::filesystem::current_path();
    std::filesystem::path docRootPath = exePath / "www";

    auto const docRoot = std::make_shared<std::string>(docRootPath.string());

    std::cout << "Server starting on " << address << ":" << port << " with " << threads << " threads." << std::endl;

    net::io_context ioc{ threads };

    std::make_shared<HttpListener>(ioc, tcp::endpoint{ address, port }, docRoot)->run();

    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] { ioc.run(); });

    ioc.run();

    for (auto& t : v)
        t.join();

    std::cout << "Server stopped." << std::endl;
    return 0;
}