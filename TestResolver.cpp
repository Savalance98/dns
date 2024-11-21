#include "DnsResolver.h"

int main() {
    try {
        DnsResolver resolver("postgres", "daniilsavin", "/tmp", 5432);
        resolver.run(); // Запускаем сервер
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}