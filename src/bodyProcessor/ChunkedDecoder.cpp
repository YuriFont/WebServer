#include "../../include/bodyProcessor/ChunkedDecoder.hpp"
#include <sstream>
#include <cstdlib>

/*
 * Construtor
 * Inicializa o decoder já em estado limpo, pronto para processar
 * uma nova requisição HTTP com Transfer-Encoding: chunked.
 */
ChunkedDecoder::ChunkedDecoder() {
    reset();
}

/*
 * Reseta completamente o estado interno do decoder.
 * Deve ser chamado sempre que um novo request chunked começa
 * (ex: após headers ou em conexões keep-alive).
 */
void ChunkedDecoder::reset() {
    _state = READ_SIZE;           // Começamos esperando o tamanho do chunk
    _currentChunkSize = 0;        // Tamanho do chunk atual
    _bytesReadInChunk = 0;        // Quantos bytes já foram lidos deste chunk
    _sizeBuffer.clear();          // Buffer temporário para o tamanho em hexadecimal
    _body.clear();                // Body final (sem framing)
}

/*
 * Indica se o corpo chunked já foi completamente recebido.
 * Isso ocorre após o chunk de tamanho zero (0\r\n\r\n).
 */
bool ChunkedDecoder::isFinished() const {
    return _state == DONE;
}

/*
 * Retorna o corpo já decodificado.
 * IMPORTANTE:
 *  - Não contém tamanhos
 *  - Não contém CRLF
 *  - Não contém framing HTTP
 */
const std::string& ChunkedDecoder::getBody() const {
    return _body;
}

void ChunkedDecoder::feed(const char* data, size_t len) {
    size_t i = 0;

    while (i < len && _state != DONE && _state != ERROR) {

        switch (_state) {

        /* ================= READ_SIZE ================= */
        case READ_SIZE:
            if (data[i] == '\r') {
                _state = READ_SIZE_CR;
                i++;
            } else {
                if (!std::isxdigit((unsigned char)data[i])) {
                    _state = ERROR;
                    return;
                }
                _sizeBuffer += data[i++];
            }
            break;

        /* ================= READ_SIZE_CR ================= */
        case READ_SIZE_CR:
            if (data[i] != '\n') {
                _state = ERROR;
                return;
            }
            i++;

            if (_sizeBuffer.empty()) {
                _state = ERROR;
                return;
            }

            _currentChunkSize = std::strtoul(_sizeBuffer.c_str(), NULL, 16);
            _sizeBuffer.clear();
            _bytesReadInChunk = 0;

            if (_currentChunkSize == 0) {
                _state = DONE;
                return;
            }

            _state = READ_DATA;
            break;

        /* ================= READ_DATA ================= */
        case READ_DATA: {
            size_t remaining = _currentChunkSize - _bytesReadInChunk;
            size_t toCopy = std::min(remaining, len - i);

            _body.append(data + i, toCopy);
            i += toCopy;
            _bytesReadInChunk += toCopy;

            if (_bytesReadInChunk == _currentChunkSize) {
                _state = READ_DATA_CR;
            }
            break;
        }

        /* ================= READ_DATA_CR ================= */
        case READ_DATA_CR:
            if (data[i] != '\r') {
                _state = ERROR;
                return;
            }
            i++;
            _state = READ_DATA_LF;
            break;

        /* ================= READ_DATA_LF ================= */
        case READ_DATA_LF:
            if (data[i] != '\n') {
                _state = ERROR;
                return;
            }
            i++;
            _state = READ_SIZE;
            break;

        default:
            _state = ERROR;
            return;
        }
    }
}
