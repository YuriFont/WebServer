#!/usr/bin/php-cgi
<?php
// 1. O PHP precisa enviar os headers, igual ao Python
echo "Content-Type: text/html\r\n\r\n";
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>PHP CGI Test</title>
    <style>
        body { font-family: 'Segoe UI', sans-serif; padding: 20px; background: #f0f2f5; }
        .card { background: white; padding: 20px; margin-bottom: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #5a5a7b; }
        pre { background: #333; color: #0f0; padding: 10px; border-radius: 4px; overflow-x: auto; }
    </style>
</head>
<body>
    <div class="card">
        <h1>🐘 PHP CGI Test Results</h1>
        <p>Webserv executou o PHP corretamente!</p>
        
        <h3>Dados Recebidos (GET/POST):</h3>
        <p><strong>_GET:</strong></p>
        <pre><?php print_r($_GET); ?></pre>
        
        <p><strong>_POST:</strong></p>
        <pre><?php print_r($_POST); ?></pre>
    </div>

    <div class="card">
        <h3>Ambiente Completo (Environment)</h3>
        <p>Aqui estão TODAS as variáveis que o webserver enviou:</p>
        <pre><?php 
            // Itera sobre tudo o que existe no $_SERVER
            foreach ($_SERVER as $key => $value) {
                // htmlspecialchars é segurança básica para não quebrar o HTML se vier lixo
                echo "<strong>[" . $key . "]</strong> => " . htmlspecialchars($value) . "\n";
            }
        ?></pre>
    </div>


    <div class="card">
         <h3>PHP Info (Full Config)</h3>
         <?php phpinfo(INFO_GENERAL); ?> 
    </div>
</body>
</html>