/** 参数管理
 * 
 *
 */
var spawn = require('child_process').spawn;
var pm = {
	pool : [],
	busy : false,
	init : function(bin){
		this.bin    = bin;
		this.handle = setInterval(this._check, 100);
	},
	getparam : function(key, cb){
		var item = {
			type : 'GET_PARAM',
			data : key,
			cb   : cb
		}
		this.pool.push(item);
	},
	setparam : function(jsonobj,cb){
		var item = {
			type : 'SET_PARAM',
			data : jsonobj,
			cb   : cb
		}
		this.pool.push(item);
	},
	check_time : function(jsonobj,cb){
		var item = {
		type : 'CHECK_TIME',
		data : jsonobj,
		cb   : cb
		}
		this.pool.push(item);
	},
	capture_screen : function(cb){
		var item = {
		type : 'CAPTURE_SCREEN',
		data : null,
		cb   : cb
		}
		this.pool.push(item);
	},
	restore_factory : function(cb){
		var item = {
		type : 'RESTORE_FACTORY',
		data : null,
		cb   : cb
		}
		this.pool.push(item);
	},		
	_check : function(){
		if(pm.pool.length<=0 || pm.busy) return;		
		pm.busy = true;
		var item = pm.pool[0];
		var cmd = [];
		cmd[0] = [item.type];
		cmd[1] = [item.data];		
		//cmd = [__dirname + "/pmproc.js", item.data];		
			
		var childproc  = spawn(pm.bin, cmd);
		childproc.stdout.on('data', function (data) {			
			if(item.type == 'GET_PARAM')
				item.rep = data.toString();
			if(item.type == 'CAPTURE_SCREEN')
				item.rep = data.toString();
		});
		childproc.on('close', function(code){
			pm.busy = false;
			pm.pool.splice(0, 1);
			//console.log('item = ' + JSON.stringify(item));
			item.cb(item.rep?item.rep:null);
			
		});
	},
	destory : function(){
		if(pm.handle)
			clearInterval(pm.handle);
	}
	
}

module.exports = pm;