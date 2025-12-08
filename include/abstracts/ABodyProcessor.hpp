#ifndef ABODYPROCESSOR_HPP
#define ABODYPROCESSOR_HPP

#include "../interfaces/IBodyProcessor.hpp"
#include "../core/ServerConfig.hpp"

class ABodyProcessor : public IBodyProcessor {

    protected:
        const ServerConfig &_config;
        bool _isFinished;
        ABodyProcessor(const ServerConfig &config);
    public:
        virtual ~ABodyProcessor() {};
};

#endif