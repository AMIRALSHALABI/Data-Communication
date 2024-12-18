var http = require('http');
var url = require('url');
const itemsJson = require('./items.json');

http.createServer(function (req, res) {
    var parsed = url.parse(req.url, true);

    if (req.method === 'GET') {
        let response = "GET: Current items in the shopping cart:\n";
        itemsJson.forEach((item, index) => {
            response += `${index + 1}. Name: ${item.name}, Price: ${item.price}, ID: ${item.id}\n`;
        });
        res.statusCode = 200;
        res.end(response);
    }

    if (req.method === 'PUT') {
        let newItemName = parsed.query.newItemName;
        let newItemPrice = parsed.query.newItemPrice;

        if (!newItemName || !newItemPrice) {
            res.statusCode = 400;
            res.end("Error: Invalid item name or price\n");
            return;
        }

        let newId = new Date().toISOString();
        let newItem = { id: newId, name: newItemName, price: parseFloat(newItemPrice) };

        itemsJson.push(newItem);
        res.statusCode = 200;
        res.end(`PUT: Added new item:\nName: ${newItem.name}, Price: ${newItem.price}, ID: ${newItem.id}\n`);
    }

    if (req.method === 'DELETE') {
        let indexToDelete = parseInt(parsed.query.index);

        if (isNaN(indexToDelete) || indexToDelete < 1 || indexToDelete > itemsJson.length) {
            res.statusCode = 400;
            res.end("Error: Invalid index\n");
            return;
        }

        let deletedItem = itemsJson.splice(indexToDelete - 1, 1);
        res.statusCode = 200;
        res.end(`DELETE: Removed item:\nName: ${deletedItem[0].name}, Price: ${deletedItem[0].price}, ID: ${deletedItem[0].id}\n`);
    }

    if (req.method === 'OPTIONS') {
        res.statusCode = 200;
        res.end();
    }
}).listen(3000, function () {
    console.log("Server started at port 3000");
});
