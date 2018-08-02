var express = require('express');
var app = express();
var request = require('request');
var https = require('https');
var fs = require('fs');
const execSync = require('child_process').exec;

var CommandQueue = require("command-queue");

var cmdq = new CommandQueue();
var queue = [];


var anfang = 0;

app.use(express.static('/home/pi/rpi-rgb-led-matrix/dev'));

fs.readFile('./index.html', function (err, html) {
  if (err) {
      throw err; 
  }       
  app.get('/', function(request, response) {  
      response.writeHeader(200, {"Content-Type": "text/html"});  
      response.write(html);  
      response.end();  
  });


  app.get('/buzzer/kill', function(req, res) {
    console.log('Kill');
    var abortcommand = 'sudo killall -9 demo';
    code = execSync(abortcommand);
  
  });
  
  
  app.get('/buzzer', function(req, res) {
  
    var id = req.query.id;
    var time = req.query.time;
    var message = req.query.message;
    var color = req.query.color;
    var new_color = color.substring(1, 8);
    request("https://maker.ifttt.com/trigger/led_message/with/key/hes_Wo9sHwVMvMmKGe47WdWafM454I8_lnnifdtTTYb?value1=" + message + "&value2=" + new_color);
    console.log(message); 
    console.log(new_color);
  
    if(typeof(id) === 'undefined') {
      id = '4';
    } 
  
    if(typeof(time) === 'undefined') {
      time = '10';
    }
    var r = parseInt(color.substring(1, 3), 16);
    var g = parseInt(color.substring(3, 5), 16);
    var b = parseInt(color.substring(5, 7), 16);

    console.log("r " + r);
    console.log("g " + g);
    console.log("b " + b);
  
    console.log('id   = ' + id);
    console.log('time = ' + time);
    //var command = 'sudo /home/pi/rpi-rgb-led-matrix/examples-api-use/demo -D ' + id + ' /home/pi/rpi-rgb-led-matrix/examples-api-use/runtext16.ppm -t ' + time + ' --led-rows=16 --led-chain=2';
    var command = 'echo "' + message + '" | sudo /home/pi/rpi-rgb-led-matrix/examples-api-use/text-example ' 
        + '--led-rows=16 --led-chain=2 -f /home/pi/rpi-rgb-led-matrix/fonts/9x18B.bdf -C "'
        + r + ',' + g + ',' + b + '"';
    var abortcommand = 'sudo killall -9 demo';
    code = execSync(abortcommand);
  
  
          setTimeout(function() {
            console.log('exec finished');
          },
            100
          );
  
  
  
          code = execSync(command);
  
    res.send('Hello World!');
  });  
  
  app.get('/color', function(req, res) {
  
    var color = req.query.color;
    var new_color = color.substring(1, 8);
    request("https://maker.ifttt.com/trigger/led_message/with/key/hes_Wo9sHwVMvMmKGe47WdWafM454I8_lnnifdtTTYb?value1=" + "Color Show " + "&value2=" + new_color);
    console.log(new_color);
    var r = parseInt(color.substring(1, 3), 16);
    var g = parseInt(color.substring(3, 5), 16);
    var b = parseInt(color.substring(5, 7), 16);

    console.log("r " + r);
    console.log("g " + g);
    console.log("b " + b);
  
    //var command = 'sudo /home/pi/rpi-rgb-led-matrix/examples-api-use/demo -D ' + id + ' /home/pi/rpi-rgb-led-matrix/examples-api-use/runtext16.ppm -t ' + time + ' --led-rows=16 --led-chain=2';
    var command = 'echo "' + message + '" | sudo /home/pi/rpi-rgb-led-matrix/examples-api-use/text-example ' 
        + '--led-rows=16 --led-chain=2 -f /home/pi/rpi-rgb-led-matrix/fonts/9x18B.bdf -C "'
        + r + ',' + g + ',' + b + '"';
    var abortcommand = 'sudo killall -9 demo';
    code = execSync(abortcommand);
  
  
          setTimeout(function() {
            console.log('exec finished');
          },
            100
          );
  
  
  
          code = execSync(command);
  
    res.send('Hello World!');
  });  
  
  
  app.listen(3000);
  console.log('server is running');
  
});

