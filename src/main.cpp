/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufonten <yufonten@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 16:09:31 by yufonten          #+#    #+#             */
/*   Updated: 2025/10/20 09:31:08 by yufonten         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Config.hpp"
#include "../include/Server.hpp"

void sigint_handler(int signum)
{
    
    std::cout << "\nCaught signal " << signum << ", shutting down server gracefully..." << std::endl;
    exit(0);
}

// criar pasta uploads, files, readonly no www
int main(int ac, char **av)
{
    signal(SIGINT, sigint_handler);
    if (ac > 2) {
        std::cerr << "Error - Usage: " << av[0] << " [config_file]" << std::endl;
        return 1;
    }
    std::string configFilePath = (av[1] ? av[1] : "config/default.conf");
    try {
        Config config(configFilePath);
        Server server(config);
        /*for (size_t i = 0; i < config.servers.size(); i++) {
            std::cout << "Server " << i + 1 << " configuration:" << std::endl;
            std::cout << i << " - server_name: " << config.servers[i].server_name << std::endl;
            std::cout << i << " - ip: " << config.servers[i].ip << std::endl;
            std::cout << i << " - port: " << config.servers[i].port << std::endl;
            std::cout << i << " - client_max_body_size: " << config.servers[i].client_max_body_size << std::endl;
            std::cout << i << " - error_page:" << std::endl;
            for (std::map<int, std::string>::const_iterator it = config.servers[i].error_pages.begin(); it != config.servers[i].error_pages.end(); ++it) {
                std::cout << "    " << it->first << " -> " << it->second << std::endl;
            }
            std::cout << i << " - locations:" << std::endl;
            for (std::map<std::string, Location>::const_iterator it = config.servers[i].locations.begin(); it != config.servers[i].locations.end(); ++it) {
                std::cout << "    Location path: " << it->first << std::endl;
                std::cout << "        root: " << it->second.getRoot() << std::endl;
                std::cout << "        methods: ";
                const std::vector<std::string>& methods = it->second.getMethods();
                for (size_t j = 0; j < methods.size(); j++) {
                    std::cout << methods[j] << " ";
                }
                std::cout << std::endl;
            }
        }*/
        server.start();
    }
    catch (const std::exception &e) {
        std::cerr << "Error - " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
