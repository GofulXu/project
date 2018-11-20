(function(){
	var os = require('os'), http = require("http");
	

	var util = {
		getIps : function(){
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
					if(ifaceName.indexOf("Virtual") >=0 ) continue; //disable like Visual box ..
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
		},
		jsonRequest : function(url,jsonobj){
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
	}
	
	module.exports = util;
}());