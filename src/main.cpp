#include "DbManager.h"
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

static std::unique_ptr<DatabaseManager> dbManager;



int main(int argc, char* argv[])
{

    std::string db_connection_string =
        "host=localhost "
        "port=5432 "
        "dbname=orderservice "
        "user=postgres "
        "password=postgres";

    dbManager = std::make_unique<DatabaseManager>(db_connection_string);
    auto const threads = std::max<int>(1, std::thread::hardware_concurrency());
    auto const address = net::ip::make_address("127.0.0.1");
    auto const port = static_cast<unsigned short>(8181);
    std::filesystem::path exe_path = std::filesystem::current_path();
    std::filesystem::path doc_root_path = exe_path / "www";

    auto const doc_root = std::make_shared<std::string>(doc_root_path.string());

    std::cout << "Server starting on " << address << ":" << port << " with " << threads << " threads." << std::endl;

    net::io_context ioc{ threads };

    std::make_shared<HttpListener>(ioc, tcp::endpoint{ address, port }, doc_root)->run();

    // «апускаем пулы потоков дл€ io_context
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] { ioc.run(); });

    ioc.run(); // √лавный поток тоже работает

    // ƒождемс€ завершени€ всех потоков (это никогда не произойдет, пока ioc.run() активен)
    for (auto& t : v)
        t.join();

    std::cout << "Server stopped." << std::endl;
    return 0;
}