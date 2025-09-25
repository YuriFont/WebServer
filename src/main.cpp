/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufonten <yufonten@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 16:09:31 by yufonten          #+#    #+#             */
/*   Updated: 2025/09/25 09:24:55 by yufonten         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Config.hpp"

int main(int ac, char **av)
{
    (void)ac;
    std::string configFilePath = (av[1] ? av[1] : "config/default.conf");
    try {
        Config config(configFilePath);
    }
    catch (const std::exception &e) {
        std::cerr << "Error - " << e.what() << std::endl;
        return 1;
    }
    return 0;
}