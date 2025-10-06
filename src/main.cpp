/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufonten <yufonten@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 16:09:31 by yufonten          #+#    #+#             */
/*   Updated: 2025/10/06 10:39:40 by yufonten         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Config.hpp"
#include "../include/Server.hpp"

int main(int ac, char **av)
{
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
