#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <pqxx/pqxx>
#include <string>
#include <optional>
#include <vector>
#include <iostream>

namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;

class DnsResolver {
public:
    DnsResolver(const std::string& dbname, const std::string& user, const std::string& host, int port);

    void handleRequest(http::request<http::string_body>& request, tcp::socket& socket);

private:
    pqxx::connection conn_;

    bool does_ip_hostname_exist(pqxx::work& txn, const std::string& value, const std::string& key);
    void insert_ip_hostname(pqxx::work& txn, const std::string& ip, const std::string& hostname);
    std::optional<std::string> get_by_ip_host(pqxx::work& txn, const std::string& value, boost::json::object& dict, const std::string& key);
    void send_response(tcp::socket& socket, const boost::json::object& dict);
};

#endif // DNS_RESOLVER_H
