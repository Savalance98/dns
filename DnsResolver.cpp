#include "DnsResolver.h"

// Конструктор
DnsResolver::DnsResolver(const std::string& dbname, const std::string& user, const std::string& host, int port)
    : conn_("dbname=" + dbname + " user=" + user + " host=" + host + " port=" + std::to_string(port)) {}

// Обработка HTTP-запроса
void DnsResolver::handleRequest(http::request<http::string_body>& request, tcp::socket& socket) {
    boost::json::object dict;
    boost::json::value jv = boost::json::parse(request.body());
    pqxx::work txn(conn_);

    if (jv.is_object()) {
        boost::json::object obj = jv.as_object();

        if (obj.contains("ip") && obj["ip"].is_array()) {
            auto ip_list = boost::json::value_to<std::vector<std::string>>(obj["ip"]);
            for (const auto& ip : ip_list) {
                if (does_ip_hostname_exist(txn, ip, "ip")) {
                    auto hostname = get_by_ip_host(txn, ip, dict, "ip");
                    if (hostname) {
                        std::cout << "HOSTNAME: " << *hostname << std::endl;
                    }
                } else {
                    std::string hostname = "ya.ru";
                    insert_ip_hostname(txn, ip, hostname);
                }
            }
        } else if (obj.contains("hostname") && obj["hostname"].is_array()) {
            auto hostname_list = boost::json::value_to<std::vector<std::string>>(obj["hostname"]);
            for (const auto& host : hostname_list) {
                if (does_ip_hostname_exist(txn, host, "hostname")) {
                    auto ip = get_by_ip_host(txn, host, dict, "hostname");
                    if (ip) {
                        std::cout << "IP: " << *ip << std::endl;
                    }
                } else {
                    std::string ip = "125.0.90.1";
                    insert_ip_hostname(txn, ip, host);
                }
            }
        }
    } else {
        std::cerr << "Ошибка: JSON не является объектом." << std::endl;
    }

    send_response(socket, dict);
}

// Проверка существования записи
bool DnsResolver::does_ip_hostname_exist(pqxx::work& txn, const std::string& value, const std::string& key) {
    pqxx::result result;
    if (key == "ip") {
        result = txn.exec_params("SELECT 1 FROM ip_addresses WHERE ip = $1;", value);
    } else if (key == "hostname") {
        result = txn.exec_params("SELECT 1 FROM ip_addresses WHERE hostname = $1;", value);
    }
    return !result.empty();
}

// Вставка записи, если её нет
void DnsResolver::insert_ip_hostname(pqxx::work& txn, const std::string& ip, const std::string& hostname) {
    txn.exec_params("INSERT INTO ip_addresses (ip, hostname) VALUES ($1, $2);", ip, hostname);
    txn.commit();
    std::cout << "Запись добавлена: IP=" << ip << ", HOSTNAME=" << hostname << std::endl;
}

// Получение IP или hostname
std::optional<std::string> DnsResolver::get_by_ip_host(pqxx::work& txn, const std::string& value, boost::json::object& dict, const std::string& key) {
    pqxx::result result;
    if (key == "ip") {
        result = txn.exec_params("SELECT hostname FROM ip_addresses WHERE ip = $1;", value);
        if (!result.empty()) {
            std::string hostname = result[0][0].as<std::string>();
            dict[value] = hostname;
            return hostname;
        }
    } else if (key == "hostname") {
        result = txn.exec_params("SELECT ip FROM ip_addresses WHERE hostname = $1;", value);
        if (!result.empty()) {
            boost::json::array json_array;
            for (const auto& row : result) {
                json_array.emplace_back(row[0].c_str());
            }
            dict[value] = json_array;
            return value;
        }
    }
    return std::nullopt;
}

// Отправка ответа клиенту
void DnsResolver::send_response(tcp::socket& socket, const boost::json::object& dict) {
    boost::json::value json_value = dict;
    std::string json_str = boost::json::serialize(json_value);

    http::response<http::string_body> response;
    response.result(http::status::ok);
    response.set(http::field::server, "DnsResolver Server");
    response.set(http::field::content_type, "application/json");
    response.body() = json_str;
    response.prepare_payload();

    boost::beast::http::write(socket, response);
}
