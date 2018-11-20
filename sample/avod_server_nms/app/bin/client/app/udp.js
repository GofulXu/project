(function(){		
	dgram = require("dgram")
	function udpServer(multiaddr, port, iface){
		var options = {
		type: 'udp4',
		reuseAddr: true
		};
		var server = dgram.createSocket(options);
		server.bind(port,iface?iface:"", function() {			
			server.addMembership(multiaddr,iface);	
		});
		server.on("message", function(buf){
		})

		server.on("error",function(err){
			console.log(err);
		});		
		
		this.send = function(msg){
			var buf = new Buffer(msg);
			server.send(buf, 0 , buf.length, port, multiaddr);
		}
	}
	
	module.exports = udpServer;
	
}())
