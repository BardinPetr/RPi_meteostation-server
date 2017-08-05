const log = console.log;

var PWS = require('wunderground-pws');
const teletype = require('teletype');

var pws = new PWS('IGORODIS2', 'k5vl14ap');
const client = teletype('192.168.1.64');

const mu = require('./meteo_utils');

if (process.platform === "win32") {
  var rl = require("readline").createInterface({
    input: process.stdin,
    output: process.stdout
  });

  rl.on("SIGINT", function() {
    process.emit("SIGINT");
  });
}

process.on("SIGINT", function() {
  log("Closing client");
  client.close();
  log("Exitting");
  process.exit();
});


function uploadDataWU(data) {
  return new Promise((resolve, reject) => {
    try {
      pws.setObservations(data);
      pws.sendObservations(function(err, success) {
        if (err) {
          log("Uploading: ERR: " + err);
          reject();
        }
        resolve();
      });
    } catch (ex) {
      reject(ex);
    }
  });
}

function requestData() {
  return new Promise((resolve, reject) => {
    client.exec('get', /(^@)(.+)(#$)/)
      .then(response => {
        if (!response)
          reject();
        resolve(response);
      });
  });
}

function parseData(data) {
  return new Promise((resolve, reject) => {
    try {
      data = data.substring(1, data.length - 1);
      data = data.split(';');
      data = {
        wu: {
          //soilmoisture: data[0],
          tempf: mu.c_to_f(data[1]),
          humidity: parseInt(data[2]),
          baromin: mu.mmHg_to_inHg(data[4]),
          dewptf: mu.c_to_f(data[5]),
          softwaretype: "RPI PWS Uploader"
        },
        all: {
          soilmoisture: data[0],
          temp: parseInt(data[1]),
          humidity: parseInt(data[2]),
          light: data[3],
          baro: parseInt(data[4]),
          dewpt: parseInt(data[5])
        }
      }
    } catch (ex) {
      reject(new Error('Parsing error: ' + ex));
    }
    resolve(data)
  });
}

var c = 0;



function run() {
  requestData().then(data => {
    parseData(data).then(data => {
      uploadDataWU(data.wu).then(() => {
        log("[WU PACKETID: " + c + "]: OK")
      }).catch((error) => {
        log("[WU PACKETID: " + c + "]: ERR")
      });
      c += 1;
    });
  });
}

run();
setInterval(run, 60000);
