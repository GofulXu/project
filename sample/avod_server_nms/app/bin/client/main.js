var child = require("child_process"),childproc,fs = require("fs")
	port = 37891,multiaddr = "225.8.8.8",dgram = require("dgram")
	http  = require("http");
var nmsserver, version;
var APP_MAIN = "app.js";
function log(msg) {
	console.log("[LOADER]" + msg)
}
var DEVICEID = getUUID();
if(fs.existsSync("deviceid"))
	DEVICEID = fs.readFileSync("deviceid").toString();
else
	fs.writeFileSync("deviceid", new Buffer(DEVICEID));
function getUUID() {
	var d = new Date().getTime();
	var uuid = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
		var r = (d + Math.random()*16)%16 | 0;
		d = Math.floor(d/16);
		return (c=='x' ? r : (r&0x7|0x8)).toString(16);
	});
	return uuid;
}

//解压缩
function unzip(zipfile,zipto, callback){
	var path = require("path")
	var execFile = require('child_process').execFile, child;
	var command = path.join(process.cwd(), "unzip.exe ")		
	if(process.arch == "arm")
		command = "/usr/bin/unzip"	
	console.log('prepare unzip ',zipfile ,"-->", zipto);
	child = execFile(command,["-o", zipfile, "-d",zipto], function(error, stdout, stderr) {
		if (error) {
			//if(type == 0)
			log('unzip failed',error,zipfile,zipto);;
			callback(error);
			return;
			
		}
		log('unzip ok');
		callback(null);		
	});
	
}

