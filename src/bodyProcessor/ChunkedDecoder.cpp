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

/*
 * Alimenta o decoder com bytes crus vindos do socket.
 * Esta função pode ser chamada múltiplas vezes com dados parciais,
 * pois chunked NÃO garante alinhamento com recv().
 */
void ChunkedDecoder::feed(const char* data, size_t len) {
    // Índice usado para percorrer o buffer recebido do socket.
    // Ele avança à medida que bytes são consumidos.
    size_t i = 0;
    /*
     * Processa os dados byte a byte enquanto:
     *  - ainda existem bytes não consumidos no buffer recebido
     *  - o parsing do corpo chunked ainda não terminou
     */
    while (i < len && _state != DONE) {

        /* ============================================================
         * ESTADO: READ_SIZE
         * ============================================================
         * Neste estado, o decoder está lendo o TAMANHO do próximo chunk.
         *
         * O tamanho:
         *  - vem em ASCII
         *  - está em hexadecimal
         *  - termina obrigatoriamente com "\r\n"
         *
         * Exemplo válido:
         *   "7\r\n"
         *   "1c\r\n"
         */
        if (_state == READ_SIZE) {

            /*
             * Se encontramos um '\r', isso pode indicar
             * o início do CRLF que encerra o tamanho do chunk.
             */
            if (data[i] == '\r') {
                i++; // Consome o '\r'

                /*
                 * Confirmamos que o próximo byte é '\n',
                 * completando o delimitador CRLF.
                 */
                if (i < len && data[i] == '\n') {
                    i++; // Consome o '\n'

                    /*
                     * Validação obrigatória:
                     * Não é permitido encontrar CRLF sem antes
                     * ter lido pelo menos um dígito hexadecimal.
                     *
                     * Exemplo inválido:
                     *   "\r\n"
                     */
                    if (_sizeBuffer.empty()) {
                        _state = DONE;
                        return;
                    }

                    /*
                     * Converte a string hexadecimal acumulada
                     * no _sizeBuffer para um número inteiro.
                     *
                     * Exemplo:
                     *   "1c"  → 28 bytes
                     */
                    _currentChunkSize =
                        std::strtoul(_sizeBuffer.c_str(), NULL, 16);

                    // Limpa o buffer para o próximo tamanho de chunk
                    _sizeBuffer.clear();

                    /*
                     * Um tamanho igual a 0 indica o LAST-CHUNK,
                     * que marca o fim do corpo HTTP.
                     *
                     * Formato:
                     *   "0\r\n"
                     *   "\r\n"
                     */
                    if (_currentChunkSize == 0) {
                        _state = DONE;
                        return;
                    }

                    /*
                     * Prepara a leitura dos dados reais do chunk:
                     *  - zera o contador de bytes lidos
                     *  - muda o estado para READ_DATA
                     */
                    _bytesReadInChunk = 0;
                    _state = READ_DATA;
                }
            }
            else {
                /*
                 * Enquanto não encontramos CRLF,
                 * acumulamos os caracteres do tamanho.
                 *
                 * Apenas dígitos hexadecimais são válidos:
                 * 0-9, a-f, A-F
                 */
                if (!std::isxdigit(static_cast<unsigned char>(data[i]))) {
                    _state = DONE;
                    return;
                }

                /*
                 * Adiciona o dígito hexadecimal ao buffer
                 * e avança o índice.
                 */
                _sizeBuffer += data[i++];
            }
        }

        /* ============================================================
         * ESTADO: READ_DATA
         * ============================================================
         * Neste estado, o decoder lê EXATAMENTE
         * _currentChunkSize bytes de dados reais.
         *
         * Somente neste estado os dados são
         * adicionados ao corpo final (_body).
         */
        else if (_state == READ_DATA) {

            /*
             * Calcula quantos bytes ainda faltam
             * para completar este chunk.
             */
            size_t remaining = _currentChunkSize - _bytesReadInChunk;

            /*
             * Define quantos bytes podem ser copiados
             * neste ciclo de processamento.
             */
            size_t toCopy = std::min(remaining, len - i);

            /*
             * Copia os dados reais do chunk para o body final.
             * Nenhum dado de framing (hex ou CRLF) entra aqui.
             */
            _body.append(data + i, toCopy);

            // Avança o índice do buffer
            i += toCopy;

            // Atualiza o total de bytes lidos neste chunk
            _bytesReadInChunk += toCopy;

            /*
             * Quando lemos exatamente o número de bytes
             * especificado no tamanho do chunk,
             * o próximo passo obrigatório do protocolo
             * é consumir o CRLF que encerra os dados.
             */
            if (_bytesReadInChunk == _currentChunkSize) {
                _state = READ_CRLF;
            }
        }

        /* ============================================================
         * ESTADO: READ_CRLF
         * ============================================================
         * Após os dados de cada chunk,
         * o protocolo HTTP exige exatamente "\r\n".
         *
         * Nenhum outro valor é permitido.
         */
        else if (_state == READ_CRLF) {

            /*
             * Verifica se há bytes suficientes no buffer
             * e se eles correspondem a "\r\n".
             */
            if (i + 1 < len && data[i] == '\r' && data[i + 1] == '\n') {

                // Consome o CRLF
                i += 2;

                /*
                 * Chunk finalizado corretamente.
                 * Retornamos ao estado READ_SIZE
                 * para ler o tamanho do próximo chunk.
                 */
                _state = READ_SIZE;
            }
            else {
                /*
                 * Qualquer coisa diferente de CRLF
                 * neste ponto viola o protocolo HTTP.
                 */
                _state = DONE;
                return;
            }
        }
    }
}
