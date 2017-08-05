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

/*
NARODMON
*/
var net = require('net');
var util = require('util');

const DEVICE_MAC = "b827ebe8e8b9";
const SENSOR_ID_1 = "b827ebe8e8b901";
const SENSOR_ID_2 = "b827ebe8e8b902";
const SENSOR_ID_3 = "b827ebe8e8b903";
const SENSOR_ID_4 = "b827ebe8e8b904";
const SENSOR_ID_5 = "b827ebe8e8b905";

function uploadDataNM(data) {
  var res;
  return new Promise((resolve, reject) => {
    try {
      var client = new net.Socket();
      client.connect(8283, 'narodmon.ru', function() {
        client.write(util.format('#%s\n#%s#%d\n#%s#%d\n#%s#%d\n#%s#%d\n#%s#%d\n##', DEVICE_MAC, SENSOR_ID_1, data.temp, SENSOR_ID_2, data.humidity, SENSOR_ID_3, data.baro, SENSOR_ID_4, data.light, SENSOR_ID_5, data.dewpt));
      });

      client.on('data', function(_data) {
        client.destroy();
        res = _data.toString('utf-8');
        res = res.substring(0, res.length - 1);
      });

      client.on('close', function() {
        if (res != "OK")
          reject(res);
        else
          resolve();
      });
    } catch (ex) {
      reject(new Error('Parsing error: ' + ex));
    }
  });
}
/*
 ******************************************************************
 */

function run() {
  requestData().then(data => {
    parseData(data).then(data => {
      uploadDataWU(data.wu).then(() => {
        log("[WU PACKETID: " + c + "]: OK")
      }).catch((error) => {
        log("[WU PACKETID: " + c + "]: ERR")
      });
      uploadDataNM(data.all).then(() => {
        log("[NM PACKETID: " + c + "]: OK")
      }).catch((error) => {
        log("[NM PACKETID: " + c + "]: ERR: " + error)
      });
      c += 1;
    });
  });
}

run();
setInterval(run, 60000);
