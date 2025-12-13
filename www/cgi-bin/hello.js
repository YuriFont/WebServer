#!/usr/bin/env node
// hello.js - exemplo de script CGI em Node.js
// Coloque em www/cgi-bin/hello.js
// Se executar via "node /path/to/hello.js" o script não precisa ser executável.
// Se você quiser execvá-lo diretamente (./hello.js), torne-o executável: chmod +x hello.js

const fs = require('fs');

function getEnvVar(name, fallback = '') {
  return process.env[name] || fallback;
}

function gatherHttpEnv() {
  const httpEnv = {};
  for (const k of Object.keys(process.env)) {
    if (k.startsWith('HTTP_')) {
      httpEnv[k] = process.env[k];
    }
  }
  return httpEnv;
}

function sendResponse(body) {
  // Imprime headers CGI/HTTP (o server deve repassar isso ao cliente)
  // Aqui usamos Content-Type simples; você pode mudar para text/html se preferir.
  process.stdout.write('Content-Type: text/plain; charset=utf-8\r\n');
  process.stdout.write(`Content-Length: ${Buffer.byteLength(body, 'utf8')}\r\n`);
  process.stdout.write('\r\n');
  process.stdout.write(body);
}

// monta o corpo da resposta textual
function makeBody(reqInfo, httpEnv, bodyText) {
  let b = '';
  b += '=== Node CGI demo ===\n\n';
  b += `REQUEST_METHOD: ${reqInfo.method}\n`;
  b += `SCRIPT_NAME: ${reqInfo.scriptName}\n`;
  b += `QUERY_STRING: ${reqInfo.queryString}\n`;
  b += `CONTENT_TYPE: ${reqInfo.contentType}\n`;
  b += `CONTENT_LENGTH: ${reqInfo.contentLength}\n\n`;

  b += '--- HTTP_* headers ---\n';
  for (const k of Object.keys(httpEnv).sort()) {
    b += `${k}: ${httpEnv[k]}\n`;
  }
  b += '\n--- Body (raw) ---\n';
  b += bodyText + '\n';
  return b;
}

// Lê body: se CONTENT_LENGTH definido, lê exatamente esse número de bytes (blocking, seguro para CGI).
// Caso contrário, coleta todos os dados do stdin até 'end'.
const contentLength = parseInt(getEnvVar('CONTENT_LENGTH', '0'), 10);
const reqInfo = {
  method: getEnvVar('REQUEST_METHOD', 'GET'),
  queryString: getEnvVar('QUERY_STRING', ''),
  contentType: getEnvVar('CONTENT_TYPE', ''),
  contentLength: isNaN(contentLength) ? 0 : contentLength,
  scriptName: getEnvVar('SCRIPT_NAME', ''),
};

const httpEnv = gatherHttpEnv();

if (reqInfo.contentLength > 0) {
  // leitura síncrona do stdin (exatamente contentLength bytes)
  try {
    const buf = Buffer.alloc(reqInfo.contentLength);
    let bytesRead = 0;
    while (bytesRead < reqInfo.contentLength) {
      const n = fs.readSync(0, buf, bytesRead, reqInfo.contentLength - bytesRead);
      if (n === 0) break; // EOF
      bytesRead += n;
    }
    const bodyText = buf.slice(0, bytesRead).toString('utf8');
    const body = makeBody(reqInfo, httpEnv, bodyText);
    sendResponse(body);
  } catch (err) {
    const body = `Error reading stdin: ${err.message}\n`;
    sendResponse(body);
  }
} else {
  // sem CONTENT_LENGTH: ler até EOF (útil para alguns cenários)
  let chunks = [];
  let total = 0;
  process.stdin.on('data', (chunk) => {
    chunks.push(chunk);
    total += chunk.length;
  });
  process.stdin.on('end', () => {
    const bodyText = Buffer.concat(chunks, total).toString('utf8');
    const body = makeBody(reqInfo, httpEnv, bodyText);
    sendResponse(body);
  });
  // garantir que lemos mesmo que não haja dados
  process.stdin.resume();
}
