#include "../../include/abstracts/ABodyProcessor.hpp"

unsigned long ABodyProcessor::_uploadCounter = 0;

ABodyProcessor::ABodyProcessor(const ServerConfig &config): _config(config), _isFinished(false) {};
