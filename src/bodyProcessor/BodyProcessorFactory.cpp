#include "../../include/bodyProcessor/BodyProcessorFactory.hpp"
#include "../../include/bodyProcessor/ChunkedBodyProcessor.hpp"

ABodyProcessor* BodyProcessorFactory::createBodyProcessor(
    const ServerConfig& config,
    const Location& location,
    const HttpRequest& request
) {
    // 1️⃣ PRIORIDADE: Transfer-Encoding: chunked
    std::string te = request.getHeader("Transfer-Encoding");
    if (!te.empty() && te.find("chunked") != std::string::npos) {
        return new ChunkedBodyProcessor(config, location, request);
    }

    // 2️⃣ application/x-www-form-urlencoded
    std::string ct = request.getHeader("Content-Type");
    if (ct == "application/x-www-form-urlencoded") {
        return new UrlEncodedProcessor(config, location, request);
    }

    // 3️⃣ multipart/form-data
    if (ct.find("multipart/form-data") != std::string::npos) {
        return new MultipartProcessor(config, location, request);
    }

    // 4️⃣ fallback (raw body)
    return new RawProcessor(config, location, request);
}