//核心app进程
var _app = {
	_command : function(msg){ //给app发送消息
		if(childproc) childproc.send(msg)
	},	
	startup : function(){		
		
		childproc = child.fork(APP_MAIN,{cwd:"app/"});	
		setTimeout(function(){ //设置设备ID
			var m = {
				type : "config",
				id   : DEVICEID
			}			
			if(childproc)
				childproc.send(JSON.stringify(m));
		},500)
		childproc.on("message", function(m){
			log(m)
		})	
		childproc.on("exit", function(){
			//setTimeout(forkit, 1000);
			log("app have exited.")
			childproc = null;
		})			
		
		if(_app.watchHandle) clearInterval(_app.watchHandle);		
		_app.watchHandle = setInterval(function(){
			log("watchdog check...")
			if(childproc == null)	{
				log("app have died.prepare restart...");
				setTimeout(_app.startup, 500);			
			}
		}, 10000);
		
	} ,
	
	shutdown :  function(){
		if(_app.watchHandle) clearInterval(_app.watchHandle);		
		if(childproc) childproc.kill();	
		childproc = null;
	},
	
	restart : function(){
		setTimeout(function(){
			_app.shutdown();
			setTimeout(function(){
				_app.startup()			
			}, 2000);
		}, 2000);
	},
	//app升级
	upgrade : function(msg) {
		var url = msg.url;
		//下载
		var file = fs.createWriteStream("app.zip");
		var req = http.get( url , function(res) {
			if(res.statusCode != 200) {
				log("download failed");
				msg.result = {
					info : "download failure."
				}
				_app._command(JSON.stringify(msg));
				return;
			}
			res.pipe(file);
			file.on('finish', function () {
				file.close(); // close() is async, call callback after close completes.
				log("download finish");
				unzip("app.zip", "app/", function(err){
					if(!err){
						_app._command(JSON.stringify(msg));
						_app.restart();
					}else{
						msg.result = {
							info : err												
						}
						_app._command(JSON.stringify(msg));
					}
					
				});			
				
			});
			file.on('error', function (err) {
				log("download error");
				msg.result = {
					info : err												
				}
				_app._command(JSON.stringify(msg));
				fs.unlink("app.zip"); // Delete the file async. (But we don't check the result)									
			});
		});
	}
}
//获得本机所有ipv4 ip
function getIps(){
	var addrInfo, ifaceDetails, _len;
	var localIPInfo = {};
	var ipv4list = [];
	//Get the network interfaces
	var networkInterfaces = require('os').networkInterfaces();
	//Iterate over the network interfaces
	for (var ifaceName in networkInterfaces) {
		ifaceDetails = networkInterfaces[ifaceName];
		//Iterate over all interface details
		for (var _i = 0, _len = ifaceDetails.length; _i < _len; _i++) {
			addrInfo = ifaceDetails[_i];
			if(addrInfo.internal) continue;
			//if(ifaceName.indexOf("Virtual") >=0 ) continue; //disable like Visual box ..
			if (addrInfo.family === 'IPv4') {
				//Extract the IPv4 address
				if (!localIPInfo[ifaceName]) {
					localIPInfo[ifaceName] = {};
				}
				localIPInfo[ifaceName].IPv4 = addrInfo.address;
				ipv4list[ipv4list.length] = addrInfo.address;
			} else if (addrInfo.family === 'IPv6') {
				//Extract the IPv6 address
				if (!localIPInfo[ifaceName]) {
					localIPInfo[ifaceName] = {};
				}
				localIPInfo[ifaceName].IPv6 = addrInfo.address;
			}
		}
	}
	return ipv4list;
}
function jsonRequest(url,jsonobj){
	var reqobj   = require('url').parse(url);
	var postData = JSON.stringify(jsonobj);
	console.log(postData);
	var options = {
	  hostname: reqobj.hostname,
	  port: reqobj.port,
	  path: reqobj.pathname,
	  method: 'POST',
	  agent  : false,
	  headers: {
		'Content-Type': 'application/json',
		'keepAlive'   : false,
		'Content-Length': postData.length
	  }
	};
	var req = http.request(options,function(res){});
	req.on('error', function(e) {
	  console.log('problem with request: ' + e.message);
	});
	req.write(postData);
	req.end();
	//req.abort();
}
function messageProcess(msg){
	try{
		//log("receive msg" + msg)		
		if(!msg.type) return;
		if(msg.target == "ALL" || msg.target == DEVICEID){

			if(msg.type == "discover") 	nmsserver = msg.server;	

			if(msg.type == "upgrade") { //升级处理					
				if(msg.url) {
					//检测当前版本是否一致						
					if(fs.existsSync("app/version.txt"))
						version = fs.readFileSync("app/version.txt")		
					log('version=' + version + 'msg.version=' + msg.version);
					if(version != msg.version)
						_app.upgrade(msg);
					else{
						msg.result = {
						info : "versions are the same."
						}
						_app._command(JSON.stringify(msg));
					}
				}				
			}else {
				//透传其它指令
				_app._command(JSON.stringify(msg));
			}
		}
		
	}catch(e){
		console.log("exception " + e);
	}
}

var udpServer = [];
function joinMemShip(iface){
	var server = dgram.createSocket({type:'udp4',reuseAddr: true});
	//var server = dgram.createSocket("udp4");
	udpServer[udpServer.length] = server;
	server.bind(port, function() {			
		console.log("prepare join membership -->" + multiaddr + " at " + iface );			
		server.addMembership(multiaddr, iface);
		console.log("join membership OK-->" + multiaddr + " at " + iface );					
	});
	server.on("message", function(buf){		
		log("receive msg:" + buf)	
		var msg = JSON.parse(buf)
		messageProcess(msg);
	})

	server.on("error",function(err){
		console.log(err);		
	}); 
}
function startServer(){
	
	var ip = getIps();
	if(ip.length ==0 ){
		setTimeout(startServer, 2000);
		console.log("wait net..")
		return;
	}
	
	udpServer = [];	
	for(var x in ip){		
		joinMemShip(ip[x])
	}
	
	_app.startup();
	
}

startServer();
return;
//test
setTimeout(function(){
	_app.cmd_config("http://172.16.7.75:999/");
},2000)
return;

