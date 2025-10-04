<?php
header("Content-Type: text/html");

echo "<html><body>";
echo "<h1>PHP CGI Test</h1>";
echo "<p>Method: " . $_SERVER['REQUEST_METHOD'] . "</p>";
echo "<p>Query string: " . $_SERVER['QUERY_STRING'] . "</p>";

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $body = file_get_contents("php://input");
    echo "<p>POST data: " . htmlspecialchars($body) . "</p>";
}

echo "</body></html>";
?>