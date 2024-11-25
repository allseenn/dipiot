const http = require('http');
const fs = require('fs');

const PORT = 1080;

// Создание сервера
const server = http.createServer((req, res) => {
    if (req.url === '/data') {
        sendJsonData(res);
    } else {
        sendHtmlResponse(res);
    }
});

// Функция для отправки HTML-ответа
function sendHtmlResponse(res) {
    fs.readFile('/usr/local/sbin/web.html', (err, data) => {
        if (err) {
            res.writeHead(500, { 'Content-Type': 'text/plain' });
            res.end('500 Internal Server Error: Could not open web.html file.');
        } else {
            res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
            res.end(data);
        }
    });
}

// Функция для обработки и отправки JSON-данных
function sendJsonData(res) {
    fs.readFile('/tmp/bsec', 'utf8', (err, data) => {
        if (err) {
            console.error('Error reading file /tmp/bsec:', err.message);
            res.writeHead(500, { 'Content-Type': 'text/plain' });
            res.end('500 Internal Server Error: Could not read /tmp/bsec file.');
            return;
        }

        try {
            // Пример обработки данных из файла
            const jsonData = processFileData(data);
            res.writeHead(200, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify(jsonData));
        } catch (e) {
            console.error('Error processing file data:', e.message);
            res.writeHead(500, { 'Content-Type': 'text/plain' });
            res.end('500 Internal Server Error: Error processing file data.');
        }
    });
}

// Пример обработки данных из файла (должен быть реализован по требованиям)
function processFileData(data) {
    // Здесь можно добавить логику обработки данных
    return { message: 'Data processed successfully', rawData: data };
}

// Запуск сервера
server.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}/`);
});

// Обработка SIGINT для корректного завершения сервера
process.on('SIGINT', () => {
    console.log('\nReceived SIGINT. Shutting down server...');
    server.close(() => {
        console.log('Server closed.');
        process.exit(0);
    });
});

