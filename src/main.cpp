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
        server.start();
    }
    catch (const std::exception &e) {
        std::cerr << "Error - " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
