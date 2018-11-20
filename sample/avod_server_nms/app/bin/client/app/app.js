var port = 37892,dgram = require("dgram"),nmsserver,
	http  = require("http"),fs = require("fs"), util = require("./util");
var exec = require("child_process").exec,childproc	
var DEVICEID = "";
var hardware_version= "";
//var operate = require('./operate');
var UdpServer = require("./udp");	
var upload = require("./upload");	
var pm = require("./pm")
pm.init("./swoperate");
function log(msg) {
	console.log("[APP]" + msg)
}
var port = 37892,multiaddr = "225.8.8.8";
var udp = new UdpServer(multiaddr, port, util.getIps()[0]);
var processor = {
	discover : function(msg, sendmsg){
		nmsserver = msg.server;	
		/*
		//保存server到参数列表
		var serverObj = {
			server : msg.server
		}
		log('save server:' + JSON.stringify(serverObj));
		operate.setparam(JSON.stringify(serverObj), function(param){
		});	
		
		sendmsg.status = 10; 
		util.jsonRequest(nmsserver, sendmsg);
		*/
		
        //operate.getparam('lan_ip|mac|serialno|ro.build.version|hardware_version|disk_status|sleep_status|ipconflict_status', function(param){
		pm.getparam('lan_ip|mac|serialno|ro.build.version|hardware_version|disk_status|sleep_status|ipconflict_status', function(param){
            var paramObj= {
			};
			try{
				paramObj= eval('('+param+')');
				if(!hardware_version){
					hardware_version= paramObj.hardware_version;
				}
			}catch(e){
			}
            for(var i in paramObj){
                sendmsg[i]= paramObj[i];
            }
			if(fs.existsSync("/data/dynamicmap/version.txt")){
				var dynamicmapVer = fs.readFileSync("/data/dynamicmap/version.txt").toString();
				sendmsg.dyVer = dynamicmapVer;
			}
			sendmsg.hardware_version=hardware_version;
            sendmsg.status = 10; 
            util.jsonRequest(msg.server, sendmsg);

			udp.send(JSON.stringify(sendmsg));
			
        });
	},
    upgrade : function(msg, sendmsg){
		if(msg.hasOwnProperty('result')){
			sendmsg.status = -1;
			sendmsg.result = msg.result;
		}
		else
			sendmsg.status = 10;
		util.jsonRequest(msg.server, sendmsg);
	},
	config : function(msg, sendmsg){
		DEVICEID = msg.id;
		
	},
	execute  : function(msg, sendmsg){
		var child = exec(msg.cmd,{cwd: msg.workdir}, function(error, stdout, stderr) {
			sendmsg.result =  {					
				stdout : stdout.toString(),
				stderr : stderr.toString()
			}
			util.jsonRequest(msg.server, sendmsg);
		});
		child.on("exit", function(code){
			
			sendmsg.status = (code ==0?10:-1);			
			
		});
	},
	pullfile : function(msg, sendmsg){
		upload.upload(msg.url,msg.files, function(err){
			if(err){
				sendmsg.status = -1; 
				sendmsg.result = {
					info : JSON.stringify(err)
				}
				util.jsonRequest(msg.server, sendmsg);
			}else{
				sendmsg.status = 10; 
				util.jsonRequest(msg.server, sendmsg);
			}
		});	    
	},
	pushfile : function(msg, sendmsg){
		//下载
		var file = fs.createWriteStream(msg.saveto);
		var req = http.get( msg.url , function(res) {
			if(res.statusCode != 200) {
				log("download failed-->" + msg.url + ", ");
				sendmsg.status = -1;
				sendmsg.result = {
					info : 'download failed, res.statusCode = '+res.statusCode
				}
				util.jsonRequest(msg.server, sendmsg);
				return;
			}
			res.pipe(file);
			file.on('finish', function () {
				file.close(); // close() is async, call callback after close completes.
				log("download finish");			
				sendmsg.status = 10;
				util.jsonRequest(msg.server, sendmsg);
			});
			file.on('error', function (err) {
				log("download error");
				fs.unlink(msg.saveto); // Delete the file async. (But we don't check the result)									
				sendmsg.status = -1;
				sendmsg.result = {
					info : err
				}
				util.jsonRequest(msg.server, sendmsg);
			});
		});
	},
	setparam : function(msg, sendmsg){
		//设置参数
		//msg.param  存储参数
		console.log("参数设置" + JSON.stringify(msg.param));
	
		sendmsg.status = 10
		var paramJson = JSON.stringify(msg.param);
		//operate.setparam(paramJson, function(param){
		pm.setparam(paramJson, function(param){
					sendmsg.result = param;
					util.jsonRequest(msg.server, sendmsg);
			});	
	},
	check_time : function(msg, sendmsg){
		//msg.param  校准时间
		console.log("校准时间" + JSON.stringify(msg.date));
	
		sendmsg.status = 10
		var paramJson = JSON.stringify(msg.date);
		pm.check_time(paramJson, function(param){
					sendmsg.result = param;
					util.jsonRequest(msg.server, sendmsg);
			});	
	},
	capture_screen : function(msg, sendmsg){
		//捕获屏幕
		console.log("捕捉屏幕：" + JSON.stringify(msg.param));
	
		//sendmsg.status = 10
		var paramJson = JSON.stringify(msg.param);
		pm.capture_screen(function(param){
			sendmsg.result = param;
			upload.upload(msg.url,param, function(err){
				if(err){
					sendmsg.status = -1; 
					sendmsg.result = {
						info : JSON.stringify(err)
					}
					util.jsonRequest(msg.server, sendmsg);
				}else{
					sendmsg.status = 10; 
					util.jsonRequest(msg.server, sendmsg);
				}
			});	    
		});	
	},
	restore_factory : function(msg, sendmsg){
		//捕获屏幕
		console.log("恢复出厂：" + JSON.stringify(msg.param));
	
		var paramJson = JSON.stringify(msg.param);
		pm.restore_factory(function(param){
			sendmsg.result = param;
			sendmsg.status = 10; 
			util.jsonRequest(msg.server, sendmsg);
		});	
	},
	getparam : function(msg, sendmsg){
		//获取参数
		sendmsg.status = 10; 
		sendmsg.result={};

		//operate.getparam('mac|serialno|ro.build.version|lan_ip|lan_gateway|ip_base|browser_region|hardware_version|hd_standard|log_level|pis_system|net_status|volume', function(param){
		//operate.getparam(msg.param, function(param){
		pm.getparam(msg.param, function(param){
			var paramObj= {
			};
			try{
				paramObj= eval('('+param+')');
			}catch(e){
			}
			for(var i in paramObj){
				sendmsg.result[i] = paramObj[i];
			}
			if(fs.existsSync("/data/dynamicmap/version.txt")){
				var dynamicmapVer = fs.readFileSync("/data/dynamicmap/version.txt").toString();
				sendmsg.result.dyVer = dynamicmapVer;
			}
			util.jsonRequest(msg.server, sendmsg);
		});	
	}
}
process.on("message", function(m){	
	var msg = JSON.parse(m);
	var sendmsg = {				
		taskid  : msg.taskid
	}
	var VER = fs.readFileSync("version.txt").toString();
	log("receive message-->" + m);
	if('config' != msg.type){
		//operate.getparam('server', function(param){
			//log('--------------param:' + param);
			//var paramObj = JSON.parse(param);
			//for(var key in paramObj)
			//	nmsserver = paramObj[key];
			//log('getServer:' + nmsserver);
			if('discover' == msg.type){			
				//operate.getparam("hardware_version", function(param){
					//var paramObj = eval("("+param+")");
					//var type;
					//for(var i in paramObj){
					//	type = paramObj[i];
					//}
					//log('read_type:' + type + ' msg_type:' + msg.hd_type);
					
					//if(type == msg.hd_type){
						//log('find the same hardware_type');
							//除了discover消息,其它消息需要立即上报已受理状态
						//var sendmsg = {				
						//	taskid   : msg.taskid,
						//	version  : version,
						//	type     : msg.type,
						//	deviceid : DEVICEID,
						//	status   : 5	//已受理				
						//}
						//util.jsonRequest(msg.server, sendmsg);
						
						for(var x in processor){
							if(x == msg.type) {
								sendmsg.type = msg.type;
								sendmsg.version = VER;
								sendmsg.deviceid = DEVICEID;
								processor[x](msg, sendmsg);
							}
						}	
					//}else{
					//	log('find the different hardware_type');
					//}
				//});
			}else{
				//除了discover消息,其它消息需要立即上报已受理状态
				var sendmsg = {				
					taskid   : msg.taskid,
					version  : VER,
					type     : msg.type,
					deviceid : DEVICEID,
					status   : 5	//已受理				
				}
				util.jsonRequest(msg.server, sendmsg);
				
				for(var x in processor){
					if(x == msg.type) {
						sendmsg.type = msg.type;
						sendmsg.version = VER;
						sendmsg.deviceid = DEVICEID;
						processor[x](msg, sendmsg);
					}
				}	
			}
		//});
	}else{
		for(var x in processor){
			if(x == msg.type) {
				sendmsg.type = msg.type;
				sendmsg.version = VER;
				sendmsg.deviceid = DEVICEID;
				processor[x](msg, sendmsg);
			}
		}
	}
})			
